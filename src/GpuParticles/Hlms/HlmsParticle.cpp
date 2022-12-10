/*
 * File: HlmsParticle.cpp
 * Author: Przemysław Bągard
 * Created: 2021-4-23
 *
 */

#include "GpuParticles/Hlms/HlmsJsonParticleAtlas.h"
#include "GpuParticles/Hlms/HlmsParticle.h"
#include "GpuParticles/Hlms/HlmsParticleDatablock.h"

#include <OgreRoot.h>
#include <OgreArchiveManager.h>
#include <OgreHlmsManager.h>
#include <OgreCamera.h>
#include <OgreHlms.h>

#include "GpuParticles/GpuParticleSystemWorld.h"

#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCommandBuffer.h>

#include <Vao/OgreReadOnlyBufferPacked.h>


Ogre::Matrix4 HlmsParticleListener::getPrevCameraVP() const
{
    return mPrevCameraVP;
}

Ogre::Vector2 HlmsParticleListener::getCameraProjectionAB() const
{
    return mCameraProjectionAB;
}

Ogre::uint32 HlmsParticleListener::getPassBufferSize(const Ogre::CompositorShadowNode* shadowNode, bool casterPass, bool dualParaboloid, Ogre::SceneManager* sceneManager) const {

    Ogre::uint32 size = 0;

    if (!casterPass) {

        unsigned int cameraViewMatrixSize = sizeof(float) * 4 * 4;
        unsigned int invCameraProjMatrixSize = sizeof(float) * 4 * 4;

        size += cameraViewMatrixSize + invCameraProjMatrixSize;
    }
    return size;
}

float* HlmsParticleListener::preparePassBuffer(const Ogre::CompositorShadowNode* shadowNode, bool casterPass, bool dualParaboloid, Ogre::SceneManager* sceneManager, float* passBufferPtr) {

    Ogre::CamerasInProgress cameras = sceneManager->getCamerasInProgress();

    if (!casterPass )
    {
        mPrevCameraVP = mCameraVP;

        Ogre::Matrix4 viewMatrix = cameras.renderingCamera->getViewMatrix(true);

//        Ogre::Vector3 cameraPos = cameras.renderingCamera->_getCachedDerivedPosition();
//        Ogre::Quaternion cameraRot = cameras.renderingCamera->_getCachedDerivedOrientation();
//        Ogre::Vector3 cameraRight = cameraRot * Ogre::Vector3::UNIT_X;
//        Ogre::Vector3 cameraUp = cameraRot * Ogre::Vector3::UNIT_Y;
//        Ogre::Vector3 cameraForward = cameraRot * Ogre::Vector3::NEGATIVE_UNIT_Z;

        Ogre::Matrix4 projectionMatrix = cameras.renderingCamera->getProjectionMatrixWithRSDepth();
        Ogre::RenderPassDescriptor *renderPassDesc = mHlms->getRenderSystem()->getCurrentPassDescriptor();
        if( renderPassDesc->requiresTextureFlipping() )
        {
            projectionMatrix[1][0]  = -projectionMatrix[1][0];
            projectionMatrix[1][1]  = -projectionMatrix[1][1];
            projectionMatrix[1][2]  = -projectionMatrix[1][2];
            projectionMatrix[1][3]  = -projectionMatrix[1][3];
        }
        Ogre::Matrix4 viewProj = projectionMatrix * viewMatrix;
        mCameraVP = viewProj;

        mCameraProjectionAB = cameras.renderingCamera->getProjectionParamsAB();

        Ogre::Matrix4 invViewProj = viewProj.inverse();
        for( size_t i=0; i<16; ++i )
            *passBufferPtr++ = (float)invViewProj[0][i];

        for( size_t i=0; i<16; ++i )
            *passBufferPtr++ = (float)viewMatrix[0][i];
    }
    return passBufferPtr;
}


HlmsParticleListener* HlmsParticle::getParticleListener()
{
    return &mParticleListener;
}

Ogre::HlmsCache HlmsParticle::preparePassHash(const Ogre::CompositorShadowNode* shadowNode, bool casterPass, bool dualParaboloid, Ogre::SceneManager* sceneManager)
{
    return Ogre::HlmsUnlit::preparePassHash(shadowNode, casterPass, dualParaboloid, sceneManager);
}

