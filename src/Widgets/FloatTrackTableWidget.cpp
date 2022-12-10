/*
 * File: FloatTrackTableWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#include "FloatTrackTableWidget.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QItemDelegate>
#include <QPainter>


namespace {

    class SpinBoxDelegate : public QItemDelegate
    {
    public:
        SpinBoxDelegate(TrackTableModel<float>& trackTableModel, float minValue, float maxValue, float stepValue, QObject *parent = 0)
            : QItemDelegate(parent)
            , mTrackTableModel(trackTableModel)
            , mMinValue(minValue)
            , mMaxValue(maxValue)
            , mStepValue(stepValue)
        {
        }

        QWidget *createEditor(QWidget *parent,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
        {
            QDoubleSpinBox* field = new QDoubleSpinBox(parent);
            field->setRange((double)mMinValue, (double)mMaxValue);
            field->setSingleStep(mStepValue);
            float value = mTrackTableModel.getValues()[index.row()].second;
            field->setValue(value);
            return field;
        }

        void setEditorData(QWidget *editor,
                           const QModelIndex &index) const
        {
            QDoubleSpinBox* field = dynamic_cast<QDoubleSpinBox*>(editor);
            float value = mTrackTableModel.getValues()[index.row()].second;
            field->setValue(value);
        }

        void setModelData(QWidget *editor,
                          QAbstractItemModel *model,
                          const QModelIndex &index) const
        {
            QDoubleSpinBox* field = dynamic_cast<QDoubleSpinBox*>(editor);
            double value = field->value();

            mTrackTableModel.setRowSecondValue(index.row(), value);
        }

        void updateEditorGeometry(QWidget *editor,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
        {
            editor->setGeometry(option.rect);
        }

        QString displayValue(int row) const
        {
            return QString::number(mTrackTableModel.getValues()[row].second);
        }

        void paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            int row = index.row();

            QString text = displayValue(row);

            // Get item style settings
            QStyleOptionViewItem myOption = option;
            myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;

            // draw text
            QApplication::style()->drawItemText ( painter, myOption.rect , myOption.displayAlignment, QApplication::palette(), true, text );
        }

        TrackTableModel<float>& mTrackTableModel;
        float mMinValue;
        float mMaxValue;
        float mStepValue;
    };

}

FloatTrackTableWidget::FloatTrackTableWidget(float minValue, float maxValue, float stepValue)
{
    SpinBoxDelegate* spinBoxDelegate = new SpinBoxDelegate(mTrackTableModel, minValue, maxValue, stepValue, this);
    setItemDelegateForColumn(1, spinBoxDelegate);
}

float FloatTrackTableWidget::lerp(const float& a, const float& b, float percent) const
{
    return a + (b-a) * percent;
}
