/*
 * File: GpuParticleGlobalGravityAffector.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include <GpuParticles/Affectors/GpuParticleGlobalGravityAffector.h>
#include <GpuParticles/GpuParticleSystemJsonManager.h>
#if !OGRE_NO_JSON
#include "rapidjson/document.h"
#endif

GpuParticleGlobalGravityAffector::GpuParticleGlobalGravityAffector()
    : mGravity(Ogre::Vector3::ZERO)
{

}

Ogre::uint32 GpuParticleGlobalGravityAffector::getAffectorEmitterBufferSize() const
{
    return sizeof(float) * 3u +           // Gravity
           sizeof(float) * 1u;            // Padding
}

float* GpuParticleGlobalGravityAffector::prepareAffectorEmitterBuffer(float* buffer) const {

    *buffer++ = mGravity.x;
    *buffer++ = mGravity.y;
    *buffer++ = mGravity.z;
    *buffer++ = 0.0f;

    return buffer;
}

#if !OGRE_NO_JSON
void GpuParticleGlobalGravityAffector::_loadJson(const rapidjson::Value& json)
{
    rapidjson::Value::ConstMemberIterator itor;

    itor = json.FindMember("gravity");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        GpuParticleSystemJsonManager::readVector3Value(itor->value, mGravity);
    }
}

void GpuParticleGlobalGravityAffector::_saveJson(Ogre::String& outString) const
{
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("gravity", outString);
    GpuParticleSystemJsonManager::toStr(mGravity, outString);
}
#endif
