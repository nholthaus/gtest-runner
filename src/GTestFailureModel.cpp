#include "domitem.h"
#include "GTestFailureModel.h"

#include <QtXml>
#include <QRegExp>

GTestFailureModel::GTestFailureModel(DomItem* root, QObject *parent)
	: QAbstractItemModel(parent), failIcon(":/images/fail"), rootItem(nullptr)
{
	if(root)
	{
		rootItem = new DomItem(root->node(), 0);
	}
}

GTestFailureModel::~GTestFailureModel()
{
	if(rootItem)
	{
		delete rootItem;
	}
}

int GTestFailureModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 8;
}

QVariant GTestFailureModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	DomItem *item = static_cast<DomItem*>(index.internalPointer());
	QString message = item->node().attributes().namedItem("message").nodeValue();

	static QRegExp filerx("(.*)[:]([0-9]+)");
	static QRegExp valueofrx("[Vv]alue of: ([^,\n]*)|[VDd]eath test: ([^,\n]*)");
	static QRegExp actualrx("[Aa]ctual[:][ ]([^,\n]*)|[Rr]esult[:][ ]([^,\n]*)|(Failed)");
	static QRegExp expectedrx("[Ee]xpected[:][ ]([^,\n]*)|[Ee]rror msg[:]\n(.*)");
	static QRegExp whichisrx("[Ww]hich is: ([^,\n]*)");
	static QRegExp nearrx("The difference between (.*) and (.*) is (.*), which exceeds (.*), where\n(.*) evaluates to(.*),\n(.*) evaluates to(.*), and\n(.*) evaluates to(.*).");
	static QRegExp predrx("\n(.*) evaluates to (.*), where\n(.*)");
	static QRegExp sehrx("(.*)\n(.*) with code (.*) thrown in the test body");

	QString filename;

	switch (role)
	{
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			filerx.indexIn(message);
			filename = QFileInfo(filerx.cap(1)).fileName();
			if (!filename.isEmpty()) return filename;
			sehrx.indexIn(message);
			return sehrx.cap(1);
		case 1:
			filerx.indexIn(message);
			return filerx.cap(2);
			break;
		case 2:
			valueofrx.indexIn(message);
			for (int i = 1; i <= valueofrx.captureCount(); ++i)
				if (!valueofrx.cap(i).isEmpty()) return valueofrx.cap(i);
			nearrx.indexIn(message);
			if(!nearrx.cap(7).isEmpty()) return nearrx.cap(7);
			predrx.indexIn(message);
			if (!predrx.cap(1).isEmpty())  return predrx.cap(1);
			sehrx.indexIn(message);
			return sehrx.cap(2);
		case 3:
			actualrx.indexIn(message);
			for (int i = 1; i <= actualrx.captureCount(); ++i)
				if (!actualrx.cap(i).isEmpty()) return actualrx.cap(i);
			nearrx.indexIn(message);
			if (!nearrx.cap(8).isEmpty()) return nearrx.cap(8);
			predrx.indexIn(message);
			if(!predrx.cap(2).isEmpty()) return predrx.cap(2);
			sehrx.indexIn(message);
			return sehrx.cap(3);
		case 4:
			expectedrx.indexIn(message);
			for (int i = 1; i <= expectedrx.captureCount(); ++i)
				if (!expectedrx.cap(i).isEmpty()) return expectedrx.cap(i);
			nearrx.indexIn(message);
			if (!nearrx.cap(5).isEmpty()) return nearrx.cap(5);
			predrx.indexIn(message);
			if (!predrx.cap(1).isEmpty()) return "true";
			return QVariant();
		case 5:
			whichisrx.indexIn(message);
			for (int i = 1; i <= whichisrx.captureCount(); ++i)
				if (!whichisrx.cap(i).isEmpty()) return whichisrx.cap(i);
			nearrx.indexIn(message);
			if (!nearrx.cap(6).isEmpty()) return nearrx.cap(6);
			predrx.indexIn(message);
			return predrx.cap(3);
		case 6:
			nearrx.indexIn(message);
			return nearrx.cap(3);
		case 7:
			nearrx.indexIn(message);
			return nearrx.cap(10);
		default:
			return QVariant();
		}
	case Qt::DecorationRole:
		if (index.column() == 0) return failIcon;
		return QVariant();
	case Qt::TextAlignmentRole:
		switch (index.column())
		{
		case 1:
			return (int)(Qt::AlignHCenter | Qt::AlignVCenter);
		default:
			return (int)(Qt::AlignLeft | Qt::AlignVCenter);
		}
	case Qt::ToolTipRole:
		return message;
	case PathRole:
		filerx.indexIn(message);
		return QFileInfo(filerx.cap(1)).canonicalFilePath();
	case LineRole:
		filerx.indexIn(message);
		return filerx.cap(2);
	default:
		return QVariant();
	}
}

Qt::ItemFlags GTestFailureModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

QVariant GTestFailureModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return tr("File Name");
		case 1:
			return tr("Line");
		case 2:
			return tr("Value of");
		case 3:
			return tr("Actual");
		case 4:
			return tr("Expected");
		case 5:
			return tr("Which is");
		case 6:
			return tr("Difference");
		case 7:
			return tr("Tolerance");
		default:
			return QVariant();
		}
	}

	return QVariant();
}

QModelIndex GTestFailureModel::index(int row, int column, const QModelIndex &parent)
const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	DomItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<DomItem*>(parent.internalPointer());

	DomItem *childItem = parentItem->child(row);
	if (childItem)
	{
		return createIndex(row, column, childItem);
	}
	else
		return QModelIndex();
}

QModelIndex GTestFailureModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
		return QModelIndex();

	DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
	DomItem *parentItem = childItem->parent();

	if (!parentItem || parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int GTestFailureModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	DomItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<DomItem*>(parent.internalPointer());

	if (parentItem == rootItem && rootItem)
	{
		int count = 0;
		QDomNodeList failures = parentItem->node().childNodes();
		for (auto i = 0; i < failures.count(); ++i)
		{
			if (failures.at(i).nodeName() == "failure") ++count;
		}
		return count;
	}
	else
		return 0;

}