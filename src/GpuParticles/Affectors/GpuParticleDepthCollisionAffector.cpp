/*
 * File: GpuParticleDepthCollisionAffector.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include <GpuParticles/Affectors/GpuParticleDepthCollisionAffector.h>
#include <GpuParticles/GpuParticleSystemJsonManager.h>
#if !OGRE_NO_JSON
#include "rapidjson/document.h"
#endif

GpuParticleDepthCollisionAffector::GpuParticleDepthCollisionAffector()
    : mEnabled(false)
{

}

Ogre::uint32 GpuParticleDepthCollisionAffector::getAffectorEmitterBufferSize() const
{
    return sizeof(float) * 3u +           // Padding
           sizeof(Ogre::uint32) * 1u;     // mEnabled (bool)
}

float* GpuParticleDepthCollisionAffector::prepareAffectorEmitterBuffer(float* buffer) const {

    *buffer++ = 0.0f;
    *buffer++ = 0.0f;
    *buffer++ = 0.0f;

    GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) mEnabled);

    return buffer;
}

#if !OGRE_NO_JSON
void GpuParticleDepthCollisionAffector::_loadJson(const rapidjson::Value& json)
{
    rapidjson::Value::ConstMemberIterator itor;

    itor = json.FindMember("enabled");
    if (itor != json.MemberEnd() && itor->value.IsBool())
    {
        mEnabled = itor->value.GetBool();
    }
}

void GpuParticleDepthCollisionAffector::_saveJson(Ogre::String& outString) const
{
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("enabled", outString);
    GpuParticleSystemJsonManager::toStr(mEnabled, outString);
}
#endif
