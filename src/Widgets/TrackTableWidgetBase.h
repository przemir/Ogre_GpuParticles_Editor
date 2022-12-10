/*
 * File: TrackTableWidgetBase.h
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#ifndef TRACKTABLEWIDGETBASE_H
#define TRACKTABLEWIDGETBASE_H

//#include <QItemDelegate>
#include <QItemDelegate>
#include <QTableView>

class TrackTableModelBase;

class TrackTableWidgetBase : public QTableView
{
    Q_OBJECT

public:
    class TimeSpinBoxDelegate : public QItemDelegate
    {
    public:
        TimeSpinBoxDelegate(TrackTableModelBase& trackTableModel,
                            TrackTableWidgetBase* trackTableWidget,
                            QObject *parent = 0);

        QWidget *createEditor(QWidget *parent,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

        void setEditorData(QWidget *editor,
                           const QModelIndex &index) const;

        void setModelData(QWidget *editor,
                          QAbstractItemModel *model,
                          const QModelIndex &index) const;

        void updateEditorGeometry(QWidget *editor,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;

        TrackTableModelBase& mTrackTableModel;
        TrackTableWidgetBase* mTrackTableWidget;
    };

public:
    TrackTableWidgetBase();

    void setMaxCount(int maxCount);

//    void selectRow(int row);

    int getCurrentSelectedRow() const;
    QList<int> getSelectedRows() const;

    virtual void mousePressEvent(QMouseEvent* event) override;

protected:
    int mMaxCount = -1;

protected slots:
    virtual void addRowAfterCurrent() = 0;
    virtual void interpolateBetweenPrevAndNext() = 0;
    virtual void moveRowPrev() = 0;
    virtual void moveRowNext() = 0;
    virtual void removeRow() = 0;

signals:
    void dataModified();
};

#endif
