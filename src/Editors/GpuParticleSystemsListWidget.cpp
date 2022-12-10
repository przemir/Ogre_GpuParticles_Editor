/*
 * File: GpuParticleSystemsListWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#include "GpuParticleSystemsListWidget.h"

#include <QEvent>
#include <QBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include <GpuParticles/GpuParticleSystemResourceManager.h>

namespace  {
    class KeyPressEater : public QObject
    {
    protected:
        bool eventFilter(QObject *obj, QEvent *event) override
        {
            if (event->type() == QEvent::KeyPress) {
    //            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    //            qDebug("Ate key press %d", keyEvent->key());
                return true;
            } else {
                // standard event processing
                return QObject::eventFilter(obj, event);
            }
        }
    };
}

GpuParticleSystemsListWidget::GpuParticleSystemsListWidget()
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    {
        mListView = new QListView();
        mListModel = new QStandardItemModel();

        mListSortFilterProxyModel = new QSortFilterProxyModel();
        mListSortFilterProxyModel->setSourceModel(mListModel);
        mListSortFilterProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        mListSortFilterProxyModel->setSortRole(Qt::DisplayRole);
        mListSortFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        mListSortFilterProxyModel->setDynamicSortFilter(true);

        mListView->setModel(mListSortFilterProxyModel);

        mListView->setAcceptDrops(false);
        mListView->setDragDropMode(QAbstractItemView::NoDragDrop);

        //Dynamically adjust contents
        mListView->setResizeMode(QListView::Adjust);

        mListView->setMovement(QListView::Static);

        mListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        static KeyPressEater* keyPressEater = new KeyPressEater();

        mListView->installEventFilter(keyPressEater);

        mainLayout->addWidget(mListView);

        connect(mListView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked()));
    }

    {
        mSearchEdit = new QLineEdit();
        connect(mSearchEdit, SIGNAL(textChanged(const QString&)), this, SLOT(searchEditChanged(const QString&)));
        mainLayout->addWidget(mSearchEdit);
    }

    setLayout(mainLayout);
}

void GpuParticleSystemsListWidget::refreshList()
{
    QSignalBlocker blocker(mListView);

    QAbstractItemModel* model = mListView->model();
    QString currentName = model->data(mListView->currentIndex(), Qt::UserRole).toString();
    mListModel->clear();

    const GpuParticleSystemResourceManager::GpuParticleSystemMap& cores = GpuParticleSystemResourceManager::getSingleton().getGpuParticleSystems();
    for(GpuParticleSystemResourceManager::GpuParticleSystemMap::const_iterator it = cores.begin();
        it != cores.end(); ++it) {

        Ogre::String nameOgreStr = it->second.name;
        QString name = QString::fromStdString(nameOgreStr);

        QStandardItem* item = new QStandardItem(name);
        item->setData(QVariant(name), Qt::UserRole);
        mListModel->appendRow(item);
        if(name == currentName) {
            QModelIndex index = item->index();
            QModelIndex mappedIndex = mListSortFilterProxyModel->mapFromSource(index);
            mListView->selectionModel()->setCurrentIndex(mappedIndex, QItemSelectionModel::Select);
        }
    }

    mListSortFilterProxyModel->sort(0, Qt::AscendingOrder);
}

void GpuParticleSystemsListWidget::chooseItem(const QString& name)
{
    QList<QStandardItem*> items = mListModel->findItems(name);
    if(items.size() > 0) {
        QModelIndex index = items[0]->index();
        QModelIndex mappedIndex = mListSortFilterProxyModel->mapFromSource(index);
        mListView->selectionModel()->setCurrentIndex(mappedIndex, QItemSelectionModel::Select);
    }
}

void GpuParticleSystemsListWidget::itemClicked()
{
    QAbstractItemModel* model = mListView->model();
    QString name = model->data(mListView->currentIndex(), Qt::UserRole).toString();

    emit itemChanged(name);
}

void GpuParticleSystemsListWidget::searchEditChanged(const QString& text)
{
    mListSortFilterProxyModel->setFilterWildcard(text);
}
