/*
 * File: GpuParticleAffectorWidgetTempl.h
 * Author: Przemysław Bągard
 * Created: 2022-12-20
 *
 */

#ifndef GPUPARTICLEAFFECTORWIDGETTEMPL_H
#define GPUPARTICLEAFFECTORWIDGETTEMPL_H

#include <Editors/GpuParticleAffectorWidget.h>


template <class T>
class GpuParticleAffectorWidgetTempl : public GpuParticleAffectorWidget
{
public:
    GpuParticleAffectorWidgetTempl() {

    }

    virtual void setEditedObject(GpuParticleAffector* affector) override {
        mEditedObject = dynamic_cast<T*>(affector);

        setEnabled(mEditedObject);

        affectorToGui();
    }

    virtual Ogre::String getAffectorPropertyName() const override {
        return mDummy.getAffectorProperty();
    }

protected:

    virtual void affectorToGui() = 0;

protected:
    T mDummy;
    T* mEditedObject = nullptr;
};

#endif
