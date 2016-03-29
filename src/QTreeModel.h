//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-runner
/// @BRIEF		A Qt abstract item model with an internal tree data structure
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

#ifndef QTreeModel_h__
#define QTreeModel_h__

//------------------------
//	INCLUDES
//------------------------

// Qt
#include <QAbstractItemModel>

// Std
#include <cassert>

// local
#include "tree.h"

//	----------------------------------------------------------------------------
//	CLASS		QTreeModel
//  ----------------------------------------------------------------------------
///	@brief		Abstract implementation of the QAbstractItemModel interface using
///				an STL-style tree as the internal data structure.
///	@details	
//  ----------------------------------------------------------------------------
template <class T>
class QTreeModel : public QAbstractItemModel
{

public:

	typedef typename Tree<T>::iterator			iterator;
	typedef typename Tree<T>::const_iterator	const_iterator;
	typedef typename Tree<T>::local_iterator	local_iterator;
	typedef typename Tree<T>::pointer			pointer;

public:

	//////////////////////////////////////////////////////////////////////////
	//		ABSTRACT INTERFACE
	//////////////////////////////////////////////////////////////////////////
	
	virtual int			columnCount(const QModelIndex &parent) const								= 0;
	virtual QVariant	data(const QModelIndex &index, int role = Qt::DisplayRole) const override	= 0;

public:

	//////////////////////////////////////////////////////////////////////////
	//		TREE MODEL INTERFACE
	//////////////////////////////////////////////////////////////////////////
	QTreeModel(QObject*parent = (QObject*)0) : QAbstractItemModel(parent)
	{
		// invisible root item
		tree.emplace_root();
		assert(tree.size() == 1);
	}

	/**
	 * @brief		convert QModelIndex to QTreeModel::iterator
	 * @details		Takes a QModelIndex to an element in the QTreeModel, and return the corresponding
	 *				iterator to the element in the underlying tree data structure.
	 * @param[in]	index QModelIndex to the referred item.
	 * @returns		iterator to the model item referred to by index.
	 */
	iterator indexToIterator(const QModelIndex& index) const
	{
		if (index.isValid())
		{
			return iterator(static_cast<pointer>(index.internalPointer()));
		}
		
		return tree.root();
	}

	/**
	* @brief		convert QTreeModel::iterator to QModelIndex
	* @details		Takes an iterator to the models underlying data structure and converts it to a
	*				QModelIndex.
	* @param[in]	item QTreeModel::iterator to the referred item.
	* @returns		index to the model item referred to by iterator.
	*/
	QModelIndex iteratorToIndex(const const_iterator& item, int column = 0) const
	{
		if (item == tree.root())
			return QModelIndex();
		else if (item != tree.end())
			return createIndex(static_cast<int>(tree.index_of(item)), column, item.internalPointer());

		return QModelIndex();
	}

	/**
	 * @brief		Inserts a row into the model.
	 * @details		Inserts a new row into the model by creating an object of the type contained in
	 *				the model in-place.
	 * @param[in]	parent iterator to the parent which the new object should be created as a child.
	 * @param[in]	args arguments to the constructor of the new 'T' object to be created. These may be
	 *				const T& or T&& parameters if move/copy construction is desired.
	 * @returns		iterator to the newly created child element.
	 */
	template <class... Args>
	iterator insertRow(const const_iterator& parent, Args... args)
	{
		if (parent != tree.end())
		{
			int position = static_cast<int>(tree.child_count(parent));
			beginInsertRows(iteratorToIndex(parent), position, position);
			auto returnItr = tree.emplace(parent, std::forward<Args>(args)...);
			endInsertRows();
			return returnItr.first;
		}

		return tree.end();
	}

	/**
	 * @brief		Inserts a row into the model.
	 * @details		This is an overloaded function using QModelIndex values instead of iterators.
	 * @see			insertRow(const iterator& parent, Args... args)
	 * @param[in]	parent index to the parent which the new object should be created as a child.
	 * @param[in]	args arguments to the constructor of the new 'T' object to be created. These may be
	 *				const T& or T&& parameters if move/copy construction is desired.
	 * @returns		index to the newly created child element.
	 */
	template <class... Args>
	QModelIndex insertRow(const QModelIndex& parent, Args... args)
	{
		return iteratorToIndex(insertRow(indexToIterator(parent), std::forward<Args>(args)...));
	}

	iterator removeRow(const_iterator row)
	{
		if(row != tree.end())
		{
			beginRemoveRows(iteratorToIndex(tree.parent(row)), (int)tree.index_of(row), (int)tree.index_of(row));
			auto returnItr = tree.erase(row);
			endRemoveRows();
			return returnItr;
		}

		return tree.end();

	}

