/*
 * File: GpuParticleAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-15
 *
 */

#ifndef GPUPARTICLEAFFECTORWIDGET_H
#define GPUPARTICLEAFFECTORWIDGET_H

#include <QWidget>
#include <OgreString.h>

class GpuParticleAffector;

class GpuParticleAffectorWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleAffectorWidget();
    virtual ~GpuParticleAffectorWidget();

    static const int MinimumTrackHeight = 200;

    virtual void setEditedObject(GpuParticleAffector* affector) = 0;

    virtual Ogre::String getAffectorPropertyName() const = 0;

protected:

    /// Updating gui from mEditedObject.
    virtual void affectorToGui() = 0;

signals:
    void affectorModified();
};

#endif
