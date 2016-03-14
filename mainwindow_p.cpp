#include "mainwindow_p.h"
#include "QStdOutSyntaxHighlighter.h"

#include "GTestFailureModel.h"
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFontDatabase>
#include <QHeaderView>

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
	testCaseTreeView(new QTreeView(q)),
	statusBar(new QStatusBar(q)),
	failureDock(new QDockWidget(q)),
	failureTreeView(new QTreeView(q)),
	failureProxyModel(new QBottomUpSortFilterProxy(q)),
	consoleDock(new QDockWidget(q)),
	consoleTextEdit(new QTextEdit(q)),
	consoleHighlighter(new QStdOutSyntaxHighlighter(consoleTextEdit))
{
	qRegisterMetaType<QVector<int>>("QVector<int>");

	QFontDatabase fontDB;
	fontDB.addApplicationFont(":/consolas");

	testCaseTreeView->setSortingEnabled(true);

	executableDock->setObjectName("executableDock");
	executableDock->setAllowedAreas(Qt::LeftDockWidgetArea);
	executableDock->setWindowTitle("Test Executables");
	executableDock->setWidget(executableDockFrame);

	executableListView->setModel(executableModel);

	executableDockFrame->setLayout(new QVBoxLayout);
	executableDockFrame->layout()->addWidget(executableListView);
	executableDockFrame->layout()->addWidget(addTestButton);

	addTestButton->setText("Add Test Executable...");

	testCaseTreeView->setModel(testCaseProxyModel);

	failureDock->setObjectName("failureDock");
	failureDock->setAllowedAreas(Qt::BottomDockWidgetArea);
	failureDock->setWindowTitle("Failures");
	failureDock->setWidget(failureTreeView);

	failureTreeView->setModel(failureProxyModel);
	failureTreeView->setAlternatingRowColors(true);

	consoleDock->setObjectName("consoleDock");
	consoleDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
	consoleDock->setWindowTitle("Console");
	consoleDock->setWidget(consoleTextEdit);

	QFont consolas("consolas", 10);
	consoleTextEdit->setFont(consolas);
	consoleTextEdit->setStyleSheet("QTextEdit { background-color: black; color: white; }");
	consoleTextEdit->setReadOnly(true);

	connect(this, &MainWindowPrivate::setStatus, statusBar, &QStatusBar::setStatusTip, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::testResultsReady, this, &MainWindowPrivate::loadTestResults, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::testResultsReady, statusBar, &QStatusBar::clearMessage, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::showMessage, statusBar, &QStatusBar::showMessage, Qt::QueuedConnection);

	// Open dialog when 'add test' is clicked
	connect(addTestButton, &QPushButton::clicked, [&]()
	{
		QString filename = QFileDialog::getOpenFileName(q_ptr, "Select Test Executable", QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(), "Text Executables (*.exe)");
		
		if (filename.isEmpty())
			return;
	
		addTestExecutable(filename, Qt::Checked, QFileInfo(filename).lastModified());
	});

	// switch testCase models when new tests are clicked
	connect(executableListView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected)
	{
		auto index = selected.indexes().first();
		selectTest(index.data(QExecutableModel::PathRole).toString());
	});

	// run the test whenever the executable changes
	connect(fileWatcher, &QFileSystemWatcher::fileChanged, [this](const QString& path)
	{	
		// only auto-run if the test is checked
		if (executableModelHash[path].data(Qt::CheckStateRole) == Qt::Checked)
		{
			emit showMessage("Change detected: " + path + ". Re-running tests...");
			runTestInThread(path);
		}
	});

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
				runTestInThread(topLeft.data(QExecutableModel::PathRole).toString());
			}			
		}

		// update previous state
		executableCheckedStateHash[path] = (Qt::CheckState)topLeft.data(Qt::CheckStateRole).toInt();
	}, Qt::QueuedConnection);

	// create a failure model when a test is clicked
	connect(testCaseTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected)
	{
		auto index = testCaseProxyModel->mapToSource(selected.indexes().first());
		DomItem* item = static_cast<DomItem*>(index.internalPointer());

		failureTreeView->setSortingEnabled(false);
		delete failureProxyModel->sourceModel();
		failureProxyModel->setSourceModel(new GTestFailureModel(item));
		failureTreeView->setSortingEnabled(true);
		for (int i = 0; i < failureProxyModel->columnCount(); ++i)
		{
			failureTreeView->resizeColumnToContents(i);
		}
	});

	// open file on double-click
	connect(failureTreeView, &QTreeView::doubleClicked, [this](const QModelIndex& index)
	{
		if (index.isValid())
			QDesktopServices::openUrl(QUrl::fromLocalFile(index.data(GTestFailureModel::PathRole).toString()));
	});

	connect(this, &MainWindowPrivate::testOutputReady, consoleTextEdit, &QTextEdit::append, Qt::QueuedConnection);
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

	bool previousResults = loadTestResults(path);
	bool runAutomatically = (item->data(Qt::CheckStateRole) == Qt::Checked);
	bool outOfDate = previousResults && (xmlResults.lastModified() < lastModified);

	executableListView->setCurrentIndex(executableModel->indexFromItem(item));

	// if there are no previous results but the test is being watched, run the test
	if ((!previousResults || outOfDate) && runAutomatically)
	{
		this->runTestInThread(path);
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: runTestInThread
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::runTestInThread(const QString& pathToTest)
{
	std::thread t([this, pathToTest]
	{
		QFileInfo info(pathToTest);
		executableModel->setData(executableModelHash[pathToTest], QExecutableModel::RUNNING, QExecutableModel::StateRole);
		QProcess testProcess;
		QStringList arguments;

		arguments << "--gtest_output=xml:" + this->xmlPath(pathToTest);

		testProcess.start(pathToTest, arguments);
		testProcess.waitForFinished(-1);

		QString output = testProcess.readAllStandardOutput();
		testProcess.closeReadChannel(QProcess::StandardOutput);
		qApp->processEvents();

		emit testResultsReady(pathToTest);

		if (!output.isEmpty())
		{
			output.append("\nTEST RUN COMPLETED: " + QDateTime::currentDateTime().toString() + "\n");
			emit testOutputReady(output);
		}

		qApp->processEvents();	
	});
	t.detach();
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: loadTestResults
//--------------------------------------------------------------------------------------------------
bool MainWindowPrivate::loadTestResults(const QString& testPath)
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
	if (doc.elementsByTagName("testsuites").item(0).attributes().namedItem("failures").nodeValue().toInt())
	{
		executableModel->setData(executableModelHash[testPath], QExecutableModel::FAILED, QExecutableModel::StateRole);
	}
	else
	{
		executableModel->setData(executableModelHash[testPath], QExecutableModel::PASSED, QExecutableModel::StateRole);
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
	for (size_t i = 0; i < testCaseTreeView->model()->columnCount(); i++)
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
}
