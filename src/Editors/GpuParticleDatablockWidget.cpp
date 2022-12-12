/*
 * File: GpuParticleDatablockWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#include "ParticleEditorData.h"
#include "GpuParticleDatablockWidget.h"

#include <ColourEditField.h>
#include <EulerDegreeHVSWidget.h>
#include <Point3dWidget.h>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <RangeWidget.h>
#include <QToolButton>
#include <QGroupBox>
#include <ColourButton.h>
#include <QTableView>
#include <SpriteTrackTableWidget.h>
#include <QPushButton>
#include <OgreHlmsManager.h>
#include <OgreRoot.h>
#include <QHeaderView>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QLineEdit>
#include <ParticleEditorFunctions.h>
#include <QMainWindow>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <OgreTextureGpuManager.h>
#include <QMessageBox>
#include <OgreFileSystem.h>
#include <OgreQtImageHelper.h>

#include <GpuParticles/Hlms/HlmsParticleDatablock.h>

#include "BlendblockWidget.h"
#include "MacroblockWidget.h"
#include "ImageChooserDialog.h"
#include "ParticleEditorAssets.h"
#include "SamplerblockWidget.h"
#include "BaseDatablockWidget.h"
#include "UnlitDatablockWidget.h"

const int GpuParticleDatablockWidget::IconSize = 64;

GpuParticleDatablockWidget::GpuParticleDatablockWidget(ParticleEditorData& _data)
    : data(_data)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    QGridLayout* grid = new QGridLayout();

    int row = 0;

    {
        mImageLabel = new QLabel();
        mImageLabel->setMinimumHeight(128);
        mImageLabel->setAlignment(Qt::AlignHCenter);

//        mImageButton = new QPushButton();
//        mImageButton->setMinimumHeight(128);
//        connect(mImageButton, SIGNAL(clicked(bool)), this, SLOT(imageButtonClicked()));
        grid->addWidget(mImageLabel, row++, 0, 1, 3);
    }

    {
        mImageNameLabel = new QLabel();
        mImageNameLabel->setAlignment(Qt::AlignHCenter);

        grid->addWidget(mImageNameLabel, row++, 0, 1, 3);
    }

    {
        QString tooltipStr = tr("Load images from resources or import new ones\n(if image is outside of resources then name cannot be repeated).");

        QLabel* label = new QLabel(tr("Load or import file:"));
        label->setToolTip(tooltipStr);
        grid->addWidget(label, row, 0);

        mLoadFromResourcesActionButton = new QToolButton();
        mLoadFromResourcesActionButton->setText(QString(tr("Resources")));
        mLoadFromResourcesActionButton->setContextMenuPolicy(Qt::ActionsContextMenu);
        mLoadFromResourcesActionButton->setPopupMode(QToolButton::InstantPopup);
        mLoadFromResourcesActionButton->setToolTip(tooltipStr);
        mLoadFromResourcesActionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        grid->addWidget(mLoadFromResourcesActionButton, row++, 1, 1, 2);

        mLoadLastPathAction = new QAction(tr("Load from last path"));
        connect(mLoadLastPathAction, SIGNAL(triggered(bool)), this, SLOT(loadFromResourceClicked()));
    }

    {
        mFlipbookSizeLabel = new QLabel(tr("Flipbook size:"));
        grid->addWidget(mFlipbookSizeLabel, row, 0);

        QHBoxLayout* lineLayout = new QHBoxLayout();

        mFlipbookSizeXSpinBox = new QSpinBox();
        mFlipbookSizeXSpinBox->setRange(1, 256);
        connect(mFlipbookSizeXSpinBox, SIGNAL(valueChanged(int)), this, SLOT(flipbookSizeChanged()));
        lineLayout->addWidget(mFlipbookSizeXSpinBox);

        mFlipbookSizeYSpinBox = new QSpinBox();
        mFlipbookSizeYSpinBox->setRange(1, 256);
        connect(mFlipbookSizeYSpinBox, SIGNAL(valueChanged(int)), this, SLOT(flipbookSizeChanged()));
        lineLayout->addWidget(mFlipbookSizeYSpinBox);

        grid->addLayout(lineLayout, row++, 1, 1, 2);
    }

    {
        mSpriteTableWidget = new QTableWidget();
        mSpriteTableWidget->setMinimumHeight(300);
        mSpriteTableWidget->setColumnCount(0);
        mSpriteTableWidget->setRowCount(0);
        mSpriteTableWidget->setIconSize(QSize(IconSize, IconSize));
        mSpriteTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        grid->addWidget(mSpriteTableWidget, row++, 0, 1, 3);
    }

    auto fillNamedLineFrame = [&](const QString& caption)->void
    {
        QHBoxLayout* lineLayout = new QHBoxLayout();
        {
            QFrame* line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            lineLayout->addWidget(line);
        }
        {
            QLabel* label = new QLabel(caption);
            label->setStyleSheet("font: bold 18px;");
            lineLayout->addWidget(label, 0, Qt::AlignHCenter);
        }
        {
            QFrame* line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            lineLayout->addWidget(line);
        }
        grid->addLayout(lineLayout, row++, 0, 1, 3);

        // Empty row
        grid->addWidget(new QLabel(), row++, 0, 1, 3);
    };

    fillNamedLineFrame(tr("Blendblock"));

    {
//        QGroupBox* blendGroup = new QGroupBox(tr("Blendblock"));
//        blendGroup->setCheckable(true);
//        blendGroup->setChecked(true);
//        QVBoxLayout* subLayout = new QVBoxLayout();
        mBlendblockWidget = new BlendblockWidget();
        mBlendblockWidget->createGui(grid, row);
//        subLayout->addWidget(mBlendblockWidget);
//        blendGroup->setLayout(subLayout);
//        connect(blendGroup, SIGNAL(toggled(bool)), mBlendblockWidget, SLOT(setVisible(bool)));
//        grid->addWidget(blendGroup, row++, 0, 1, 3);
    }

    fillNamedLineFrame(tr("Macroblock"));

    {
//        QGroupBox* macroGroup = new QGroupBox(tr("Macroblock"));
//        macroGroup->setCheckable(true);
//        macroGroup->setChecked(true);
//        QVBoxLayout* subLayout = new QVBoxLayout();
        mMacroblockWidget = new MacroblockWidget();
        mMacroblockWidget->createGui(grid, row);
//        subLayout->addWidget(mMacroblockWidget);
//        macroGroup->setLayout(subLayout);
//        connect(macroGroup, SIGNAL(toggled(bool)), mMacroblockWidget, SLOT(setVisible(bool)));
//        grid->addWidget(macroGroup, row++, 0, 1, 3);
    }

    fillNamedLineFrame(tr("Samplerblock"));

    {
//        QGroupBox* samplerGroup = new QGroupBox(tr("Samplerblock"));
//        samplerGroup->setCheckable(true);
//        samplerGroup->setChecked(true);
//        QVBoxLayout* subLayout = new QVBoxLayout();
        mSamplerblockWidget = new SamplerblockWidget();
        mSamplerblockWidget->createGui(grid, row);
//        subLayout->addWidget(mSamplerblockWidget);
//        samplerGroup->setLayout(subLayout);
//        connect(samplerGroup, SIGNAL(toggled(bool)), mSamplerblockWidget, SLOT(setVisible(bool)));
//        grid->addWidget(samplerGroup, row++, 0, 1, 3);
    }

    fillNamedLineFrame(tr("Unlit"));
    {
        mUnlitDatablockWidget = new UnlitDatablockWidget();
        mUnlitDatablockWidget->createGui(grid, row);
    }

    fillNamedLineFrame(tr("Base"));
    {
        mBaseDatablockWidget = new BaseDatablockWidget();
        mBaseDatablockWidget->createGui(grid, row);
    }

    mainLayout->addLayout(grid);
    QSpacerItem* spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addItem(spacerItem);

    setLayout(mainLayout);

    setEnabled(false);
}

void GpuParticleDatablockWidget::setEditedObject(HlmsParticleDatablock* datablock)
{
    mEditedDatablock = datablock;

    setEnabled(mEditedDatablock != nullptr);

    particleDatablockToGui();

    mBlendblockWidget->setEditedObject(mEditedDatablock, false);
    mMacroblockWidget->setEditedObject(mEditedDatablock, false);
    mSamplerblockWidget->setEditedObject(mEditedDatablock, 0);
    mUnlitDatablockWidget->setEditedObject(mEditedDatablock);
    mBaseDatablockWidget->setEditedObject(mEditedDatablock);
}

void GpuParticleDatablockWidget::particleDatablockToGui()
{
    {
        QSignalBlocker bl(mFlipbookSizeXSpinBox);
        mFlipbookSizeXSpinBox->setValue(mEditedDatablock ? mEditedDatablock->getFlipbookSize().col : 1);
    }

    {
        QSignalBlocker bl(mFlipbookSizeYSpinBox);
        mFlipbookSizeYSpinBox->setValue(mEditedDatablock ? mEditedDatablock->getFlipbookSize().row : 1);
    }

    updateImage();
    updateSpriteTable();
}

void GpuParticleDatablockWidget::updateSpriteTable()
{
    mSpriteTableWidget->clear();

    if(!mEditedDatablock) {
        mSpriteTableWidget->setColumnCount(0);
        mSpriteTableWidget->setRowCount(0);
        return;
    }

    HlmsParticleDatablock::SpriteCoord flipbookSize = mEditedDatablock->getFlipbookSize();
    QStringList rowLabels;
    QStringList colLabels;
    for (int i = 0; i < flipbookSize.col; ++i) {
        colLabels.push_back(QString::number(i));
    }
    for (int i = 0; i < flipbookSize.row; ++i) {
        rowLabels.push_back(QString::number(i));
    }

    mSpriteTableWidget->setColumnCount(colLabels.size());
    mSpriteTableWidget->setRowCount(rowLabels.size());

    mSpriteTableWidget->setHorizontalHeaderLabels(colLabels);
    mSpriteTableWidget->setVerticalHeaderLabels(rowLabels);


    const ParticleEditorAssets::DatablockData* datablockData = data.mParticleEditorAssets->getDatablock(QString::fromStdString(*mEditedDatablock->getNameStr()));

    if(datablockData) {

        const int IconMargin = 12;
        for (int i = 0; i < flipbookSize.col; ++i) {
            mSpriteTableWidget->setColumnWidth(i, IconSize+IconMargin);
        }
        for (int i = 0; i < flipbookSize.row; ++i) {
            mSpriteTableWidget->setRowHeight(i, IconSize+IconMargin);
        }

        for (int i = 0; i < datablockData->mSprites.size(); ++i) {
            QIcon icon = datablockData->mSprites[i];
            HlmsParticleDatablock::SpriteCoord coord;
            coord.col = i % flipbookSize.col;
            coord.row = i / flipbookSize.col;

            QTableWidgetItem* item = new QTableWidgetItem(icon, QString());
            item->setFlags(Qt::ItemIsEnabled);
            mSpriteTableWidget->setItem(coord.row, coord.col, item);
        }
    }

}

void GpuParticleDatablockWidget::refreshLoadFromResourcesActions()
{
    // clear old actions
    {
        QList<QAction*> oldActions = mLoadFromResourcesActionButton->actions();
        for (int i = 0; i < oldActions.size(); ++i) {
            mLoadFromResourcesActionButton->removeAction(oldActions[i]);

            if(oldActions[i] != mLoadLastPathAction) {
                delete oldActions[i];
            }
        }
    }


    std::set<Ogre::String> resourcePaths;
    data.mParticleEditorFunctions->getAllTexturePaths(resourcePaths);

    for(std::set<Ogre::String>::iterator it = resourcePaths.begin();
        it != resourcePaths.end(); ++it) {

        QString path = QString::fromStdString(*it);
        QAction* action = new QAction(path);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(loadFromResourceClicked()));
        mLoadFromResourcesActionButton->addAction(action);
    }

    mLoadFromResourcesActionButton->addAction(mLoadLastPathAction);
}

void GpuParticleDatablockWidget::updateImage()
{
    Ogre::TextureGpu* texture = data.mChoosenGpuParticleDatablock ? data.mChoosenGpuParticleDatablock->getEmissiveTexture() : NULL;
    ParticleEditorAssets::TextureData* textureData = nullptr;
    if(texture) {
        QString str = QString::fromStdString(texture->getNameStr());
        textureData = data.mParticleEditorAssets->getTexture(str);

        if(!textureData) {
            mImageNameLabel->setText(str + " (missing)");
        }
        else {
            mImageNameLabel->setText(str);
        }
    }
    else {
        mImageNameLabel->setText(QString());
    }

    if(textureData) {
        QPixmap pixmap = textureData->mIconPixmap;
        mImageLabel->setPixmap(pixmap);
    }
    else {
        mImageLabel->setPixmap(QPixmap());
    }
}

void GpuParticleDatablockWidget::onParticleDatablockModified()
{
    if(!mEditedDatablock) {
        return;
    }

    emit particleDatablockModified();
}

void GpuParticleDatablockWidget::flipbookSizeChanged()
{
    if(!mEditedDatablock) {
        return;
    }

    mEditedDatablock->setFlipbookSize(mFlipbookSizeXSpinBox->value(), mFlipbookSizeYSpinBox->value());

    data.mParticleEditorFunctions->recalculateParticleDatablockSpriteIcons(mEditedDatablock);
    updateSpriteTable();

    emit particleDatablockModified();
}

void GpuParticleDatablockWidget::imageButtonClicked()
{
//    QSize dialogSize = data.mWidgets.getQMainWindow()->size();
//    dialogSize.setWidth(dialogSize.width()*0.8);
//    dialogSize.setHeight(dialogSize.height()*0.8);

//    ImageChooserDialog dialog(data, dialogSize, this);
//    int code = dialog.exec();


}

void GpuParticleDatablockWidget::loadFromResourceClicked()
{
    QObject* s = sender();
    QString initPathStr;
    if(s != mLoadLastPathAction) {
        for (int i = 0; i < mLoadFromResourcesActionButton->actions().size(); ++i) {
            if(s == mLoadFromResourcesActionButton->actions()[i]) {
                initPathStr = mLoadFromResourcesActionButton->actions()[i]->text();
                break;
            }
        }
    }

    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Load image file"), initPathStr, "*.*");
    if(!QFile::exists(fileName)) {
        return;
    }

    QFileInfo fileInfo(fileName);
    Ogre::String dirPathStr = fileInfo.dir().absolutePath().toStdString();
    Ogre::String fname = fileInfo.fileName().toStdString();

    Ogre::TextureGpuManager *textureManager =
            data.getRenderSystem()->getTextureGpuManager();


    Ogre::TextureGpu* texture = textureManager->findTextureNoThrow(fname);
    bool forceReloadTexture = false;
    if(texture) {
        // found texture in resources

        QString resourceFilePath;
        bool ok = false;

        if(data.mCustomTextures.contains(fileInfo.fileName())) {
            // texture was loaded manually (not by resource manager).

            if(fileInfo.absoluteFilePath() == data.mCustomTextures[fileInfo.fileName()]) {
                ok = true;
            }
            else {
                resourceFilePath = data.mCustomTextures[fileInfo.fileName()];
            }
        }
        else {
            Ogre::TextureGpuManager::ResourceEntryMap::const_iterator findIt = textureManager->getEntries().find(fname);
            const Ogre::TextureGpuManager::ResourceEntry& entry = findIt->second;

            Ogre::ResourceGroupManager &resourceGroupManager = Ogre::ResourceGroupManager::getSingleton();
            Ogre::Archive* archive = nullptr;

            try {
                archive = resourceGroupManager._getArchiveToResource(entry.name, entry.resourceGroup);
            }
            catch( Ogre::Exception &e ) {

                // Most probably particleDatablock was loaded before texture.
                archive = nullptr;

                forceReloadTexture = true;
            }

            if(!archive) {
                resourceFilePath = "<NoArchiveFound> " + fileInfo.fileName();
            }
            else {
                QString archivePath = QString::fromStdString(archive->getName());

                QFileInfo fileInfo2(archivePath + "/" + fileInfo.fileName());
                if(fileInfo == fileInfo2 ) {
                    // texture in resources
                    ok = true;
                }
                else {
                    resourceFilePath = fileInfo2.absoluteFilePath();
                }
            }
        }

        if(!forceReloadTexture) {
            if(ok) {
                if(!data.mParticleEditorAssets->mTextures.contains(fileInfo.fileName())) {
                    texture->scheduleTransitionTo(Ogre::GpuResidency::Resident);
                    texture->waitForData();

                    Ogre::Image2 image;
                    image.convertFromTexture(texture, 0, 1, true);

                    data.mParticleEditorAssets->generatePixmapToTexture(fileInfo.fileName(), image);
                    if(OgreQtImageHelper::isCompressedFormat(image)) {
            //            QMessageBox::warning(this, tr("Loading texture completed!"), tr("Compressed texture formats may not be displayed as editor's icons!"));
                    }
                }
            }
            else {
                QString msg = tr("Texture with such name already exists!\nChoosen: %1\nResouce: %2").arg(fileInfo.absoluteFilePath()).arg(resourceFilePath);
                QMessageBox::critical(this, tr("Loading texture failed!"), msg);
                return;
            }
        }
    }

    if(!texture || forceReloadTexture)
    {
        // add

        Ogre::FileSystemArchive arch(dirPathStr, "FileSystem", true);
        arch.load();

        Ogre::DataStreamPtr stream = arch.open(fname);

        Ogre::Image2 *imagePtr = new Ogre::Image2();
        try {
            imagePtr->load(stream);
        }
        catch( Ogre::Exception &e )
        {
            QString msg = tr("Couldn't load image file! Ogre Exception occured!\n");
            msg += QString(e.what());
            QMessageBox::critical(this, tr("Loading texture failed!"), msg);
            delete imagePtr;
            return;
        }
        catch (...) {
            QMessageBox::critical(this, tr("Loading texture failed!"), tr("Couldn't load image file! Ogre Exception occured!"));
            delete imagePtr;
            return;
        }

        data.mParticleEditorAssets->generatePixmapToTexture(fileInfo.fileName(), *imagePtr);
        if(OgreQtImageHelper::isCompressedFormat(*imagePtr)) {
//            QMessageBox::warning(this, tr("Loading texture completed!"), tr("Compressed texture formats may not be displayed as editor's icons!"));
        }


        if(forceReloadTexture) {
            textureManager->destroyTexture(texture);
        }

        texture = textureManager->createTexture(
                    fname, fname, Ogre::GpuPageOutStrategy::Discard,
                    Ogre::TextureFlags::AutomaticBatching | Ogre::TextureFlags::PrefersLoadingFromFileAsSRGB,
                    Ogre::TextureTypes::Type2D );

        // Ogre will call "delete imagePtr" when done, because we're passing
        // true to autoDeleteImage argument in scheduleTransitionTo
        texture->scheduleTransitionTo( Ogre::GpuResidency::Resident, imagePtr, true );

        texture->waitForData();

        data.mCustomTextures.insert(fileInfo.fileName(), fileInfo.absoluteFilePath());

        QMessageBox::information(this, tr("Loading texture completed!"), tr("Added texture \"%1\" to resources.").arg(QString::fromStdString(fname)));
    }

    // set info to datablock
    {
        data.mChoosenGpuParticleDatablock->setTexture(0, texture);

        data.mParticleEditorFunctions->refreshParticleDatablocks();
        data.mParticleEditorFunctions->updateParticleDatablocksWidgets();
        data.mParticleEditorFunctions->recalculateParticleDatablockSpriteIcons(data.mChoosenGpuParticleDatablock);
        updateImage();
        updateSpriteTable();
    }

    emit particleDatablockModified();
}

