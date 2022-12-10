/*
 * File: GpuParticleDatablocksListWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#ifndef GPUPARTICLEDATABLOCKSLISTWIDGET_H
#define GPUPARTICLEDATABLOCKSLISTWIDGET_H

#include <QWidget>

class QLineEdit;
class QListView;
class QStandardItemModel;
class QSortFilterProxyModel;

class GpuParticleDatablocksListWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleDatablocksListWidget();

//    void refreshList();
    void setDatablockList(const QStringList& datablockList, const QVector<QPixmap>& icons);

private:
    QLineEdit* mSearchEdit;
    QListView* mListView;
    QStandardItemModel* mListModel;
    QSortFilterProxyModel* mListSortFilterProxyModel;

private slots:
    void itemClicked();
    void searchEditChanged(const QString& text);

signals:
    void itemChanged(const QString& text);
};

#endif
