/*
 * File: ParticleEditorAssets.h
 * Author: Przemysław Bągard
 * Created: 2021-7-23
 *
 */

#ifndef PARTICLEEDITORASSETS_H
#define PARTICLEEDITORASSETS_H

#include <QPixmap>
#include <QMap>
#include <QIcon>


class ParticleEditorAssets
{
public:

    /// Texture file - images.
    class TextureData
    {
    public:

        static const int PixmapSize;

        QPixmap mOriginalSizePixmap;
        QPixmap mIconPixmap;
//        QIcon mIcon;
    };

    /// ParticleDatablock data (sprites).
    class DatablockData
    {
    public:
        QVector<QPixmap> mSprites;
    };

public:
    ParticleEditorAssets();

    QMap<QString, TextureData> mTextures;
    QMap<QString, DatablockData> mDatablocks;

    QPixmap mNoTextureIconPixmap;

    TextureData* getTexture(const QString& name);
    DatablockData* getDatablock(const QString& name);
};

#endif
