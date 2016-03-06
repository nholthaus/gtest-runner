#include "mainwindow.h"
#include "qexecutablemodel.h"
#include "appinfo.h"

#include <thread>

#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QFrame>
#include <QHash>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QPersistentModelIndex>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTreeView>
#include <qglobal.h>

//--------------------------------------------------------------------------------------------------
//	CLASS: MainWindowPrivate
//--------------------------------------------------------------------------------------------------
/// @brief		Private members of MainWindow
/// @details	
//--------------------------------------------------------------------------------------------------
class MainWindowPrivate
{
public:

	Q_DECLARE_PUBLIC(MainWindow);

	MainWindow*								q_ptr;

	QDockWidget*							executableDock;							///< Dock Widget for the gtest executable selector.

	QFrame*									executableDockFrame;					///< Frame for containing the dock's sub-widgets
	QListView*								executableListView;						///< Widget to display and select gtest executables	
	QExecutableModel*						executableModel;						///< Item model for test executables.
	QPushButton*							addTestButton;							///< Button which adds a test to the monitored tests.

	QHash<QString, QFileSystemWatcher*>		fileWatcherHash;						///< Hash table to store the file system watchers.
	QHash<QString, QPersistentModelIndex>	executableModelHash;					///< Hash for finding entries in the executable model.
	QTreeView*								testCaseTreeView;						///< Tree view where the test results are displayed.

public:

	explicit MainWindowPrivate(MainWindow* q) :
		q_ptr(q),
		executableDock(new QDockWidget(q)),
		executableDockFrame(new QFrame(q)),
		executableListView(new QListView(q)),
		executableModel(new QExecutableModel(q)),
		addTestButton(new QPushButton(q)),
		testCaseTreeView(new QTreeView(q))
	{

	};

	void addTestExecutable(const QString& path, Qt::CheckState checked = Qt::Checked, QDateTime lastModified = QDateTime::currentDateTime())
	{
		QFileInfo fileinfo(path);
		if (!fileinfo.exists())
			return;

		qDebug() << fileinfo.lastModified();
		QStandardItem* item = new QStandardItem(fileinfo.baseName());
		item->setCheckable(true);
		item->setCheckState(checked);
		item->setData(path, QExecutableModel::PathRole);
		item->setData(QExecutableModel::NOT_RUNNING, QExecutableModel::StateRole);

		executableModel->appendRow(item);
		executableModelHash.insert(path, QPersistentModelIndex(executableModel->index(executableModel->rowCount() - 1, 0)));

		fileWatcherHash.insert(path, new QFileSystemWatcher(QStringList() << path, q_ptr));

		QObject::connect(fileWatcherHash[path], &QFileSystemWatcher::fileChanged, [this](const QString& path)
		{
			this->runTestInThread(path);
		});
	}

	void runTestInThread(const QString& pathToTest) const
	{
		std::thread t([this, pathToTest]
		{
			qDebug() << "running test" << pathToTest;
			executableModel->setData(executableModelHash[pathToTest], QExecutableModel::RUNNING, QExecutableModel::StateRole);
			QProcess testProcess;
			bool success = testProcess.startDetached(pathToTest);
			testProcess.waitForFinished(-1);
			qDebug() << "got here";
			return success;
		});
		t.detach();
	}

	void saveSettings() const
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

	void loadSettings()
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

};	// CLASS: MainWindowPrivate

//--------------------------------------------------------------------------------------------------
//	FUNCTION: MainWindow
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow() : QMainWindow(), d_ptr(new MainWindowPrivate(this))
{
	Q_D(MainWindow);

	this->setCentralWidget(d->testCaseTreeView);
	this->setWindowIcon(QIcon(":images/logo"));

	d->executableDock->setAllowedAreas(Qt::LeftDockWidgetArea);
	d->executableDock->setWindowTitle("Test Executables");
	d->executableDock->setWidget(d->executableDockFrame);
	this->addDockWidget(Qt::LeftDockWidgetArea, d->executableDock);

	d->executableDockFrame->setLayout(new QVBoxLayout);
	d->executableDockFrame->layout()->addWidget(d->executableListView);
	d->executableDockFrame->layout()->addWidget(d->addTestButton);

	d->executableListView->setModel(d->executableModel);

	d->addTestButton->setText("Add Test Executable...");
	connect(d->addTestButton, &QPushButton::clicked, [&, d]()
	{
		QString filename = QFileDialog::getOpenFileName(d->q_ptr, "Select Test Executable", QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(), "Text Executables (*.exe)");
		
		if (filename.isEmpty())
			return;

		d->addTestExecutable(filename);
	});

	// restore settings
	d->loadSettings();
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: ~MainWindow
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{

}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: closeEvent
//--------------------------------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
	Q_D(MainWindow);
	d->saveSettings();
	QMainWindow::closeEvent(event);
}

