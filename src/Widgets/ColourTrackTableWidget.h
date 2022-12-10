/*
 * File: ColourTrackTableWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#ifndef COLOURTRACKTABLEWIDGET_H
#define COLOURTRACKTABLEWIDGET_H

#include "TrackTableModel.h"
#include "TrackTableWidget.h"
#include <OgreColourValue.h>

class ColourTrackTableWidget : public TrackTableWidget<Ogre::ColourValue>
{
public:
    ColourTrackTableWidget();

protected:

    virtual Ogre::ColourValue lerp(const Ogre::ColourValue& a, const Ogre::ColourValue& b, float percent) const override;
};

#endif
