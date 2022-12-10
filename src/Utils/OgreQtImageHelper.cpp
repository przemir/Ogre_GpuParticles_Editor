/*
 * File: OgreQtImageHelper.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-23
 *
 */

#include "OgreQtImageHelper.h"

#include <OgreImage2.h>
#include <OgreTextureBox.h>
#include <QImage>
#include <QPixmap>

OgreQtImageHelper::OgreQtImageHelper()
{

}

bool OgreQtImageHelper::isCompressedFormat(const Ogre::Image2& ogreImage)
{
    return Ogre::PixelFormatGpuUtils::isCompressed( ogreImage.getPixelFormat() );
}

void OgreQtImageHelper::ogreImageToQImage(QImage& result, const Ogre::Image2& ogreImage, int maxSize)
{
    Ogre::TextureBox texBox = ogreImage.getData(0);
    result = QImage(texBox.width, texBox.height, QImage::Format_ARGB32);

    for (unsigned int iY = 0; iY < texBox.height; ++iY) {
        for (unsigned int iX = 0; iX < texBox.width; ++iX) {
            Ogre::ColourValue ogreColour = texBox.getColourAt(iX, iY, 0, ogreImage.getPixelFormat());
            QRgb rgb2 = qRgb(ogreColour.r*255, ogreColour.g*255, ogreColour.b*255);
            result.setPixel(iX, iY, rgb2);
        }
    }

    if(maxSize > 0) {
        result = result.scaled(maxSize, maxSize);
    }
}

void OgreQtImageHelper::ogreImageToQPixmap(QPixmap& result, const Ogre::Image2& ogreImage, int maxSize)
{
    QImage qImage;
    ogreImageToQImage(qImage, ogreImage, maxSize);
    result = QPixmap::fromImage(qImage);
}
