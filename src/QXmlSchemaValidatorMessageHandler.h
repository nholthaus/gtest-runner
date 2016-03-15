//--------------------------------------------------------------------------------------------------
// 
///	@PROJECT	gtest-gui
/// @BRIEF		Handler for XML schema validation error messages
///	@DETAILS	Implements QAbstractMessageHandler
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
#ifndef QXmlSchemaValidatorMessageHandler_h__
#define QXmlSchemaValidatorMessageHandler_h__

//------------------------
//	INCLUDES
//------------------------
#include <QAbstractMessageHandler>

class QXmlSchemaValidatorMessageHandler : public QAbstractMessageHandler
{
public:
	QXmlSchemaValidatorMessageHandler()
		: QAbstractMessageHandler(0)
	{
	}

	QString statusMessage() const
	{
		return m_description;
	}

	int line() const
	{
		return m_sourceLocation.line();
	}

	int column() const
	{
		return m_sourceLocation.column();
	}

protected:
	virtual void handleMessage(QtMsgType type, const QString &description,
		const QUrl &identifier, const QSourceLocation &sourceLocation)
	{
		Q_UNUSED(type);
		Q_UNUSED(identifier);

		m_messageType = type;
		m_description = description;
		m_sourceLocation = sourceLocation;
	}

private:

	QtMsgType m_messageType;
	QString m_description;
	QSourceLocation m_sourceLocation;
};


#endif // QXmlSchemaValidatorMessageHandler_h__