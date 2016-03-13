//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-gui
/// @BRIEF		GTest std::out syntax highlighter
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

#ifndef QStdOutSyntaxHighlighter_h__
#define QStdOutSyntaxHighlighter_h__

//------------------------
//	INCLUDES
//------------------------
#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <QColor>

//	----------------------------------------------------------------------------
//	CLASS		QStdOutSyntaxHighlighter
//  ----------------------------------------------------------------------------
///	@brief		provides syntax highlighting for the std::out console.
///	@details	
//  ----------------------------------------------------------------------------
class QStdOutSyntaxHighlighter : public QSyntaxHighlighter
{
	enum blockState
	{

	};

public:

	QStdOutSyntaxHighlighter(QTextEdit* parent) : QSyntaxHighlighter(parent)
	{
		HighlightingRule rule;

// 		errorFormat.setFontWeight(QFont::Bold);
// 		errorFormat.setForeground(QColor("#ff7f7f"));
// 		rule.pattern = QRegExp("ERROR:.*");
// 		rule.format = errorFormat;
// 		highlightingRules.append(rule);

// 		connectionFormat.setForeground(QColor("#FFD700"));
// 		rule.pattern = QRegExp(".*Connection.*");
// 		rule.format = connectionFormat;
// 		highlightingRules.append(rule);
	}

	void highlightBlock(const QString &text) override
	{
		foreach(const HighlightingRule &rule, highlightingRules) {
			QRegExp expression(rule.pattern);
			int index = expression.indexIn(text);
			while (index >= 0) {
				int length = expression.matchedLength();
				setFormat(index, length, rule.format);
				index = expression.indexIn(text, index + length);
			}
		}
	}

private:

	struct HighlightingRule
	{
		QRegExp					pattern;
		QTextCharFormat			format;
	};
	QVector<HighlightingRule>	highlightingRules;

// 	QTextCharFormat				errorFormat;														///< Highlight style for errors.
// 	QTextCharFormat				connectionFormat;													///< Highlight style for network connection related messages.

};

#endif // QStdErrSyntaxHighlighter_h__