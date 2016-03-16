#include "QBottomUpSortFilterProxy.h"



// --------------------------------------------------------------------------------
// 	FUNCTION: QBottomUpSortFilterProxy (public )
// --------------------------------------------------------------------------------
QBottomUpSortFilterProxy::QBottomUpSortFilterProxy(QObject *parent /*= (QObject*)0*/) : QSortFilterProxyModel(parent)
{

}
// --------------------------------------------------------------------------------
// 	FUNCTION: ~QBottomUpSortFilterProxy (public )
// --------------------------------------------------------------------------------
QBottomUpSortFilterProxy::~QBottomUpSortFilterProxy()
{

}
// --------------------------------------------------------------------------------
// 	FUNCTION: filterAcceptsRow (public )
// --------------------------------------------------------------------------------
bool QBottomUpSortFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool iAmAccepted = sourceModel()->data(sourceParent, Qt::DisplayRole).toString().contains(filterRegExp());
	std::string debug = sourceModel()->data(sourceParent, Qt::DisplayRole).toString().toStdString();
	return (filterAcceptsDescendant(sourceRow, sourceParent) || filterAcceptsAncestor(sourceParent));
	return true;
}
// --------------------------------------------------------------------------------
// 	FUNCTION: filterAcceptsDescendant (public )
// --------------------------------------------------------------------------------
bool QBottomUpSortFilterProxy::filterAcceptsDescendant(int sourceRow, const QModelIndex &sourceParent) const
{
	// This function is inclusive of the original row queried in addition to all its descendants.
	QModelIndex rowToTest = sourceModel()->index(sourceRow, 0, sourceParent);

	std::string debug = sourceModel()->data(rowToTest).toString().toStdString();

	/// do bottom to top filtering
	if (sourceModel()->hasChildren(rowToTest))
	{
		for (int i = 0; i < sourceModel()->rowCount(rowToTest); ++i)
		{
			// if the filter accepts the child
			if (filterAcceptsDescendant(i, rowToTest))
				return true;
		}
	}

	return  sourceModel()->data(rowToTest).toString().contains(filterRegExp());
}
// --------------------------------------------------------------------------------
// 	FUNCTION: filterAcceptsAncestor (public )
// --------------------------------------------------------------------------------
bool QBottomUpSortFilterProxy::filterAcceptsAncestor(const QModelIndex &sourceIndex) const
{
	QModelIndex sourceParentIndex = sourceIndex.parent();

	std::string debug = sourceModel()->data(sourceParentIndex).toString().toStdString();

	/// do bottom to top filtering
	if (sourceParentIndex != QModelIndex())
	{
		// if the filter accepts the parent
		if (filterAcceptsAncestor(sourceParentIndex))
			return true;
	}

	return sourceModel()->data(sourceIndex).toString().contains(filterRegExp());
}