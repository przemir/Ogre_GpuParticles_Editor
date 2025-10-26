/*
 * File: TrackTableWidgetBase.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#include "TrackTableModelBase.h"
#include "TrackTableWidgetBase.h"

#include <QAction>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QMouseEvent>

TrackTableWidgetBase::TimeSpinBoxDelegate::TimeSpinBoxDelegate(TrackTableModelBase& trackTableModel,
                                                               TrackTableWidgetBase* trackTableWidget,
                                                               QObject *parent)
    : QItemDelegate(parent)
    , mTrackTableModel(trackTableModel)
    , mTrackTableWidget(trackTableWidget)
{
}

QWidget* TrackTableWidgetBase::TimeSpinBoxDelegate::createEditor(QWidget *parent,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QModelIndex &index) const
{
    QDoubleSpinBox* field = new QDoubleSpinBox(parent);
    float value = mTrackTableModel.getTimes()[index.row()];
    field->setValue(value);
    field->setSingleStep(0.01f);
    return field;
}

void TrackTableWidgetBase::TimeSpinBoxDelegate::setEditorData(QWidget *editor,
                                                              const QModelIndex &index) const
{
    QDoubleSpinBox* field = dynamic_cast<QDoubleSpinBox*>(editor);
    float value = mTrackTableModel.getTimes()[index.row()];
    field->setValue(value);
}

void TrackTableWidgetBase::TimeSpinBoxDelegate::setModelData(QWidget *editor,
                                                             QAbstractItemModel *model,
                                                             const QModelIndex &index) const
{
    double oldValue =  mTrackTableModel.getTimes()[index.row()];

    QDoubleSpinBox* field = dynamic_cast<QDoubleSpinBox*>(editor);
    float value = field->value();

    mTrackTableModel.setTime(index.row(), value);

    int candidateI = -1;

    if(value < oldValue) {
        for (int i = index.row()-1; i >= 0; --i) {
            float cellValue = mTrackTableModel.getTimes()[i];
            if(value < cellValue) {
                candidateI = i;
            }
        }
    }
    else if(value > oldValue) {
        for (int i = index.row()+1; i < model->rowCount(); ++i) {
            float cellValue = mTrackTableModel.getTimes()[i];
            if(value > cellValue) {
                candidateI = i;
            }
        }
    }

    if(candidateI != -1) {
        mTrackTableModel.moveSingleRow(index.row(), candidateI);

//        mTrackTableWidget->clearSelection();
//        mTrackTableWidget->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Select);

        mTrackTableWidget->selectRow(candidateI);
    }
}

void TrackTableWidgetBase::TimeSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

TrackTableWidgetBase::TrackTableWidgetBase()
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    verticalHeader()->hide();
    horizontalHeader()->setStretchLastSection(true);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    {
        QAction* action = new QAction(tr("Add row"));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(addRowAfterCurrent()));
        addAction(action);
    }

    {
        QAction* action = new QAction(tr("Interpolate between prev and next"));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(interpolateBetweenPrevAndNext()));
        addAction(action);
    }

    {
        QAction* action = new QAction(tr("Move row prev"));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(moveRowPrev()));
        addAction(action);
    }

    {
        QAction* action = new QAction(tr("Move row next"));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(moveRowNext()));
        addAction(action);
    }

    {
        QAction* action = new QAction(tr("Remove row"));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(removeRow()));
        addAction(action);
    }
}

void TrackTableWidgetBase::setMaxCount(int maxCount)
{
    mMaxCount = maxCount;
}

int TrackTableWidgetBase::getCurrentSelectedRow() const
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) {
        return -1;
    }
    bool is = selectionModel()->selectedIndexes().contains(index);
    if(is) {
        return index.row();
    }
    return -1;
}

QList<int> TrackTableWidgetBase::getSelectedRows() const
{
    QList<int> rows;
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    for (int i = 0; i < indexes.size(); ++i) {
        rows.push_back(indexes[i].row());
    }

#if QT_VERSION_MAJOR >= 6
    QSet<int> rowsSet(rows.begin(), rows.end());
    rows = QList<int>(rowsSet.begin(), rowsSet.end());
    std::sort(rows.begin(), rows.end());
#else
    rows = rows.toSet().toList();
    qSort(rows);
#endif
    return rows;
}

void TrackTableWidgetBase::mousePressEvent(QMouseEvent* event)
{
    QModelIndex item = indexAt(event->pos());

    if (item.isValid())
    {
        QTableView::mousePressEvent(event);
    }
    else
    {
        clearSelection();
        const QModelIndex index;
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
    }
}
