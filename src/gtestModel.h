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

#ifndef gtestModel_h__
#define gtestModel_h__

//------------------------------
//	INCLUDE
//------------------------------

#include <QAbstractItemModel>
#include <QDomDocument>
#include <QIcon>
#include <QModelIndex>

#include "domitem.h"

class GTestModel : public QAbstractItemModel
{
	Q_OBJECT

public:

	enum Roles
	{
		FailureRole = Qt::UserRole,
	};

	enum Sections
	{
		Name = 0,
		TestNumber,
		Failures,
		Time,
		Tests,
		Errors,
		Disabled,
		Timestamp,
		Last		// always leave this as the last one.
	};

public:

	explicit GTestModel(QDomDocument document, QObject *parent = 0);
	~GTestModel();

	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:

	void removeComments(QDomNode &node);

private:

	QDomDocument domDocument;
	DomItem *rootItem;

	QIcon grayIcon;
	QIcon greenIcon;
	QIcon yellowIcon;
	QIcon redIcon;
};

#endif // gtestModel_h__
