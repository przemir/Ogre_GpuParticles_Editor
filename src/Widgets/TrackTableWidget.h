/*
 * File: TrackTableWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#ifndef TRACKTABLEWIDGET_H
#define TRACKTABLEWIDGET_H

#include "TrackTableModel.h"
#include "TrackTableWidgetBase.h"

template <class T>
class TrackTableWidget : public TrackTableWidgetBase
{
public:
    TrackTableWidget();


    TrackTableModel<T> mTrackTableModel;

    void moveRow(int from, int to);

protected:
    virtual void addRowAfterCurrent() override;
    virtual void interpolateBetweenPrevAndNext() override;
    virtual void moveRowPrev() override;
    virtual void moveRowNext() override;
    virtual void removeRow() override;

    virtual T defaultValue() const;
    virtual T lerp(const T& a, const T& b, float percent) const = 0;
};

template <class T>
TrackTableWidget<T>::TrackTableWidget()
{
    setModel(&mTrackTableModel);

    TimeSpinBoxDelegate* timeSpinBoxDelegate = new TimeSpinBoxDelegate(mTrackTableModel,
                                                                       this,
                                                                       this);
    setItemDelegateForColumn(0, timeSpinBoxDelegate);


    connect(&mTrackTableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SIGNAL(dataModified()));
}

template<class T>
void TrackTableWidget<T>::moveRow(int from, int to)
{
    mTrackTableModel.moveSingleRow(from, to);
}

template<class T>
void TrackTableWidget<T>::addRowAfterCurrent()
{
    if(mMaxCount >= 0 && mTrackTableModel.rowCount() >= mMaxCount) {
        return;
    }

    QModelIndex currIndex = currentIndex();
    int currentRow = currIndex.row();

    if(currIndex.isValid()) {
        mTrackTableModel.insertRow(currentRow+1, mTrackTableModel.mValues[currentRow]);
        selectRow(currentRow+1);
    }
    else {
        mTrackTableModel.insertRow(0, qMakePair(0.0f, defaultValue()));
        selectRow(0);
    }

    emit dataModified();
}

template<class T>
void TrackTableWidget<T>::interpolateBetweenPrevAndNext()
{
    int currentRow = currentIndex().row();
    int count = mTrackTableModel.rowCount();

    if(count <= 1) {
        return;
    }

    if(currentRow > 0 && currentRow < count-1) {
        QPair<float, T> prevValue = mTrackTableModel.getValues()[currentRow-1];
        QPair<float, T> currValue = mTrackTableModel.getValues()[currentRow];
        QPair<float, T> nextValue = mTrackTableModel.getValues()[currentRow+1];
        QPair<float, T> value = currValue;

        if(currValue.first <= prevValue.first) {
            value.second = prevValue.second;
            mTrackTableModel.setRowValue(currentRow, value);
        }
        else if(currValue.first >= nextValue.first) {
            value.second = nextValue.second;
            mTrackTableModel.setRowValue(currentRow, value);
        }
        else {
            float percent = (currValue.first - prevValue.first) / (nextValue.first - prevValue.first);
            value.second = lerp(prevValue.second, nextValue.second, percent);
            mTrackTableModel.setRowValue(currentRow, value);
        }
    }
    else if(currentRow == 0) {
        QPair<float, T> value = mTrackTableModel.getValues()[currentRow];
        value.second = mTrackTableModel.getValues()[currentRow+1].second;
        mTrackTableModel.setRowValue(currentRow, value);
    }
    else if(currentRow == count-1) {
        QPair<float, T> value = mTrackTableModel.getValues()[currentRow];
        value.second = mTrackTableModel.getValues()[currentRow-1].second;
        mTrackTableModel.setRowValue(currentRow, value);
    }

    emit dataModified();
}

template<class T>
void TrackTableWidget<T>::moveRowPrev()
{
    int row = getCurrentSelectedRow();
    if(row <= 0) {
        return;
    }

    float prevTime = mTrackTableModel.getValues()[row-1].first;

    moveRow(row, row-1);
//    mTrackTableModel.moveRow(QModelIndex(), row, QModelIndex(), row-1);
    mTrackTableModel.setTime(row-1, prevTime);

//    clearSelection();
//    selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Select);

    selectRow(row-1);

    emit dataModified();
}

template<class T>
void TrackTableWidget<T>::moveRowNext()
{
    int row = getCurrentSelectedRow();
    if(row < 0 || row >= mTrackTableModel.rowCount()-1) {
        return;
    }

    float nextTime = mTrackTableModel.getValues()[row+1].first;

//    mTrackTableModel.moveRow(QModelIndex(), row, QModelIndex(), row+2);
    moveRow(row, row+1);
    mTrackTableModel.setTime(row+1, nextTime);

//    clearSelection();
//    setCurrentIndex(QModelIndex());

//    QModelIndex index = mTrackTableModel.index(row+1, 0);
//    setCurrentIndex(index);
//    selectionModel()->select(index, QItemSelectionModel::Select);

    selectRow(row+1);


    emit dataModified();
}

template<class T>
void TrackTableWidget<T>::removeRow()
{
    QList<int> rows = getSelectedRows();

    for (int i = rows.size()-1; i >= 0; --i) {
        mTrackTableModel.removeRow(rows[i]);
    }

    clearSelection();
    selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Select);


    emit dataModified();
}

template<class T>
T TrackTableWidget<T>::defaultValue() const
{
    return T();
}



#endif
