/*
 * File: GpuParticleSetSizeTrackAffector.h
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#ifndef GPUPARTICLESETSIZETRACKAFFECTOR_H
#define GPUPARTICLESETSIZETRACKAFFECTOR_H

#include "../GpuParticleAffector.h"
#include <map>
#include <OgreVector2.h>

class GpuParticleSetSizeTrackAffector : public GpuParticleAffector
{
public:
    GpuParticleSetSizeTrackAffector();
    virtual GpuParticleAffector* clone() const override { return OGRE_NEW GpuParticleSetSizeTrackAffector(*this); }

    virtual AffectorType getType() const override { return AFFECTOR_SET_SIZE_TRACK; }

    virtual Ogre::uint32 getAffectorParticleBufferSize() const override { return 0; }

    virtual Ogre::uint32 getAffectorEmitterBufferSize() const override;

    virtual float* prepareAffectorEmitterBuffer(float* buffer) const override;

    /// Property name which will be used in compute and particle shader
    /// to add additional code.
    virtual Ogre::String getAffectorProperty() const override { return "affector_set_size_track"; }

#if !OGRE_NO_JSON
    virtual void _loadJson(const rapidjson::Value &json);

    virtual void _saveJson(Ogre::String &outString) const;
#endif

public:
    bool mEnabled;
    typedef std::map<float, Ogre::Vector2> Vector2Track;
    Vector2Track mSizeTrack;
};

#endif
