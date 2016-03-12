#include "mainwindow_p.h"

#include <QCryptographicHash>

//--------------------------------------------------------------------------------------------------
//	FUNCTION: MainWindowPrivate
//--------------------------------------------------------------------------------------------------
MainWindowPrivate::MainWindowPrivate(MainWindow* q) :
	q_ptr(q),
	executableDock(new QDockWidget(q)),
	executableDockFrame(new QFrame(q)),
	executableListView(new QListView(q)),
	executableModel(new QExecutableModel(q)),
	addTestButton(new QPushButton(q)),
	testCaseTreeView(new QTreeView(q)),
	statusBar(new QStatusBar(q))
{
	qRegisterMetaType<QVector<int>>("QVector<int>");

	connect(this, &MainWindowPrivate::setStatus, statusBar, &QStatusBar::setStatusTip, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::testResultsReady, this, &MainWindowPrivate::loadTestResults, Qt::QueuedConnection);
	connect(this, &MainWindowPrivate::testResultsReady, statusBar, &QStatusBar::clearMessage, Qt::QueuedConnection);

	connect(executableListView, &QListView::clicked, [this](const QModelIndex& index)
	{
		selectTest(index.model()->data(index, QExecutableModel::PathRole).toString());
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
	QFileInfo xmlResults(xmlPath(path));
	QStandardItem* item = new QStandardItem(fileinfo.baseName());
	item->setCheckable(true);
	item->setCheckState(checked);
	item->setData(path, QExecutableModel::PathRole);
	item->setData(fileinfo.lastModified(), QExecutableModel::LastModifiedRole);

	// determine state
	item->setData(QExecutableModel::NOT_RUNNING);

	executableModel->appendRow(item);
	executableModelHash.insert(path, QPersistentModelIndex(executableModel->index(executableModel->rowCount() - 1, 0)));
	fileWatcherHash.insert(path, new QFileSystemWatcher(QStringList() << path, q_ptr));

	bool previousResults = loadTestResults(path);
	bool runAutomatically = (item->data(Qt::CheckStateRole) == Qt::Checked);
	bool outOfDate = previousResults && (xmlResults.lastModified() < fileinfo.lastModified());

	// if there are no previous results but the test is being watched, run the test
	qDebug() << previousResults << runAutomatically << outOfDate;
	if ((!previousResults || outOfDate) && runAutomatically)
	{
		qDebug() << "RE-RUNNING";
		this->runTestInThread(path);
	}

	// run the test whenever the executable changes
	QObject::connect(fileWatcherHash[path], &QFileSystemWatcher::fileChanged, [this](const QString& path)
	{
		statusBar->showMessage("Change detected: " + path + ". Re-running tests...");
		this->runTestInThread(path);
	});
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: runTestInThread
//--------------------------------------------------------------------------------------------------
void MainWindowPrivate::runTestInThread(const QString& pathToTest)
{
	qDebug() << "RUNNING TEST";
	std::thread t([this, pathToTest]
	{
		QFileInfo info(pathToTest);
		executableModel->setData(executableModelHash[pathToTest], QExecutableModel::RUNNING, QExecutableModel::StateRole);
		QProcess testProcess;
		QStringList arguments;
		qDebug() << this->xmlPath(pathToTest);
		arguments << "--gtest_output=xml:" + this->xmlPath(pathToTest);

		testProcess.start(pathToTest, arguments);
		testProcess.waitForFinished(-1);

		emit testResultsReady(pathToTest);
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
	testCaseTreeView->setModel(new GTestModel(testResultsHash[testPath], testCaseTreeView));
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
