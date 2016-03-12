//--------------------------------------------------------------------------------------------------
//	
//	Passive Correlator
//	Group 108 - Tactical Defense Systems
//	MIT Lincoln Laboratory
//	244 Wood St.
//	Lexington, MA 02420-9108
//	
//--------------------------------------------------------------------------------------------------
//
///	@file			QBottomUpSortFilterProxy.h
///	@brief			Filter Proxy Model which searches from the bottom up
/// @details		Unlike the traditional QSortFilterProxyModel, this model will match children, and show them and their parents if they match the filter
///					Reference: http://www.qtcentre.org/archive/index.php/t-12390.html
//
//--------------------------------------------------------------------------------------------------
//
///	@author			Nic Holthaus
/// @date			September 25, 2014
//
//--------------------------------------------------------------------------------------------------

#ifndef QBottomUpSortFilterProxy_h__
#define QBottomUpSortFilterProxy_h__

//------------------------
//	INCLUDES
//------------------------
#include <QSortFilterProxyModel>

//	--------------------------------------------------------------------------------
///	@class		QBottomUpSortFilterProxy
///	@brief		Filter Proxy Model which searches from the bottom up.
///	@details	Unlike the traditional QSortFilterProxyModel, this model will match
///				children, and show them and their parents if they match the filter.
///				Thus, for a tree view, if any node in the hierarchy matches the regex,
///				the entire branch it lives in will be shown.
//  --------------------------------------------------------------------------------
class QBottomUpSortFilterProxy : public QSortFilterProxyModel
{
	Q_OBJECT

public:

	QBottomUpSortFilterProxy(QObject *parent = (QObject*)0);
	~QBottomUpSortFilterProxy();

protected:

	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
	bool filterAcceptsDescendant(int sourceRow, const QModelIndex &sourceParent) const;
	bool filterAcceptsAncestor(const QModelIndex &sourceIndex) const;

private:



};


#endif // QBottomUpSortFilterProxy_h__