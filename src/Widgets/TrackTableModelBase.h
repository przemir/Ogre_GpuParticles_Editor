/*
 * File: TrackTableModelBase.h
 * Author: Przemysław Bągard
 * Created: 2021-7-19
 *
 */

#ifndef TRACKTABLEMODELBASE_H
#define TRACKTABLEMODELBASE_H

#include <QAbstractTableModel>

class TrackTableModelBase  : public QAbstractTableModel
{
    Q_OBJECT
public:
    TrackTableModelBase();

    virtual QVector<float> getTimes() const = 0;
    virtual void setTime(int row, float time) = 0;
    virtual void moveSingleRow(int from, int to) = 0;

    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
//    rowCount();
//    data();

signals:
    /// Emits when something about track has changed like changed value,
    /// added/removed row etc.
    void trackChanged();
};



#endif
