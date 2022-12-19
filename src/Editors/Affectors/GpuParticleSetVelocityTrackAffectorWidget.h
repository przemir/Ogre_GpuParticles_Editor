/*
 * File: GpuParticleSetVelocityTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETVELOCITYTRACKAFFECTORWIDGET_H
#define GPUPARTICLESETVELOCITYTRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidget.h"

class GpuParticleSetVelocityTrackAffector;
class FloatTrackTableWidget;
class QGroupBox;

class GpuParticleSetVelocityTrackAffectorWidget : public GpuParticleAffectorWidget
{
    Q_OBJECT
public:
    GpuParticleSetVelocityTrackAffectorWidget();

    void createGui();

    virtual void setEditedObject(GpuParticleAffector* affector) override;

private:
    void affectorToGui();

private slots:
    void onAffectorModified();

private:
    GpuParticleSetVelocityTrackAffector* mEditedObject = nullptr;

    QGroupBox* mVelocityTrackGroup;
    FloatTrackTableWidget* mVelocityTrackTableWidget;
};

#endif
