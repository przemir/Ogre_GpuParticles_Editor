/*
 * File: GpuParticleSystemJsonManager.cpp
 * Author: Przemysław Bągard
 * Created: 2021-6-12
 *
 */

#if !OGRE_NO_JSON

#include <OgreCommon.h>
#include <OgreHlmsJson.h>

#include <GpuParticles/GpuParticleSystemJsonManager.h>
#include <GpuParticles/GpuParticleSystemResourceManager.h>
#include <GpuParticles/GpuParticleSystem.h>
#include <OgreLogManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreStringConverter.h>

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

template<> GpuParticleSystemJsonManager *Ogre::Singleton<GpuParticleSystemJsonManager>::msSingleton = 0;

GpuParticleSystemJsonManager::GpuParticleSystemJsonManager()
{
    mScriptPatterns.push_back("*.gpuparticle.json");
    Ogre::ResourceGroupManager::getSingleton()._registerScriptLoader(this);
}

const Ogre::StringVector& GpuParticleSystemJsonManager::getScriptPatterns() const
{
    return mScriptPatterns;
}

void GpuParticleSystemJsonManager::parseScript(Ogre::DataStreamPtr& stream,
                                               const Ogre::String& groupName)
{
    Ogre::vector<char>::type fileData;
    fileData.resize( stream->size() + 1 );
    if( !fileData.empty() )
    {
        stream->read( &fileData[0], stream->size() );

        Ogre::String additionalTextureExtension;
        ResourceToTexExtensionMap::const_iterator itExt =
                mAdditionalTextureExtensionsPerGroup.find( groupName );

        if( itExt != mAdditionalTextureExtensionsPerGroup.end() )
            additionalTextureExtension = itExt->second;

        //Add null terminator just in case (to prevent bad input)
        fileData.back() = '\0';

        loadGpuParticleSystems( stream->getName(),
                                groupName,
                                &fileData[0],
                                additionalTextureExtension );
    }


    //    Ogre::LogManager::getSingletonPtr()->logMessage(result.infos[i].print().toStdString() + "\n");
}

Ogre::Real GpuParticleSystemJsonManager::getLoadingOrder() const
{
    return 1000.0f;
}

void GpuParticleSystemJsonManager::loadGpuParticleSystems(const Ogre::String& filename,
                                                          const Ogre::String& resourceGroup,
                                                          const char* jsonString,
                                                          const Ogre::String& additionalTextureExtension)
{
    rapidjson::Document d;
    d.Parse( jsonString );

    if( d.HasParseError() )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS,
                     "Invalid JSON string in file " + filename + " at line " +
                     Ogre::StringConverter::toString( d.GetErrorOffset() ) + " Reason: " +
                     rapidjson::GetParseError_En( d.GetParseError() ),
                     "GpuParticleSystemJsonManager::loadGpuParticleSystems");
    }

    //Load particle systems
    rapidjson::Value::ConstMemberIterator itor = d.FindMember("systems");
    if( itor != d.MemberEnd() && itor->value.IsObject() )
    {
        const rapidjson::Value &systems = itor->value;

        rapidjson::Value::ConstMemberIterator itSystem = systems.MemberBegin();
        rapidjson::Value::ConstMemberIterator enSystem = systems.MemberEnd();

        while( itSystem != enSystem )
        {
            if( itSystem->value.IsObject() )
            {
                const char *datablockName = itSystem->name.GetString();
                try
                {
                    GpuParticleSystem *gpuParticleSystem = GpuParticleSystemResourceManager::getSingleton().
                                                           createParticleSystem( datablockName, datablockName, filename, resourceGroup );
                    loadGpuParticleSystem( itSystem->value, gpuParticleSystem );
                }
                catch( Ogre::Exception &e )
                {
                    //Ignore datablocks that already exist (useful for reloading materials)
                    if( e.getNumber() != Ogre::Exception::ERR_DUPLICATE_ITEM )
                        throw;
                    else
                        Ogre::LogManager::getSingleton().logMessage( e.getFullDescription() );
                }
            }

            ++itSystem;
        }
    }

}

