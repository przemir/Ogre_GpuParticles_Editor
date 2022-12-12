/*
 * File: ParticleEditorAssets.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-23
 *
 */

#include "ParticleEditorAssets.h"

#include <OgreQtImageHelper.h>
#include <QPainter>

const int ParticleEditorAssets::TextureData::PixmapSize = 128;

ParticleEditorAssets::ParticleEditorAssets()
{
    {
        mNoTextureIconPixmap = QPixmap(TextureData::PixmapSize, TextureData::PixmapSize);
        mNoTextureIconPixmap.fill(QColor(255, 255, 255, 255));

        QPainter painter( &mNoTextureIconPixmap );
        painter.setFont( QFont("Arial", 16) );
        painter.setBrush(QBrush(Qt::black));
        painter.drawText( QPoint(25, 100), "Empty" );
    }

    {
        mCompressedTextureIconPixmap = QPixmap(TextureData::PixmapSize, TextureData::PixmapSize);
        mCompressedTextureIconPixmap.fill(QColor(128, 128, 128, 255));

        QPainter painter( &mCompressedTextureIconPixmap );
        painter.setFont( QFont("Arial", 12) );
        painter.setBrush(QBrush(Qt::black));
        painter.drawText( QPoint(10, 100), "Compressed" );
    }

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

void ParticleEditorAssets::generatePixmapToTexture(const QString& name, const Ogre::Image2& image)
{
    TextureData& textureData = mTextures[name];
    if(OgreQtImageHelper::isCompressedFormat(image)) {
        textureData.mOriginalSizePixmap = mCompressedTextureIconPixmap;
        textureData.mIconPixmap = mCompressedTextureIconPixmap;
    }
    else {
        OgreQtImageHelper::ogreImageToQPixmap(textureData.mOriginalSizePixmap, image, -1);
        textureData.mIconPixmap = textureData.mOriginalSizePixmap.scaled(ParticleEditorAssets::TextureData::PixmapSize, ParticleEditorAssets::TextureData::PixmapSize);
    }
}
