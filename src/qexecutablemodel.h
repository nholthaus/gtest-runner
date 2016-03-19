//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-gui
/// @BRIEF		model definition for the test executables
///	@DETAILS	
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

#ifndef qexecutablemodel_h__
#define qexecutablemodel_h__

//------------------------------
//	INCLUDE
//------------------------------

#include <QScopedPointer>
#include <QStandardItemModel>

//------------------------------
//	FORWARD DECLARATIONS
//------------------------------

class QExecutableModelPrivate;

//--------------------------------------------------------------------------------------------------
//	CLASS: QExecutableModel
//--------------------------------------------------------------------------------------------------
/// @brief		model for test executables
/// @details	
//--------------------------------------------------------------------------------------------------
class QExecutableModel : public QStandardItemModel
{
public:

	enum Columns
	{
//		AdvancedOptionsColumn,
		NameColumn,
		ProgressColumn,
	};

	enum Roles
	{
		PathRole = Qt::ToolTipRole,
		StateRole = Qt::UserRole,
		LastModifiedRole = Qt::UserRole + 1,
		ProgressRole = Qt::UserRole + 2,
	};

	enum States
	{
		NOT_RUNNING, 
		RUNNING, 
		PASSED, 
		FAILED,
 	};

 	explicit QExecutableModel(QObject* parent = nullptr);
	virtual ~QExecutableModel();

	bool hasChildren(const QModelIndex& parent) const override;

 	Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:

	
	
private:

	Q_DECLARE_PRIVATE(QExecutableModel);

	QScopedPointer<QExecutableModelPrivate>	d_ptr;


};	// CLASS: QExecutableModel

#endif // qexecutablemodel_h__