void GpuParticleSystemJsonManager::loadGpuParticleSystem(const rapidjson::Value& json, GpuParticleSystem* gpuParticleSystem)
{
    rapidjson::Value::ConstMemberIterator itor = json.FindMember("emitters");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        const rapidjson::Value &array = itor->value;

        const rapidjson::SizeType arraySize = array.Size();
        for (rapidjson::SizeType i = 0; i<arraySize; ++i)
        {
            const rapidjson::Value &emitter = array[i];
            if( emitter.IsObject() )
            {
                GpuParticleEmitter *particleEmitterCore = OGRE_NEW GpuParticleEmitter();
                loadGpuParticleEmitter( emitter, particleEmitterCore );
                gpuParticleSystem->addEmitter(particleEmitterCore);
            }
        }
    }

}

void GpuParticleSystemJsonManager::loadGpuParticleEmitter(const rapidjson::Value& json, GpuParticleEmitter* gpuParticleEmitter)
{
    rapidjson::Value::ConstMemberIterator itor;

    itor = json.FindMember("pos");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        readVector3Value(itor->value, gpuParticleEmitter->mPos);
    }

    itor = json.FindMember("rot");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        readQuaternionValue(itor->value, gpuParticleEmitter->mRot);
    }

    itor = json.FindMember("spriteMode");
    if( itor != json.MemberEnd() && itor->value.IsString() )
    {
        gpuParticleEmitter->mSpriteMode = GpuParticleEmitter::strToSpriteMode(itor->value.GetString());
    }

    itor = json.FindMember("datablock");
    if( itor != json.MemberEnd() && itor->value.IsString() )
    {
        char const *datablockName = itor->value.GetString();
        gpuParticleEmitter->mDatablockName = datablockName;

//        loadTexture( subobj, blocks, i, unlitDatablock, resourceGroup );
    }

    itor = json.FindMember("spriteTrack");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        const rapidjson::Value& array = itor->value;
        const rapidjson::SizeType arraySize = std::min((rapidjson::SizeType)GpuParticleEmitter::MaxSprites, array.Size());
        for (rapidjson::SizeType i = 0; i<arraySize; ++i)
        {
            if(array[i].IsArray() && array[i].Size() >= 2 && array[i][0].IsNumber()) {
                float time = static_cast<float>(array[i][0].GetDouble());
                if(array[i][1].IsArray() && array[i][1].Size() >= 2) {
                    gpuParticleEmitter->mSpriteTimes.push_back(time);
                    HlmsParticleDatablock::SpriteCoord coord;
                    coord.col = array[i][1][0].GetInt();
                    coord.row = array[i][1][1].GetInt();
                    gpuParticleEmitter->mSpriteFlipbookCoords.push_back(coord);
                }
            }
        }
    }

    itor = json.FindMember("emitterLifetime");
    if (itor != json.MemberEnd() && itor->value.IsNumber())
    {
        gpuParticleEmitter->mEmitterLifetime = static_cast<float>(itor->value.GetDouble());
    }

    itor = json.FindMember("emissionRate");
    if (itor != json.MemberEnd() && itor->value.IsNumber())
    {
        gpuParticleEmitter->mEmissionRate = static_cast<float>(itor->value.GetDouble());
    }

    itor = json.FindMember("burstMode");
    if (itor != json.MemberEnd() && itor->value.IsBool())
    {
        gpuParticleEmitter->mBurstMode = itor->value.GetBool();
    }

    itor = json.FindMember("burstParticles");
    if (itor != json.MemberEnd() && itor->value.IsNumber())
    {
        gpuParticleEmitter->mBurstParticles = itor->value.GetDouble();
    }

    itor = json.FindMember("spawnShape");
    if( itor != json.MemberEnd() && itor->value.IsString() )
    {
        gpuParticleEmitter->mSpawnShape = GpuParticleEmitter::strToSpawnShape(itor->value.GetString());
    }

    itor = json.FindMember("spawnShapeDimensions");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        readVector3Value(itor->value, gpuParticleEmitter->mSpawnShapeDimensions);
    }

    itor = json.FindMember("faderMode");
    if( itor != json.MemberEnd() && itor->value.IsString() )
    {
        gpuParticleEmitter->mFaderMode = GpuParticleEmitter::strToFaderMode(itor->value.GetString());
    }

    itor = json.FindMember("particleFaderStartTime");
    if (itor != json.MemberEnd() && itor->value.IsNumber())
    {
        gpuParticleEmitter->mParticleFaderStartTime = static_cast<float>(itor->value.GetDouble());
    }

    itor = json.FindMember("particleFaderEndTime");
    if (itor != json.MemberEnd() && itor->value.IsNumber())
    {
        gpuParticleEmitter->mParticleFaderEndTime = static_cast<float>(itor->value.GetDouble());
    }

    itor = json.FindMember("uniformSize");
    if (itor != json.MemberEnd() && itor->value.IsBool())
    {
        gpuParticleEmitter->mUniformSize = itor->value.GetBool();
    }

    itor = json.FindMember("billboardType");
    if( itor != json.MemberEnd() && itor->value.IsString() )
    {
        gpuParticleEmitter->mBillboardType = GpuParticleEmitter::strToBillboardType(itor->value.GetString());
    }

    itor = json.FindMember("colourA");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        readColourValue(itor->value, gpuParticleEmitter->mColourA);
    }

    itor = json.FindMember("colourB");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        readColourValue(itor->value, gpuParticleEmitter->mColourB);
    }

    itor = json.FindMember("direction");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        readVector3Value(itor->value, gpuParticleEmitter->mDirection);
    }

    // min-max values
    itor = json.FindMember("particleLifetime");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        readMinMaxFloatValue(itor->value, gpuParticleEmitter->mParticleLifetimeMin, gpuParticleEmitter->mParticleLifetimeMax);
    }

    itor = json.FindMember("size");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        readMinMaxFloatValue(itor->value, gpuParticleEmitter->mSizeMin, gpuParticleEmitter->mSizeMax);
    }

    itor = json.FindMember("sizeY");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        readMinMaxFloatValue(itor->value, gpuParticleEmitter->mSizeYMin, gpuParticleEmitter->mSizeYMax);
    }

    itor = json.FindMember("spotAngle");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        readMinMaxFloatValue(itor->value, gpuParticleEmitter->mSpotAngleMin, gpuParticleEmitter->mSpotAngleMax);
    }

    itor = json.FindMember("directionVelocity");
    if (itor != json.MemberEnd() && itor->value.IsArray())
    {
        readMinMaxFloatValue(itor->value, gpuParticleEmitter->mDirectionVelocityMin, gpuParticleEmitter->mDirectionVelocityMax);
    }

    itor = json.FindMember("affectors");
    if (itor != json.MemberEnd() && itor->value.IsObject())
    {
        GpuParticleSystemResourceManager& gpuParticleSystemResourceManager = GpuParticleSystemResourceManager::getSingleton();

        const rapidjson::Value &affectors = itor->value;

        rapidjson::Value::ConstMemberIterator itAffector = affectors.MemberBegin();
        rapidjson::Value::ConstMemberIterator enAffector = affectors.MemberEnd();

        while( itAffector != enAffector )
        {
            if( itAffector->value.IsObject() )
            {
                const char *affectorName = itAffector->name.GetString();
                Ogre::String affectorNameStr = affectorName;
                const GpuParticleAffector* registeredAffector = gpuParticleSystemResourceManager.getAffectorByProperty(affectorNameStr);

                if(affectorNameStr.empty()) {
                    OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS,
                                 "Empty affector property name during loading.",
                                 "GpuParticleSystemJsonManager::loadGpuParticleEmitter");
                } else if(registeredAffector) {
                    GpuParticleAffector* newAffector = registeredAffector->clone();
                    newAffector->_loadJson(itAffector->value);
                    gpuParticleEmitter->addAffector(newAffector);
                }
                else {
                    OGRE_EXCEPT( Ogre::Exception::ERR_ITEM_NOT_FOUND,
                                 "Not registered affector '" + affectorNameStr + "' inside GpuParticleSystemResourceManager.",
                                 "GpuParticleSystemJsonManager::loadGpuParticleEmitter");
                }
            }

            ++itAffector;
        }
    }
}

