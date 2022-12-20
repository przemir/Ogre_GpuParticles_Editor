/*
 * File: GpuParticleSetVelocityTrackAffector.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include <GpuParticles/Affectors/GpuParticleSetVelocityTrackAffector.h>
#include <GpuParticles/GpuParticleSystemJsonManager.h>
#if !OGRE_NO_JSON
#include "rapidjson/document.h"
#endif

GpuParticleSetVelocityTrackAffector::GpuParticleSetVelocityTrackAffector()
    : mEnabled(false)
{

}

Ogre::uint32 GpuParticleSetVelocityTrackAffector::getAffectorEmitterBufferSize() const
{
    return sizeof(Ogre::uint32) * 1u +     // mEnabled (bool)
           sizeof(float) * 8u +            // Velocity track times
           sizeof(float) * 8u;             // Velocity track values
}

float* GpuParticleSetVelocityTrackAffector::prepareAffectorEmitterBuffer(float* buffer) const {

    GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) mEnabled);
    GpuParticleAffectorCommon::uploadFloatTrack(buffer, mVelocityTrack, 1.0f);

    return buffer;
}

#if !OGRE_NO_JSON
void GpuParticleSetVelocityTrackAffector::_loadJson(const rapidjson::Value& json)
{
    rapidjson::Value::ConstMemberIterator itor;

    itor = json.FindMember("enabled");
    if (itor != json.MemberEnd() && itor->value.IsBool())
    {
        mEnabled = itor->value.GetBool();
    }

    itor = json.FindMember("velocityTrack");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        GpuParticleSystemJsonManager::readFloatTrack(itor->value, mVelocityTrack);
    }
}

void GpuParticleSetVelocityTrackAffector::_saveJson(Ogre::String& outString) const
{
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("enabled", outString);
    GpuParticleSystemJsonManager::toStr(mEnabled, outString);

    outString += ",";
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("velocityTrack", outString);
    GpuParticleSystemJsonManager::writeFloatTrack(mVelocityTrack, outString);
}
#endif
