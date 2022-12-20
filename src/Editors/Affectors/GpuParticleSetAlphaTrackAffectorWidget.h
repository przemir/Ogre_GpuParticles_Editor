/*
 * File: GpuParticleSetAlphaTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETALPHATRACKAFFECTORWIDGET_H
#define GPUPARTICLESETALPHATRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidgetTempl.h"

#include <GpuParticles/Affectors/GpuParticleSetAlphaTrackAffector.h>

class FloatTrackTableWidget;
class QGroupBox;

class GpuParticleSetAlphaTrackAffectorWidget : public GpuParticleAffectorWidgetTempl<GpuParticleSetAlphaTrackAffector>
{
    Q_OBJECT
public:
    GpuParticleSetAlphaTrackAffectorWidget();

    void createGui();

private:
    virtual void affectorToGui() override;

private slots:
    void onAffectorModified();

private:
    QGroupBox* mAlphaTrackGroup;
    FloatTrackTableWidget* mAlphaTrackTableWidget;
};

#endif