void GpuParticleSystemJsonManager::saveGpuParticleSystem(const GpuParticleSystem* gpuParticleSystem, Ogre::String& outString)
{
    outString += "{";
    outString += "\n\t" + quote("systems") + ":";
    outString += "\n\t{";

    Ogre::String particleSystemName = GpuParticleSystemResourceManager::getSingleton().getGpuParticleSystemName(gpuParticleSystem);
    outString += "\n\t\t" + quote(particleSystemName) + ":";
    outString += "\n\t\t{";

    outString += "\n\t\t\t" + quote("emitters") + ":";
    outString += "\n\t\t\t[";

    for (size_t i = 0; i < gpuParticleSystem->getEmitters().size(); ++i) {
        outString += "\n\t\t\t\t{";
        saveGpuParticleEmitter(gpuParticleSystem->getEmitters()[i], outString);
        outString += "\n\t\t\t\t}";
        if(i != gpuParticleSystem->getEmitters().size()-1) {
            outString += ",";
        }
    }

    outString += "\n\t\t\t]";
    outString += "\n\t\t}";
    outString += "\n\t}";
    outString += "\n}";
}

void GpuParticleSystemJsonManager::saveGpuParticleEmitter(const GpuParticleEmitter* gpuParticleEmitter, Ogre::String& outString)
{
    writeGpuEmitterVariable("pos", outString);
    toStr(gpuParticleEmitter->mPos, outString);

    outString += ",";
    writeGpuEmitterVariable("rot", outString);
    toStr(gpuParticleEmitter->mRot, outString);

    outString += ",";
    writeGpuEmitterVariable("burstMode", outString);
    toStr(gpuParticleEmitter->mBurstMode, outString);

    outString += ",";
    writeGpuEmitterVariable("emissionRate", outString);
    outString += Ogre::StringConverter::toString( gpuParticleEmitter->mEmissionRate );

    outString += ",";
    writeGpuEmitterVariable("burstParticles", outString);
    outString += Ogre::StringConverter::toString( gpuParticleEmitter->mBurstParticles );

    outString += ",";
    writeGpuEmitterVariable("emitterLifetime", outString);
    outString += Ogre::StringConverter::toString( gpuParticleEmitter->mEmitterLifetime );

    outString += ",";
    writeGpuEmitterVariable("particleLifetime", outString);
    toStrMinMax(gpuParticleEmitter->mParticleLifetimeMin, gpuParticleEmitter->mParticleLifetimeMax, outString);

    outString += ",";
    writeGpuEmitterVariable("datablock", outString);
    outString += quote(gpuParticleEmitter->mDatablockName);

    outString += ",";
    writeGpuEmitterVariable("spriteMode", outString);
    outString += quote(GpuParticleEmitter::spriteModeToStr(gpuParticleEmitter->mSpriteMode));

    //
    outString += ",";
    writeGpuEmitterVariable("spriteTrack", outString);
    outString += "[";
    size_t count = gpuParticleEmitter->mSpriteFlipbookCoords.size();
    for (size_t i = 0; i < count; ++i) {
        if(i != 0) {
            outString += ", ";
        }

        float time = i < gpuParticleEmitter->mSpriteTimes.size() ? gpuParticleEmitter->mSpriteTimes[i] : 0.0f;
//        Ogre::String value = gpuParticleEmitter->mSpriteNames[i];
        const GpuParticleEmitter::SpriteCoord& coord = gpuParticleEmitter->mSpriteFlipbookCoords[i];

        outString += '[';
        outString += Ogre::StringConverter::toString( time );
        outString += ", [";
        outString += Ogre::StringConverter::toString( coord.col );
        outString += ", ";
        outString += Ogre::StringConverter::toString( coord.row );
        outString += "]]";
    }
    outString += "]";
    //

    outString += ",";
    writeGpuEmitterVariable("spawnShape", outString);
    outString += quote(GpuParticleEmitter::spawnShapeToStr(gpuParticleEmitter->mSpawnShape));

    outString += ",";
    writeGpuEmitterVariable("spawnShapeDimensions", outString);
    toStr(gpuParticleEmitter->mSpawnShapeDimensions, outString);

    outString += ",";
    writeGpuEmitterVariable("faderMode", outString);
    outString += quote(GpuParticleEmitter::faderModeToStr(gpuParticleEmitter->mFaderMode));

    outString += ",";
    writeGpuEmitterVariable("particleFaderStartTime", outString);
    outString += Ogre::StringConverter::toString( gpuParticleEmitter->mParticleFaderStartTime );

    outString += ",";
    writeGpuEmitterVariable("particleFaderEndTime", outString);
    outString += Ogre::StringConverter::toString( gpuParticleEmitter->mParticleFaderEndTime );

    outString += ",";
    writeGpuEmitterVariable("uniformSize", outString);
    toStr(gpuParticleEmitter->mUniformSize, outString);

    outString += ",";
    writeGpuEmitterVariable("billboardType", outString);
    outString += quote(GpuParticleEmitter::billboardTypeToStr(gpuParticleEmitter->mBillboardType));

    outString += ",";
    writeGpuEmitterVariable("colourA", outString);
    toStr(gpuParticleEmitter->mColourA, outString);

    outString += ",";
    writeGpuEmitterVariable("colourB", outString);
    toStr(gpuParticleEmitter->mColourB, outString);

    outString += ",";
    writeGpuEmitterVariable("direction", outString);
    toStr(gpuParticleEmitter->mDirection, outString);

    outString += ",";
    writeGpuEmitterVariable("size", outString);
    toStrMinMax(gpuParticleEmitter->mSizeMin, gpuParticleEmitter->mSizeMax, outString);

    outString += ",";
    writeGpuEmitterVariable("sizeY", outString);
    toStrMinMax(gpuParticleEmitter->mSizeYMin, gpuParticleEmitter->mSizeYMax, outString);

    outString += ",";
    writeGpuEmitterVariable("spotAngle", outString);
    toStrMinMax(gpuParticleEmitter->mSpotAngleMin, gpuParticleEmitter->mSpotAngleMax, outString);

    outString += ",";
    writeGpuEmitterVariable("directionVelocity", outString);
    toStrMinMax(gpuParticleEmitter->mDirectionVelocityMin, gpuParticleEmitter->mDirectionVelocityMax, outString);

    outString += ",";
    outString += "\n\t\t\t\t\t" + quote("affectors") + ":";
    outString += "\n\t\t\t\t\t{";
    const GpuParticleEmitter::AffectorByNameMap& emitterAffectorMap = gpuParticleEmitter->getAffectorByNameMap();
    for (GpuParticleEmitter::AffectorByNameMap::const_iterator it = emitterAffectorMap.begin();
         it != emitterAffectorMap.end(); ++it) {

        if(it != emitterAffectorMap.begin()) {
            outString += ",";
        }

        const GpuParticleAffector* affector = it->second;

        outString += "\n\t\t\t\t\t\t" + quote(affector->getAffectorProperty()) + ":";
        outString += "\n\t\t\t\t\t\t{";
        affector->_saveJson(outString);
        outString += "\n\t\t\t\t\t\t}";
    }
    outString += "\n\t\t\t\t\t}";
}

