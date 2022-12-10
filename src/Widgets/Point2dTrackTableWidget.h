/*
 * File: Point2dTrackTableWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#ifndef POINT2DTRACKTABLEWIDGET_H
#define POINT2DTRACKTABLEWIDGET_H

#include "TrackTableModel.h"
#include "TrackTableWidget.h"
#include <OgreVector2.h>

class Point2dTrackTableWidget : public TrackTableWidget<Ogre::Vector2>
{
public:
    Point2dTrackTableWidget();

protected:

    virtual Ogre::Vector2 defaultValue() const override;
    virtual Ogre::Vector2 lerp(const Ogre::Vector2& a, const Ogre::Vector2& b, float percent) const override;
};

#endif
