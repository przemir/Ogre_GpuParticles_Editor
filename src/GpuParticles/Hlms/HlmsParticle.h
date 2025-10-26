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
class GpuParticleAffector;

class HlmsParticleListener : public Ogre::HlmsListener {

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

    virtual Ogre::uint32 getPassBufferSize(const Ogre::CompositorShadowNode* shadowNode,
                                           bool casterPass,
                                           bool dualParaboloid,
                                           Ogre::SceneManager* sceneManager) const override;

    virtual float* preparePassBuffer(const Ogre::CompositorShadowNode* shadowNode,
                                     bool casterPass,
                                     bool dualParaboloid,
                                     Ogre::SceneManager* sceneManager,
                                     float* passBufferPtr) override;
    Ogre::Matrix4 getPrevCameraVP() const;
    Ogre::Vector2 getCameraProjectionAB() const;

    static const Ogre::String InsertEmitterAffectors_PieceKey;
    void generateEmitterCoreDataAffectorsCode(Ogre::String& resultPiece, const std::vector<const GpuParticleAffector*>& affectorList);

    static const Ogre::String InsertParticleAffectors_PieceKey;
    void generateParticleDataAffectorsCode(Ogre::String& resultPiece, const std::vector<const GpuParticleAffector*>& affectorList);

};

class HlmsParticle : public Ogre::HlmsUnlit
{
public:
    static const Ogre::HlmsTypes ParticleHlmsType = Ogre::HLMS_USER0;

private:

    HlmsParticleListener mParticleListener;

    /// Those slots must match bindings inside files 500.Structs_piece_vs_piece_ps.hlsl and glsl
    static const int ParticleDataTexSlot = 5;
    static const int BucketGroupDataTexSlot = 3;
    static const int EmitterCoreDataTexSlot = 2;

public:
    HlmsParticle(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders)
        : Ogre::HlmsUnlit(dataFolder, libraryFolders, ParticleHlmsType, "particle")
    {
        setListener(&mParticleListener);
        mParticleListener.setHlms(this);

        // Porting to OgreNext4.0
        // https://github.com/OGRECave/ogre-next/commit/9ee6dd793481b5378e9a68fd445a34435b802e1b
        // https://github.com/OGRECave/ogre-next/commit/396f2298e04f7f4bfb648e8494549d23b4cb9bd4

//        mTexUnitSlotStart = 6u; // should be calculated later
//        mSamplerUnitSlotStart = 6u;

        mReservedTexSlots = 6u;
        mReservedTexBufferSlots = 0u;
    }

    ~HlmsParticle() override = default;

    void setupRootLayout( Ogre::RootLayout &rootLayout, size_t tid  ) override;

    Ogre::HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode, bool casterPass,
                                     bool dualParaboloid, Ogre::SceneManager *sceneManager ) override;

    void calculateHashForPreCreate(Ogre::Renderable* renderable, Ogre::PiecesMap* inOutPieces) override;

    Ogre::Hlms::PropertiesMergeStatus notifyPropertiesMergedPreGenerationStep(size_t     tid,
                                                                              Ogre::PiecesMap *inOutPieces) override {

        Hlms::PropertiesMergeStatus status = HlmsUnlit::notifyPropertiesMergedPreGenerationStep(tid, inOutPieces);

        // setTextureReg(Ogre::VertexShader, "texParticleData", ParticleDataTexSlot);

        return status;
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

    //-----------------------------------------------------------------------------------
    Ogre::uint32 fillBuffersForV2(const Ogre::HlmsCache* cache,
        const Ogre::QueuedRenderable& queuedRenderable,
        bool casterPass, Ogre::uint32 lastCacheHash,
		Ogre::CommandBuffer* commandBuffer) override;


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
