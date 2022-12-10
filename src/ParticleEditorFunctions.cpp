/*
 * File: ParticleEditorFunctions.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-2
 *
 */

#include "ParticleEditorAssets.h"
#include "ParticleEditorFunctions.h"

#include <GpuParticles/Hlms/HlmsParticle.h>
#include <GpuParticles/Hlms/HlmsParticleDatablock.h>

#include <OgreHlmsManager.h>
#include <OgreImage2.h>
#include <OgreQtImageHelper.h>
#include <OgreRoot.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>

#include <Editors/GpuParticleDatablocksListWidget.h>
#include <Editors/GpuParticleEmitterWidget.h>
#include <Editors/GpuParticleSystemsListWidget.h>
#include <Editors/ParticleEditorData.h>

#include <GpuParticles/GpuParticleSystem.h>
#include <GpuParticles/GpuParticleSystemResourceManager.h>

ParticleEditorFunctions::ParticleEditorFunctions(ParticleEditorData& _data)
    : data(_data)
{

}

void ParticleEditorFunctions::updateParticleSystemsWidgets()
{
    data.mWidgets.mGpuParticleSystemsListWidget->refreshList();
}

void ParticleEditorFunctions::refreshParticleDatablocks()
{
    data.mDatablockList.clear();

    QVector<QPair<QString, Ogre::TextureGpu*> > textures;
    QVector<const HlmsParticleDatablock*> particleDatablocks;

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType) );
    for(Ogre::Hlms::HlmsDatablockMap::const_iterator it = hlmsParticle->getDatablockMap().begin();
        it != hlmsParticle->getDatablockMap().end(); ++it) {

        const Ogre::Hlms::DatablockEntry& entry = it->second;

        const Ogre::HlmsDatablock* datablock = entry.datablock;
        if(datablock == hlmsParticle->getDefaultDatablock()) {
            continue;
        }

        if(datablock->mType == HlmsParticle::ParticleHlmsType) {
            const Ogre::String& str = entry.name;
            const HlmsParticleDatablock* particleDatablock = dynamic_cast<const HlmsParticleDatablock*>(datablock);
            QString datablockName = QString::fromStdString(str);

            Ogre::TextureGpu* texture = datablock->getEmissiveTexture();
            if(particleDatablock && texture) {
                QString textureName = QString::fromStdString(texture->getNameStr());
                data.mDatablockList.push_back(qMakePair(datablockName, textureName));

                if(!checkIfTextureFileExists(texture)) {
                    continue;
                }

                if(!data.mParticleEditorAssets->mTextures.contains(textureName)) {
                    textures.push_back(qMakePair(textureName, texture));
                    texture->scheduleTransitionTo(Ogre::GpuResidency::Resident);
                }

                if(!data.mParticleEditorAssets->mDatablocks.contains(datablockName)) {
                    particleDatablocks.push_back(particleDatablock);
                }
            }
            else if(particleDatablock) {
                data.mDatablockList.push_back(qMakePair(datablockName, QString()));
            }
        }
    }

    for (int i = 0; i < textures.size(); ++i) {
        QString name = textures[i].first;
        Ogre::TextureGpu* texture = textures[i].second;

        texture->waitForData();

        Ogre::Image2 image;
        image.convertFromTexture(texture, 0, 1, true);

        if(OgreQtImageHelper::isCompressedFormat(image)) {
            continue;
        }

        ParticleEditorAssets::TextureData& textureData = data.mParticleEditorAssets->mTextures[name];

        OgreQtImageHelper::ogreImageToQPixmap(textureData.mOriginalSizePixmap, image, -1);
        textureData.mIconPixmap = textureData.mOriginalSizePixmap.scaled(ParticleEditorAssets::TextureData::PixmapSize, ParticleEditorAssets::TextureData::PixmapSize);
    }

    for (int i = 0; i < particleDatablocks.size(); ++i) {
        const HlmsParticleDatablock* particleDatablock = particleDatablocks[i];
        data.mParticleEditorFunctions->recalculateParticleDatablockSpriteIcons(particleDatablock);
    }
}

void ParticleEditorFunctions::updateParticleDatablocksWidgets()
{
    QStringList datablockNames;
    QVector<QPixmap> icons;
    datablockNames.reserve(data.mDatablockList.size());
    icons.reserve(data.mDatablockList.size());
    for (int i = 0; i < data.mDatablockList.size(); ++i) {
        datablockNames.push_back(data.mDatablockList[i].first);
        QString name = data.mDatablockList[i].second;
        QPixmap pixmap;
        if(data.mParticleEditorAssets->mTextures.contains(name)) {
            pixmap = data.mParticleEditorAssets->mTextures[name].mIconPixmap;
        }
        else {
            pixmap = data.mParticleEditorAssets->mNoTextureIconPixmap;
        }

        icons.push_back(pixmap);
    }
    data.mWidgets.mGpuParticleEmitterWidget->setDatablockList(datablockNames, icons);
    data.mWidgets.mGpuParticleDatablocksListWidget->setDatablockList(datablockNames, icons);
}

