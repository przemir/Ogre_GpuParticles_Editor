/*
 * File: TrackTableModelBase.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-19
 *
 */

#include "TrackTableModelBase.h"

TrackTableModelBase::TrackTableModelBase()
{
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SIGNAL(trackChanged()));
}

int TrackTableModelBase::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant TrackTableModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        if(orientation == Qt::Horizontal) {
            if(section == 0) {
                return "Time";
            }
            if(section == 1) {
                return "Value";
            }
        }
    }

    return QVariant();
}
