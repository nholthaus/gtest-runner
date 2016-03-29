#include "qexecutablemodel.h"

#include <QDebug>
#include <QIcon>
#include <QFileInfo>

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
			return QFileInfo(itr->path).baseName();
		default:
			return QVariant();
		}
	case Qt::CheckStateRole:
		if (index.column() == NameColumn)
			return itr->autorun;
		else
			return QVariant();
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
		return QFileInfo(itr->path).baseName();
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
		itr->autorun = (Qt::CheckState)value.toInt();
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
//	FUNCTION: supportedDropActions
//--------------------------------------------------------------------------------------------------
Q_INVOKABLE Qt::DropActions QExecutableModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: index
//--------------------------------------------------------------------------------------------------
QModelIndex QExecutableModel::index(const QString& path) const
{
	QModelIndex index = d_ptr->indexCache[path];

	// check the cache to see if we know the index
	if(index.isValid())
	{
		// if it hasn't changed since last time
		if (index.data(QExecutableModel::PathRole).toString() == path)
		{
			return index;
		}
	}

	auto itr = std::find(begin(), end(), path);
	index = iteratorToIndex(itr);
		
	// cache for later use
	d_ptr->indexCache[path] = index;

	return index;
}

