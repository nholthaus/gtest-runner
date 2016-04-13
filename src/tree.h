//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-runner
/// @BRIEF		An STL-style generic tree container
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

#ifndef tree_h__
#define tree_h__

//------------------------
//	PRAGMAS
//------------------------
#pragma warning(push)	
#pragma warning(disable:4520)	// this is safe as there really aren't multiple default constructors.... the non-template version without args will always be preferred. If you don't trust me, it was done in BOOST: https://svn.boost.org/trac/boost/ticket/10844.

//------------------------
//	INCLUDES
//------------------------
#include <memory>
#include <vector>
#include <iterator>
#include <list>
#include <tuple>
#include <utility>
#include <iostream>

//------------------------
//	FORWARD DECLARATIONS
//------------------------
template <typename T> class PreOrderIterator;
template <typename T> class ConstPreOrderIterator;
template <typename T> class ChildIterator;
template <typename T> class ConstChildIterator;
template <typename T> class TreeNode;

//	----------------------------------------------------------------------------
//	CLASS		Tree
//  ----------------------------------------------------------------------------
///	@brief		an STL style unbalanced k-ary tree.
///	@details	This Tree is unbounded in k (a node can have an arbitrary amount
///				of children), depth, and is unbalanced. Because of this, it is
///				suitable for storing arbitrarily hierarchical data (file structure,
///				family tree, etc) but is probably inefficient for algorithms other
///				than pre-order traversal, specifically searching.\n\n
///				<b> Space Requirement:</b> O(n) for a Tree with n total nodes.\n\n
///				<b><i> CONTAINER PROPERTIES</i></b>\n
///				<i><b> Sequence </b></i> - Elements in sequence containers are ordered in a strict 
///				hierarchical sequence. Individual elements are accessed by their position in this 
///				sequence relative to their parent.\n
///				<i><b> Doubly-linked Nodes </b></i> - Each node keeps information on how to locate 
///				its parent and children, allowing constant time insert and erase operations 
///				(for childless nodes) before or after a specific element, but no direct random access.\n
///				<i><b> Sub-trees </b></i> - looping over an iterator that does not point to the root
///				node has the effect of traversing <i>only</i> the sub-tree of that node.\n
//  ----------------------------------------------------------------------------
template<typename T>
class Tree
{
public:

	using iterator = PreOrderIterator<T>;															///< A forward iterator to value_type.
	using const_iterator = ConstPreOrderIterator<T>;												///< A forward iterator to const value_type.
	using local_iterator = ChildIterator<T>;														///< A forward iterator to just the children (value_type) of a given Treenode.
	using const_local_iterator = ConstChildIterator<T>;												///< A forward iterator to just the children (const value_type) of a given Treenode.					
	using difference_type = ptrdiff_t;																///< A signed integral type, identical to:iterator_traits<iterator>::difference_type
	using size_type = size_t;																		///< An unsigned integral type that can represent any non-negative value of difference_type.
	using value_type = T;																			///< The template parameter (T), representing the values stored in the tree.
	using pointer = TreeNode<const T>*;																///< For the default allocator: value_type*.
	using reference = TreeNode<const T>&;															///< For the default allocator: value_type&.

public:

