/*
 * File: GpuParticleSetColourTrackAffector.h
 * Author: Przemysław Bągard
 * Created: 2022-12-15
 *
 */

#ifndef GPUPARTICLESETCOLOURTRACKAFFECTOR_H
#define GPUPARTICLESETCOLOURTRACKAFFECTOR_H

#include "../GpuParticleAffector.h"
#include <map>
#include <OgreVector3.h>

class GpuParticleSetColourTrackAffector : public GpuParticleAffector
{
public:
    GpuParticleSetColourTrackAffector();
    virtual GpuParticleAffector* clone() const override { return OGRE_NEW GpuParticleSetColourTrackAffector(*this); }

    virtual Ogre::uint32 getAffectorParticleBufferSize() const override { return 0; }

    virtual Ogre::uint32 getAffectorEmitterBufferSize() const override;

    virtual float* prepareAffectorEmitterBuffer(float* buffer) const override;

    /// Property name which will be used in compute and particle shader
    /// to add additional code.
    virtual Ogre::String getAffectorProperty() const override { return "affector_set_colour_track"; }

#if !OGRE_NO_JSON
    virtual void _loadJson(const rapidjson::Value &json);

    virtual void _saveJson(Ogre::String &outString) const;
#endif

public:
    bool mEnabled;
    /// Note that only first GpuParticleAffectorCommon::MaxTrackValues will be used.
    typedef std::map<float, Ogre::Vector3> Vector3Track;
    Vector3Track mColourTrack;
};

#endif
