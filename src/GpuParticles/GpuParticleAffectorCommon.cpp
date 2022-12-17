/*
 * File: GpuParticleAffectorCommon.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-14
 *
 */

#include "GpuParticleAffectorCommon.h"

void GpuParticleAffectorCommon::uploadFloatTrack(float*& buffer, const std::map<float, float>& track, float defaultStartValue)
{
    // Track times
    {
        float lastTimeValue = 0.0f;
        size_t i = 0;
        for(std::map<float, float>::const_iterator it = track.begin();
            it != track.end(); ++it, ++i) {
            if(i >= MaxTrackValues) {
                break;
            }

            lastTimeValue = it->first;
            *buffer++ = lastTimeValue;
        }
        for (; i < MaxTrackValues; ++i) {
            *buffer++ = lastTimeValue;
        }
    }

    // Track values
    {
        float lastValue = defaultStartValue;
        size_t i = 0;
        for(std::map<float, float>::const_iterator it = track.begin();
            it != track.end(); ++it, ++i) {
            if(i >= MaxTrackValues) {
                break;
            }

            lastValue = it->second;
            *buffer++ = lastValue;
        }
        for (; i < MaxTrackValues; ++i) {
            *buffer++ = lastValue;
        }
    }
}

void GpuParticleAffectorCommon::uploadVector2Track(float*& buffer, const std::map<float, Ogre::Vector2>& track)
{
    uploadTrack<Ogre::Vector2, 2, MaxTrackValues>(buffer, track, Ogre::Vector2::ZERO);
}

void GpuParticleAffectorCommon::uploadVector3Track(float*& buffer, const std::map<float, Ogre::Vector3>& track, const Ogre::Vector3& defaultStartValue)
{
    uploadTrack<Ogre::Vector3, 3, MaxTrackValues>(buffer, track, defaultStartValue);
}

void GpuParticleAffectorCommon::uploadU32ToFloatArray(float*& buffer, Ogre::uint32 value)
{
    //#define AS_U32PTR( x ) reinterpret_cast<uint32*RESTRICT_ALIAS>(x)
    //#endif
    //    *AS_U32PTR( buffer ) = emitterCore->mUniformSize ? 1u : 0u;      ++buffer;

    *(reinterpret_cast<Ogre::uint32*RESTRICT_ALIAS>(buffer)) = value;
    ++buffer;
}
