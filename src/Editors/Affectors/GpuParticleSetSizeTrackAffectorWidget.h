/*
 * File: GpuParticleSetSizeTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETSIZETRACKAFFECTORWIDGET_H
#define GPUPARTICLESETSIZETRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidget.h"

class GpuParticleSetSizeTrackAffector;
class Vector2TrackTableWidget;
class QGroupBox;

class GpuParticleSetSizeTrackAffectorWidget : public GpuParticleAffectorWidget
{
    Q_OBJECT
public:
    GpuParticleSetSizeTrackAffectorWidget();

    void createGui();

    virtual void setEditedObject(GpuParticleAffector* affector) override;

private:
    void affectorToGui();

private slots:
    void onAffectorModified();

private:
    GpuParticleSetSizeTrackAffector* mEditedObject = nullptr;

    QGroupBox* mSizeTrackGroup;
    Vector2TrackTableWidget* mSizeTrackTableWidget;
};

#endif