void ParticleEditorFunctions::loadAllUnloadedTextures()
{
    Ogre::TextureGpuManager *textureManager =
            data.getRenderSystem()->getTextureGpuManager();

    QVector<QPair<QString, Ogre::TextureGpu*> > textures;

    const Ogre::TextureGpuManager::ResourceEntryMap& entries = textureManager->getEntries();
    for(Ogre::TextureGpuManager::ResourceEntryMap::const_iterator it = entries.begin();
        it != entries.end(); ++it) {

        const Ogre::TextureGpuManager::ResourceEntry& entry = it->second;

        if(!entry.texture) {
            continue;
        }

        QString textureName = QString::fromStdString(entry.name);
        Ogre::TextureGpu* texture = entry.texture;

        if(!data.mParticleEditorAssets->mTextures.contains(textureName)) {
            textures.push_back(qMakePair(textureName, texture));
        }
    }

//    for (int i = 0; i < textures.size(); ++i) {
//        textures[i].second->scheduleTransitionTo(Ogre::GpuResidency::OnSystemRam);
//    }

    for (int i = 0; i < textures.size() && i < 10; ++i) {
        QString name = textures[i].first;

        Ogre::ResourceGroupManager &resourceGroupManager = Ogre::ResourceGroupManager::getSingleton();
        Ogre::Archive* archive = resourceGroupManager._getArchiveToResource(textures[i].second->getNameStr());

        QString archiveName = QString::fromStdString(archive->getName());
        archiveName += "/" + name;


        archive->load();
        Ogre::DataStreamPtr stream = archive->open(textures[i].second->getNameStr());

////        bool QPixmap::loadFromData(const uchar *data, uint len, const char *format = Q_NULLPTR, Qt::ImageConversionFlags flags = Qt::AutoColor)

        Ogre::Image2 image;
        image.load(stream);

        if(OgreQtImageHelper::isCompressedFormat(image)) {
            continue;
        }

        if(image.getWidth() > 128) {
            float percent = 128.0 / (float)image.getWidth();
            image.resize(128, image.getHeight()*percent);
        }

        ParticleEditorAssets::TextureData& textureData = data.mParticleEditorAssets->mTextures[name];

//        QIcon icon(archiveName);
//        textureData.mOriginalSizePixmap = QPixmap(archiveName);

        OgreQtImageHelper::ogreImageToQPixmap(textureData.mOriginalSizePixmap, image, -1);
        textureData.mIconPixmap = textureData.mOriginalSizePixmap.scaled(ParticleEditorAssets::TextureData::PixmapSize, ParticleEditorAssets::TextureData::PixmapSize);

//        textureData.mIcon = QIcon(archiveName);
//        icon;
    }
}

void ParticleEditorFunctions::getAllTexturePaths(std::set<Ogre::String>& pathSet)
{
    Ogre::TextureGpuManager *textureManager =
            data.getRenderSystem()->getTextureGpuManager();

    Ogre::ResourceGroupManager &resourceGroupManager = Ogre::ResourceGroupManager::getSingleton();

    const Ogre::TextureGpuManager::ResourceEntryMap& entries = textureManager->getEntries();
    for(Ogre::TextureGpuManager::ResourceEntryMap::const_iterator it = entries.begin();
        it != entries.end(); ++it) {

        const Ogre::TextureGpuManager::ResourceEntry& entry = it->second;

        if(!entry.texture) {
            continue;
        }

        Ogre::Archive* archive = nullptr;
        try {
            archive = resourceGroupManager._getArchiveToResource(entry.name, entry.resourceGroup);
        }
        catch(...) {
            // ignore
        }

        if(archive) {
            pathSet.insert(archive->getName());
        }
    }
}

void ParticleEditorFunctions::loadTexture(const Ogre::String& initPath)
{
    QString initPathStr;
    if(!initPath.empty()) {
        initPathStr = QString::fromStdString(initPath);
    }


}

bool ParticleEditorFunctions::checkIfTextureFileExists(const Ogre::TextureGpu* texture) const
{
    Ogre::ResourceGroupManager &resourceGroupManager = Ogre::ResourceGroupManager::getSingleton();

    Ogre::TextureGpuManager *textureManager =
            data.getRenderSystem()->getTextureGpuManager();

    Ogre::String name, resourceGroup;
    Ogre::TextureGpuManager::ResourceEntryMap::const_iterator itor = textureManager->getEntries().find( texture->getName() );
    if( itor != textureManager->getEntries().end() )
    {
        name = itor->second.name;
        resourceGroup = itor->second.resourceGroup;
    }

    if(name.empty()) {
        return false;
    }

    return resourceGroupManager.resourceExistsInAnyGroup(name) || data.mCustomTextures.contains(QString::fromStdString(name));
}

