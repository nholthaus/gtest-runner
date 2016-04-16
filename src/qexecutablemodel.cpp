#include "qexecutablemodel.h"

#include <QByteArray>
#include <QDebug>
#include <QIcon>
#include <QFileInfo>
#include <QMimeData>

//--------------------------------------------------------------------------------------------------
//	CLASS: QExecutableModelPrivate
//--------------------------------------------------------------------------------------------------
/// @brief		Private data of QExecutable Model
/// @details	
//--------------------------------------------------------------------------------------------------
class QExecutableModelPrivate
{
public:

	QIcon grayIcon;
	QIcon greenIcon;
	QIcon yellowIcon;
	QIcon redIcon;

	explicit QExecutableModelPrivate() :
		grayIcon(QIcon(":images/gray")),
		greenIcon(QIcon(":images/green")),
		yellowIcon(QIcon(":images/yellow")),
		redIcon(QIcon(":images/red"))
	{};

	QHash<QString, QModelIndex> indexCache;		// used to speed up finding indices.

};	// CLASS: QExecutableModelPrivate


//--------------------------------------------------------------------------------------------------
//	FUNCTION: QExecutableModel
//--------------------------------------------------------------------------------------------------
QExecutableModel::QExecutableModel(QObject* parent /*= nullptr*/) : QTreeModel<ExecutableData>(parent),
	d_ptr(new QExecutableModelPrivate)
{

}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: ~QExecutableModel
//--------------------------------------------------------------------------------------------------
QExecutableModel::~QExecutableModel()
{

}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: columnCount
//--------------------------------------------------------------------------------------------------
Q_INVOKABLE int QExecutableModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return 3;
}

