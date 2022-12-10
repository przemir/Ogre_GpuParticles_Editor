/*
 * File: TrackTableModel.h
 * Author: Przemysław Bągard
 * Created: 2021-7-19
 *
 */

#ifndef TRACKTABLEMODEL_H
#define TRACKTABLEMODEL_H

#include "TrackTableModelBase.h"

template <class T>
class TrackTableModel : public TrackTableModelBase
{
public:
    TrackTableModel();

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
//    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual void moveSingleRow(int from, int to) override;

//    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRow(int row, const QPair<float, T>& value);

    virtual QVector<float> getTimes() const override;
    virtual void setTime(int row, float time) override;

    const QVector<QPair<float, T> >& getValues() const;
    void setValues(const QVector<QPair<float, T> >& values);

    void setRowValue(int row, const QPair<float, T>& value);
    void setRowSecondValue(int row, const T& value);

    void clear();

public:
    QVector<QPair<float, T> > mValues;
};

//

template <class T>
TrackTableModel<T>::TrackTableModel()
{

}

template<class T>
Qt::ItemFlags TrackTableModel<T>::flags(const QModelIndex& index) const
{
//    return Qt::ItemFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    Qt::ItemFlags flags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    if(index.column() == 0) {
        flags = flags | Qt::ItemIsSelectable;
    }
    return flags;
}

template<class T>
int TrackTableModel<T>::rowCount(const QModelIndex& parent) const
{
    return mValues.size();
}

template<class T>
QVariant TrackTableModel<T>::data(const QModelIndex& index, int role) const
{
    if(index.column() == 0) {
        if(role == Qt::DisplayRole) {
            return mValues[index.row()].first;
        }
    }

    if(index.column() == 1) {
        if(role == Qt::DisplayRole) {
//            colum
//            TrackTableDelegateBase
//            return mValues[index].first;
        }
    }

//    return QAbstractItemModel::data(index, role);

    return QVariant();
}

template<class T>
QVector<float> TrackTableModel<T>::getTimes() const
{
    QVector<float> times;
    times.reserve(mValues.size());
    for (int i = 0; i < mValues.size(); ++i) {
        times.push_back(mValues[i].first);
    }
    return times;
}

template<class T>
const QVector<QPair<float, T> >& TrackTableModel<T>::getValues() const
{
    return mValues;
}

template<class T>
void TrackTableModel<T>::setValues(const QVector<QPair<float, T> >& values)
{
    this->beginResetModel();
    mValues = values;
    this->endResetModel();
    emit trackChanged();
}

template<class T>
void TrackTableModel<T>::clear()
{
    this->beginResetModel();
    mValues.clear();
    this->endResetModel();
    emit trackChanged();
}

//template<class T>
//bool TrackTableModel<T>::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
//{
//    // beginMoveRows and endMoveRows seems weird for moving cell one field up.

//    if(count > 1) {
//        return false;   // not handled
//    }



//    bool ok = this->beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
//    if(!ok) {
//        return false;
//    }

//    QVector<QPair<float, T> > temp;
//    for (int i = sourceRow; i <= sourceRow+count-1; ++i) {
//        temp.push_back(mValues[i]);
//    }
//    mValues.remove(sourceRow, count);
//    for (int i = temp.size()-1; i >= 0; --i) {
//        mValues.insert(destinationChild, temp[i]);
//    }
//    this->endMoveRows();
//    return true;
//}

template<class T>
bool TrackTableModel<T>::insertRow(int row, const QPair<float, T>& value)
{
    this->beginInsertRows(QModelIndex(), row, row);
    mValues.insert(row, value);
    this->endInsertRows();
    emit trackChanged();
    return true;
}

template<class T>
bool TrackTableModel<T>::insertRows(int row, int count, const QModelIndex& parent)
{
    this->beginInsertRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        mValues.insert(row, QPair<float, T>(0.0f, T()));
    }
    this->endInsertRows();
    emit trackChanged();
    return true;
}

template<class T>
bool TrackTableModel<T>::removeRows(int row, int count, const QModelIndex& parent)
{
    this->beginRemoveRows(parent, row, row+count-1);
    mValues.remove(row, count);
    this->endRemoveRows();
    emit trackChanged();
    return true;
}

template<class T>
void TrackTableModel<T>::setRowValue(int row, const QPair<float, T>& value)
{
    mValues[row] = value;
    emit dataChanged(QModelIndex().child(row, 0), QModelIndex().child(row, 1));
}

template<class T>
void TrackTableModel<T>::setTime(int row, float time)
{
    mValues[row].first = time;
    emit dataChanged(QModelIndex().child(row, 0), QModelIndex().child(row, 0));
}

template<class T>
void TrackTableModel<T>::setRowSecondValue(int row, const T& value)
{
    mValues[row].second = value;
    emit dataChanged(QModelIndex().child(row, 1), QModelIndex().child(row, 1));
}


template<class T>
void TrackTableModel<T>::moveSingleRow(int from, int to)
{
    QPair<float, T> value = getValues()[from];
    removeRow(from);
    insertRow(to, value);
}



#endif
