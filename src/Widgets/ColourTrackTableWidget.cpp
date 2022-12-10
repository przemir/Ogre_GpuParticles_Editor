/*
 * File: ColourTrackTableWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#include "ColourButton.h"
#include "ColourEditField.h"
#include "ColourTrackTableWidget.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QItemDelegate>
#include <QPainter>


namespace {

    class ColourDelegate : public QItemDelegate
    {
    public:
        ColourDelegate(TrackTableModel<Ogre::ColourValue>& trackTableModel, QObject *parent = 0)
            : QItemDelegate(parent)
            , mTrackTableModel(trackTableModel)
        {
        }

        QWidget *createEditor(QWidget *parent,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
        {
            ColourEditField* field = new ColourEditField(parent);

            Ogre::ColourValue ogreColour = mTrackTableModel.getValues()[index.row()].second;
            field->setColour(ogreColour);

            return field;
        }

        void setEditorData(QWidget *editor,
                           const QModelIndex &index) const
        {
            ColourEditField* field = dynamic_cast<ColourEditField*>(editor);

            Ogre::ColourValue ogreColour = mTrackTableModel.getValues()[index.row()].second;
            field->setColour(ogreColour);
        }

        void setModelData(QWidget *editor,
                          QAbstractItemModel *model,
                          const QModelIndex &index) const
        {
            ColourEditField* field = dynamic_cast<ColourEditField*>(editor);

            Ogre::ColourValue ogreColour = field->getColour();
            mTrackTableModel.setRowSecondValue(index.row(), ogreColour);
        }

        void updateEditorGeometry(QWidget *editor,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
        {
            editor->setGeometry(option.rect);
        }

        void paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            int row = index.row();

            Ogre::ColourValue ogreColour = mTrackTableModel.getValues()[row].second;
            QColor qtColor = ColourButton::ogreColourToQt(ogreColour);
            painter->fillRect(option.rect, qtColor);
        }

        TrackTableModel<Ogre::ColourValue>& mTrackTableModel;
    };
}

ColourTrackTableWidget::ColourTrackTableWidget()
{
    ColourDelegate* valueColumnDelegate = new ColourDelegate(mTrackTableModel, this);
    setItemDelegateForColumn(1, valueColumnDelegate);
}

Ogre::ColourValue ColourTrackTableWidget::lerp(const Ogre::ColourValue& a, const Ogre::ColourValue& b, float percent) const
{
    return a + (b-a) * percent;
}
