/*
 * File: Vector2TrackTableWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#include "Vector2TrackTableWidget.h"
#include "Point2dWidget.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QItemDelegate>
#include <QPainter>


namespace {

    class Delegate : public QItemDelegate
    {
    public:
        Delegate(TrackTableModel<Ogre::Vector2>& trackTableModel, QObject *parent = 0)
            : QItemDelegate(parent)
            , mTrackTableModel(trackTableModel)
        {
        }

        QWidget *createEditor(QWidget *parent,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
        {
            Point2dWidget* field = new Point2dWidget(parent);
            field->setAutoFillBackground(true);
            Ogre::Vector2 value = mTrackTableModel.getValues()[index.row()].second;
            field->setValue(value, false);
            return field;
        }

        void setEditorData(QWidget *editor,
                           const QModelIndex &index) const
        {
            Point2dWidget* field = dynamic_cast<Point2dWidget*>(editor);
            Ogre::Vector2 value = mTrackTableModel.getValues()[index.row()].second;
            field->setValue(value, false);
        }

        void setModelData(QWidget *editor,
                          QAbstractItemModel *model,
                          const QModelIndex &index) const
        {
            Point2dWidget* field = dynamic_cast<Point2dWidget*>(editor);
            Ogre::Vector2 value = field->getValue();

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
            QString strX = QString::number(mTrackTableModel.getValues()[row].second.x);
            QString strY = QString::number(mTrackTableModel.getValues()[row].second.y);
            return "(" + strX + ", " + strY + ")";
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

        TrackTableModel<Ogre::Vector2>& mTrackTableModel;
    };

}

Vector2TrackTableWidget::Vector2TrackTableWidget()
{
    Delegate* valueColumnDelegate = new Delegate(mTrackTableModel, this);
    setItemDelegateForColumn(1, valueColumnDelegate);
}

Ogre::Vector2 Vector2TrackTableWidget::defaultValue() const
{
    return Ogre::Vector2::ZERO;
}

Ogre::Vector2 Vector2TrackTableWidget::lerp(const Ogre::Vector2& a, const Ogre::Vector2& b, float percent) const
{
    return a + (b-a) * percent;
}