void HlmsParticle::calculateHashForPreCreate(Ogre::Renderable* renderable, Ogre::PiecesMap* inOutPieces) {

    setProperty( *Ogre::HlmsBaseProp::UvCountPtrs[0], 2);

    HlmsUnlit::calculateHashForPreCreate(renderable, inOutPieces);

    setProperty( "hlms_colour",  1);

    const Ogre::Renderable::CustomParameterMap &customParams = renderable->getCustomParameters();
    if( customParams.find( GpuParticleSystemWorld::RenderableTypeId ) != customParams.end() )
    {
        setProperty( "particleWorldEnabled", 1 );

        Ogre::Renderable::CustomParameterMap::const_iterator it = customParams.find(GpuParticleSystemWorld::RenderableCustomParamBucketSize);
        if(it != customParams.end()) {
            Ogre::Vector4 vec = it->second;
            setProperty( "BucketSize", (Ogre::uint32)vec.x);
        }
    }
}

HlmsParticle* HlmsParticle::registerHlms(const Ogre::String& rootHlmsFolder,
                                         const Ogre::String& particleRootHlmsFolder,
                                         bool withHlmsPathPrefix)
{
    bool mainFilesInParticle = true;

    Ogre::String mainFolderPath;
    Ogre::String particleMainFolderPath;
    Ogre::StringVector libraryFolderPaths;
    Ogre::StringVector libraryParticleFolderPaths;

    getDefaultPaths(mainFolderPath, libraryFolderPaths);
    getAdditionalDefaultPaths(particleMainFolderPath, libraryParticleFolderPaths, withHlmsPathPrefix);
    if(mainFilesInParticle) {
        libraryFolderPaths.push_back(mainFolderPath);
    }
    else {
        libraryParticleFolderPaths.push_back(particleMainFolderPath);
    }

    Ogre::ArchiveVec archive;

    for (Ogre::String str : libraryFolderPaths) {

        Ogre::Archive* archiveLibrary = Ogre::ArchiveManager::getSingleton().load(rootHlmsFolder + str, "FileSystem", true);

        archive.push_back(archiveLibrary);
    }

    for (Ogre::String str : libraryParticleFolderPaths) {

        Ogre::Archive* archiveLibrary = Ogre::ArchiveManager::getSingleton().load(particleRootHlmsFolder + str, "FileSystem", true);

        archive.push_back(archiveLibrary);
    }

    Ogre::Archive* archiveUnlit = nullptr;
    if(mainFilesInParticle) {
        archiveUnlit = Ogre::ArchiveManager::getSingleton().load(particleRootHlmsFolder + particleMainFolderPath, "FileSystem", true);
    }
    else {
        archiveUnlit = Ogre::ArchiveManager::getSingleton().load(rootHlmsFolder + mainFolderPath, "FileSystem", true);
    }
    HlmsParticle* hlms = OGRE_NEW HlmsParticle(archiveUnlit, &archive);
    Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlms);

    return hlms;
}

Ogre::uint32 HlmsParticle::fillBuffersFor(const Ogre::HlmsCache* cache, const Ogre::QueuedRenderable& queuedRenderable, bool casterPass, Ogre::uint32 lastCacheHash, Ogre::CommandBuffer* commandBuffer, bool isV1)
{

    const Ogre::Renderable::CustomParameterMap &customParams = queuedRenderable.renderable->getCustomParameters();

    Ogre::uint32 startBucketIndex = 0;

    if( customParams.find( GpuParticleSystemWorld::RenderableTypeId ) != customParams.end() )
    {
        GpuParticleSystemWorld::ParticleRenderable* particleRenderable = dynamic_cast<GpuParticleSystemWorld::ParticleRenderable*>(queuedRenderable.renderable);
        GpuParticleSystemWorld* particleSystemWorld = particleRenderable->getGpuParticleSystemWorld();

        // Particles data
        {
            Ogre::ReadOnlyBufferPacked* particleDataTexBuffer = particleSystemWorld->getParticleBufferAsReadOnly();
            int totalSize = GpuParticleSystemWorld::ParticleDataStructSize * particleSystemWorld->getMaxParticles();
            *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(Ogre::VertexShader, ParticleDataTexSlot, particleDataTexBuffer, 0, totalSize);
            *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(Ogre::PixelShader, ParticleDataTexSlot, particleDataTexBuffer, 0, totalSize);
    //        rebindTexBuffer( commandBuffer );
        }

        // Bucket data
        {
            Ogre::ReadOnlyBufferPacked* bucketGroupsBuffer = particleSystemWorld->getEntryBucketBufferAsReadOnly();
            int totalSize = GpuParticleSystemWorld::EntryBucketDataStructSize * particleSystemWorld->getBucketGroupsCountToRender();
            *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(Ogre::VertexShader, BucketGroupDataTexSlot, bucketGroupsBuffer, 0, totalSize);
            *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(Ogre::PixelShader, BucketGroupDataTexSlot, bucketGroupsBuffer, 0, totalSize);
    //        rebindTexBuffer( commandBuffer );
        }

        // Emitter core data
        {
            Ogre::ReadOnlyBufferPacked* emitterCoreBuffer = particleSystemWorld->getEmitterCoreBufferAsReadOnly();
            int totalSize = GpuParticleSystemWorld::EmitterCoreDataStructSize * particleSystemWorld->getMaxEmitterCores();
            *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(Ogre::VertexShader, EmitterCoreDataTexSlot, emitterCoreBuffer, 0, totalSize);
            *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(Ogre::PixelShader, EmitterCoreDataTexSlot, emitterCoreBuffer, 0, totalSize);
    //        rebindTexBuffer( commandBuffer );
        }

        startBucketIndex = particleRenderable->getCachedStartBucketIndex();
    }

    // Keep in mind that only the first one has to be correct, as Ogre's auto instancing will increment the base value automatically.
    Ogre::uint32 drawId = Ogre::HlmsUnlit::fillBuffersFor(cache, queuedRenderable, casterPass, lastCacheHash, commandBuffer, isV1);

    // After Ogre::HlmsUnlit::fillBuffersFor const buffer was already filled. There was one
    // unused value though at position [3]. Buffer  was shifted by 4.

    // Beware that const buffer .w is not needed in unlit, but may be used in pbs
    Ogre::uint32 * RESTRICT_ALIAS currentMappedConstBuffer = mCurrentMappedConstBuffer;
    *(currentMappedConstBuffer-1) = startBucketIndex;

    return drawId;
}

