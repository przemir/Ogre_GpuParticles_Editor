/*
 * File: GpuParticleDepthCollisionAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLEDEPTHCOLLISIONAFFECTORWIDGET_H
#define GPUPARTICLEDEPTHCOLLISIONAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidgetTempl.h"

#include <GpuParticles/Affectors/GpuParticleDepthCollisionAffector.h>

class QCheckBox;

class GpuParticleDepthCollisionAffectorWidget : public GpuParticleAffectorWidgetTempl<GpuParticleDepthCollisionAffector>
{
    Q_OBJECT
public:
    GpuParticleDepthCollisionAffectorWidget();

    void createGui();

private:
    virtual void affectorToGui() override;

private slots:
    void onAffectorModified();

private:
    QCheckBox* mEnabledCheckBox;
};

#endif
