/*
 * File: GpuParticleSetSizeTrackAffector.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include <GpuParticles/Affectors/GpuParticleSetSizeTrackAffector.h>
#include <GpuParticles/GpuParticleSystemJsonManager.h>
#if !OGRE_NO_JSON
#include "rapidjson/document.h"
#endif

GpuParticleSetSizeTrackAffector::GpuParticleSetSizeTrackAffector()
    : mEnabled(false)
{

}

Ogre::uint32 GpuParticleSetSizeTrackAffector::getAffectorEmitterBufferSize() const
{
    return sizeof(Ogre::uint32) * 1u +     // mEnabled (bool)
           sizeof(float) * 8u +            // Size track times
           sizeof(float) * 2u * 8u;        // Size track values
}

float* GpuParticleSetSizeTrackAffector::prepareAffectorEmitterBuffer(float* buffer) const {

    GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) mEnabled);
    GpuParticleAffectorCommon::uploadVector2Track(buffer, mSizeTrack, Ogre::Vector2(1.0f, 1.0f));

    return buffer;
}

#if !OGRE_NO_JSON
void GpuParticleSetSizeTrackAffector::_loadJson(const rapidjson::Value& json)
{
    rapidjson::Value::ConstMemberIterator itor;

    itor = json.FindMember("enabled");
    if (itor != json.MemberEnd() && itor->value.IsBool())
    {
        mEnabled = itor->value.GetBool();
    }

    itor = json.FindMember("sizeTrack");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        GpuParticleSystemJsonManager::readVector2Track(itor->value, mSizeTrack);
    }
}

void GpuParticleSetSizeTrackAffector::_saveJson(Ogre::String& outString) const
{
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("enabled", outString);
    GpuParticleSystemJsonManager::toStr(mEnabled, outString);

    outString += ",";
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("sizeTrack", outString);
    GpuParticleSystemJsonManager::writeVector2Track(mSizeTrack, outString);
}
#endif
