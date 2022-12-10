/*
 * File: GpuParticleDatablocksListWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#include "GpuParticleDatablocksListWidget.h"

#include <QEvent>
#include <OgreHlmsManager.h>
#include <QBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include <GpuParticles/Hlms/HlmsParticle.h>

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

GpuParticleDatablocksListWidget::GpuParticleDatablocksListWidget()
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    {
        mListView = new QListView();
        mListView->setIconSize(QSize(64, 64));
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

//void GpuParticleDatablocksListWidget::refreshList()
//{
//    QSignalBlocker blocker(mListView);

//    QAbstractItemModel* model = mListView->model();
//    QString currentName = model->data(mListView->currentIndex(), Qt::UserRole).toString();
//    mListModel->clear();

//    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
//    HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));

//    for(std::map<Ogre::IdString, Ogre::Hlms::DatablockEntry>::const_iterator it = hlmsParticle->getDatablockMap().begin();
//        it != hlmsParticle->getDatablockMap().end(); ++it) {

////        const Ogre::Hlms::DatablockEntry& datablockEntry = it->second;

//        Ogre::String nameOgreStr = it->second.name;
//        QString name = QString::fromStdString(nameOgreStr);

//        QStandardItem* item = new QStandardItem(name);
//        item->setData(QVariant(name), Qt::UserRole);
////        item->setIcon();
//        mListModel->appendRow(item);
//        if(name == currentName) {
//            QModelIndex index = item->index();
//            QModelIndex mappedIndex = mListSortFilterProxyModel->mapFromSource(index);
//            mListView->selectionModel()->setCurrentIndex(mappedIndex, QItemSelectionModel::Select);
//        }
//    }

//    mListSortFilterProxyModel->sort(0, Qt::AscendingOrder);
//}

void GpuParticleDatablocksListWidget::setDatablockList(const QStringList& datablockList, const QVector<QPixmap>& icons)
{
    QSignalBlocker blocker(mListView);

    QAbstractItemModel* model = mListView->model();
    QString currentName = model->data(mListView->currentIndex(), Qt::UserRole).toString();
    mListModel->clear();

    for (int i = 0; i < datablockList.size(); ++i) {

        QString name = datablockList[i];

        QStandardItem* item = new QStandardItem(name);
        item->setData(QVariant(name), Qt::UserRole);
        item->setIcon(icons[i]);
        mListModel->appendRow(item);
        if(name == currentName) {
            QModelIndex index = item->index();
            QModelIndex mappedIndex = mListSortFilterProxyModel->mapFromSource(index);
            mListView->selectionModel()->setCurrentIndex(mappedIndex, QItemSelectionModel::Select);
        }
    }

    mListSortFilterProxyModel->sort(0, Qt::AscendingOrder);
}

void GpuParticleDatablocksListWidget::itemClicked()
{
    QAbstractItemModel* model = mListView->model();
    QString name = model->data(mListView->currentIndex(), Qt::UserRole).toString();

    emit itemChanged(name);
}

void GpuParticleDatablocksListWidget::searchEditChanged(const QString& text)
{
    mListSortFilterProxyModel->setFilterWildcard(text);
}