	QModelIndex removeRow(int row, const QModelIndex &parent = QModelIndex())
	{
		return iteratorToIndex(removeRow(tree.child_at(indexToIterator(parent), row)));
	}

	/**
	 * @brief		Returns root element of model.
	 * @returns		iterator to the root element of the model, or end() if the model is empty.
	 */
	iterator root() const
	{
		return tree.begin();
	}

	/**
	* @brief		Returns first element of model.  
	* @ details		This is the same as root(), but added for clearer semantics in loops.
	* @returns		iterator to the root element of the model, or end() if the model is empty.
	*/
	iterator begin() const
	{
		return ++tree.begin();
	}

	/**
	* @brief		Returns end element of model.
	* @returns		iterator to the end element of the model.
	*/
	iterator end() const
	{
		return tree.end();
	}

	/**
	 * @brief		prints the contents of the internal tree to qDebug for debugging.
	 */
	void print() const
	{
		for (auto itr = tree.begin(); itr != tree.end() ; ++itr)
		{
			qDebug() << itr->name().c_str();
		}
	}

public:

	//////////////////////////////////////////////////////////////////////////
	//		INHERITED INTERFACE
	//////////////////////////////////////////////////////////////////////////


	virtual QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
	{
		if (!hasIndex(row, column, parent))
			return QModelIndex();

		iterator parentItr;

		if (!parent.isValid())
			parentItr = tree.begin();
		else
			parentItr = static_cast<pointer>(parent.internalPointer());

		try
		{
			iterator childItr(tree.child_at(parentItr, row));
			return createIndex(row, column, childItr.internalPointer());
		}
		catch (std::out_of_range)
		{
			return QModelIndex();
		}
	}

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	virtual QModelIndex	parent(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return QModelIndex();

		iterator childItr = static_cast<pointer>(index.internalPointer());
		iterator parentItr = tree.parent(childItr);

		if (parentItr == tree.root())
			return QModelIndex();

		return createIndex(static_cast<int>(tree.index_of(parentItr)), 0, parentItr.internalPointer());
	}

	virtual iterator parent(const iterator& item) const
	{
		return tree.parent(item);
	}

	virtual int	rowCount(const QModelIndex& parent = QModelIndex()) const override
	{		
		iterator parentItr;
		if (parent.column() > 0)
			return 0;

		if (!parent.isValid())
			parentItr = root();
		else
			parentItr = static_cast<pointer>(parent.internalPointer());
			
		return rowCount(parentItr);
	}

	virtual int rowCount(const iterator& item) const
	{
		return static_cast<int>(tree.child_count(item));
	}

	virtual QModelIndex sibling(int row, int column, const QModelIndex &index) const override
	{
		if (!index.isValid())
			return QModelIndex();

		iterator itemItr = static_cast<pointer>(index.internalPointer());
		iterator parentItr = tree.parent(itemItr);

		if (row < tree.child_count(parentItr))
			return createIndex(row, column, (tree.child_at(parentItr, row)).internalPointer());
		else
			return QModelIndex();
	}

	virtual iterator sibling(int index, const iterator& item) const
	{
		return tree.child_at(tree.parent(item), index);
	}

	virtual local_iterator sibling_begin(const iterator& item) const
	{
		return tree.begin_children(tree.parent(item));
	}

	virtual local_iterator sibling_end(const iterator& item) const
	{
		return tree.end_children(tree.parent(item));
	}

	virtual bool hasChildren(const QModelIndex &parent) const override
	{
		iterator parentItr;

		if (!parent.isValid())
			parentItr = root();
		else
			parentItr = static_cast<pointer>(parent.internalPointer());

		return hasChildren(parentItr);
	}

	virtual bool hasChildren(const iterator& parent) const
	{
		return tree.child_count(parent) > 0;
	}

	virtual QList<QModelIndex> children(const QModelIndex& index) const
	{
		QList<QModelIndex> list;
		for (int i = 0; i < rowCount(index); i++)
		{
			list << index.child(i, 0);
		}
		return list;
	}

private:

	// disallowed because child insertion on the Tree can't be done before Row, as called for in the
	// QAbstractItemModel interface documentation: http://qt-project.org/doc/qt-4.8/qabstractitemmodel.html#insertRows
	virtual bool insertRows(int row, int count, const QModelIndex &parent) override
	{
		return false;
	}

	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
	{
		return false;
	}

protected:

	Tree<T>			tree;																			///< Internal storage of model data

};

#endif // QTreeModel_h__