void GpuParticleSystemJsonManager::readVector3Value(const rapidjson::Value& json, Ogre::Vector3& value)
{
    const rapidjson::SizeType arraySize = std::min(3u, json.Size());
    for (rapidjson::SizeType i = 0; i < arraySize; ++i) {
        if(json[i].IsNumber()) {
            value[i] = static_cast<float>(json[i].GetDouble());
        }
        else {
            value[i] = 0.0f;
        }
    }
}

void GpuParticleSystemJsonManager::readVector2Value(const rapidjson::Value& json, Ogre::Vector2& value)
{
    const rapidjson::SizeType arraySize = std::min(2u, json.Size());
    for (rapidjson::SizeType i = 0; i < arraySize; ++i) {
        if(json[i].IsNumber()) {
            value[i] = static_cast<float>(json[i].GetDouble());
        }
        else {
            value[i] = 0.0f;
        }
    }
}

void GpuParticleSystemJsonManager::readQuaternionValue(const rapidjson::Value& json, Ogre::Quaternion& value)
{
    const rapidjson::SizeType arraySize = std::min(4u, json.Size());
    for (rapidjson::SizeType i = 0; i < arraySize; ++i) {
        if(json[i].IsNumber()) {
            value[i] = static_cast<float>(json[i].GetDouble());
        }
        else {
            value[i] = 0.0f;
        }
    }
    value.normalise();
}

