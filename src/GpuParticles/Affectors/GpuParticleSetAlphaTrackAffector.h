/*
 * File: GpuParticleSetAlphaTrackAffector.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETALPHATRACKAFFECTOR_H
#define GPUPARTICLESETALPHATRACKAFFECTOR_H

#include "../GpuParticleAffector.h"
#include <map>

class GpuParticleSetAlphaTrackAffector : public GpuParticleAffector
{
public:
    GpuParticleSetAlphaTrackAffector();
    virtual GpuParticleAffector* clone() const override { return new GpuParticleSetAlphaTrackAffector(*this); }

    virtual Ogre::uint32 getAffectorParticleBufferSize() const override { return 0; }

    virtual Ogre::uint32 getAffectorEmitterBufferSize() const override;

    virtual float* prepareAffectorEmitterBuffer(float* buffer) const override;

    /// Property name which will be used in compute and particle shader
    /// to add additional code.
    virtual Ogre::String getAffectorProperty() const override { return "affector_set_alpha_track"; }

#if !OGRE_NO_JSON
    virtual void _loadJson(const rapidjson::Value &json);

    virtual void _saveJson(Ogre::String &outString) const;
#endif

public:
    bool mEnabled;
    typedef GpuParticleAffectorCommon::FloatTrack FloatTrack;
    FloatTrack mAlphaTrack;
};

#endif
