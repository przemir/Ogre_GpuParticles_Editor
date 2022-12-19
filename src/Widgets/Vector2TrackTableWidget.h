/*
 * File: Vector2TrackTableWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#ifndef VECTOR2TRACKTABLEWIDGET_H
#define VECTOR2TRACKTABLEWIDGET_H

#include "TrackTableModel.h"
#include "TrackTableWidget.h"
#include <OgreVector2.h>

class Vector2TrackTableWidget : public TrackTableWidget<Ogre::Vector2>
{
public:
    Vector2TrackTableWidget();

protected:

    virtual Ogre::Vector2 defaultValue() const override;
    virtual Ogre::Vector2 lerp(const Ogre::Vector2& a, const Ogre::Vector2& b, float percent) const override;
};

#endif
