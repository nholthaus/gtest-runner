#include "domitem.h"
#include "gtestModel.h"

#include <QIcon>
#include <QtXml>

GTestModel::GTestModel(QDomDocument document, QObject *parent)
	: QAbstractItemModel(parent), domDocument(document), grayIcon(":/images/gray"),
	greenIcon(":/images/green"), yellowIcon(":/images/yellow"), redIcon(":/images/red")
{
	removeComments(domDocument);
	rootItem = new DomItem(domDocument, 0);
}

GTestModel::~GTestModel()
{
	delete rootItem;
}

int GTestModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 7;
}

QVariant GTestModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	DomItem *item = static_cast<DomItem*>(index.internalPointer());

	QDomNode node = item->node();
	QStringList attributes;
	QDomNamedNodeMap attributeMap = node.attributes();

	switch(role)
	{
	case Qt::DisplayRole:
		switch (index.column())
		{
			//name
		case 0:
			return attributeMap.namedItem("name").nodeValue();
			// failures
		case 1:
			if (attributeMap.namedItem("failures").isNull())
				return node.childNodes().count();
			else
				return attributeMap.namedItem("failures").nodeValue();
			// time
		case 2:
			return attributeMap.namedItem("time").nodeValue().toDouble() * 1000;
		case 3:
			return attributeMap.namedItem("tests").nodeValue();
		case 4:
			return attributeMap.namedItem("errors").nodeValue();
		case 5:
			return attributeMap.namedItem("disabled").nodeValue();
		case 6:
			return attributeMap.namedItem("timestamp").nodeValue();
		default:
			return QVariant();
		}
		break;
	case Qt::DecorationRole:
		if (index.column() == 0)
		{
			if (attributeMap.namedItem("name").nodeValue().contains("DISABLED_"))
				return grayIcon;
			if (!attributeMap.namedItem("failures").isNull())
			{
				if (attributeMap.namedItem("failures").nodeValue().toInt() > 0)
					return redIcon;
				else
					return greenIcon;
			}
			else
			{
				if (node.childNodes().count())
					return redIcon;
				else
					return greenIcon;
			}
		}
		break;
	case Qt::TextAlignmentRole:
		switch (index.column())
		{
		case 0:
			return Qt::AlignLeft;
		case 1:
			return Qt::AlignCenter;
		case 2:
			return Qt::AlignCenter;
		case  3:
			return Qt::AlignCenter;
		case 4:
			return Qt::AlignCenter;
		case 5:
			return Qt::AlignCenter;
		case 6:
			return Qt::AlignLeft;
		default:
			return Qt::AlignLeft;
		}
	default:
		return QVariant();
	}

	return QVariant();
}

Qt::ItemFlags GTestModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

QVariant GTestModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return tr("Name");
		case 1:
			return tr("Failures");
		case 2:
			return tr("Time (ms)");
		case 3:
			return tr("Tests");
		case 4:
			return tr("Errors");
		case 5:
			return tr("Disabled");
		case 6:
			return tr("Timestamp");
		default:
			return QVariant();
		}
	}

	return QVariant();
}

QModelIndex GTestModel::index(int row, int column, const QModelIndex &parent)
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
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex GTestModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
		return QModelIndex();

	DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
	DomItem *parentItem = childItem->parent();

	if (!parentItem || parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int GTestModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	DomItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<DomItem*>(parent.internalPointer());

	return parentItem->node().childNodes().count();
}

void GTestModel::removeComments(QDomNode &node)
{
	if (node.hasChildNodes())
	{
		// call remove comments recursively on all the child nodes
		// iterate backwards because once a node is removed, the
		// remaining nodes shift down in index, meaning iterating
		// forward would skip over some.
		for (int i = node.childNodes().count() - 1; i >= 0; i--)
		{
			QDomNode child = node.childNodes().at(i);
			// Uh-oh, recursion!!
			removeComments(child);
		}
	}
	else
	{
		// if the node has no children, check if it's a comment
		if (node.nodeType() == QDomNode::ProcessingInstructionNode ||
			node.nodeType() == QDomNode::CommentNode)
		{
			// if so, get rid of it.
			node.parentNode().removeChild(node);
		}
	}
}