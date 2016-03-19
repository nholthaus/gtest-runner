#include "GTestFailureModel.h"
#include "QStdOutSyntaxHighlighter.h"
#include "mainwindow_p.h"

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFontDatabase>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenuBar>
#include <QTimer>
#include <QStyle>

//--------------------------------------------------------------------------------------------------
//	FUNCTION: MainWindowPrivate
//--------------------------------------------------------------------------------------------------
MainWindowPrivate::MainWindowPrivate(MainWindow* q) :
	q_ptr(q),
	executableDock(new QDockWidget(q)),
	executableDockFrame(new QFrame(q)),
	executableListView(new QListView(q)),
	executableModel(new QExecutableModel(q)),
	testCaseProxyModel(new QBottomUpSortFilterProxy(q)),
	addTestButton(new QPushButton(q)),
	fileWatcher(new QFileSystemWatcher(q)),
	centralFrame(new QFrame(q)),
	testCaseFilterEdit(new QLineEdit(q)),
	testCaseTreeView(new QTreeView(q)),
	statusBar(new QStatusBar(q)),
	failureDock(new QDockWidget(q)),
	failureTreeView(new QTreeView(q)),
	failureProxyModel(new QBottomUpSortFilterProxy(q)),
	consoleDock(new QDockWidget(q)),
	consoleTextEdit(new QTextEdit(q)),
	consoleHighlighter(new QStdOutSyntaxHighlighter(consoleTextEdit)),
	systemTrayIcon(new QSystemTrayIcon(QIcon(":/images/logo"), q)),
	mostRecentFailurePath("")
{
	qRegisterMetaType<QVector<int>>("QVector<int>");

	QFontDatabase fontDB;
	fontDB.addApplicationFont(":/fonts/consolas");
	QFont consolas("consolas", 10);

	centralFrame->setLayout(new QVBoxLayout);
	centralFrame->layout()->addWidget(testCaseFilterEdit);
	centralFrame->layout()->addWidget(testCaseTreeView);
	centralFrame->layout()->setContentsMargins(0, 5, 0, 0);

	executableDock->setObjectName("executableDock");
	executableDock->setAllowedAreas(Qt::LeftDockWidgetArea);
	executableDock->setWindowTitle("Test Executables");
	executableDock->setWidget(executableDockFrame);

	executableListView->setModel(executableModel);
	executableListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	executableDockFrame->setLayout(new QVBoxLayout);
	executableDockFrame->layout()->addWidget(executableListView);
	executableDockFrame->layout()->addWidget(addTestButton);

	addTestButton->setText("Add Test Executable...");

	testCaseFilterEdit->setPlaceholderText("Filter Test Output...");
	testCaseFilterEdit->setClearButtonEnabled(true);

	testCaseTreeView->setSortingEnabled(true);
	testCaseTreeView->sortByColumn(GTestModel::TestNumber, Qt::AscendingOrder);
	testCaseTreeView->setModel(testCaseProxyModel);

	testCaseProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	failureDock->setObjectName("failureDock");
	failureDock->setAllowedAreas(Qt::BottomDockWidgetArea);
	failureDock->setWindowTitle("Failures");
	failureDock->setWidget(failureTreeView);

	failureTreeView->setModel(failureProxyModel);
	failureTreeView->setAlternatingRowColors(true);

	consoleDock->setObjectName("consoleDock");
	consoleDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
	consoleDock->setWindowTitle("Console Output");
	consoleDock->setWidget(consoleTextEdit);

	consoleTextEdit->setFont(consolas);
	QPalette p = consoleTextEdit->palette();
	p.setColor(QPalette::Base, Qt::black);
	p.setColor(QPalette::Text, Qt::white);
	consoleTextEdit->setPalette(p);
	consoleTextEdit->setReadOnly(true);

	systemTrayIcon->show();

	createExecutableContextMenu();
	createTestMenu();
	createOptionsMenu();
	createWindowMenu();

	connect(this, &MainWindowPrivate::setStatus, statusBar, &QStatusBar::setStatusTip, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::testResultsReady, this, &MainWindowPrivate::loadTestResults, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::testResultsReady, statusBar, &QStatusBar::clearMessage, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::showMessage, statusBar, &QStatusBar::showMessage, Qt::QueuedConnection);

	// Open dialog when 'add test' is clicked
	connect(addTestButton, &QPushButton::clicked, addTestAction, &QAction::trigger);

	// switch testCase models when new tests are clicked
	connect(executableListView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected)
	{
		if(!selected.isEmpty())
		{
			auto index = selected.indexes().first();
			selectTest(index.data(QExecutableModel::PathRole).toString());
		}
	});

	// run the test whenever the executable changes
	connect(fileWatcher, &QFileSystemWatcher::fileChanged, [this](const QString& path)
	{	
		QModelIndex m = executableModelHash[path];
		if (m.isValid())
		{
			QStandardItem* exeItem = executableModel->itemFromIndex(m);
			exeItem->setData(QDateTime::currentDateTime(), QExecutableModel::LastModifiedRole);

			// only auto-run if the test is checked
			if (m.data(Qt::CheckStateRole) == Qt::Checked)
			{
				emit showMessage("Change detected: " + path + "...");
				// add a little delay to avoid running multiple instances of the same test build,
				// and to avoid running the file before visual studio is done writting it.
				QTimer::singleShot(500, [this, path] {emit runTestInThread(path, true); });
			}
			else
			{
				QModelIndex m = executableModelHash[path];
				if (m.isValid())
				{
					QStandardItem* exeItem = executableModel->itemFromIndex(m);
					exeItem->setData(QExecutableModel::NOT_RUNNING, QExecutableModel::StateRole);
				}
			}
		}
		
	});

	// run test when signaled to. Queued connection so that multiple quick invocations will
	// be collapsed together.
	connect(this, &MainWindowPrivate::runTest, this, &MainWindowPrivate::runTestInThread, Qt::QueuedConnection);

	// update filewatcher when directory changes
	connect(fileWatcher, &QFileSystemWatcher::directoryChanged, [this](const QString& path)
	{
		// This could be caused by the re-build of a watched test (which cause additionally cause the
		// watcher to stop watching it), so just in case add all the test paths back.
		this->fileWatcher->addPaths(executablePaths);
	});

	// re-rerun tests when auto-testing is re-enabled
	connect(executableModel, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles)
	{
		QString path = topLeft.data(QExecutableModel::PathRole).toString();
		Qt::CheckState prevState = executableCheckedStateHash[path];
		// Only re-run IFF the check box state goes from unchecked to checked AND
		// the data has gotten out of date since the checkbox was off.
		if (topLeft.data(Qt::CheckStateRole) == Qt::Checked && prevState == Qt::Unchecked)
		{
			QString path = topLeft.data(QExecutableModel::PathRole).toString();
			QFileInfo xml(xmlPath(path));
			QFileInfo exe(path);

			if (xml.lastModified() < exe.lastModified())
			{
				// out of date! re-run.
				emit showMessage("Automatic testing enabled for: " + topLeft.data(Qt::DisplayRole).toString() + ". Re-running tests...");
				runTestInThread(topLeft.data(QExecutableModel::PathRole).toString(), true);
			}			
		}

		// update previous state
		executableCheckedStateHash[path] = (Qt::CheckState)topLeft.data(Qt::CheckStateRole).toInt();
	}, Qt::QueuedConnection);

	// filter test results when the filter is changed
	connect(testCaseFilterEdit, &QLineEdit::textChanged, this, [this](const QString& text)
	{
		testCaseProxyModel->setFilterRegExp(text);
		testCaseTreeView->expandAll();
	});

	// create a failure model when a test is clicked
	connect(testCaseTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected)
	{
		auto index = testCaseProxyModel->mapToSource(selected.indexes().first());
		DomItem* item = static_cast<DomItem*>(index.internalPointer());

		if (index.isValid())
		{
			if (index.data(GTestModel::FailureRole).toInt() > 0)
				failureTreeView->header()->show();
			else
				failureTreeView->header()->hide();
		}

		failureTreeView->setSortingEnabled(false);
		delete failureProxyModel->sourceModel();
		failureProxyModel->setSourceModel(new GTestFailureModel(item));
		failureTreeView->setSortingEnabled(true);
		for (int i = 0; i < failureProxyModel->columnCount(); ++i)
		{
			failureTreeView->resizeColumnToContents(i);
		}
	});

	// open failure dock on test double-click
	connect(testCaseTreeView, &QTreeView::doubleClicked, [this](const QModelIndex& index)
	{
		if (index.isValid())
			failureDock->show();
	});

	// open file on double-click
	connect(failureTreeView, &QTreeView::doubleClicked, [this](const QModelIndex& index)
	{
		if (index.isValid())
			QDesktopServices::openUrl(QUrl::fromLocalFile(index.data(GTestFailureModel::PathRole).toString()));
	});

	// display test output in the console window
	connect(this, &MainWindowPrivate::testOutputReady, this, [this](QString text)
	{
		// add the new test output
		consoleTextEdit->moveCursor(QTextCursor::End);
		consoleTextEdit->insertPlainText(text);
		consoleTextEdit->moveCursor(QTextCursor::End);
		consoleTextEdit->ensureCursorVisible();
	}, Qt::QueuedConnection);

	// update test progress
	connect(this, &MainWindowPrivate::testProgress, this, [this](QString test, int complete, int total)
	{
		qDebug() << test << complete << total << ((double)complete / total) * 100;
	});

	// open the GUI when a tray message is clicked
	connect(systemTrayIcon, &QSystemTrayIcon::messageClicked, [this]
	{		
		q_ptr->setWindowState(Qt::WindowActive);
		q_ptr->raise();
		if (!mostRecentFailurePath.isEmpty())
			selectTest(mostRecentFailurePath);
	});
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: xmlPath
//--------------------------------------------------------------------------------------------------
QString MainWindowPrivate::xmlPath(const QString& testPath) const
{
	QFileInfo testInfo(testPath);
	QString hash = QCryptographicHash::hash(testPath.toLatin1(), QCryptographicHash::Md5).toHex();
	return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first() + "/" + hash + ".xml";
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: addTestExecutable
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::addTestExecutable(const QString& path, Qt::CheckState checked /*= Qt::Checked*/, QDateTime lastModified /*= QDateTime::currentDateTime()*/)
{
	QFileInfo fileinfo(path);

	if (!fileinfo.exists())
		return;

	executableCheckedStateHash[path] = checked;

	QFileInfo xmlResults(xmlPath(path));
	QStandardItem* item = new QStandardItem(fileinfo.baseName());

	item->setData(path, QExecutableModel::PathRole);
	item->setData(lastModified, QExecutableModel::LastModifiedRole);
	item->setData(QExecutableModel::NOT_RUNNING);	
	item->setCheckable(true);
	item->setCheckState(checked);

	executableModel->appendRow(item);
	executableModelHash.insert(path, QPersistentModelIndex(executableModel->index(executableModel->rowCount() - 1, 0)));
	
	fileWatcher->addPath(fileinfo.dir().canonicalPath());
	fileWatcher->addPath(path);
	executablePaths << path;

	bool previousResults = loadTestResults(path, false);
	bool runAutomatically = (item->data(Qt::CheckStateRole) == Qt::Checked);
	bool outOfDate = previousResults && (xmlResults.lastModified() < lastModified);

	executableListView->setCurrentIndex(executableModel->indexFromItem(item));

	testRunningHash[path] = false;

	// if there are no previous results but the test is being watched, run the test
	if ((!previousResults || outOfDate) && runAutomatically)
	{
		this->runTestInThread(path, false);
	}
	else if (outOfDate && !runAutomatically)
	{
		QModelIndex m = executableModelHash[path];
		if (m.isValid())
		{
			QStandardItem* exeItem = executableModel->itemFromIndex(m);
			exeItem->setData(QExecutableModel::NOT_RUNNING, QExecutableModel::StateRole);
		}
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: runTestInThread
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::runTestInThread(const QString& pathToTest, bool notify)
{
	if (!testRunningHash[pathToTest])
	{
		testRunningHash[pathToTest] = true;

		std::thread t([this, pathToTest, notify]
		{
			QEventLoop loop;

			QFileInfo info(pathToTest);
			executableModel->setData(executableModelHash[pathToTest], QExecutableModel::RUNNING, QExecutableModel::StateRole);
			QProcess testProcess;
			QStringList arguments;

			bool first = true;
			int tests = 0;
			int progress = 0;

			// when the process finished, read any remaining output then quit the loop
			connect(&testProcess, static_cast<void (QProcess::*)(int)>(&QProcess::finished), &loop, [&]
			{
				QString output = testProcess.readAllStandardOutput();
				output.append("\nTEST RUN COMPLETED: " + QDateTime::currentDateTime().toString("yyyy-MMM-dd hh:mm:ss.zzz") + "\n\n");

				emit testOutputReady(output);
				emit testResultsReady(pathToTest, notify);
				emit testProgress(pathToTest, 0, 0);

				loop.exit();
			});

			// SET GTEST ARGS
			arguments << "--gtest_output=xml:" + this->xmlPath(pathToTest);

			testProcess.start(pathToTest, arguments);

			// get the first line of output. If we don't get it in a timely manner, the test is
			// probably bugged out so kill it.
			if (!testProcess.waitForReadyRead(500))
			{
				testProcess.kill();
				testRunningHash[pathToTest] = false;
				
				emit testProgress(pathToTest, 0, 0);
				emit testOutputReady("");
				
				return;
			}

			// print test output as it becomes available
			connect(&testProcess, &QProcess::readyReadStandardOutput, &loop, [&, pathToTest]
			{
				QString output = testProcess.readAllStandardOutput();

				// parse the first output line for the number of tests so we can
				// keep track of progress
				if (first)
				{
					// get the number of tests
					static QRegExp rx("([0-9]+) tests");
					rx.indexIn(output);
					tests = rx.cap(1).toInt();
					first = false;
				}
				else
				{
					static QRegExp rx("(\\[.*OK.*\\]|\\[.*FAILED.*\\])");
					if (rx.indexIn(output) != -1)
						progress++;
				}

				emit testProgress(pathToTest, progress, tests);
				emit testOutputReady(output);
				
				testRunningHash[pathToTest] = false;
			});

			loop.exec();

		});
		t.detach();
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: loadTestResults
//--------------------------------------------------------------------------------------------------
bool MainWindowPrivate::loadTestResults(const QString& testPath, bool notify)
{
	QFileInfo xmlInfo(xmlPath(testPath));

	if (!xmlInfo.exists())
	{
		return false;
	}

	QDomDocument doc(testPath);
	QFile file(xmlInfo.absoluteFilePath());
	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(q_ptr, "Error", "Could not open file located at: " + xmlPath(testPath));
		return false;
	}
	if (!doc.setContent(&file))
	{
		file.close();
		return false;
	}
	file.close();

	testResultsHash[testPath] = doc;

	// if the test that just ran is selected, update the view
	if (executableListView->selectionModel()->currentIndex().data(QExecutableModel::PathRole).toString() == testPath)
	{
		selectTest(testPath);
	}

	// set executable icon
	int numErrors = doc.elementsByTagName("testsuites").item(0).attributes().namedItem("failures").nodeValue().toInt();
	if (numErrors)
	{
		executableModel->setData(executableModelHash[testPath], QExecutableModel::FAILED, QExecutableModel::StateRole);
		mostRecentFailurePath = testPath;
		// only show notifications AFTER the initial startup, otherwise the user
		// could get a ton of messages every time they open the program. The messages
		if (notify && notifyOnFailureAction->isChecked())
		{
			systemTrayIcon->showMessage("Test Failure", QFileInfo(testPath).baseName() + " failed with " + QString::number(numErrors) + " errors.");
		}
	}
	else
	{
		executableModel->setData(executableModelHash[testPath], QExecutableModel::PASSED, QExecutableModel::StateRole);
		if(notify && notifyOnSuccessAction->isChecked())
		{
			systemTrayIcon->showMessage("Test Successful", QFileInfo(testPath).baseName() + " ran with no errors.");
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: selectTest
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::selectTest(const QString& testPath)
{
	delete testCaseProxyModel->sourceModel();
	delete failureProxyModel->sourceModel();
	testCaseTreeView->setSortingEnabled(false);
	testCaseProxyModel->setSourceModel(new GTestModel(testResultsHash[testPath]));
	failureProxyModel->clear();
	testCaseTreeView->setSortingEnabled(true);
	testCaseTreeView->expandAll();
	
	// make sure the right entry is selected
	QModelIndexList indices = executableModel->match(executableModel->index(0,0), QExecutableModel::PathRole, testPath);
	if (indices.size())
	{
		executableListView->setCurrentIndex(indices.first());
	}
	
	for (int i = 0; i < testCaseTreeView->model()->columnCount(); i++)
	{
		testCaseTreeView->resizeColumnToContents(i);
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: saveSettings
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::saveSettings() const
{
	Q_Q(const MainWindow);
	QSettings settings(APPINFO::organization, APPINFO::name);
	settings.setValue("geometry", q->saveGeometry());
	settings.setValue("windowState", q->saveState());

	// save executable information
	settings.beginWriteArray("tests");
	for (int row = 0; row < executableModel->rowCount(); row++)
	{
		settings.setArrayIndex(row);
		settings.setValue("path", executableModel->data(executableModel->index(row, 0), QExecutableModel::PathRole).toString());
		settings.setValue("checked", executableModel->data(executableModel->index(row, 0), Qt::CheckStateRole).toInt());
		settings.setValue("lastModified", executableModel->data(executableModel->index(row, 0), QExecutableModel::LastModifiedRole).toDateTime());
	}
	settings.endArray();

	settings.beginGroup("options");
	{
		settings.setValue("notifyOnFailure", notifyOnFailureAction->isChecked());
		settings.setValue("notifyOnSuccess", notifyOnSuccessAction->isChecked());
	}
	settings.endGroup();
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: loadSettings
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::loadSettings()
{
	Q_Q(MainWindow);
	QSettings settings(APPINFO::organization, APPINFO::name);
	q->restoreGeometry(settings.value("geometry").toByteArray());
	q->restoreState(settings.value("windowState").toByteArray());

	int size = settings.beginReadArray("tests");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		QString path = settings.value("path").toString();
		Qt::CheckState checked = static_cast<Qt::CheckState>(settings.value("checked").toInt());
		QDateTime lastModified = settings.value("lastModified").toDateTime();
		addTestExecutable(path, checked, lastModified);
	}
	settings.endArray();

	settings.beginGroup("options");
	{
		if (!settings.value("notifyOnFailure").isNull()) notifyOnFailureAction->setChecked(settings.value("notifyOnFailure").toBool());
		if (!settings.value("notifyOnSuccess").isNull()) notifyOnSuccessAction->setChecked(settings.value("notifyOnSuccess").toBool());
	}
	settings.endGroup();
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: removeTest
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::removeTest(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QString path = index.data(QExecutableModel::PathRole).toString();

	if (QMessageBox::question(this->q_ptr, QString("Remove Test?"), "Do you want to remove test " + QFileInfo(path).baseName() + "?",
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
	{
		executableListView->setCurrentIndex(index);

		// remove all data related to this test
		executablePaths.removeAll(path);
		executableModelHash.remove(path);
		testResultsHash.remove(path);
		fileWatcher->removePath(path);

		QAbstractItemModel* oldFailureModel = failureProxyModel->sourceModel();
		QAbstractItemModel* oldtestCaseModel = testCaseProxyModel->sourceModel();
		failureProxyModel->setSourceModel(new GTestFailureModel(nullptr));
		testCaseProxyModel->setSourceModel(new GTestModel(QDomDocument()));
		delete oldFailureModel;
		delete oldtestCaseModel;

		executableModel->removeRow(index.row(), index.parent());
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: getTestDialog
//--------------------------------------------------------------------------------------------------
QModelIndex MainWindowPrivate::getTestIndexDialog(const QString& label)
{
	bool ok;
	QStringList tests;

	for (int i = 0; i < executableModel->rowCount(); ++i)
	{
		tests << executableModel->index(i, 0).data().toString();
	}
	QString selected = QInputDialog::getItem(this->q_ptr, "Select Test", label, tests, 0, false, &ok);

	QModelIndexList matches = executableModel->match(executableModel->index(0, 0), Qt::DisplayRole, selected);
	if (ok && matches.size())
		return matches.first();
	else
		return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: createExecutableContextMenu
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::createExecutableContextMenu()
{
	Q_Q(MainWindow);

	executableContextMenu = new QMenu(executableListView);

	runTestAction = new QAction(q->style()->standardIcon(QStyle::SP_BrowserReload), "Run Test...", executableContextMenu);
	removeTestAction = new QAction(q->style()->standardIcon(QStyle::SP_DialogCloseButton), "Remove Test", executableContextMenu);

	executableContextMenu->addAction(runTestAction);
	executableContextMenu->addAction(removeTestAction);

	executableListView->setContextMenuPolicy(Qt::CustomContextMenu);
	
	connect(executableListView, &QListView::customContextMenuRequested, [this, q](const QPoint& pos)
	{
		QModelIndex indexUnderMouse = executableListView->indexAt(pos);
		if (indexUnderMouse.isValid())
		{
			executableContextMenu->exec(executableListView->mapToGlobal(pos));
		}
	});

	connect(runTestAction, &QAction::triggered, [this]
	{
		QString path = executableListView->currentIndex().data(QExecutableModel::PathRole).toString();
		runTestInThread(path, false);
	});

	connect(removeTestAction, &QAction::triggered, [this]
	{
		removeTest(executableListView->currentIndex());
	});
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: createTestMenu
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::createTestMenu()
{
	Q_Q(MainWindow);

	testMenu = new QMenu("Test", q);

	addTestAction = new QAction(QIcon(":/images/green"), "Add Test...", testMenu);
	selectAndRemoveTestAction = new QAction(q->style()->standardIcon(QStyle::SP_DialogCloseButton), "Remove Test...", testMenu);
	selectAndRunTest = new QAction(q->style()->standardIcon(QStyle::SP_BrowserReload), "Run Test...", testMenu);
	selectAndRunTest->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F5));

	testMenu->addAction(addTestAction);
	testMenu->addAction(selectAndRemoveTestAction);
	testMenu->addSeparator();
	testMenu->addAction(selectAndRunTest);

	q->menuBar()->addMenu(testMenu);

	connect(addTestAction, &QAction::triggered,	[this]()
	{
		QString filter;
#ifdef Q_OS_WIN32
		filter = "Text Executables (*.exe)";
#else
		filter = "Text Executables (*)";
#endif
		QString filename = QFileDialog::getOpenFileName(q_ptr, "Select Test Executable", QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(), filter);

		if (filename.isEmpty())
			return;

		addTestExecutable(filename, Qt::Checked, QFileInfo(filename).lastModified());
	});

	connect(selectAndRemoveTestAction, &QAction::triggered, [this]
	{
		removeTest(getTestIndexDialog("Select test to remove:"));
	});

	connect(selectAndRunTest, &QAction::triggered, [this]
	{
		QModelIndex index = getTestIndexDialog("Select Test to run:");
		if(index.isValid())
			runTestInThread(index.data(QExecutableModel::PathRole).toString(), false);
	});
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: createOptionsMenu
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::createOptionsMenu()
{
	Q_Q(MainWindow);

	optionsMenu = new QMenu("Options", q);

	notifyOnFailureAction = new QAction("Notify on auto-run Failure", optionsMenu);
	notifyOnSuccessAction = new QAction("Notify on auto-run Success", optionsMenu);
	notifyOnFailureAction->setCheckable(true);
	notifyOnFailureAction->setChecked(true);
	notifyOnSuccessAction->setCheckable(true);
	notifyOnSuccessAction->setChecked(false);

	optionsMenu->addAction(notifyOnFailureAction);
	optionsMenu->addAction(notifyOnSuccessAction);

	q->menuBar()->addMenu(optionsMenu);
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: createViewMenu
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::createWindowMenu()
{
	Q_Q(MainWindow);

	windowMenu = new QMenu("Window", q);
	windowMenu->addAction(executableDock->toggleViewAction());
	windowMenu->addAction(failureDock->toggleViewAction());
	windowMenu->addAction(consoleDock->toggleViewAction());
	
	q->menuBar()->addMenu(windowMenu);
}
