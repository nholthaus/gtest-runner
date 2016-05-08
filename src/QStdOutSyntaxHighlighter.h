//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-runner
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

 		blockFormat.setForeground(QColor("#00ff00"));
 		rule.pattern = QRegExp("\\[((?!\\s+DEATH\\s+).)*\\]");
 		rule.format = blockFormat;
		highlightingRules.append(rule);

		errorFormat.setForeground(QColor("#ff0000"));
		rule.pattern = QRegExp("\\[.*FAILED.*\\]");
		rule.format = errorFormat;
		highlightingRules.append(rule);

		errorFormat.setForeground(QColor("#ffd700"));
		rule.pattern = QRegExp("TEST RUN .*");
		rule.format = errorFormat;
		highlightingRules.append(rule);
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

	QTextCharFormat				errorFormat;													///< Highlight style for errors.
	QTextCharFormat				blockFormat;													///< Highlight style for network connection related messages.
	QTextCharFormat				timestampFormat;												///< highlight style for timestamps
};

#endif // QStdErrSyntaxHighlighter_h__