void GpuParticleSystemJsonManager::readColourValue(const rapidjson::Value& json, Ogre::ColourValue& value)
{
    const rapidjson::SizeType arraySize = std::min(4u, json.Size());
    for (rapidjson::SizeType i = 0; i < arraySize; ++i) {
        if(json[i].IsNumber()) {
            value[i] = static_cast<float>(json[i].GetDouble());
        }
        else {
            value[i] = 0.0f;
        }
    }
}

void GpuParticleSystemJsonManager::readMinMaxFloatValue(const rapidjson::Value& array, float& valueMin, float& valueMax)
{
    if(array.Size() == 1) {
        valueMin = static_cast<float>(array[0].GetDouble());
        valueMax = valueMin;
    }
    else if(array.Size() >= 2) {
        valueMin = static_cast<float>(array[0].GetDouble());
        valueMax = static_cast<float>(array[1].GetDouble());
    }
}

void GpuParticleSystemJsonManager::readFloatTrack(const rapidjson::Value& array, GpuParticleAffectorCommon::FloatTrack& valueMax)
{
    const rapidjson::SizeType arraySize = std::min((rapidjson::SizeType)GpuParticleAffectorCommon::MaxTrackValues, array.Size());
    for (rapidjson::SizeType i = 0; i<arraySize; ++i)
    {
        if(array[i].IsArray() && array[i].Size() >= 2 && array[i][0].IsNumber()) {
            float time = static_cast<float>(array[i][0].GetDouble());
            if(array[i][1].IsNumber()) {
                valueMax[time] = static_cast<float>(array[i][1].GetDouble());
            }
        }
    }
}

