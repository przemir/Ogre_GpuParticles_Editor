/*
 * File: FloatTrackTableWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-20
 *
 */

#ifndef FLOATTRACKTABLEWIDGET_H
#define FLOATTRACKTABLEWIDGET_H

#include "TrackTableModel.h"
#include "TrackTableWidget.h"


class FloatTrackTableWidget : public TrackTableWidget<float>
{
public:
    FloatTrackTableWidget(float minValue, float maxValue, float stepValue);

protected:

    virtual float lerp(const float& a, const float& b, float percent) const override;
};

#endif
