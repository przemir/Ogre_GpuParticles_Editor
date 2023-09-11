/*
 * File: GpuParticleAffector.h
 * Author: Przemysław Bągard
 * Created: 2022-12-14
 *
 */

#ifndef GPUPARTICLEAFFECTOR_H
#define GPUPARTICLEAFFECTOR_H

#include <OgreCommon.h>
#include "GpuParticleAffectorCommon.h"

#if !OGRE_NO_JSON

// Forward declaration for |Document|.
namespace rapidjson
{
    class CrtAllocator;
    template <typename> class MemoryPoolAllocator;
    template <typename> struct UTF8;
    //template <typename, typename, typename> class GenericDocument;
    //typedef GenericDocument< UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator > Document;

    template <typename BaseAllocator> class MemoryPoolAllocator;
    template <typename Encoding, typename>  class GenericValue;
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;
}

#endif

class GpuParticleAffector
{
public:
    GpuParticleAffector();
    virtual ~GpuParticleAffector();
    virtual GpuParticleAffector* clone() const = 0;

    /// Additional space per particle (every particle hold by GpuParticleSystemWorld).
    /// Should be aligned to sizeof(float)*4.
    virtual Ogre::uint32 getAffectorParticleBufferSize() const { return 0; }

    /// Additional space per emitter core (not instance).
    /// Should be aligned to sizeof(float)*4.
    virtual Ogre::uint32 getAffectorEmitterBufferSize() const { return 0; }

    /// Uploads affector data to emitter core struct in gpu.
    virtual float* prepareAffectorEmitterBuffer(float* buffer) const { return buffer; }

    /// Property name which will be used in compute and particle shader
    /// to add additional code.
    virtual Ogre::String getAffectorProperty() const = 0;

#if !OGRE_NO_JSON
    virtual void _loadJson(const rapidjson::Value &json) {}

    virtual void _saveJson(Ogre::String &outString) const {}
#endif

};

#endif
