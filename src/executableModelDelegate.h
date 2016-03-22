//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-runner
/// @BRIEF		Delegate definitions for the exectuable test model
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

#ifndef executableModelDelegate_h__
#define executableModelDelegate_h__

//------------------------------
//	INCLUDE
//------------------------------

#include <QStyledItemDelegate>

//--------------------------------------------------------------------------------------------------
//	CLASS: QProgressBarDelegate
//--------------------------------------------------------------------------------------------------
/// @brief		Represents progress role data as a progress bar
/// @details	
//--------------------------------------------------------------------------------------------------
class QProgressBarDelegate : public QStyledItemDelegate
{
public:

	explicit QProgressBarDelegate(QObject* parent = 0);
	virtual ~QProgressBarDelegate();
	

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:

	
	
private:



};	// CLASS: QProgressBarDelegate

#endif // executableModelDelegate_h__