/*
 * File: GpuParticleSetSizeTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETSIZETRACKAFFECTORWIDGET_H
#define GPUPARTICLESETSIZETRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidgetTempl.h"

#include <GpuParticles/Affectors/GpuParticleSetSizeTrackAffector.h>

class Vector2TrackTableWidget;
class QGroupBox;

class GpuParticleSetSizeTrackAffectorWidget : public GpuParticleAffectorWidgetTempl<GpuParticleSetSizeTrackAffector>
{
    Q_OBJECT
public:
    GpuParticleSetSizeTrackAffectorWidget();

    void createGui();

private:
    virtual void affectorToGui() override;

private slots:
    void onAffectorModified();

private:
    QGroupBox* mSizeTrackGroup;
    Vector2TrackTableWidget* mSizeTrackTableWidget;
};

#endif
