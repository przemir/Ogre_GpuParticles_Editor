/*
 * File: GpuParticleSystemJsonManager.h
 * Author: Przemysław Bągard
 * Created: 2021-6-12
 *
 */

#ifndef GpuParticleSystemJsonManager_H
#define GpuParticleSystemJsonManager_H

#if !OGRE_NO_JSON

#include "OgreStringVector.h"
#include "OgreIdString.h"
#include "GpuParticleEmitter.h"
#include <OgreCommon.h>
#include <ogrestd/map.h>

#include <OgreScriptLoader.h>
#include <OgreSingleton.h>

class GpuParticleSystem;
class GpuParticleEmitter;

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

/// Reads scripts in '*.gpuparticle.json' files
class GpuParticleSystemJsonManager
        : public Ogre::Singleton<GpuParticleSystemJsonManager>
        , public Ogre::ScriptLoader
{
public:

    GpuParticleSystemJsonManager();

    virtual const Ogre::StringVector& getScriptPatterns(void) const override;
    virtual void parseScript(Ogre::DataStreamPtr& stream, const Ogre::String& groupName) override;
    virtual Ogre::Real getLoadingOrder(void) const override;

    void loadGpuParticleSystems( const Ogre::String &filename,
                                 const Ogre::String &resourceGroup,
                                 const char *jsonString,
                                 const Ogre::String &additionalTextureExtension );

    static void loadGpuParticleSystem( const rapidjson::Value &json, GpuParticleSystem *gpuParticleSystem );
    static void loadGpuParticleEmitter(const rapidjson::Value &json, GpuParticleEmitter *gpuParticleEmitter );

    static void saveGpuParticleSystem( const GpuParticleSystem *gpuParticleSystem, Ogre::String &outString );
    static void saveGpuParticleEmitter( const GpuParticleEmitter *gpuParticleEmitter, Ogre::String &outString );

public:
    typedef Ogre::map<Ogre::String, Ogre::String>::type ResourceToTexExtensionMap;
    ResourceToTexExtensionMap mAdditionalTextureExtensionsPerGroup;

private:
    Ogre::StringVector mScriptPatterns;

    /// Writes '\n\t\t"var": '
    static void writeGpuEmitterVariable( const Ogre::String& var,  Ogre::String& outString);

public:
    static void readVector3Value(const rapidjson::Value &json, Ogre::Vector3& value);
    static void readVector2Value(const rapidjson::Value &json, Ogre::Vector2& value);
    static void readQuaternionValue(const rapidjson::Value &json, Ogre::Quaternion& value);
    static void readColourValue(const rapidjson::Value &json, Ogre::ColourValue& value);
    static void readMinMaxFloatValue(const rapidjson::Value &json, float& valueMin, float& valueMax);
    static void readFloatTrack(const rapidjson::Value &array, GpuParticleEmitter::FloatTrack& valueMax);
    static void readVector2Track(const rapidjson::Value &array, GpuParticleEmitter::Vector2Track& valueMax);
    static void readVector3Track(const rapidjson::Value &array, GpuParticleEmitter::Vector3Track& valueMax);

    static void toStr( const Ogre::ColourValue &value, Ogre::String &outString );
    static void toStr( const Ogre::Vector2 &value, Ogre::String &outString );
    static void toStr( const Ogre::Vector3 &value, Ogre::String &outString );
    static void toStr( const Ogre::Vector4 &value, Ogre::String &outString );
    static void toStr( const Ogre::Quaternion &value, Ogre::String &outString );
    static void toStr( const bool &value, Ogre::String &outString );
    static void toStrMinMax( float valueMin, float valueMax, Ogre::String &outString );

    /// writeGpuAffectorVariable differs from writeGpuEmitterVariable with number indentations ('\t')
    static void writeGpuAffectorVariable( const Ogre::String& var,  Ogre::String& outString);
    static Ogre::String quote( const Ogre::String& value );

    static void writeFloatTrack(const GpuParticleEmitter::FloatTrack& valueTrack, Ogre::String& outString);
    static void writeVector2Track(const GpuParticleEmitter::Vector2Track& valueTrack, Ogre::String& outString);
    static void writeVector3Track(const GpuParticleEmitter::Vector3Track& valueTrack, Ogre::String& outString);
};

#endif

#endif
