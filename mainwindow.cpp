#include "mainwindow.h"

#include <QDebug>
#include <QDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QFrame>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
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

	explicit MainWindowPrivate(MainWindow* q) : 
		q_ptr(q),
		executableDock(new QDockWidget(q)),
		executableDockFrame(new QFrame(q)),
		executableListWidget(new QListWidget(q)),
		addTestButton(new QPushButton(q)),
		testCaseTreeView(new QTreeView(q))
	{

	};

	MainWindow*		q_ptr;

	QDockWidget*	executableDock;							///< Dock Widget for the gtest executable selector.

	QFrame*			executableDockFrame;					///< Frame for containing the dock's sub-widgets
	QListWidget*	executableListWidget;					///< Widget to display and select gtest executables	
	QPushButton*	addTestButton;							///< Button which adds a test to the monitored tests.

	QTreeView*		testCaseTreeView;						///< Tree view where the test results are displayed.

};	// CLASS: MainWindowPrivate

//--------------------------------------------------------------------------------------------------
//	FUNCTION: MainWindow
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow() : QMainWindow(), d_ptr(new MainWindowPrivate(this))
{
	Q_D(MainWindow);

	this->setCentralWidget(d->testCaseTreeView);

	d->executableDock->setAllowedAreas(Qt::LeftDockWidgetArea);
	d->executableDock->setWindowTitle("Test Executables");
	d->executableDock->setWidget(d->executableDockFrame);
	this->addDockWidget(Qt::LeftDockWidgetArea, d->executableDock);

	d->executableDockFrame->setLayout(new QVBoxLayout);
	d->executableDockFrame->layout()->addWidget(d->executableListWidget);
	d->executableDockFrame->layout()->addWidget(d->addTestButton);

	d->addTestButton->setText("Add Test Executable...");
	connect(d->addTestButton, &QPushButton::clicked, [&, d]()
	{
		QString filename = QFileDialog::getOpenFileName(d->q_ptr, "Select Test Executable", QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(), "Text Executables (*.exe)");
		
		if (filename.isEmpty())
			return;

		QFileInfo fileinfo(filename);
		QListWidgetItem* item = new QListWidgetItem(fileinfo.baseName());
		d->executableListWidget->addItem(item);
	});
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: ~MainWindow
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{

}