	//////////////////////////////////////////////////////////////////////////
	//		CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @defgroup	constructor Constructor
	 * @brief		Construct Tree.
	 * @{
	 */

	/**
	 * @brief		Empty container constructor (default constructor).
	 * @details		Constructs a Tree container with no elements.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> N/A.\n
	 *				<b> Data Races:</b> N/A.\n
	 *				<b> Exception Safety:</b> <i>No-Throw guarantee</i>: This function never throws.\n
	 */
	Tree() : m_rootNode(nullptr) {};

	/**
	 * @brief		Copy constructor.
	 * @details		Creates a new Tree which is a deep copy of Tree <i>other</i>.\n\n
	 *				<b> Complexity:</b> Linear in size of <i>other</i>.\n
	 *				<b> Iterator Validity:</b> No Changes.\n
	 *				<b> Data Races:</b> All copied elements are accessed..\n
	 *				<b> Exception Safety:</b> <i>Strong guarantee</i>: no effects in case an exception is 
	 *				thrown.\n
	 * @param[in]	other Tree container instance to be copied.
	 */
	Tree(const Tree<T>& other)	
	{
		if (!other.empty())
		{
			this->m_rootNode = std::make_unique<TreeNode<const T>>(*other.m_rootNode);
		}
	}

	/**
	 * @brief		Move constructor.
	 * @details		Constructs a Tree container by moving (and taking ownership) of the contents of
	 *				<i>other</i>. After the move, <i>other</i> is left in an unspecified, but valid,
	 *				state.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> All iterators, pointers, and references to 
	 *				<i>other</i> are invalidated.\n
	 *				<b> Data Races:</b> <i>other</i> is modified.\n
	 *				<b> Exception Safety:</b> <i>Strong guarantee</i>: no effects in case an exception is 
	 *				thrown.\n
	 * @param[in]	other Tree container instance to be moved.
	 */
	Tree(Tree<T>&& other) : m_rootNode(std::move(other.m_rootNode)) {};

	/** @} */ // END OF CONSTRUCTOR

	//////////////////////////////////////////////////////////////////////////
	//		DESTRUCTOR
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @defgroup	destructor Destructor
	 * @brief		Tree destructor.
	 * @{
	 */

	/**
	 * @brief		Destructor.
	 * @details		Releases (and destroys) the Tree and all resources held by the Tree instance.\n\n
	 *				<b> Complexity:</b> Linear in size.\n
	 *				<b> Iterator Validity:</b> All iterators, pointers and references are invalidated..\n
	 *				<b> Data Races:</b> The container and all its elements are modified.\n
	 *				<b> Exception Safety:</b> <i>No-Throw guarantee</i>: This function never throws.\n
	 */
	~Tree() = default;

	/** @} */ // END OF DESTRUCTOR

	//////////////////////////////////////////////////////////////////////////
	//		ASSIGNMENT
	//////////////////////////////////////////////////////////////////////////

	/**
	* @defgroup		operator Operator=
	* @brief		Copy/Move Tree content.
	* @{
	*/

	/**
	* @brief		Copy container content.
	* @details		Copies the contents of <i>other</i> into <i>this</i>.\n\n
	*				<b> Complexity:</b> Linear in size of <i>other</i>.\n
	*				<b> Iterator Validity:</b> No Changes.\n
	*				<b> Data Races:</b> All copied elements are accessed..\n
	*				<b> Exception Safety:</b> <i>Strong guarantee</i>: no effects in case an exception is
	*				thrown.\n
	* @param[in]	other Tree container instance to be copied.
	* @returns		*this.
	*/
	Tree& operator=(const Tree& other)
	{
		if (!other.empty())
		{
			this->m_rootNode = std::make_unique<TreeNode<const T>>(*other.m_rootNode);
		}
		return *this;
	}

	/**
	* @brief		Move container content.
	* @details		Moves the contents of <i>other</i> into <i>this</i>. After the move, <i>other</i> 
	*				is left in an unspecified, but valid, state.\n\n
	*				<b> Complexity:</b> Constant.\n
	*				<b> Iterator Validity:</b> All iterators, pointers, and references to
	*				<i>other</i> are invalidated.\n
	*				<b> Data Races:</b> <i>other</i> is modified.\n
	*				<b> Exception Safety:</b> <i>Strong guarantee</i>: no effects in case an exception is
	*				thrown.\n
	* @param[in]	other Tree container instance to be moved.
	* @returns		*this.
	*/
	Tree& operator=(Tree&& other)
	{
		this->m_rootNode = std::move(other.m_rootNode);
		return *this;
	}

	/** @} */ // END OF OPERATOR=

	//////////////////////////////////////////////////////////////////////////
	//		ITERATORS
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @defgroup	iterators Iterators
	 * @brief		Methods to construct iterators to the Tree container.
	 * @{
	 */

	/**
	 * @brief		Return iterator to beginning.
	 * @details		Because the Tree is traversed in pre-order, begin will always point to the
	 *				root node of the tree, or nullptr if no root has been defined. If the container 
	 *				is empty, the returned iterator value shall not be dereferenced.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a set is safe.\n
	 *				<b> Exception Safety:</b> <i>No-throw guarantee</i>: this member function never throws exceptions.
	 *				The copy construction or assignment of the returned iterator is also guaranteed to never throw.
	 * @returns		A mutable iterator referring to the first element in the Tree container.\n
	 */
	iterator begin() const /*noexcept*/
	{
		return iterator(this->m_rootNode.get());
	}

	/**
	 * @brief		Returns an iterator referring to the past-the-end element in the Tree container.
	 * @details		The past-the-end element is the theoretical element that would follow the last
	 *				element in the set container. It does not point to any element, and thus shall
	 *				not be dereferenced. Because the ranges used by functions of the standard library
	 *				do not include the element pointed by their closing iterator, this function is
	 *				often used in combination with Tree::begin to specify a range including all the
	 *				elements in the container. If the container is empty, this function returns the
	 *				same as Tree::begin.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a set is safe.\n
	 *				<b> Exception Safety:</b> <i>No-throw guarantee</i>: this member function never throws exceptions.
	 *				The copy construction or assignment of the returned iterator is also guaranteed to never throw.
	 * @returns		an iterator referring to the past-the-end element in the Tree container. If three
	 *				tree is const-qualified, the function returns a const_iterator.\n
	 */
	iterator end() const /*noexcept*/
	{
		return iterator(nullptr);
	}

	/**
	 * @brief		Return local_iterator to beginning of children.
	 * @details		Returns a local iterator to the beginning of the children of the given parent node. 
	 *				The children are not ordered.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	parent iterator the the parent node for which an iterator to the beginning of its
	 *				children will be returned.
	 * @returns		local iterator to the children of the given <i>parent</i> node.
	 */
	local_iterator begin_children(const const_iterator& parent) const /*noexcept*/
	{
		return local_iterator(parent.m_pointer);
	}

	/**
	 * @brief		Returns end of children.
	 * @details		Returns a mutable local_iterator to the end of the children of the given parent node.
	 *				The children are not ordered.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No Changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	parent iterator the the parent node for which an iterator to the end of its
	 *				children will be returned.
	 * @returns		local_iterator to the end of the children of the given <i>parent</i> node.
	 */
	local_iterator end_children(const const_iterator& parent) const /*noexcept*/
	{
		return local_iterator(nullptr);
	}

	/**
	 * @brief		Return a const_iterator to the beginning of the Tree.
	 * @details		This function is equivalant to begin() with the caveat that the returned iterator
	 *				points to immutable objects.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a set is safe.\n
	 * @see			begin().\n
	 *				<b> Exception Safety:</b> <i>No-throw guarantee</i>: this member function never throws exceptions.\n
	 * @returns		a const_iterator to the beginning of the sequence.\n
	 */
	const_iterator cbegin() const /*noexcept*/
	{
		return const_iterator(this->m_rootNode.get());
	}

	/**
	* @brief		Returns a const_iterator referring to the past-the-end element in the Tree container.
	* @details		This function is equivalant to end() with the caveat that the returned iterator
	*				points to immutable objects.\n\n
	*				<b> Complexity:</b> Constant.\n
	*				<b> Iterator Validity:</b> No changes.\n
	*				<b> Data Races:</b> The container is accessed (neither the const nor the
	*				non-const versions modify the container). Concurrently accessing the elements
	*				of a set is safe.\n
	*				<b> Exception Safety:</b> <i>No-throw guarantee</i>: this member function never throws exceptions.\n
	* @see			end().
	*				The copy construction or assignment of the returned iterator is also guaranteed to never throw.
	* @returns		a const_iterator referring to the past-the-end element in the Tree container.
	*/
	const_iterator cend() const /*noexcept*/
	{
		return const_iterator(nullptr);
	}

	/**
	* @brief		Return const_local_iterator to beginning of children.
	* @details		Returns a local iterator to the beginning of the children of the given parent node.
	*				The children are not ordered.\n\n
	*				<b> Complexity:</b> Constant.\n
	*				<b> Iterator Validity:</b> No changes.\n
	*				<b> Data Races:</b> The container is accessed (neither the const nor the
	*				non-const versions modify the container). Concurrently accessing the elements
	*				of a Tree is safe.\n
	*				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	* @param[in]	parent iterator the the parent node for which an iterator to the beginning of its
	*				children will be returned.
	* @returns		local iterator to the children of the given <i>parent</i> node.
	*/
	const_local_iterator cbegin_children(const const_iterator& parent) const
	{
		return const_local_iterator(parent.m_pointer);
	}

	/**
	 * @brief		Returns end of children.
	 * @details		Returns a const_local_iterator to the end of the children of the given parent node.
	 *				The children are not ordered.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No Changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	parent iterator the the parent node for which an iterator to the end of its
	 *				children will be returned.
	 * @returns		const_local_iterator to the end of the children of the given <i>parent</i> node.
	 */
	const_local_iterator cend_children(const const_iterator& parent) const /*noexcept*/
	{
		return  const_local_iterator(nullptr);
	}

	/**
	 * @brief		Returns iterator to parent node.
	 * @details		The parent of the root node is equal to the value return by end(), which should
	 *				not be dereferenced.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the 
	 *				non-const versions modify the container). Concurrently accessing the elements 
	 *				of a Tree is safe..\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	node iterator to the node of which the parent iterator will be returned.\n
	 * @returns		iterator to the parent node of the given <i>node</i>.\n 
	 */
	iterator parent(const const_iterator& node) const /*noexcept*/
	{
		if (node.m_pointer)
		{
			return iterator(node.m_pointer->m_parent);
		}

		return nullptr;
	}

	/**
	* @brief		Returns const_iterator to parent node.
	* @details		The parent of the root node is equal to the value return by end(), which should
	*				not be dereferenced.\n\n
	*				<b> Complexity:</b> Constant.\n
	*				<b> Iterator Validity:</b> No changes.\n
	*				<b> Data Races:</b> The container is accessed (neither the const nor the
	*				non-const versions modify the container). Concurrently accessing the elements
	*				of a Tree is safe..\n
	*				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	* @param[in]	node iterator to the node of which the parent iterator will be returned.\n
	* @returns		iterator to the parent node of the given <i>node</i>.\n
	*/
	const_iterator cparent(const const_iterator& node) const /*noexcept*/
	{
		if (node.m_pointer)
		{
			return const_iterator(node.m_pointer->m_parent);
		}

		return nullptr;
	}

	/**
	 * @brief		Returns an iterator to the root node.
	 * @details		This is equivalent to begin(), and is only included because there may be cases
	 *				where a root() function semantically makes more sense than calling begin().\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a set is safe.\n
	 *				<b> Exception Safety:</b> <i>No-throw guarantee</i>: this member function never throws exceptions.
	 *				The copy construction or assignment of the returned iterator is also guaranteed to never throw.
	 * @returns		mutable iterator to the root node of the Tree container.
	 */
	iterator root() const /*noexcept*/
	{
		return begin();
	}

	/** @} */ // END OF ITERATORS

	//////////////////////////////////////////////////////////////////////////
	//		CAPACITY
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @defgroup	capacity Capacity
	 * @brief		Methods to query the size of the Tree container.
	 * @{
	 */

	/**
	 * @brief		Returns a value indicating whether or not the Tree is empty.
	 * @details		<b> Complexity:</b> Constant time.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed. Concurrently accessing the
	 *				elements of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @returns		true if the Tree contains no data nodes, false otherwise.\n
	 */
	bool empty() const /*noexcept*/
	{
		return (this->m_rootNode == nullptr);
	}

	/**
	 * @brief		Return container size.
	 * @details		Returns the number of elements in the Tree container.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed. Concurrently accessing the
	 *				elements of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @returns		The number of elements in the container. Member type size_type is an unsigned 
	 *				integral type.\n
	 */
	size_type size() const /*noexcept*/
	{
		if (this->m_rootNode)
		{
			return this->m_rootNode->m_count;
		}
		else
		{
			return 0;
		}		
	}

	/**
	 * @brief		Return the maximum size.
	 * @details		This is the maximum potential size the container can reach due to known system 
	 *				or library implementation limitations, but the container is by no means guaranteed 
	 *				to be able to reach that size: it can still fail to allocate storage at any point 
	 *				before that size is reached.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed. Concurrently accessing the
	 *				elements of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @returns		Returns the maximum number of elements that the Tree container could potentially hold.\n
	 */
	size_type max_size() const /*noexcept*/
	{
		return std::numeric_limits<size_type>::max() / sizeof(TreeNode<T>);
	}

	/** @} */ // END OF CAPACITY

	//////////////////////////////////////////////////////////////////////////
	//		MODIFIERS
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @defgroup	modifiers Modifiers
	 * @brief		Methods to change the contents of the Tree container.
	 * @{
	 */

	/**
	 * @brief		Insert Element.
	 * @details		Copies the given <i>value</i> into the Tree as a child of <i>parent</i>. This
	 *				effectively increases the Tree size by one. Internally, the children are not
	 *				kept in any particular order.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is modified. Concurrently accessing existing
	 *				elements is safe, although iterating ranges in the container is not..\n
	 *				<b> Exception Safety:</b> There are no changes in the container in case of exception (strong guarantee).\n
	 * @see			emplace(), emplace_root().\n	
	 * @param[in]	parent position where <i>value</i> will be inserted as a child.
	 * @param[in]	value element to be inserted as a child of <i>parent</i>.
	 * @returns		iterator to the newly inserted element.
	 */
	iterator insert(const const_iterator& parent, const value_type& value) 
	{
		if (parent.m_pointer)
		{
			parent.m_pointer->m_children.emplace_back(TreeNode<T>::make_unique(parent.m_pointer, value));
			return iterator(parent.m_pointer->m_children.back().get());
		}
		else
		{
			return nullptr;
		}
	}

	iterator insert(const const_iterator& parent, size_t index, const value_type& value)
	{
		if (parent.m_pointer)
		{
			auto itr = parent.m_pointer->m_children.insert(std::next(parent.m_pointer->m_children.begin(), index),TreeNode<T>::make_unique(parent.m_pointer, value));
			return iterator(std::next(parent.m_pointer->m_children.begin(), index)->get());
		}
		else
		{
			return nullptr;
		}
	}

	/**
	* @brief		Insert Element.
	* @details		Moves the given <i>value</i> into the Tree as a child of <i>parent</i>. This
	*				effectively increases the Tree size by one. Internally, the children are not
	*				kept in any particular order.\n\n
	*				<b> Complexity:</b> Constant.\n
	*				<b> Iterator Validity:</b> No changes.\n
	*				<b> Data Races:</b> The container is modified. Concurrently accessing existing
	*				elements is safe, although iterating ranges in the container is not..\n
	*				<b> Exception Safety:</b> There are no changes in the container in case of exception (strong guarantee).\n
	* @see			emplace(), emplace_root().\n
	* @param[in]	parent position where <i>value</i> will be inserted as a child.
	* @param[in]	value element to be inserted as a child of <i>parent</i>.
	* @returns		iterator to the newly inserted element.
	*/
	iterator insert(const const_iterator& parent, value_type&& value)
	{
		if (parent.m_pointer)
		{
			parent.m_pointer->m_children.emplace_back(TreeNode<T>::make_unique(parent.m_pointer, std::move(value)));
			return iterator(parent.m_pointer->m_children.back().get());
		}

		return nullptr;
	}

	/**
	 * @brief		Erase elements.
	 * @details		Erases the sub-tree formed with <i>position</i> as the root node. This effectively
	 *				reduces the container size by Tree::count(position) elements, which are
	 *				destroyed.\n\n
	 *				<b> Complexity:</b> Linear with the size of the sub-tree being erased (destructions).\n
	 *				<b> Iterator Validity:</b> Iterators, pointers and references referring to 
	 *				elements removed by the function (i.e. descendants of <i>positions</i>) are 
	 *				invalidated. All other iterators, pointers and references keep their validity.\n
	 *				<b> Data Races:</b> The container is modified. The elements removed are modified. 
	 *				Concurrently accessing other elements is safe, although iterating ranges in the
	 *				container is not.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	position iterator to the root position of the sub-tree to be removed from the tree.
	 * @returns		iterator to the next valid element in the Tree after the removed elements.
	 */
	iterator erase(const const_iterator& position)
	{
		if (position.m_pointer == this->m_rootNode.get())
		{
			clear();
			return nullptr;
		}
		else if (position.m_pointer == nullptr)
		{
			return begin();
		}
		else
		{
			// advance a return iterator to the next thing after what we are erasing. There's really
			// no way to do this after the erase as there isn't enough state information left.
			iterator retVal = begin();
			std::advance(retVal, position.m_pointer->m_count + std::distance<const_iterator>(retVal, position));

			TreeNode<const T>* parent = position.m_pointer->m_parent;
			auto& siblings = parent->m_children;
			auto end = siblings.end();
			for (auto itr = siblings.begin(); itr != end; ++itr)
			{
				if (itr->get() == position.m_pointer)
				{
					siblings.erase(itr);
					return retVal;
				}
			}
			return nullptr;
		}
	}

	/**
	 * @brief		Erase elements
	 * @details		Erases the sub-tree formed by the element with the given <i>value</i> as its root.
	 *				This effectively reduces the container size by Tree::count(Tree::find(value)) 	 
	 *				elements, which are destroyed.\n\n
	 *				<b> Complexity:</b> Linear with the size of the sub-tree being erased (destructions).\n
	 *				<b> Iterator Validity:</b> Iterators, pointers and references referring to
	 *				elements removed by the function (i.e. descendants of <i>positions</i>) are
	 *				invalidated. All other iterators, pointers and references keep their validity.\n
	 *				<b> Data Races:</b> The container is modified. The elements removed are modified.
	 *				Concurrently accessing other elements is safe, although iterating ranges in the
	 *				container is not.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	value element to be erased from the Tree.
	 * @returns		iterator to the next valid element in the Tree after the removed elements.
	 */
	iterator erase(const value_type& value)
	{
		return erase(find(value));
	}

	/**
	 * @brief		Construct and insert root element.
	 * @details		Inserts a new element in the tree. This new element is constructed in place
	 *				using args as the arguments for its construction. The insertion only takes place
	 *				if a root node has not previously been defined. If inserted,
	 *				this effectively increases the container size by one. Child items cannot be
	 *				emplaced/inserted into the Tree until a root node has been defined. A similar
	 *				member function exists, insert_root(), which copies or moves a root node into the
	 *				tree.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is modified. Concurrently accessing existing
	 *				elements is safe, although iterating ranges in the container is not.\n
	 *				<b> Exception Safety:</b> <i>Strong guarantee</i>: if an exception is thrown, there are
	 *				no changes in the container.\n
	 * @param[in]	args Arguments forwarded to construct the new element.
	 * @returns		If the function successfully inserts the element, the function returns a pair of
	 *				an iterator to the newly inserted root element and a value of true.\n
	 *				Otherwise, it returns an iterator to the existing root element and a value of false.\n
	 *				Member type iterator is a forward iterator type that points to an element. Pair
	 *				is an STL container class template declared in <utility>.\n
	 */
	template<class... Args>
	std::pair<iterator, bool> emplace_root(Args&&... args)
	{
		if (!this->m_rootNode)
		{
			// can't use std::make_unique because the TreeNode constructor is private. Use TreeNode 
			// make_unique factory function instead.
			this->m_rootNode = TreeNode<T>::make_unique(nullptr, std::forward<Args>(args)...);
			return std::pair<iterator, bool>(
				std::piecewise_construct,
				std::forward_as_tuple(this->m_rootNode.get()),
				std::forward_as_tuple(true)
				);
		}
		else
		{
			// already have a root node, this operation is invalid.
			return std::pair<iterator, bool>(
				std::piecewise_construct,
				std::forward_as_tuple(this->m_rootNode.get()),
				std::forward_as_tuple(false)
				);
		}
	}

	/**
	 * @brief		Construct and insert an element as a child of parent.
	 * @details		Inserts a new element in the tree. This new element is constructed in place
	 *				using args as the arguments for its construction. The insertion only takes place
	 *				if a root node has been defined and the parent iterator is valid. If inserted,
	 *				this effectively increases the container size by one. Internally, Tree containers
	 *				do not order the children of a node. A similar member function exists, insert,
	 *				which either copies or moves existing objects into the container.\n\n
	 *				<b> Complexity:</b> Log(N) in size on average, O(N) in the worst case.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is modified. Concurrently accessing existing
	 *				elements is safe, although iterating ranges in the container is not.\n
	 *				<b> Exception Safety:</b> <i>Strong guarantee</i>: if an exception is thrown, there are
	 *				no changes in the container.\n
	 * @param[in]	parent iterator to the element of which the new element will be inserted as a child.
	 * @param[in]	args Arguments forwarded to construct the new element.
	 * @returns		If the function successfully inserts the element, the function returns a pair of
	 *				an iterator to the newly inserted element and a value of true.\n
	 *				Otherwise, it returns an iterator to the end of the container and a value of false.\n
	 *				Member type iterator is a forward iterator type that points to an element. Pair
	 *				is an STL container class template declared in <utility>.\n
	 */
	template<class... Args>
	std::pair<iterator, bool> emplace(const const_iterator& parent, Args&&... args)
	{
		if (this->m_rootNode && parent)
		{
			parent.m_pointer->m_children.emplace_back(TreeNode<T>::make_unique(parent.m_pointer, std::forward<Args>(args)...));
			return std::pair<iterator, bool>(
				std::piecewise_construct,
				std::forward_as_tuple(parent.m_pointer->m_children.back().get()),
				std::forward_as_tuple(false)
				);		
		}
		else
		{
			return std::pair<iterator, bool>(
				std::piecewise_construct,
				std::forward_as_tuple(end().m_pointer),
				std::forward_as_tuple(false)
				);
		}
	}

	/**
	 * @brief		Removes all elements from the Tree container (which are destroyed), leaving the container with a size of 0.
	 * @details
	 *				<b> Complexity:</b> Linear in size (destructions).\n
	 *				<b> Iterator Validity:</b> All iterators, pointers and references related to this container are invalidated.\n
	 *				<b> Data Races:</b> The container is modified.\n
	 *				All contained elements are modified.\n
	 *				<b> Exception Safety: <i>No-throw guarantee</i>: this member function never throws exceptions.</b>\n
	 */
	void clear() /*noexcept*/
	{
		this->m_rootNode.reset(nullptr);
	}

	/** @} */ // END OF MODIFIERS

	//////////////////////////////////////////////////////////////////////////
	//		OPERATIONS
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @defgroup	operations Operations
	 * @brief		Operations that can be preformed by the Tree container.
	 * @{
	 */

	/**
	 * @brief		Returns iterator to value.
	 * @details		Performs a depth first search, and returns an iterator to the element whose value
	 *				matches the <i>value</i> parameter, or.\n\n
	 *				<b> Complexity:</b> In the worst case, linear in size of the Tree. If a starting
	 *				position is given, linear in the size of the sub-tree whose root is equal to 
	 *				the position. \n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	value value to be searched for.
	 * @param[in]	position starting iterator position for the search. If no position is specified,
	 *				searching will commence with the root node. If a position is specified, only
	 *				the sub-tree whose root node is pointed to be the iterator will be searched.\n
	 * @returns		iterator to the element in the Tree whose value matches <i>value</i>. If no
	 *				such element is found, Tree::end() is returned.\n
	 * @{
	 
	 */
	iterator find(const value_type& value) const /*noexcept*/
	{
		return find(value, begin());
	}

	iterator find(const value_type& value, iterator position) const /*noexcept*/
	{
		auto treeEnd = end();

		if (position == treeEnd || !this->m_rootNode)
		{
			return std::move(treeEnd);
		}

		for (position; position != treeEnd; ++position)
		{
			if (*position == value)
			{
				return std::move(position);
			}
		}

		return std::move(treeEnd);
	}

	const_iterator find(const value_type& value, const_iterator position) const /*noexcept*/
	{
		auto treeEnd = cend();

		if (position == treeEnd || !this->m_rootNode)
		{
			return std::move(treeEnd);
		}

		for (position; position != treeEnd; ++position)
		{
			if (*position == value)
			{
				return std::move(position);
			}
		}

		return std::move(treeEnd);
	}
	// @}

	/**
	 * @brief		Swap content.
	 * @details		Exchanges the content of the container by the content of <i>rhs</i> (right-hand side), 
	 *				which is another Tree of the same type. Sizes may differ. After the call to this 
	 *				member function, the elements in this container are those which were in <i>rhs</i> 
	 *				before the call, and the elements of <i>rhs</i> are those which were in this. 
	 *				All iterators, references and pointers remain valid for the swapped objects.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> All iterators, pointers and references referring to 
	 *				elements in both containers remain valid, but now are referring to elements in 
	 *				the other container, and iterate in it.\n
	 *				<b> Data Races:</b> Both the container and <i>rhs</i> are modified. No contained 
	 *				elements are accessed by the call (although see iterator validity above).\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	rhs right-hand side of the swap operation, a Tree which which will be swapped 
	 *				with <i>this</i> Tree.
	 */
	void swap(Tree& rhs)
	{
		std::swap(this->m_rootNode, rhs.m_rootNode);
	}

	/**
	 * @brief		Returns child at index.
	 * @details		Returns the child of the given <i>parent</i> at the given <i>index</i>.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No Changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the 
	 *				non-const versions modify the container). The reference returned can be used to 
	 *				access or modify elements. Concurrently accessing or modifying different elements 
	 *				is safe.\n
	 *				<b> Exception Safety:</b> <i>Strong guarantee</i>: if an exception is thrown, there are 
	 *				no changes in the container. It throws out_of_range if <i>index</i> is out of 
	 *				bounds.\n
	 * @param[in]	parent parent node whose child at the given <i>index</i> will be returned.
	 * @param[in]	index index value of the child to be returned. Must be within the bounds of the
	 *				the parents number of children or std::out_of_bounds will be thrown.
	 * @returns		iterator to the child of the given <i>parent</i> at the given <i>index</i>.
	 * @{
	 */
	iterator child_at(const iterator& parent, size_type index) const
	{
		if (parent.m_pointer)
		{
			return (parent.m_pointer->m_children.at(index)).get();
		}

		return nullptr;
	}

	const_iterator child_at(const const_iterator& parent, size_type index) const
	{
		if (parent.m_pointer)
		{
			return (parent.m_pointer->m_children.at(index)).get();
		}

		return nullptr;
	}
	// @}

	/**
	 * @brief		Return count of parent sub-tree.
	 * @details		Returns the size of the sub-tree formed if parent were the tree's root node. This
	 *				value is inclusive of the parent node itself. It is the amount the size of the tree
	 *				would be reduced by if this node were moved or erased.\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No Changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	parent parent node of which to take the count.
	 * @returns		Size of the sub-tree that would be formed with parent as the root.
	 */
	size_type count(const const_iterator& parent) const /*noexcept*/
	{
		if (parent.m_pointer)
		{
			return parent.m_pointer->m_count;
		}

		return 0;
	}

	/**
	 * @brief		Returns child count of parent.
	 * @details		Returns the number of children contained by the given <i>parent</i>. This value
	 *				only represents the direct children of <i>parent</i>, NOT all of its ancestors.
	 *				To find the total number of ancestors (inclusive), see count().\n\n
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No Changes.\n
	 *				<b> Data Races:</b> The container is accessed (neither the const nor the
	 *				non-const versions modify the container). Concurrently accessing the elements
	 *				of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @see			count().
	 * @param[in]	parent parent node for which the child count is requested.
	 * @returns		size_type number of children of the given <i>parent</i> node.
	 */
	size_type child_count(const const_iterator& parent) const /*noexcept*/
	{
		if (parent.m_pointer)
		{
			return parent.m_pointer->m_children.size();
		}

		return 0;
	}

	/**
	 * @brief		Returns index of <i>node</i>.
	 * @details		Returns the index of the <i>node</i> in its parent's list of children, by searching
	 *				forward from the beginning of the parent's children list. Checking for the error
	 *				return of -1 should not be required in most cases, as only an invalid iterator as
	 *				input could cause the search to fail.\n\n
	 *				This function is not required for iteration, and is provided mainly for compatibility
	 *				with the `QAbstractItemModel` interface, which requires row indices for each element.\n\n
	 *				<b> Complexity:</b> Up to linear with the number of siblings of <i>node</i>.\n
	 *				<b> Iterator Validity:</b> No Changes.\n
	 *				<b> Data Races:</b> The container is accessed. Concurrently accessing the elements
	 *				of a Tree is safe.\n
	 *				<b> Exception Safety:</b> This function never throws exceptions (no-throw guarantee).\n
	 * @param[in]	node element for which to return its index.
	 * @returns		Index of the node in the parent's list of children, or -1 in exceptional error cases.
	 */
	size_type index_of(const const_iterator& node) const /*noexcept*/
	{
		if (node.m_pointer)
		{
			if (node.m_pointer->m_parent)
			{
				auto end = node.m_pointer->m_parent->m_children.end();
				for (auto itr = node.m_pointer->m_parent->m_children.begin(); itr != end; ++itr)
				{
					if ((**itr).m_value == *node)
					{
						return std::distance(node.m_pointer->m_parent->m_children.begin(), itr);
					}
				}
				return -1;
			}
		}

		return 0;

	}

	/** @} */ // END OF OPERATIONS

private:

	std::unique_ptr<TreeNode<const T>>	m_rootNode;

};

//	----------------------------------------------------------------------------
//	CLASS		TreeNode
//  ----------------------------------------------------------------------------
///	@brief		
///	@details	This class is not copyable. The TreeNode class is an implementation
///				detail of the tree, and should not be created or accessed directly
///				by users of the Tree class.
//  ----------------------------------------------------------------------------
template<typename T>
class TreeNode
{
	template <class C> friend class Tree;
	template <class C> friend class PreOrderIterator;
	template <class C> friend class ConstPreOrderIterator;
	template <class C> friend class ChildIterator;
	template <class C> friend class ConstChildIterator;

public:

	TreeNode(const TreeNode& other, TreeNode<const T>* parent = nullptr ) :
		m_count(0),
		m_value(other.m_value),
		m_parent(parent),
		m_children({})
	{
		auto end = other.m_children.cend();
		for (auto itr = other.m_children.cbegin(); itr != end; ++itr)
		{
			this->m_children.push_back(std::make_unique<TreeNode<const T>>(**itr, this));
		}
		incrementCount();

#ifdef tree_h__TEST_TREE
		std::cout << "    TreeNode::copy constructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	TreeNode(TreeNode&& other) :
		m_count(std::move(other.m_count)),
		m_value(std::move(other.m_value)),
		m_parent(std::move(other.m_parent)),
		m_children(std::move(other.m_children))
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    TreeNode::move constructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	~TreeNode()
	{
		decrementCount();

#ifdef tree_h__TEST_TREE
		std::cout << "    TreeNode::destructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

protected:

	TreeNode() = delete;

	TreeNode& operator=(const TreeNode& other)		
	{
		this->m_value = other.m_value;

		auto end = other.m_children.cend();
		for (auto itr = other.m_children.cbegin(); itr != end; ++itr)
		{
			this->m_children.push_back(std::make_unique<TreeNode<const T>>(**itr));
			this->m_children.back()->m_parent = this;
		}
#ifdef tree_h__TEST_TREE
		std::cout << "    TreeNode::copy assignment" << std::endl;
#endif // tree_h__TEST_TREE
	}

	TreeNode& operator=(TreeNode&& other)
	{
		this->m_count(std::move(other.m_count));
		this->m_value(std::move(other.m_value));
		this->m_parent(std::move(other.m_parent));
		this->m_children(std::move(other.m_children));
#ifdef tree_h__TEST_TREE
		std::cout << "    TreeNode::move assignment" << std::endl;
#endif // tree_h__TEST_TREE
	}

	/// factory for creating unique pointers to tree nodes. This is required because of the private
	/// constructor (unique_ptr can't be friended). 
	/// See http://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
	/// for details on why this works.
	template<class... Args>
	static std::unique_ptr<TreeNode<const T>> make_unique(TreeNode<const T>* parent, Args&&... args)
	{
		// create a local derived class with a public constructor which make_unique can use. This won't
		// break the encapsulation as only this function has access to the derived class.
		struct make_unique_enabler : public TreeNode<const T>
		{
			make_unique_enabler(TreeNode<const T>* parent, Args&&... args) :
			TreeNode<const T>(parent, std::forward<Args>(args)...) {}
		};
		return std::make_unique<make_unique_enabler>(parent, std::forward<Args>(args)...);
	}

	/// The constructor is protected so that only friend classes can create nodes.
	/// This is due to the fact that the parent owns the child, and if a child was constructed
	/// outside of a parent then it wouldn't be possible to ensure that the parent pointed to
	/// actually owned the child.
	template<class... Args>
	TreeNode(TreeNode<T>* parent, Args&&... args) :
		m_count(0),
		m_value(std::forward<Args>(args)...),
		m_parent(parent),
		m_children()
	{	
		incrementCount();

#ifdef tree_h__TEST_TREE
		std::cout << "    TreeNode::constructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	void incrementCount()
	{
		this->m_count++;
		// increment the count back up the tree.
		if (this->m_parent != nullptr)
		{
			this->m_parent->incrementCount();
		}
	}

	void decrementCount()
	{
		this->m_count--;
		// decrement the count back up the tree.
		if (this->m_parent != nullptr)
		{
			this->m_parent->decrementCount();
		}
	}


protected:

	unsigned int									m_count;										///< Number of sub-nodes of this node (inclusive)
	T												m_value;										///< Value stored in the node.
	TreeNode<const T>*								m_parent;										///< Parent node of this node.
	std::vector<std::unique_ptr<TreeNode<const T>>>	m_children;										///< Children of this node.
};

//	----------------------------------------------------------------------------
//	CLASS		ConstTreeIterator
//  ----------------------------------------------------------------------------
///	@brief		Constant, Pre-order, depth-first traversal forward iterator for an unbalanced n-ary tree.
///	@details	For a Tree with n nodes, the Tree can be traversed in O(n) time. 
///				in the best case, the complexity of incrementing the 
///				iterator is O(1), and in the worst case is O(k), where k is the depth of the tree.
///				since the Tree can be unbalanced, O(k) can range from O(log[n]) to O(n).
///
///				The Treeinterface was inspired primarily by std::set, and if the Treedocumentation
///				is unclear reading about std::set will probably shed light on the usage of the
///				interface.
//  ----------------------------------------------------------------------------
template <typename T>
class ConstPreOrderIterator : public std::iterator<std::forward_iterator_tag, T>
{

	template <class C> friend class Tree;

public:

	/// Typedef for the TreeNodes child class iterator.
	// Done this way because the type of this->m_children is an implementation detail of the TreeNode, and
	// I very may well want to change that as we go along.
	using ChildType = decltype(TreeNode<T>::m_children);
	using ChildIterator = typename ChildType::iterator;

	/// Stores the beginning and end iterator of a given nodes children
	using PathNode = std::pair<typename std::decay<ChildIterator>::type, typename std::decay<ChildIterator>::type>;

	/// Stack to store the 'path' to the currently pointed-to node. Like a Hansel & Gretel bread-crumb trail.
	//  This is used to not get eaten by witches living in candy houses. Also to traverse the tree.
	using PathStack = std::vector<PathNode>;

	/**
	* @brief				constructs an iterator from the given Tree with starting location p.
	* @details
	* @param[in]	tree	Treereference to iterate over.
	* @param[in]	p		starting iterator position. nullptr signifies the end of the tree.
	*/
	ConstPreOrderIterator() /*noexcept*/ :
		m_pointer(nullptr),
		m_pathStack({})
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::default contructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	ConstPreOrderIterator(TreeNode<const T>* p) /*noexcept*/ :
		m_pointer(p),
		m_pathStack({})
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::contructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	ConstPreOrderIterator(const ConstPreOrderIterator& itr) /*noexcept*/ :
		m_pointer(itr.m_pointer),
		m_pathStack({})
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::copy constructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	ConstPreOrderIterator(ConstPreOrderIterator&& itr) /*noexcept*/ :
		m_pointer(std::move(itr.m_pointer)),
		m_pathStack(std::move(itr.m_pathStack))
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::move constructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	virtual ~ConstPreOrderIterator()
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::destructor" << std::endl;
#endif // tree_h__TEST_TREE
	}

	/**
	 * @brief		advances the iterator one position.
	 * @details		Overload this function in derived classes to provide different iterator behavior.
	 */
	virtual void next() /*noexcept*/
	{
		this->m_pointer = preOrderTraversalIncrement(this->m_pointer);
	}

	ConstPreOrderIterator& operator=(const ConstPreOrderIterator& other) /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::copy assignment" << std::endl;
#endif // tree_h__TEST_TREE
		this->m_pointer = other.m_pointer;
		return *this;
	}

	ConstPreOrderIterator& operator=(ConstPreOrderIterator&& other) /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::move assignment" << std::endl;
#endif // tree_h__TEST_TREE
		this->m_pointer = std::move(other.m_pointer);
		this->m_pathStack = std::move(other.m_pathStack);
		return *this;
	}

	ConstPreOrderIterator& operator++() /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::increment (prefix)" << std::endl;
#endif // tree_h__TEST_TREE
		next();

		return *this;
	}

	ConstPreOrderIterator operator++(int) /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::increment (postfix)" << std::endl;
#endif // tree_h__TEST_TREE
		ConstPreOrderIterator tmp(*this);
		operator++();
		return tmp;
	}

	bool operator==(const ConstPreOrderIterator& rhs) const { return this->m_pointer == rhs.m_pointer; }
	bool operator!=(const ConstPreOrderIterator& rhs) const { return !(*this == rhs); }

	const T& operator*()  const { return this->m_pointer->m_value; }
	const T* operator->() const { return &(this->m_pointer->m_value); }

	explicit operator bool() const
	{
		return this->m_pointer != nullptr;
	}

	/**
	 * @brief		Get raw pointer.	
	 * @details		Can be used to get a raw pointer from the iterator. State information about
	 *				the tree traversal will not be available to the raw pointer however. The design 
	 *				intention was for compatibility with Qt and libraries which do not support 
	 *				iterators.Use with caution, or better yet, not at all!
	 * @returns		TreeNode<const T>*
	 */
	TreeNode<const T>* internalPointer() const
	{
		return this->m_pointer;
	}

protected:

	TreeNode<const T>* preOrderTraversalIncrement(TreeNode<const T>* node)
	{
		// push the child node iterator onto the path stack
		this->m_pathStack.emplace_back(std::make_pair(node->m_children.begin(), node->m_children.end()));

		if (node->m_children.size())
		{
			// return the first child if there are children
			return (*node->m_children.begin()).get();
		}
		else
		{
			// then, go back up the Tree until a parent with un-traversed path is found.
			do
			{
				// go back up the tree 1 level
				this->m_pathStack.pop_back();

				if (this->m_pathStack.empty())
				{
					// we're back at the root and we've already looked through all the children.
					return nullptr;
				}

				// increment the child iterator
				++this->m_pathStack.back().first;

			} while (this->m_pathStack.back().first == this->m_pathStack.back().second);

			return (*this->m_pathStack.back().first).get();
		}
	}

protected:

	TreeNode<const T>*	m_pointer;
	PathStack			m_pathStack;

};

//	----------------------------------------------------------------------------
//	CLASS		TreeIterator
//  ----------------------------------------------------------------------------
///	@brief		Mutable, Pre-order, depth-first traversal forward iterator for an unbalanced n-ary tree.
///	@details	For a Tree with n nodes, the Tree can be traversed in O(n) time. 
///				in the best case, the complexity of incrementing the 
///				iterator is O(1), and in the worst case is O(k), where k is the depth of the tree.
///				since the Tree can be unbalanced, O(k) can range from O(log[n]) to O(n).
///
///				The Treeinterface was inspired primarily by std::set, and if the Treedocumentation
///				is unclear reading about std::set will probably shed light on the usage of the
///				interface.
//  ----------------------------------------------------------------------------
template <typename T>
class PreOrderIterator : public ConstPreOrderIterator<T>
{

	template <class C> friend class Tree;

public:

	PreOrderIterator() = default;
	PreOrderIterator(TreeNode<const T>* p) : ConstPreOrderIterator<T>(p) {};
	PreOrderIterator(const PreOrderIterator& other) : ConstPreOrderIterator<T>(other) {};
	PreOrderIterator(PreOrderIterator&& other) : ConstPreOrderIterator<T>(std::move(other)) {};
	~PreOrderIterator() = default;
	
	// operators
	PreOrderIterator& operator=(const PreOrderIterator& other) /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::copy assignment" << std::endl;
#endif // tree_h__TEST_TREE
		this->m_pointer = other.m_pointer;
		return *this;
	}

	PreOrderIterator& operator=(PreOrderIterator&& other) /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::move assignment" << std::endl;
#endif // tree_h__TEST_TREE
		this->m_pointer = std::move(other.m_pointer);
		this->m_pathStack = std::move(other.m_pathStack);
		return *this;
	}

	PreOrderIterator& operator++() /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::increment (prefix)" << std::endl;
#endif // tree_h__TEST_TREE
		
		this->next();
		return *this;
	}

	PreOrderIterator& operator++(int) /*noexcept*/
	{
#ifdef tree_h__TEST_TREE
		std::cout << "    iterator::increment (postfix)" << std::endl;
#endif // tree_h__TEST_TREE
		ConstPreOrderIterator<T> tmp(*this);
		operator++();
		return tmp;
	}

	// Return mutable pointers to the node's value
	T& operator*()  const { return const_cast<T&>(this->m_pointer->m_value); }
	T* operator->() const { return const_cast<T*>(&(this->m_pointer->m_value)); }

};

//	----------------------------------------------------------------------------
//	CLASS		ConstChildIterator
//  ----------------------------------------------------------------------------
///	@brief		Constant forward iterator which traverses child nodes.
///	@details	Traversal complexity is linear with the number of children, incrementing
///				is O(1).
//  ----------------------------------------------------------------------------
template <typename T>
class ConstChildIterator : public ConstPreOrderIterator<T>
{

