/*
 * File: GpuParticleSystemsListWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#ifndef GPUPARTICLESYSTEMSLISTWIDGET_H
#define GPUPARTICLESYSTEMSLISTWIDGET_H

#include <QWidget>

class QLineEdit;
class QListView;
class QStandardItemModel;
class QSortFilterProxyModel;

class GpuParticleSystemsListWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleSystemsListWidget();

    void refreshList();
    void chooseItem(const QString& name);

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
