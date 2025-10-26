/*
 * File: GpuParticleGlobalGravityAffector.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLEGLOBALGRAVITYAFFECTOR_H
#define GPUPARTICLEGLOBALGRAVITYAFFECTOR_H

#include "../GpuParticleAffector.h"
#include <map>

class GpuParticleGlobalGravityAffector : public GpuParticleAffector
{
public:
    GpuParticleGlobalGravityAffector();
    virtual GpuParticleAffector* clone() const override { return OGRE_NEW GpuParticleGlobalGravityAffector(*this); }

    virtual Ogre::uint32 getAffectorParticleBufferSize() const override { return 0; }

    virtual Ogre::uint32 getAffectorEmitterBufferSize() const override;

    virtual float* prepareAffectorEmitterBuffer(float* buffer) const override;

    /// Property name which will be used in compute and particle shader
    /// to add additional code.
    virtual Ogre::String getAffectorProperty() const override { return "affector_global_gravity"; }

#if !OGRE_NO_JSON
    virtual void _loadJson(const rapidjson::Value &json);

    virtual void _saveJson(Ogre::String &outString) const;
#endif

public:
    Ogre::Vector3 mGravity;
};

#endif