	template <class C> friend class Tree;

public:

	ConstChildIterator() = default;
	~ConstChildIterator() = default;

	ConstChildIterator(TreeNode<const T>* parent) : 
		ConstPreOrderIterator<T>(nullptr) 
	{
		if (parent != nullptr)
		{
			if (this->m_pathStack.empty())
			{
				// push the child node iterator onto the path stack
				this->m_pathStack.emplace_back(std::make_pair(parent->m_children.begin(), parent->m_children.end()));
				if (!parent->m_children.empty())
				{
					this->m_pointer = this->m_pathStack.back().first->get();
				}
			}
		}
	}

	virtual void next()
	{
		this->m_pointer = childIncrement(this->m_pointer);
	}

protected:

	TreeNode<const T>* childIncrement(TreeNode<const T>* parent)
	{
		if (++this->m_pathStack.back().first != this->m_pathStack.back().second)
		{
			return (this->m_pathStack.back().first)->get();
		}
		else
		{
			return nullptr;

		}
	}
};

//	----------------------------------------------------------------------------
//	CLASS		ChildIterator
//  ----------------------------------------------------------------------------
///	@brief		Mutable forward iterator which traverses child nodes.
///	@details	Traversal complexity is linear with the number of children, incrementing
///				is O(1).
//  ----------------------------------------------------------------------------
template <typename T>
class ChildIterator : public ConstChildIterator<T>
{

	template <class C> friend class Tree;

public:

	ChildIterator() = default;
	~ChildIterator() = default;

	ChildIterator(TreeNode<const T>* parent) : ConstChildIterator<T>(parent) {}

	// Return mutable pointers to the node's value
	T& operator*()  const { return const_cast<T&>(this->m_pointer->m_value); }
	T* operator->() const { return const_cast<T*>(&(this->m_pointer->m_value)); }

	operator PreOrderIterator<T>() const
	{
		return PreOrderIterator<T>(this->m_pointer);
	}
};

#pragma warning(pop)
#endif // tree_h__