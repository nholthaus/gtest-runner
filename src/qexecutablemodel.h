//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-runner
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

#include "QTreeModel.h"

#include <QDateTime>

//------------------------------
//	FORWARD DECLARATIONS
//------------------------------

class QExecutableModelPrivate;

//--------------------------------------------------------------------------------------------------
//	CLASS: ExecutableData
//--------------------------------------------------------------------------------------------------
/// @brief		container for data about the test executable
/// @details	
//--------------------------------------------------------------------------------------------------
struct ExecutableData
{
	enum States
	{
		NOT_RUNNING,
		RUNNING,
		PASSED,
		FAILED,
	};

	/// allow implcit path conversion to help search the model
	ExecutableData(QString path = "") : path(path) {};

	QString			path;				///< Full, absolute path to test executable
	bool			autorun;			///< Whether to autorun the tests when they change.
	States			state;				///< Current state of test execution
	QDateTime		lastModified;		///< Last time the executable was modified
	double			progress;			///< Test run completeness, from 0 to 100
	QString			filter;				///< filter to be applied on gtest command line
	int				repeat;				///< Number of times to repeat the test. Can be -1.
	Qt::CheckState	runDisabled;		///< gtest command line to run disabled tests
	Qt::CheckState	shuffle;			///< gtest command line option to shuffle tests
	int				randomSeed;			///< Random seed for the shuffle
	QString			otherArgs;			///< any other dumb-ass args the user thinks I forgot.

	/// executable data needs to be unique per-path, so that's a good equality check
	friend bool operator==(const ExecutableData& lhs, const ExecutableData& rhs)
	{
		return lhs.path == rhs.path;
	}

	friend bool operator!=(const ExecutableData& lhs, const ExecutableData& rhs)
	{
		return !(lhs == rhs);
	}

};	// CLASS: ExecutableData

//--------------------------------------------------------------------------------------------------
//	CLASS: QExecutableModel
//--------------------------------------------------------------------------------------------------
/// @brief		model for test executables
/// @details	
//--------------------------------------------------------------------------------------------------
class QExecutableModel : public QTreeModel<ExecutableData>
{
public:

	enum Columns
	{
		AdvancedOptionsColumn,
		NameColumn,
		ProgressColumn,
	};

	enum Roles
	{
		PathRole = Qt::ToolTipRole,
		StateRole = Qt::UserRole,
		LastModifiedRole = Qt::UserRole + 1,
		ProgressRole = Qt::UserRole + 2,
		FilterRole = Qt::UserRole + 3,
		RepeatTestsRole = Qt::UserRole + 4,
		RunDisabledTestsRole = Qt::UserRole + 5,
		ShuffleRole = Qt::UserRole + 6,
		RandomSeedRole = Qt::UserRole + 7,
		ArgsRole = Qt::UserRole + 8,
		NameRole = Qt::UserRole + 9,
		AutorunRole = Qt::UserRole + 10,
	};

 	explicit QExecutableModel(QObject* parent = nullptr);
	virtual ~QExecutableModel();

 	Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	Q_INVOKABLE virtual Qt::DropActions supportedDropActions() const override;
	virtual Qt::DropActions supportedDragActions() const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	virtual QModelIndex removeRow(int row, const QModelIndex &parent = QModelIndex()) override;
	virtual QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
	virtual QMap<int, QVariant> itemData(const QModelIndex &index) const override;
	virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;
	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
	virtual QStringList mimeTypes() const override;
	virtual QMimeData * mimeData(const QModelIndexList &indexes) const override;


	/// return an index from a path
	QModelIndex index(const QString& path) const;



private:

	Q_DECLARE_PRIVATE(QExecutableModel);

	QScopedPointer<QExecutableModelPrivate>	d_ptr;

};	// CLASS: QExecutableModel

#endif // qexecutablemodel_h__