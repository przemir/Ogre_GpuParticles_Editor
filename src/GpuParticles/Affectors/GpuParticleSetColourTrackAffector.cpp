/*
 * File: GpuParticleSetColourTrackAffector.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-15
 *
 */

#include "GpuParticleSetColourTrackAffector.h"
#include <GpuParticles/GpuParticleSystemJsonManager.h>
#if !OGRE_NO_JSON
#include "rapidjson/document.h"
#endif

GpuParticleSetColourTrackAffector::GpuParticleSetColourTrackAffector()
{

}

Ogre::uint32 GpuParticleSetColourTrackAffector::getAffectorEmitterBufferSize() const
{
    return sizeof(Ogre::uint32) * 1u +     // mUseColourTrack (bool)
           sizeof(float) * 8u +            // Colour track times
           sizeof(float) * 3u * 8u;        // Colour (rgb) track values
}

float* GpuParticleSetColourTrackAffector::prepareAffectorEmitterBuffer(float* buffer) const {

    GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) mEnabled);
    GpuParticleAffectorCommon::uploadVector3Track(buffer, mColourTrack, Ogre::Vector3(1.0f, 1.0f, 1.0f));

    return buffer;
}

#if !OGRE_NO_JSON
void GpuParticleSetColourTrackAffector::_loadJson(const rapidjson::Value& json)
{
    rapidjson::Value::ConstMemberIterator itor;

    itor = json.FindMember("enabled");
    if (itor != json.MemberEnd() && itor->value.IsBool())
    {
        mEnabled = itor->value.GetBool();
    }

    itor = json.FindMember("colourTrack");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        GpuParticleSystemJsonManager::readVector3Track(itor->value, mColourTrack);
    }
}

void GpuParticleSetColourTrackAffector::_saveJson(Ogre::String& outString) const
{
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("enabled", outString);
    GpuParticleSystemJsonManager::toStr(mEnabled, outString);

    outString += ",";
    GpuParticleSystemJsonManager::writeGpuAffectorVariable("colourTrack", outString);
    GpuParticleSystemJsonManager::writeVector3Track(mColourTrack, outString);
}
#endif
