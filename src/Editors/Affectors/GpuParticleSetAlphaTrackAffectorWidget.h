/*
 * File: GpuParticleSetAlphaTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETALPHATRACKAFFECTORWIDGET_H
#define GPUPARTICLESETALPHATRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidget.h"

class GpuParticleSetAlphaTrackAffector;
class FloatTrackTableWidget;
class QGroupBox;

class GpuParticleSetAlphaTrackAffectorWidget : public GpuParticleAffectorWidget
{
    Q_OBJECT
public:
    GpuParticleSetAlphaTrackAffectorWidget();

    void createGui();

    virtual void setEditedObject(GpuParticleAffector* affector) override;

private:
    void affectorToGui();

private slots:
    void onAffectorModified();

private:
    GpuParticleSetAlphaTrackAffector* mEditedObject = nullptr;

    QGroupBox* mAlphaTrackGroup;
    FloatTrackTableWidget* mAlphaTrackTableWidget;
};

#endif
