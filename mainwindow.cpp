
#include "mainwindow_p.h"



//--------------------------------------------------------------------------------------------------
//	FUNCTION: MainWindow
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow() : QMainWindow(), d_ptr(new MainWindowPrivate(this))
{
	Q_D(MainWindow);

	this->setStatusBar(d->statusBar);

	this->setCentralWidget(d->testCaseTreeView);
	this->setWindowIcon(QIcon(":images/logo"));

	d->testCaseTreeView->setSortingEnabled(true);

	d->executableDock->setObjectName("executableDock");
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

