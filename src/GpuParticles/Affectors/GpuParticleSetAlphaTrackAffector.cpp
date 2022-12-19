/*
 * File: GpuParticleSetAlphaTrackAffector.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include "GpuParticleSetAlphaTrackAffector.h"
#include <GpuParticles/GpuParticleSystemJsonManager.h>
#if !OGRE_NO_JSON
#include "rapidjson/document.h"
#endif

GpuParticleSetAlphaTrackAffector::GpuParticleSetAlphaTrackAffector()
    : mEnabled(false)
{

}

Ogre::uint32 GpuParticleSetAlphaTrackAffector::getAffectorEmitterBufferSize() const
{
    return sizeof(Ogre::uint32) * 1u +     // mEnabled (bool)
           sizeof(float) * 8u +            // Alpha track times
           sizeof(float) * 8u;             // Alpha track values
}

float* GpuParticleSetAlphaTrackAffector::prepareAffectorEmitterBuffer(float* buffer) const {

    GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) mEnabled);
    GpuParticleAffectorCommon::uploadFloatTrack(buffer, mAlphaTrack, 1.0f);

    return buffer;
}

#if !OGRE_NO_JSON
void GpuParticleSetAlphaTrackAffector::_loadJson(const rapidjson::Value& json)
{
    rapidjson::Value::ConstMemberIterator itor;

    itor = json.FindMember("enabled");
    if (itor != json.MemberEnd() && itor->value.IsBool())
    {
        mEnabled = itor->value.GetBool();
    }

    itor = json.FindMember("alphaTrack");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        GpuParticleSystemJsonManager::readFloatTrack(itor->value, mAlphaTrack);
    }
}

void GpuParticleSetAlphaTrackAffector::_saveJson(Ogre::String& outString) const
{
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("enabled", outString);
    GpuParticleSystemJsonManager::toStr(mEnabled, outString);

    outString += ",";
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("alphaTrack", outString);
    GpuParticleSystemJsonManager::writeFloatTrack(mAlphaTrack, outString);
}
#endif