void GpuParticleSystemJsonManager::readVector2Track(const rapidjson::Value& array, GpuParticleAffectorCommon::Vector2Track& valueMax)
{
    const rapidjson::SizeType arraySize = std::min((rapidjson::SizeType)GpuParticleAffectorCommon::MaxTrackValues, array.Size());
    for (rapidjson::SizeType i = 0; i<arraySize; ++i)
    {
        if(array[i].IsArray() && array[i].Size() >= 2 && array[i][0].IsNumber()) {
            float time = static_cast<float>(array[i][0].GetDouble());
            if(array[i][1].IsArray()) {
                Ogre::Vector2 trackValue = Ogre::Vector2::ZERO;
                readVector2Value(array[i][1], trackValue);
                valueMax[time] = trackValue;
            }
        }
    }
}

void GpuParticleSystemJsonManager::readVector3Track(const rapidjson::Value& array, GpuParticleAffectorCommon::Vector3Track& valueMax)
{
    const rapidjson::SizeType arraySize = std::min((rapidjson::SizeType)GpuParticleAffectorCommon::MaxTrackValues, array.Size());
    for (rapidjson::SizeType i = 0; i<arraySize; ++i)
    {
        if(array[i].IsArray() && array[i].Size() >= 2 && array[i][0].IsNumber()) {
            float time = static_cast<float>(array[i][0].GetDouble());
            if(array[i][1].IsArray()) {
                Ogre::Vector3 trackValue = Ogre::Vector3::ZERO;
                readVector3Value(array[i][1], trackValue);
                valueMax[time] = trackValue;
            }
        }
    }
}

void GpuParticleSystemJsonManager::toStr( const Ogre::ColourValue &value, Ogre::String &outString )
{
    Ogre::HlmsJson::toStr(value, outString);
}

void GpuParticleSystemJsonManager::toStr( const Ogre::Vector2 &value, Ogre::String &outString )
{
    Ogre::HlmsJson::toStr(value, outString);
}

void GpuParticleSystemJsonManager::toStr( const Ogre::Vector3 &value, Ogre::String &outString )
{
    Ogre::HlmsJson::toStr(value, outString);
}

