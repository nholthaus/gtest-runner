
#include "mainwindow_p.h"

#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>

//--------------------------------------------------------------------------------------------------
//	FUNCTION: MainWindow
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QStringList tests, bool reset) : QMainWindow(), d_ptr(new MainWindowPrivate(tests, reset, this))
{
	Q_D(MainWindow);

	this->setStatusBar(d->statusBar);

	this->setCentralWidget(d->centralFrame);
	this->setWindowIcon(QIcon(":images/logo"));

	this->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	this->addDockWidget(Qt::LeftDockWidgetArea, d->executableDock);
	this->addDockWidget(Qt::BottomDockWidgetArea, d->consoleDock);
	this->addDockWidget(Qt::BottomDockWidgetArea, d->failureDock);
	this->tabifyDockWidget(d->failureDock, d->consoleDock);
	d->failureDock->raise();

	this->setDockNestingEnabled(true);

	// accept drops
	this->setAcceptDrops(true);

	// restore settings
	d->loadSettings();

	// add tests from the command line
	for (auto itr = tests.begin(); itr != tests.end(); ++itr)
	{
		QFileInfo info(*itr);
		d->addTestExecutable(info.absoluteFilePath(), true, info.lastModified());
	}
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

//--------------------------------------------------------------------------------------------------
//	FUNCTION: changeEvent
//--------------------------------------------------------------------------------------------------
void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: sizeHint
//--------------------------------------------------------------------------------------------------
QSize MainWindow::sizeHint() const
{
	QDesktopWidget* desktop = QApplication::desktop();
	return 0.5 * desktop->screen(desktop->primaryScreen())->size();
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: dragEnterEvent
//--------------------------------------------------------------------------------------------------
void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
	
	if (e->mimeData()->hasUrls()) 
	{
		QFileInfo info(e->mimeData()->urls().first().toLocalFile());
		if (info.isExecutable())
			e->acceptProposedAction();
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: dropEvent
//--------------------------------------------------------------------------------------------------
void MainWindow::dropEvent(QDropEvent *e)
{
	QFileInfo info(e->mimeData()->urls().first().toLocalFile());
	d_ptr->addTestExecutable(info.absoluteFilePath(), true, info.lastModified());
}
