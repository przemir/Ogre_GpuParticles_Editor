/*
 * File: GpuParticleDepthCollisionAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLEDEPTHCOLLISIONAFFECTORWIDGET_H
#define GPUPARTICLEDEPTHCOLLISIONAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidget.h"

class GpuParticleDepthCollisionAffector;
class QCheckBox;

class GpuParticleDepthCollisionAffectorWidget : public GpuParticleAffectorWidget
{
    Q_OBJECT
public:
    GpuParticleDepthCollisionAffectorWidget();

    void createGui();

    virtual void setEditedObject(GpuParticleAffector* affector) override;

private:
    void affectorToGui();

private slots:
    void onAffectorModified();

private:
    GpuParticleDepthCollisionAffector* mEditedObject = nullptr;

    QCheckBox* mEnabledCheckBox;
};

#endif
