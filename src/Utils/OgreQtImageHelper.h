/*
 * File: OgreQtImageHelper.h
 * Author: Przemysław Bągard
 * Created: 2021-7-23
 *
 */

#ifndef OGREQTIMAGEHELPER_H
#define OGREQTIMAGEHELPER_H

namespace Ogre {
    class Image2;
}

class QImage;
class QPixmap;

class OgreQtImageHelper
{
public:
    OgreQtImageHelper();

    static void ogreImageToQImage(QImage& result, const Ogre::Image2& ogreImage, int maxSize = 0);
    static void ogreImageToQPixmap(QPixmap& result, const Ogre::Image2& ogreImage, int maxSize = 0);
};

#endif
