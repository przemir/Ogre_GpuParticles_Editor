/*
 * File: GpuParticleAffectorCommon.h
 * Author: Przemysław Bągard
 * Created: 2022-12-14
 *
 */

#ifndef GPUPARTICLEAFFECTORCOMMON_H
#define GPUPARTICLEAFFECTORCOMMON_H

#include <OgrePlatform.h>
#include <OgreVector2.h>
#include <OgreVector3.h>

#include <map>

class GpuParticleAffector;

class GpuParticleAffectorCommon
{
public:
    static const int MaxTrackValues = 8;

    /// Note that affector may clip to MaxTrackValues during upload to gpu.
    typedef std::map<float, float> FloatTrack;
    typedef std::map<float, Ogre::Vector2> Vector2Track;
    typedef std::map<float, Ogre::Vector3> Vector3Track;

    template <class T, int Elements, int Size>
    static void uploadTrack( float *RESTRICT_ALIAS &buffer, const std::map<float, T> &track,
                             const T &defaultStartValue );
    static void uploadVector2Track( float *RESTRICT_ALIAS &buffer,
                                    const std::map<float, Ogre::Vector2> &track,
                                    const Ogre::Vector2 &defaultStartValue = Ogre::Vector2::ZERO );
    static void uploadVector3Track( float *RESTRICT_ALIAS &buffer,
                                    const std::map<float, Ogre::Vector3> &track,
                                    const Ogre::Vector3 &defaultStartValue = Ogre::Vector3::ZERO );
    static void uploadFloatTrack( float *RESTRICT_ALIAS &buffer, const std::map<float, float> &track,
                                  float defaultStartValue );
    static void uploadU32ToFloatArray( float *RESTRICT_ALIAS &buffer, Ogre::uint32 value );
};

template <class T, int Elements, int Size>
void GpuParticleAffectorCommon::uploadTrack( float *RESTRICT_ALIAS &buffer,
                                             const std::map<float, T> &track,
                                             const T &defaultStartValue )
{
    // Track times
    {
        float lastTimeValue = 0.0f;
        size_t i = 0;
        for( typename std::map<float, T>::const_iterator it = track.begin(); it != track.end();
             ++it, ++i )
        {
            if( i >= Size )
            {
                break;
            }

            lastTimeValue = it->first;
            *buffer++ = lastTimeValue;
        }
        for( ; i < Size; ++i )
        {
            // add second to each next value to avoid 0-length
            lastTimeValue += 1.0f;
            *buffer++ = lastTimeValue;
        }
    }

    // Track values
    T lastValue = defaultStartValue;
    for( int k = 0; k < Elements; ++k )
    {
        size_t i = 0;
        for( typename std::map<float, T>::const_iterator it = track.begin(); it != track.end();
             ++it, ++i )
        {
            if( i >= Size )
            {
                break;
            }

            lastValue[k] = it->second[k];
            *buffer++ = lastValue[k];
        }
        for( ; i < Size; ++i )
        {
            *buffer++ = lastValue[k];
        }
    }
}

#endif
