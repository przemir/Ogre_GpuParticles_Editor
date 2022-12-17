/*
 * File: GpuParticleAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-15
 *
 */

#ifndef GPUPARTICLEAFFECTORWIDGET_H
#define GPUPARTICLEAFFECTORWIDGET_H

#include <QWidget>

class GpuParticleAffector;

class GpuParticleAffectorWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleAffectorWidget();

    static const int MinimumTrackHeight = 200;

    virtual void setEditedObject(GpuParticleAffector* affector) = 0;

signals:
    void affectorModified();
};

#endif
