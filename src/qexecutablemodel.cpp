#include "qexecutablemodel.h"

#include <QIcon>

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

};	// CLASS: QExecutableModelPrivate


//--------------------------------------------------------------------------------------------------
//	FUNCTION: QExecutableModel
//--------------------------------------------------------------------------------------------------
QExecutableModel::QExecutableModel(QObject* parent /*= nullptr*/) : QStandardItemModel(parent),
	d_ptr(new QExecutableModelPrivate)
{

}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: hasChildren
//--------------------------------------------------------------------------------------------------
bool QExecutableModel::hasChildren(const QModelIndex& parent) const
{
	if (parent == QModelIndex())
		return true;
	else
		return false;
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

	switch (role)
	{
	case Qt::DecorationRole:
		if(index.column() == 0)
		{
			switch (data(index, StateRole).toInt())
			{
			case NOT_RUNNING:
				return d->grayIcon;
				break;
			case RUNNING:
				return d->yellowIcon;
				break;
			case PASSED:
				return d->greenIcon;
				break;
			case FAILED:
				return d->redIcon;
				break;
			default:
				return QIcon();
				break;
			}
			break;
		} 
	default:
		return QStandardItemModel::data(index, role);
		break;
	}
}