void HlmsParticle::frameEnded()
{
    Ogre::RenderSystem* renderSystem = getRenderSystem();
    renderSystem->_setTexture(ParticleDataTexSlot, 0, false);
    renderSystem->_setTexture(EmitterDataTexSlot, 0, false);
    renderSystem->_setTexture(BucketGroupDataTexSlot, 0, false);
    renderSystem->_setTexture(EmitterCoreDataTexSlot, 0, false);

    HlmsUnlit::frameEnded();
}

Ogre::HlmsDatablock* HlmsParticle::createDatablockImpl(Ogre::IdString datablockName,
                                                       const Ogre::HlmsMacroblock* macroblock,
                                                       const Ogre::HlmsBlendblock* blendblock,
                                                       const Ogre::HlmsParamVec& paramVec)
{
    return OGRE_NEW HlmsParticleDatablock( datablockName, this, macroblock, blendblock, paramVec );
}

HlmsParticleDatablock* HlmsParticle::cloneFromUnlitDatablock( const Ogre::HlmsUnlitDatablock* srcDatablock, const Ogre::String& name )
{
//    OgreProfileExhaustive( "HlmsDatablock::clone" );

    Ogre::HlmsDatablock *datablock = this->createDatablock( name, name,
                                                            Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(),
                                                            Ogre::HlmsParamVec() );
    HlmsParticleDatablock* particleDatablock = static_cast<HlmsParticleDatablock*>(datablock);

    particleDatablock->copyFromUnlitDatablockImpl(srcDatablock);

    return particleDatablock;
}

void HlmsParticle::_loadJson(const rapidjson::Value& jsonValue, const Ogre::HlmsJson::NamedBlocks& blocks, Ogre::HlmsDatablock* datablock, const Ogre::String& resourceGroup, Ogre::HlmsJsonListener* listener, const Ogre::String& additionalTextureExtension) const
{
    Ogre::HlmsUnlit::_loadJson(jsonValue, blocks, datablock, resourceGroup, listener, additionalTextureExtension);
    HlmsJsonParticleAtlas jsonParticleAtlas( mHlmsManager, mRenderSystem->getTextureGpuManager() );
    jsonParticleAtlas.loadMaterial( jsonValue, blocks, datablock, resourceGroup );
}

void HlmsParticle::_saveJson(const Ogre::HlmsDatablock* datablock, Ogre::String& outString, Ogre::HlmsJsonListener* listener, const Ogre::String& additionalTextureExtension) const
{
    Ogre::HlmsUnlit::_saveJson(datablock, outString, listener, additionalTextureExtension);
    outString += ",";       // at the end of last unlit parameter
    HlmsJsonParticleAtlas jsonParticleAtlas( mHlmsManager, mRenderSystem->getTextureGpuManager() );
    jsonParticleAtlas.saveMaterial( datablock, outString );
}
