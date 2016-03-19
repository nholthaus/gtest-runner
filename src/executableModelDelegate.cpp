#include "executableModelDelegate.h"
#include "qexecutablemodel.h"

#include <QApplication>
#include <QtGlobal>
#include <QtMath>
#include <QPainter>
#include <QDebug>

//--------------------------------------------------------------------------------------------------
//	FUNCTION: QProgressBarDelegate
//--------------------------------------------------------------------------------------------------
QProgressBarDelegate::QProgressBarDelegate(QObject* parent /*= 0*/) : QStyledItemDelegate(parent)
{

}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: ~QProgressBarDelegate
//--------------------------------------------------------------------------------------------------
QProgressBarDelegate::~QProgressBarDelegate()
{

}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: paint
//--------------------------------------------------------------------------------------------------
void QProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	double progress = index.data(QExecutableModel::ProgressRole).toDouble() * 100;
	if (std::isnan(progress) || progress <= 0.0 || progress >= 100.0)
	{
		QStyledItemDelegate::paint(painter, option, index);
	}
	else
	{
		const QFontMetrics &fm = option.fontMetrics;
		auto height = qMin(qCeil(QFontMetricsF(fm).height()) + 2, option.rect.height());
		int adjust = (option.rect.height() - height) / 2;

		double progress = index.data(QExecutableModel::ProgressRole).toDouble() * 100;

		QStyleOptionProgressBar progressBarOption;
		progressBarOption.rect = option.rect.adjusted(2, adjust, -2, -adjust);
		progressBarOption.rect.setWidth(option.rect.width() - 2 * 2);
		progressBarOption.minimum = 0;
		progressBarOption.maximum = 100;
		progressBarOption.progress = progress;

		if (option.state & QStyle::State_Selected)
		{
			painter->setBrush(option.palette.highlightedText());
		}

		QApplication::style()->drawControl(QStyle::CE_ProgressBar,
			&progressBarOption, painter);
	}
}

//--------------------------------------------------------------------------------------------------
//	FUNCTION: sizeHint
//--------------------------------------------------------------------------------------------------
QSize QProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(50, QStyledItemDelegate::sizeHint(option, index).height());
}
