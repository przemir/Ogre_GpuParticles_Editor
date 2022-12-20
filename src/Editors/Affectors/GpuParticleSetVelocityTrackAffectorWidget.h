/*
 * File: GpuParticleSetVelocityTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETVELOCITYTRACKAFFECTORWIDGET_H
#define GPUPARTICLESETVELOCITYTRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidgetTempl.h"

#include <GpuParticles/Affectors/GpuParticleSetVelocityTrackAffector.h>

class FloatTrackTableWidget;
class QGroupBox;

class GpuParticleSetVelocityTrackAffectorWidget : public GpuParticleAffectorWidgetTempl<GpuParticleSetVelocityTrackAffector>
{
    Q_OBJECT
public:
    GpuParticleSetVelocityTrackAffectorWidget();

    void createGui();

private:
    virtual void affectorToGui() override;

private slots:
    void onAffectorModified();

private:
    QGroupBox* mVelocityTrackGroup;
    FloatTrackTableWidget* mVelocityTrackTableWidget;
};

#endif
