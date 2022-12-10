/*
 * File: HlmsParticle.h
 * Author: Przemysław Bągard
 * Created: 2021-4-23
 *
 */

#ifndef HLMSPARTICLE_H
#define HLMSPARTICLE_H

#include <OgreHlmsListener.h>
#include <OgreHlmsUnlit.h>
#include <OgreRenderQueue.h>

class GpuParticleSystemWorld;
class HlmsParticleDatablock;

class HlmsParticleListener : public Ogre::HlmsListener {

    float mWindStrength{ 0.5 };
    float mGlobalTime{ 0 };
    Ogre::Hlms* mHlms = nullptr;
    Ogre::Matrix4 mCameraVP;
    Ogre::Matrix4 mPrevCameraVP;    // Camera View * Projection from previous frame
    Ogre::Vector2 mCameraProjectionAB = Ogre::Vector2::ZERO;

public:
    HlmsParticleListener() = default;
    virtual ~HlmsParticleListener() = default;

    void setHlms(Ogre::Hlms* hlms)
    {
        mHlms = hlms;
    }

    void setTime(float time) {
        mGlobalTime = time;
    }
    void addTime(float time) {
        mGlobalTime += time;
    }

    virtual Ogre::uint32 getPassBufferSize(const Ogre::CompositorShadowNode* shadowNode,
                                           bool casterPass,
                                           bool dualParaboloid,
                                           Ogre::SceneManager* sceneManager) const;

    virtual float* preparePassBuffer(const Ogre::CompositorShadowNode* shadowNode,
                                     bool casterPass,
                                     bool dualParaboloid,
                                     Ogre::SceneManager* sceneManager,
                                     float* passBufferPtr);
    Ogre::Matrix4 getPrevCameraVP() const;
    Ogre::Vector2 getCameraProjectionAB() const;
};

class HlmsParticle : public Ogre::HlmsUnlit
{
public:
    static const Ogre::HlmsTypes ParticleHlmsType = Ogre::HLMS_USER0;

private:

    HlmsParticleListener mParticleListener;
    static const int ParticleDataTexSlot = 14;
    static const int BucketGroupDataTexSlot = 12;
    static const int EmitterDataTexSlot = 13; // deprecated
    static const int EmitterCoreDataTexSlot = 11;

public:
    HlmsParticle(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders)
        : Ogre::HlmsUnlit(dataFolder, libraryFolders, ParticleHlmsType, "particle")
    {
        setListener(&mParticleListener);
        mParticleListener.setHlms(this);

//        mReservedTexSlots = 2u;
    }

    virtual ~HlmsParticle() = default;

    virtual Ogre::HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                             bool casterPass, bool dualParaboloid,
                                             Ogre::SceneManager *sceneManager ) override;

    void calculateHashForPreCreate(Ogre::Renderable* renderable, Ogre::PiecesMap* inOutPieces) override;

    void notifyPropertiesMergedPreGenerationStep(void) {

        HlmsUnlit::notifyPropertiesMergedPreGenerationStep();

        setTextureReg(Ogre::VertexShader, "texParticleData", ParticleDataTexSlot);
    }

    static void getAdditionalDefaultPaths(Ogre::String &outDataFolderPath, Ogre::StringVector& outLibraryFoldersPaths, bool withHlmsPathPrefix = true) {

        //We need to know what RenderSystem is currently in use, as the
        //name of the compatible shading language is part of the path
        Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
        Ogre::String shaderSyntax = "GLSL";
        if (renderSystem->getName() == "Direct3D11 Rendering Subsystem")
            shaderSyntax = "HLSL";
        else if (renderSystem->getName() == "Metal Rendering Subsystem")
            shaderSyntax = "Metal";

        Ogre::String prefix;
        if(withHlmsPathPrefix) {
            prefix = "Hlms/";
        }

        //Fill the library folder paths with the relevant folders
        outLibraryFoldersPaths.clear();
//        outLibraryFoldersPaths.push_back(prefix + "Particle/" + shaderSyntax);
        outLibraryFoldersPaths.push_back(prefix + "Particle/Any");
        outLibraryFoldersPaths.push_back(prefix + "Compute");

        //Fill the data folder path
        outDataFolderPath = prefix + "Particle/" + shaderSyntax;
    }

    static HlmsParticle* registerHlms(const Ogre::String& rootHlmsFolder,
                                      const Ogre::String& particleRootHlmsFolder,
                                      bool withHlmsPathPrefix = true);

    Ogre::uint32 fillBuffersForV1(const Ogre::HlmsCache* cache,
        const Ogre::QueuedRenderable& queuedRenderable,
        bool casterPass, Ogre::uint32 lastCacheHash,
        Ogre::CommandBuffer* commandBuffer)
    {
        return fillBuffersFor(cache, queuedRenderable, casterPass,
            lastCacheHash, commandBuffer, true);
    }
    //-----------------------------------------------------------------------------------
    Ogre::uint32 fillBuffersForV2(const Ogre::HlmsCache* cache,
        const Ogre::QueuedRenderable& queuedRenderable,
        bool casterPass, Ogre::uint32 lastCacheHash,
        Ogre::CommandBuffer* commandBuffer)
    {
        return fillBuffersFor(cache, queuedRenderable, casterPass,
            lastCacheHash, commandBuffer, false);
    }

    /// Buffers for one renderable
    Ogre::uint32 fillBuffersFor(const Ogre::HlmsCache* cache, const Ogre::QueuedRenderable& queuedRenderable,
        bool casterPass, Ogre::uint32 lastCacheHash,
        Ogre::CommandBuffer* commandBuffer, bool isV1);


    virtual void frameEnded() override;

    virtual Ogre::HlmsDatablock* createDatablockImpl( Ogre::IdString datablockName,
                                                      const Ogre::HlmsMacroblock *macroblock,
                                                      const Ogre::HlmsBlendblock *blendblock,
                                                      const Ogre::HlmsParamVec &paramVec ) override;

    HlmsParticleDatablock* cloneFromUnlitDatablock(const Ogre::HlmsUnlitDatablock* srcDatablock, const Ogre::String& name);

#if !OGRE_NO_JSON
    /// @copydoc Hlms::_loadJson
    virtual void _loadJson( const rapidjson::Value &jsonValue, const Ogre::HlmsJson::NamedBlocks &blocks,
                            Ogre::HlmsDatablock *datablock, const Ogre::String &resourceGroup,
                            Ogre::HlmsJsonListener *listener, const Ogre::String &additionalTextureExtension ) const override;
    /// @copydoc Hlms::_saveJson
    virtual void _saveJson( const Ogre::HlmsDatablock *datablock, Ogre::String &outString,
                            Ogre::HlmsJsonListener *listener,
                            const Ogre::String &additionalTextureExtension ) const override;
#endif

    HlmsParticleListener* getParticleListener();
};

#endif
