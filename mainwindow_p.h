//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	project
/// @BRIEF		brief
///	@DETAILS	details
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
// and associated documentation files (the "Software"), to deal in the Software without 
// restriction, including without limitation the rights to use, copy, modify, merge, publish, 
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or 
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// ATTRIBUTION:
// Parts of this work have been adapted from: 
//
//--------------------------------------------------------------------------------------------------
// 
// Copyright (c) 2016 Nic Holthaus
// 
//--------------------------------------------------------------------------------------------------

#ifndef mainwindow_p_h__
#define mainwindow_p_h__

//------------------------------
//	INCLUDE
//------------------------------
#include "mainwindow.h"
#include "qexecutablemodel.h"
#include "appinfo.h"
#include "gtestModel.h"

#include <thread>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDockWidget>
#include <QDomDocument>
#include <QEventLoop>
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
#include <QStatusBar>
#include <QTreeView>

#include <qglobal.h>

//--------------------------------------------------------------------------------------------------
//	CLASS: MainWindowPrivate
//--------------------------------------------------------------------------------------------------
/// @brief		Private members of MainWindow
/// @details	
//--------------------------------------------------------------------------------------------------
class MainWindowPrivate : public QObject
{
	Q_OBJECT

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
	QHash<QString, QDomDocument>			testResultsHash;						///< Hash table storing the xml test results for each test path.

	QStatusBar*								statusBar;								///< status

signals:

	void testResultsReady(QString);													///< Signal emitted when new test results are ready
	void setStatus(QString);

public:

	explicit MainWindowPrivate(MainWindow* q);

	QString xmlPath(const QString& testPath) const;

	void addTestExecutable(const QString& path, Qt::CheckState checked = Qt::Checked, QDateTime lastModified = QDateTime::currentDateTime());

	void runTestInThread(const QString& pathToTest);

	bool loadTestResults(const QString& testPath);

	void selectTest(const QString& testPath);

	void saveSettings() const;

	void loadSettings();

};	// CLASS: MainWindowPrivate


#endif // mainwindow_p_h__