void ParticleEditorFunctions::recalculateParticleDatablockSpriteIcons(const HlmsParticleDatablock* particleDatablock)
{
    QString name = QString::fromStdString(*particleDatablock->getNameStr());
    ParticleEditorAssets::DatablockData& datablockData = data.mParticleEditorAssets->mDatablocks[name];
    datablockData.mSprites.clear();

    if(!particleDatablock) {
        return;
    }

    if(!particleDatablock->getEmissiveTexture()) {
        if(particleDatablock->getIsFlipbook()) {
            HlmsParticleDatablock::SpriteCoord flipbookSize = particleDatablock->getFlipbookSize();
            for (size_t row = 0; row < flipbookSize.row; ++row) {
                for (size_t col = 0; col < flipbookSize.col; ++col) {
                    datablockData.mSprites.push_back(data.mParticleEditorAssets->mNoTextureIconPixmap);
                }
            }
        }
        else {
            const HlmsParticleDatablock::SpriteList& spriteList = particleDatablock->getSprites();
            for(HlmsParticleDatablock::SpriteList::const_iterator it = spriteList.begin();
                it != spriteList.end(); ++it) {
                datablockData.mSprites.push_back(data.mParticleEditorAssets->mNoTextureIconPixmap);
            }
        }
        return;
    }

    QString textureName = QString::fromStdString(particleDatablock->getEmissiveTexture()->getNameStr());
    ParticleEditorAssets::TextureData& textureData = data.mParticleEditorAssets->mTextures[textureName];

    if(particleDatablock->getIsFlipbook()) {
        HlmsParticleDatablock::SpriteCoord flipbookSize = particleDatablock->getFlipbookSize();

        int width = textureData.mOriginalSizePixmap.width();
        int height = textureData.mOriginalSizePixmap.height();

        int sizeX = (1.0f / (float)flipbookSize.col) * width;
        int sizeY = (1.0f / (float)flipbookSize.row) * height;

        for (size_t row = 0; row < flipbookSize.row; ++row) {
            float yPercent = (float)row / (float)flipbookSize.row;
            for (size_t col = 0; col < flipbookSize.col; ++col) {
                float xPercent = (float)col / (float)flipbookSize.col;

                QRect rect(xPercent*width, yPercent*height, sizeX, sizeY);

                QPixmap spritePixmap = textureData.mOriginalSizePixmap.copy(rect);

                datablockData.mSprites.push_back(spritePixmap);
            }
        }
    }
    else {
        const HlmsParticleDatablock::SpriteList& spriteList = particleDatablock->getSprites();
        for(HlmsParticleDatablock::SpriteList::const_iterator it = spriteList.begin();
            it != spriteList.end(); ++it) {

            const HlmsParticleDatablock::Sprite& sprite = *it;

            QRect rect(sprite.mLeft, sprite.mBottom, sprite.mSizeX, sprite.mSizeY);

            QPixmap spritePixmap = textureData.mOriginalSizePixmap.copy(rect);

            datablockData.mSprites.push_back(spritePixmap);
        }
    }
}

void ParticleEditorFunctions::fillParticleSystemNames(std::set<Ogre::String>& result)
{
    const GpuParticleSystemResourceManager::GpuParticleSystemMap& gpuParticleSystemMap = GpuParticleSystemResourceManager::getSingleton().getGpuParticleSystems();
    for(GpuParticleSystemResourceManager::GpuParticleSystemMap::const_iterator it = gpuParticleSystemMap.begin();
        it != gpuParticleSystemMap.end(); ++it) {

        result.insert(it->second.name);
    }
}

void ParticleEditorFunctions::fillDatablockNames(std::set<Ogre::String>& result, Ogre::HlmsTypes hlmsType)
{
    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    Ogre::Hlms* hlms = hlmsManager->getHlms(hlmsType);

    // Iterate through all pbs datablocks
    Ogre::Hlms::HlmsDatablockMap::const_iterator itorPbs = hlms->getDatablockMap().begin();
    Ogre::Hlms::HlmsDatablockMap::const_iterator endPbs = hlms->getDatablockMap().end();
    while( itorPbs != endPbs)
    {
        Ogre::HlmsDatablock* datablock = itorPbs->second.datablock;
        result.insert(*datablock->getNameStr());
        ++itorPbs;
    }
}

void ParticleEditorFunctions::fillParticleSystemsUsingDatablock(std::map<Ogre::String, const GpuParticleSystem*>& result, const Ogre::String& datablockName)
{
    const GpuParticleSystemResourceManager::GpuParticleSystemMap& gpuParticleSystemMap = GpuParticleSystemResourceManager::getSingleton().getGpuParticleSystems();
    for(GpuParticleSystemResourceManager::GpuParticleSystemMap::const_iterator it = gpuParticleSystemMap.begin();
        it != gpuParticleSystemMap.end(); ++it) {

        const Ogre::String& name = it->second.name;
        const GpuParticleSystem* particleSystem = it->second.gpuParticleSystem;

        for (size_t i = 0; i < particleSystem->getEmitters().size(); ++i) {
            if(particleSystem->getEmitters()[i]->mDatablockName == datablockName) {
                // found
                result.insert(std::make_pair(name, particleSystem));
                break;
            }
        }
    }
}