void GpuParticleSystemJsonManager::toStr( const Ogre::Vector4 &value, Ogre::String &outString )
{
    Ogre::HlmsJson::toStr(value, outString);
}

void GpuParticleSystemJsonManager::toStr(const Ogre::Quaternion& value, Ogre::String& outString)
{
    outString += '[';
    outString += Ogre::StringConverter::toString( value[0] );
    outString += ", ";
    outString += Ogre::StringConverter::toString( value[1] );
    outString += ", ";
    outString += Ogre::StringConverter::toString( value[2] );
    outString += ", ";
    outString += Ogre::StringConverter::toString( value[3] );
    outString += ']';
}

void GpuParticleSystemJsonManager::toStr(const bool& value, Ogre::String& outString)
{
    outString += value ? "true" : "false";
}

void GpuParticleSystemJsonManager::toStrMinMax(float valueMin, float valueMax, Ogre::String& outString)
{
    Ogre::HlmsJson::toStr(Ogre::Vector2(valueMin, valueMax), outString);
}

void GpuParticleSystemJsonManager::writeGpuEmitterVariable(const Ogre::String& var, Ogre::String& outString)
{
    static const Ogre::String& prefix = "\n\t\t\t\t\t";
    outString += prefix + quote(var) + ": ";
}

void GpuParticleSystemJsonManager::writeGpuAffectorVariable(const Ogre::String& var, Ogre::String& outString)
{
    static const Ogre::String& prefix = "\n\t\t\t\t\t\t\t";
    outString += prefix + quote(var) + ": ";
}

Ogre::String GpuParticleSystemJsonManager::quote(const Ogre::String& value)
{
    return "\"" + value + "\"";
}

void GpuParticleSystemJsonManager::writeFloatTrack(const GpuParticleAffectorCommon::FloatTrack& valueTrack, Ogre::String& outString)
{
    outString += "[";
    for (GpuParticleAffectorCommon::FloatTrack::const_iterator it = valueTrack.begin(); it != valueTrack.end(); ++it)
    {
        if(it != valueTrack.begin()) {
            outString += ", ";
        }
        
        float time = it->first;
        float value = it->second;
        
        outString += '[';
        outString += Ogre::StringConverter::toString( time );
        outString += ", ";
        outString += Ogre::StringConverter::toString( value );
        outString += ']';
    }
    outString += "]";
}

void GpuParticleSystemJsonManager::writeVector2Track(const GpuParticleAffectorCommon::Vector2Track& valueTrack, Ogre::String& outString)
{
    outString += "[";
    for (GpuParticleAffectorCommon::Vector2Track::const_iterator it = valueTrack.begin(); it != valueTrack.end(); ++it)
    {
        if(it != valueTrack.begin()) {
            outString += ", ";
        }
        
        float time = it->first;
        Ogre::Vector2 value = it->second;
        
        outString += '[';
        outString += Ogre::StringConverter::toString( time );
        outString += ", ";
        outString += '[';
        outString += Ogre::StringConverter::toString( value.x );
        outString += ", ";
        outString += Ogre::StringConverter::toString( value.y );
        outString += ']';
        outString += ']';
    }
    outString += "]";
}

void GpuParticleSystemJsonManager::writeVector3Track(const GpuParticleAffectorCommon::Vector3Track& valueTrack, Ogre::String& outString)
{
    outString += "[";
    for (GpuParticleAffectorCommon::Vector3Track::const_iterator it = valueTrack.begin(); it != valueTrack.end(); ++it)
    {
        if(it != valueTrack.begin()) {
            outString += ", ";
        }
        
        float time = it->first;
        Ogre::Vector3 value = it->second;
        
        outString += '[';
        outString += Ogre::StringConverter::toString( time );
        outString += ", ";
        outString += '[';
        outString += Ogre::StringConverter::toString( value.x );
        outString += ", ";
        outString += Ogre::StringConverter::toString( value.y );
        outString += ", ";
        outString += Ogre::StringConverter::toString( value.z );
        outString += ']';
        outString += ']';
    }
    outString += "]";
}

#endif
