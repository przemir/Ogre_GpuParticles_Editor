/*
 * File: GpuParticleGlobalGravityAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLEGLOBALGRAVITYAFFECTORWIDGET_H
#define GPUPARTICLEGLOBALGRAVITYAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidget.h"

class GpuParticleGlobalGravityAffector;
class Point3dWidget;

class GpuParticleGlobalGravityAffectorWidget : public GpuParticleAffectorWidget
{
    Q_OBJECT
public:
    GpuParticleGlobalGravityAffectorWidget();

    void createGui();

    virtual void setEditedObject(GpuParticleAffector* affector) override;

private:
    void affectorToGui();

private slots:
    void onAffectorModified();

private:
    GpuParticleGlobalGravityAffector* mEditedObject = nullptr;

    Point3dWidget* mGravityWidget;
};

#endif
