/*
 * File: ParticleEditorAssets.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-23
 *
 */

#include "ParticleEditorAssets.h"

const int ParticleEditorAssets::TextureData::PixmapSize = 128;

ParticleEditorAssets::ParticleEditorAssets()
{
    mNoTextureIconPixmap = QPixmap(TextureData::PixmapSize, TextureData::PixmapSize);
    mNoTextureIconPixmap.fill(QColor(255, 255, 255, 255));
}

ParticleEditorAssets::TextureData* ParticleEditorAssets::getTexture(const QString& name)
{
    QMap<QString, TextureData>::iterator it = mTextures.find(name);
    if(it != mTextures.end()) {
        return &it.value();
    }
    return nullptr;
}

ParticleEditorAssets::DatablockData* ParticleEditorAssets::getDatablock(const QString& name)
{
    QMap<QString, DatablockData>::iterator it = mDatablocks.find(name);
    if(it != mDatablocks.end()) {
        return &it.value();
    }
    return nullptr;
}
