/*
 * File: ImageChooserDialog.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-5
 *
 */

#include "ImageChooserDialog.h"
#include "ParticleEditorData.h"

#include <QEvent>
#include <OgreTextureGpuManager.h>
#include <QBoxLayout>
#include <QListView>
#include <QLabel>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QCheckBox>
#include <ParticleEditorFunctions.h>
#include <ParticleEditorAssets.h>
#include <OgreRenderSystem.h>

namespace {
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

ImageChooserDialog::ImageChooserDialog(ParticleEditorData& _data, const QSize& size, QWidget* parent)
    : QDialog(parent)
    , data(_data)
{
    createGui();
    resize(size);
    createListItems();
}

void ImageChooserDialog::createGui()
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    {
        QHBoxLayout* searchLineLayout = new QHBoxLayout();

        searchLineLayout->addWidget(new QLabel(tr("Filter:")));

        mSearchEdit = new QLineEdit();
        mSearchEdit->setMinimumHeight(26);
        connect(mSearchEdit, SIGNAL(textChanged(const QString&)), this, SLOT(searchEditChanged()));
        searchLineLayout->addWidget(mSearchEdit);

        mSearchUseWildcardsCheckBox = new QCheckBox(tr("Use wildcards"));
        mSearchUseWildcardsCheckBox->setChecked(true);
        connect(mSearchUseWildcardsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(searchEditChanged()));
        searchLineLayout->addWidget(mSearchUseWildcardsCheckBox);

        mainLayout->addLayout(searchLineLayout);
    }

    {
//        mListView = new QListWidget();
        mListView = new QListView();
        mListView->setFlow(QListView::LeftToRight);

        mListModel = new QStandardItemModel();

        mListSortFilterProxyModel = new QSortFilterProxyModel();
//        mListSortFilterProxyModel->setSourceModel(mListModel);
        mListSortFilterProxyModel->setSourceModel(mListModel);
        mListSortFilterProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        mListSortFilterProxyModel->setSortRole(Qt::DisplayRole);
        mListSortFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        mListSortFilterProxyModel->setDynamicSortFilter(true);

        mListView->setModel(mListSortFilterProxyModel);

        mListView->setAcceptDrops(false);
        mListView->setDragDropMode(QAbstractItemView::DragOnly);

        //Dynamically adjust contents
        mListView->setResizeMode(QListView::Adjust);

        int iconSize = 128;
        int gridSizeX = 138;
        int gridSizeY = 150;

        mListView->setIconSize(QSize(iconSize, iconSize));

        //This is an arbitrary value, but it forces the layout into a grid
        mListView->setGridSize(QSize(gridSizeX, gridSizeY));

        //As an alternative to using setGridSize(), set a fixed spacing in the layout:
//        mListView->setSpacing(128);

        //And the most important part:
        mListView->setViewMode(QListView::IconMode);
//        mListView->setMovement(QListView::Static);
        mListView->setMovement(QListView::Snap);

        mListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        static KeyPressEater* keyPressEater = new KeyPressEater();

        mListView->installEventFilter(keyPressEater);

        mainLayout->addWidget(mListView);

        connect(mListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotDoubleClicked()));
    }

    setLayout(mainLayout);
}

void ImageChooserDialog::createListItems()
{
    data.mParticleEditorFunctions->loadAllUnloadedTextures();

    Ogre::TextureGpuManager *textureManager =
            data.getRenderSystem()->getTextureGpuManager();

    const Ogre::TextureGpuManager::ResourceEntryMap& entries = textureManager->getEntries();
    for(Ogre::TextureGpuManager::ResourceEntryMap::const_iterator it = entries.begin();
        it != entries.end(); ++it) {

        const Ogre::TextureGpuManager::ResourceEntry& entry = it->second;
        QString textureName = QString::fromStdString(entry.name);
        ParticleEditorAssets::TextureData* textureData = data.mParticleEditorAssets->getTexture(textureName);
        if(!textureData) {
            continue;
        }

        QStandardItem* item = new QStandardItem();
        item->setIcon(QIcon(textureData->mIconPixmap));
        item->setTextAlignment(Qt::AlignBottom | Qt::AlignHCenter);
        item->setText(textureName);
        item->setData(QVariant(textureName), Qt::UserRole);
        mListModel->appendRow(item);
    }
}

void ImageChooserDialog::searchEditChanged()
{
    bool useWildcards = mSearchUseWildcardsCheckBox->isChecked();
    QString text = mSearchEdit->text();
    if(useWildcards) {
        mListSortFilterProxyModel->setFilterWildcard(text);
    }
    else {
        mListSortFilterProxyModel->setFilterFixedString(text);
    }
}

void ImageChooserDialog::slotDoubleClicked()
{

}