// --------------------------------------------------------------------------------------------------
// 	FUNCTION: data
// --------------------------------------------------------------------------------------------------
Q_INVOKABLE QVariant QExecutableModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
	if (!index.isValid())
		return QVariant();

	Q_D(const QExecutableModel);

	auto itr = indexToIterator(index);
	if (itr == tree.end())
		return QVariant();

	QString name = QFileInfo(itr->path).baseName();

	switch (role)
	{
	case Qt::DecorationRole:
 		if(index.column() == NameColumn)
 		{
			switch (data(index, StateRole).toInt())
			{
			case ExecutableData::NOT_RUNNING:
				return d->grayIcon;
				break;
			case ExecutableData::RUNNING:
				return d->yellowIcon;
				break;
			case ExecutableData::PASSED:
				return d->greenIcon;
				break;
			case ExecutableData::FAILED:
				return d->redIcon;
				break;
			default:
				return QIcon();
				break;
			}
			break;
		} 
	case Qt::DisplayRole:
		switch (index.column())
		{
		case NameColumn:
			return data(index, NameRole);
		default:
			return QVariant();
		}
	case Qt::CheckStateRole:
		if (index.column() == NameColumn)
		{
			if (itr->autorun)
				return Qt::Checked;
			else
				return Qt::Unchecked;
		}
		else
			return QVariant();
	case AutorunRole:
		return itr->autorun;
	case PathRole:
		return itr->path;
	case StateRole:
		return itr->state;
	case LastModifiedRole:
		return itr->lastModified;
	case ProgressRole:
		return itr->progress;
	case FilterRole:
		return itr->filter;
	case RepeatTestsRole:
		return itr->repeat;
	case RunDisabledTestsRole:
		return itr->runDisabled;
	case ShuffleRole:
		return itr->shuffle;
	case RandomSeedRole:
		return itr->randomSeed;
	case ArgsRole:
		return itr->otherArgs;
	case NameRole:
		if (itr->path.contains("Debug"))
			name.append(" (Debug)");
		else if (itr->path.contains("RelWithDebInfo"))
			name.append(" (RelWithDebInfo)");
		else if (itr->path.contains("Release"))
			name.append(" (Release)");
		else if (itr->path.contains("MinSizeRel"))
			name.append(" (MinSizeRel)");
		return name;
	default:
		return QVariant();
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: setData
//--------------------------------------------------------------------------------------------------
Q_INVOKABLE bool QExecutableModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
	auto itr = indexToIterator(index);
	switch (role)
	{
	case Qt::EditRole || Qt::DisplayRole || QExecutableModel::PathRole:
		itr->path = value.toString();
		break;
	case Qt::CheckStateRole:
		itr->autorun = value.toBool();
	case AutorunRole:
		itr->autorun = value.toBool();
		break;
	case StateRole:
		itr->state = (ExecutableData::States)value.toInt();
		break;
	case LastModifiedRole:
		itr->lastModified = value.toDateTime();
		break;
	case ProgressRole:
		itr->progress = value.toDouble();
		break;
	case FilterRole:
		itr->filter = value.toString();
		break;
	case RepeatTestsRole:
		itr->repeat = value.toInt();
		break;
	case RunDisabledTestsRole:
		itr->runDisabled = (Qt::CheckState)value.toInt();
		break;
	case ShuffleRole:
		itr->shuffle = (Qt::CheckState)value.toInt();
		break;
	case RandomSeedRole:
		itr->randomSeed = value.toInt();
		break;
	case ArgsRole:
		itr->otherArgs = value.toString();
		break;
	default:
		return false;
	}

	// signal that the whole row has changed
	QModelIndex right = index.sibling(index.row(), columnCount());
	emit dataChanged(index, right);
	return true;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: index
//--------------------------------------------------------------------------------------------------
QModelIndex QExecutableModel::index(const QString& path) const
{
// 	if(d_ptr->indexCache.contains(path))
// 	{
// 		QModelIndex index = d_ptr->indexCache[path];
// 
// 		// check the cache to see if we know the index
// 		if (index.isValid())
// 		{
// 			// if it hasn't changed since last time
// 			if (index.data(QExecutableModel::PathRole).toString() == path)
// 			{
// 				return index;
// 			}
// 		}
// 	}

	auto itr = std::find(begin(), end(), path);
	QModelIndex index = iteratorToIndex(itr);
		
	// cache for later use
	d_ptr->indexCache[path] = index;

	return index;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: index
//--------------------------------------------------------------------------------------------------
QModelIndex QExecutableModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
	return QTreeModel::index(row, column, parent);
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: mimeTypes
//--------------------------------------------------------------------------------------------------
QStringList QExecutableModel::mimeTypes() const
{
	QStringList types;
	types << "application/x.text.executableData.list";
	return types;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: mimeData
//--------------------------------------------------------------------------------------------------
QMimeData * QExecutableModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	foreach(const QModelIndex &index, indexes) {
		if (index.isValid() && index.column() == 0) 
		{
			QVariant PathRoleText = data(index, PathRole);
			QVariant StateRoleText = data(index, StateRole);
			QVariant LastModifiedRoleText = data(index, LastModifiedRole);
			QVariant ProgressRoleText = data(index, ProgressRole);
			QVariant FilterRoleText = data(index, FilterRole);
			QVariant RepeatTestsRoleText = data(index, RepeatTestsRole);
			QVariant RunDisabledTestsRoleText = data(index, RunDisabledTestsRole);
			QVariant ShuffleRoleText = data(index, ShuffleRole);
			QVariant RandomSeedRoleText = data(index, RandomSeedRole);
			QVariant ArgsRoleText = data(index, ArgsRole);
			QVariant NameRoleText = data(index, NameRole);
			QVariant AutorunRoleText = data(index, AutorunRole);
			stream << PathRoleText << StateRoleText << LastModifiedRoleText << ProgressRoleText << FilterRoleText << RepeatTestsRoleText <<
				RunDisabledTestsRoleText << ShuffleRoleText << RandomSeedRoleText << ArgsRoleText << NameRoleText << AutorunRoleText;
		}
	}

	mimeData->setData("application/x.text.executableData.list", encodedData);
	return mimeData;

}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: dropMimeData
//--------------------------------------------------------------------------------------------------
bool QExecutableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction)
		return true;

	if (!(supportedDragActions() & action))
		return false;

	if (data->hasFormat("application/x.text.executableData.list"))
	{
		QByteArray encodedData = data->data("application/x.text.executableData.list");
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
		QList<QMap<int, QVariant>> newItems;
		int count = 0;

		while (!stream.atEnd()) {
			QMap<int, QVariant> itemData;
			stream >> itemData[PathRole];
			stream >> itemData[StateRole];
			stream >> itemData[LastModifiedRole];
			stream >> itemData[ProgressRole];	// doesn't seem to be there
			stream >> itemData[FilterRole];
			stream >> itemData[RepeatTestsRole];
			stream >> itemData[RunDisabledTestsRole];
			stream >> itemData[ShuffleRole];
			stream >> itemData[RandomSeedRole];
			stream >> itemData[ArgsRole];
			stream >> itemData[NameRole];
			stream >> itemData[AutorunRole];
			newItems.push_back(itemData);
			++count;
		}

		if (row < 0)
			row = rowCount(parent);

		insertRows(row, count, parent);

		for (int i = 0; i < count; ++i)
		{
			setItemData(index(row + i, 0, parent), newItems[i]);
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: itemData
//--------------------------------------------------------------------------------------------------
QMap<int, QVariant> QExecutableModel::itemData(const QModelIndex &index) const
{
	auto itr = indexToIterator(index);
	QMap<int, QVariant> ret;
	ret[PathRole] = itr->path;
	ret[StateRole] = itr->state;
	ret[LastModifiedRole] = itr->lastModified;
	ret[ProgressRole] = itr->progress;
	ret[FilterRole] = itr->filter;
	ret[RepeatTestsRole] = itr->repeat;
	ret[RunDisabledTestsRole] = itr->runDisabled;
	ret[ShuffleRole] = itr->shuffle;
	ret[RandomSeedRole] = itr->randomSeed;
	ret[ArgsRole] = itr->otherArgs;
	ret[AutorunRole] = itr->autorun;
	return ret;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: setItemData
//--------------------------------------------------------------------------------------------------
bool QExecutableModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
	Q_D(QExecutableModel);
	// invalidate the cache on a remove
	d->indexCache.clear();

	auto itr = indexToIterator(index);
	itr->path = roles[PathRole].toString();
	itr->state = (ExecutableData::States)roles[StateRole].toInt();
	itr->lastModified = roles[LastModifiedRole].toDateTime();
	itr->progress = roles[ProgressRole].toInt();
	itr->filter = roles[FilterRole].toString();
	itr->repeat = roles[RepeatTestsRole].toInt();
	itr->runDisabled = (Qt::CheckState)roles[RunDisabledTestsRole].toInt();
	itr->shuffle = (Qt::CheckState)roles[ShuffleRole].toInt();
	itr->randomSeed = roles[RandomSeedRole].toInt();
	itr->otherArgs = roles[ArgsRole].toString();
	itr->autorun = roles[AutorunRole].toBool();
	return true;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: moveRows
//--------------------------------------------------------------------------------------------------
bool QExecutableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
	throw std::logic_error("The method or operation is not implemented.");
}

// --------------------------------------------------------------------------------------------------
// 	FUNCTION: supportedDragActions
// --------------------------------------------------------------------------------------------------
Qt::DropActions QExecutableModel::supportedDragActions() const
{
	return Qt::MoveAction;
}

// --------------------------------------------------------------------------------------------------
// 	FUNCTION: supportedDropActions
// --------------------------------------------------------------------------------------------------
Q_INVOKABLE Qt::DropActions QExecutableModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: flags
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags QExecutableModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags f;

	if (index.isValid())
		f = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
	else
		f = Qt::ItemIsDropEnabled;

	switch (index.column())
	{
	case NameColumn: 
		f |= Qt::ItemIsUserCheckable;
		break;
	default:
		break;
	}

	return f;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: insertRows
//--------------------------------------------------------------------------------------------------
bool QExecutableModel::insertRows(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
	Q_D(QExecutableModel);
	// invalidate the cache on a remove
	d->indexCache.clear();
	return QTreeModel::insertRows(row, count, parent);
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: removeRows
//--------------------------------------------------------------------------------------------------
bool QExecutableModel::removeRows(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
	Q_D(QExecutableModel);
	// invalidate the cache on a remove
	d->indexCache.clear();
	return QTreeModel::removeRows(row, count, parent);
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: removeRow
//--------------------------------------------------------------------------------------------------
QModelIndex QExecutableModel::removeRow(int row, const QModelIndex &parent /*= QModelIndex()*/)
{
	Q_D(QExecutableModel);
	// invalidate the cache on a remove
	d->indexCache.clear();
	return QTreeModel::removeRow(row, parent);
}

