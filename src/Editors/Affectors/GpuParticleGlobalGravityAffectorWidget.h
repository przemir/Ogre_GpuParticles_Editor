/*
 * File: GpuParticleGlobalGravityAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLEGLOBALGRAVITYAFFECTORWIDGET_H
#define GPUPARTICLEGLOBALGRAVITYAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidgetTempl.h"

#include <GpuParticles/Affectors/GpuParticleGlobalGravityAffector.h>

class Point3dWidget;

class GpuParticleGlobalGravityAffectorWidget : public GpuParticleAffectorWidgetTempl<GpuParticleGlobalGravityAffector>
{
    Q_OBJECT
public:
    GpuParticleGlobalGravityAffectorWidget();

    void createGui();

private:
    virtual void affectorToGui() override;

private slots:
    void onAffectorModified();

private:
    Point3dWidget* mGravityWidget;
};

#endif
