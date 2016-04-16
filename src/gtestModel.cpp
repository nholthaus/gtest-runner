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
	return Last;
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
		case Name:
			return attributeMap.namedItem("name").nodeValue();
		case TestNumber:
			return item->row();
		case Failures:
			if (attributeMap.namedItem("failures").isNull())
				return node.childNodes().count();
			else
				return attributeMap.namedItem("failures").nodeValue();
		case Time:
			return attributeMap.namedItem("time").nodeValue().toDouble() * 1000;
		case Tests:
			return attributeMap.namedItem("tests").nodeValue();
		case Errors:
			return attributeMap.namedItem("errors").nodeValue();
		case Disabled:
			return attributeMap.namedItem("disabled").nodeValue();
		case Timestamp:
			return attributeMap.namedItem("timestamp").nodeValue();
		default:
			return QVariant();
		}
		break;
	case Qt::DecorationRole:
		if (index.column() == 0)
		{
			if (attributeMap.namedItem("status").nodeValue().contains("notrun"))
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
		case Name:
			return Qt::AlignLeft;
		case TestNumber:
			return Qt::AlignCenter;
		case Failures:
			return Qt::AlignCenter;
		case Time:
			return Qt::AlignCenter;
		case Tests:
			return Qt::AlignCenter;
		case Errors:
			return Qt::AlignCenter;
		case Disabled:
			return Qt::AlignCenter;
		case Timestamp:
			return Qt::AlignLeft;
		default:
			return Qt::AlignLeft;
		}
	case FailureRole:
		return data(this->index(index.row(), Failures, index.parent()), Qt::DisplayRole);
	default:
		return QVariant();
	}

	return QVariant();
}

Qt::ItemFlags GTestModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	switch (index.column())
	{
	case Name:
	default:
		return QAbstractItemModel::flags(index);;
	}
}

QVariant GTestModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case Name:
			return tr("Name");
		case TestNumber:
			return tr("Test #");
		case Failures:
			return tr("Failures");
		case Time:
			return tr("Time (ms)");
		case Tests:
			return tr("Tests");
		case Errors:
			return tr("Errors");
		case Disabled:
			return tr("Disabled");
		case Timestamp:
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

	// don't show failure nodes in the test model. They'll go in a separate model.
	if (parentItem->node().toElement().firstChild().nodeName() == "failure")
		return 0;

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