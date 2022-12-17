/*
 * File: GpuParticleSystemWorld.cpp
 * Author: Przemysław Bągard
 * Created: 2021-5-4
 *
 */

#include "GpuParticles/GpuParticleSystemWorld.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"
#include "Vao/OgreUavBufferPacked.h"
#include "Vao/OgreReadOnlyBufferPacked.h"

#include "OgreSceneManager.h"
#include "Math/Array/OgreObjectData.h"

#include "OgreRoot.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"
#include <OgreHlmsCompute.h>
#include <OgreHlmsComputeJob.h>

#include <OgreCompositorNode.h>
#include <OgreCompositorWorkspace.h>
#include <OgreImage2.h>
#include <OgreSceneManager.h>

#include <OgreLogManager.h>

#include "GpuParticleAffectorCommon.h"

//#include <Other/DebugVariable.h>

#include <GpuParticles/Hlms/HlmsParticle.h>
#include <GpuParticles/Hlms/HlmsParticleDatablock.h>

using namespace Ogre;

namespace
{
//    struct ParticleData
//    {
//        /// Position
//        float px, py, pz;

//        /// Rotate xz
//        float r;

//        /// Colour
//        float cr, cg, cb, ca;

//        /// Size
//        float sx, sy;

//        /// Sprite percentage: ceil is taken as result and is casted to uint
//        /// (it is easier to interpolate that way).
//        float spritePercent;

//        float lifetime;
//        float maxLifetime;

//        float dirX, dirY, dirZ;
//        float dirVelocity;
//    };
}

/// Like thread group data, but with BucketSize.
struct GpuParticleSystemWorld::BucketGroupData
{
    //    Ogre::uint32 bucketId = 0;
    Ogre::uint32 emitterInstanceId = 0;
    Ogre::uint32 emitterCoreId = 0;

    /// When we spawn new particles and in effect use more than one bucket for the same emitter instance,
    /// this is to inform, that this bucket will be newer than the previous one.
    /// Not important in other places than creating new particles.
    //    Ogre::uint32 timeOffset = 0;

    /// Range - particles with localId < fromParticleInBucket and localId > tillParticleInBucket
    /// will be skipped during shader processing.
    //    Ogre::uint32 fromParticleInBucket = 0;
    //    Ogre::uint32 tillParticleInBucket = 0;

    /// Index of last particle. Particles are ordered from youngest to oldest,
    /// so we start from last particle.
    Ogre::uint32 lastParticleIndex = 0;

    /// How many particles to handle (0 <= particleCount < BucketSize)
    Ogre::uint32 particleCount = 0;

    /// WorkBucket (like WorkGroup but it is per bucket size) may use particles from max 2 buckets.
    Ogre::uint32 nextBucketParticleIndex = 0;
};

const int GpuParticleSystemWorld::ParticleDataStructSize = sizeof(float) * 17u; // count number of floats inside 'ParticleData' struct

const int GpuParticleSystemWorld::EntryBucketDataStructSize = sizeof(Ogre::uint32) * 5u;

const int GpuParticleSystemWorld::EmitterInstanceDataStructSize =
        sizeof(float) * 16u +       // Emitter location matrix
        sizeof(Ogre::uint32) * 1u;  // run (bool)


const int GpuParticleSystemWorld::EmitterCoreDataStructSize =
        sizeof(float) * 8u +            // Colour range
        sizeof(float) * 4u +            // Size range
        sizeof(float) * 2u +            // Spot angle range
        sizeof(float) * 2u +            // Direction velocity range
        sizeof(float) * 2u +            // Lifetime range
        sizeof(float) * 1u +            // nextParticleSpawnTime
        sizeof(float) * 3u +            // Gravity
        sizeof(Ogre::uint32) * 1u +     // mDepthCollision
        sizeof(Ogre::uint32) * 1u +     // mGenerateRandomSpriteRange (upper possible index value).
        sizeof(Ogre::uint32) * 1u +     // mUseSpriteTrack (bool)
        sizeof(float) * 8u +            // Sprite track times (tresholds, when new sprite is used)
        sizeof(float) * 4u * 8u +       // Sprite texture coordinate ranges (for eight slots array)
        sizeof(Ogre::uint32) * 1u +     // mUseColourTrack (bool)
        sizeof(float) * 8u +            // Colour track times
        sizeof(float) * 3u * 8u +       // Colour (rgb) track values
        sizeof(Ogre::uint32) * 1u +     // mUseAlphaTrack (bool)
        sizeof(float) * 8u +            // Alpha track times
        sizeof(float) * 8u +            // Alpha track values
        sizeof(Ogre::uint32) * 1u +     // mUseFader (bool) (fader may be for alpha only or for all colour)
        sizeof(float) * 1u +            // mFaderStartPhaseTime (start phase of particle for fader)
        sizeof(float) * 1u +            // mFaderEndPhaseTime (last 'mFaderEndPhaseTime' seconds of particle life for fader)
        sizeof(Ogre::uint32) * 1u +     // mUseSizeTrack (bool)
        sizeof(float) * 8u +            // Size track times
        sizeof(float) * 2u * 8u +       // Size (rgb) track values
        sizeof(Ogre::uint32) * 1u +     // mUseVelocityTrack (bool)
        sizeof(float) * 8u +            // Velocity track times
        sizeof(float) * 8u +            // Velocity track values (only scalar because direction is important)
        sizeof(Ogre::uint32) * 1u +     // isUniformSize (bool)
        sizeof(Ogre::uint32) * 1u +     // billboardType (Ogre::v1::BillboardType)
        sizeof(Ogre::uint32) * 1u +     // Spawn shape
        sizeof(float) * 3u;             // Spawn shape params (like box dimensions)

const int GpuParticleSystemWorld::ParticleWorldDataStructSize =
        sizeof(float) * 16u +       // camera prev View * Projection
        sizeof(float) * 16u +       // camera prev View * Projection Inversion
        sizeof(float) * 2u +        // camera projection AB
        sizeof(float) * 1u +        // elapsed time
        sizeof(Ogre::uint32) * 1u;  // randomIteration


const int GpuParticleSystemWorld::RenderableTypeId = 5002; // Magic number to identify this renderable insinde HlmsParticle
//const int GpuParticleSystemWorld::RenderableCustomParamBucketSize = 5003; // Bucket size param (to not dynamic_cast renderable during HlmsParticle::calculateHashForPreCreate).


GpuParticleSystemWorld::GpuParticleSystemWorld(Ogre::IdType id,
                                               Ogre::ObjectMemoryManager* objectMemoryManager,
                                               Ogre::SceneManager* manager,
                                               Ogre::uint8 renderQueueId,
                                               HlmsParticleListener* hlmsParticleListener,
                                               const std::vector<GpuParticleAffector*>& affectors,
                                               bool useDepthTexture,
                                               CompositorWorkspace* compositorWorkspace,
                                               IdString depthTextureCompositorNode,
                                               IdString depthTextureName)
    : MovableObject( id, objectMemoryManager, manager, renderQueueId )
    , mVaoManager(manager->getDestinationRenderSystem()->getVaoManager())
    , mMaxParticles(0)
    , mMaxEmitterInstances(0)
    , mMaxEmitterCores(0)
    , mBucketCount(0)
    , mBucketSize(0)
    , mThreadsPerGroup(0)
    , mGroupsPerBucket(0)
    , mParticleDataStructFinalSize(0)
    , mEmitterCoreDataStructFinalSize(0)
    , mHlmsParticleListener(hlmsParticleListener)
    , mUseDepthTexture(useDepthTexture)
    , mInitLocationInUpdate(true)
    , mCompositorWorkspace(compositorWorkspace)
    , mDepthTextureCompositorNode(depthTextureCompositorNode)
    , mDepthTextureName(depthTextureName)
{
    //Set the bounds!!! Very important! If you don't set it, the object will not
    //appear on screen as it will always fail the frustum culling.
    //This example uses an infinite aabb; but you really want to use an Aabb as tight
    //as possible for maximum efficiency (so Ogre avoids rendering an object that
    //is off-screen)
    //Note the WorldAabb and the WorldRadius will be automatically updated by Ogre
    //every frame as rendering begins (it's calculated based on the local version
    //combined with the scene node's transform).
    Ogre::Aabb aabb( Ogre::Aabb::BOX_INFINITE );
    mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
    mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
    mObjectData.mLocalRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();
    mObjectData.mWorldRadius[mObjectData.mIndex] = std::numeric_limits<Real>::max();

    // Just to validate AffectorType is used not more than once.
    std::map<AffectorType, const GpuParticleAffector*> registeredAffectorMap;

    // GpuParticleSystemWorld takes ownerhip of afftector list.,
    mRegisteredAffectorList.reserve(affectors.size());
    for (size_t i = 0; i < affectors.size(); ++i) {
        const GpuParticleAffector* affector = affectors[i];

        if(affector->getAffectorProperty().empty()) {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "GpuParticleAffector cannot have empty property name.",
                         "GpuParticleSystemWorld::GpuParticleSystemWorld" );
            OGRE_DELETE affector;
        }
        else if(registeredAffectorMap.find(affector->getType()) != registeredAffectorMap.end()) {
            OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM,
                         "There exists already GpuParticleAffector with the same slot "
                         + Ogre::StringConverter::toString((int)affector->getType())
                         + ". Affector property '" + affector->getAffectorProperty() + "' collide with '"
                         + registeredAffectorMap[affector->getType()]->getAffectorProperty() + "'.",
                         "GpuParticleSystemWorld::GpuParticleSystemWorld" );
            OGRE_DELETE affector;
        }
        else {
            registeredAffectorMap[affector->getType()] = affector;
            mRegisteredAffectorList.push_back(affector);
        }
    }

    if(mUseDepthTexture) {
        if(!mCompositorWorkspace) {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "'mCompositorWorkspace' can't be null for mUseDepthTexture == true.",
                         "GpuParticleSystemWorld::GpuParticleSystemWorld" );
        }
        if(mDepthTextureCompositorNode == Ogre::IdString()) {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "'mDepthTextureCompositorNode' can't be empty for mUseDepthTexture == true.",
                         "GpuParticleSystemWorld::GpuParticleSystemWorld" );
        }
        if(mDepthTextureName == Ogre::IdString()) {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "'mDepthTextureName' can't be empty for mUseDepthTexture == true.",
                         "GpuParticleSystemWorld::GpuParticleSystemWorld" );
        }
    }
}

GpuParticleSystemWorld::~GpuParticleSystemWorld()
{
    mRenderables.clear();
    for (size_t i = 0; i < mParticleRenderables.size(); ++i) {
        delete mParticleRenderables[i];
    }
    mParticleRenderables.clear();
    destroyBuffers();

    for (size_t i = 0; i < mRegisteredAffectorList.size(); ++i) {
        OGRE_DELETE mRegisteredAffectorList[i];
    }
}

void GpuParticleSystemWorld::registerEmitterCore(const GpuParticleEmitter* particleEmitterCore)
{
    GpuParticleEmitterMap::iterator it = mRegisteredEmitterCoresSet.find(particleEmitterCore);
    if(it != mRegisteredEmitterCoresSet.end()) {

        // Multiple registration - just increase registered counter
        ++it->second;

        return;
    }

    if(mRegisteredEmitterCores.size() >= mMaxEmitterCores) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "Exceeded maximum number of particle emitter cores."
                     " Initialize system with more 'maxParticleSystems'.",
                     "GpuParticleSystemWorld::registerEmitterCore" );
    }

    mRegisteredEmitterCores.push_back(particleEmitterCore);
    mRegisteredEmitterCoresSet.insert(std::pair<const GpuParticleEmitter*, Ogre::uint16>(particleEmitterCore, 1));

    // find if renderable exists.

    bool found = false;
    for (size_t i = 0; i < mParticleRenderables.size(); ++i) {
        ParticleRenderable* particleRenderable = mParticleRenderables[i];
        if(particleRenderable->mDatablockName == particleEmitterCore->mDatablockName) {
            found = true;
            break;
        }
    }

    if(!found) {
        // create new renderable
        ParticleRenderable* particleRenderable = new ParticleRenderable(this, mVaoManager, mIndexBuffer, particleEmitterCore->mDatablockName, mBucketSize);

        mParticleRenderables.push_back(particleRenderable);
        mRenderables.push_back(particleRenderable);
    }

    mEmitterCoreBufferDirty = true;
}

void GpuParticleSystemWorld::registerEmitterCore(const std::vector<GpuParticleEmitter*>& emitters)
{
    for (size_t i = 0; i < emitters.size(); ++i) {
        registerEmitterCore(emitters[i]);
    }
}

void GpuParticleSystemWorld::registerEmitterCore(const GpuParticleSystem* particleSystem)
{
    for (size_t i = 0; i < particleSystem->getEmitters().size(); ++i) {
        registerEmitterCore(particleSystem->getEmitters()[i]);
    }
}

void GpuParticleSystemWorld::unregisterEmitterCore(const GpuParticleEmitter* particleEmitterCore)
{
    GpuParticleEmitterMap::iterator it = mRegisteredEmitterCoresSet.find(particleEmitterCore);
    if(it == mRegisteredEmitterCoresSet.end()) {
        return;
    }

    if(it->second > 1) {

        // Just decrease registered counter.
        --it->second;
        return;
    }

    mRegisteredEmitterCoresSet.erase(it);

    for (unsigned int i = 0; i < mRegisteredEmitterCores.size(); ++i) {
        if(mRegisteredEmitterCores[i] == particleEmitterCore) {
            mRegisteredEmitterCores.erase(mRegisteredEmitterCores.begin()+i);
            break;
        }
    }

    // check if there is any GpuParticleEmitter with the same datablock
    bool found = false;
    Ogre::String datablockName = particleEmitterCore->mDatablockName;
    for (unsigned int i = 0; i < mRegisteredEmitterCores.size(); ++i) {
        if(mRegisteredEmitterCores[i]->mDatablockName == datablockName) {
            found = true;
            break;
        }
    }

    if(!found) {
        destroyParticleRenderable(datablockName);
    }

    mEmitterCoreBufferDirty = true;
}

void GpuParticleSystemWorld::unregisterEmitterCore(const std::vector<GpuParticleEmitter*>& emitters)
{
    for (size_t i = 0; i < emitters.size(); ++i) {
        unregisterEmitterCore(emitters[i]);
    }
}

void GpuParticleSystemWorld::unregisterEmitterCore(const GpuParticleSystem* particleSystem)
{
    for (size_t i = 0; i < particleSystem->getEmitters().size(); ++i) {
        unregisterEmitterCore(particleSystem->getEmitters()[i]);
    }
}

void GpuParticleSystemWorld::makeEmitterCoresDirty()
{
    mEmitterCoreBufferDirty = true;
}

bool GpuParticleSystemWorld::canAdd(const GpuParticleEmitter* emitterCore) const
{
    if(mRegisteredEmitterCoresSet.find(emitterCore) == mRegisteredEmitterCoresSet.end()) {
        return false;
    }

    if(mEmitterInstances.size() >= mMaxEmitterInstances) {
        return false;
    }

    uint32 buckets = estimateRequiredBucketCount(emitterCore);
    if(buckets > (uint32)mAvailableBucketsStack.size()) {
        return false;
    }

    return true;
}

bool GpuParticleSystemWorld::canAdd(const std::vector<GpuParticleEmitter*>& emitters) const
{
    for (size_t i = 0; i < emitters.size(); ++i) {
        if(!canAdd(emitters[i])) {
            return false;
        }
    }
    return true;
}

bool GpuParticleSystemWorld::canAdd(const GpuParticleSystem* particleSystem) const
{
    for (size_t i = 0; i < particleSystem->getEmitters().size(); ++i) {
        if(!canAdd(particleSystem->getEmitters()[i])) {
            return false;
        }
    }
    return true;
}

void GpuParticleSystemWorld::stop(uint64 instanceId, bool destroyAllParticles)
{
    for (int i = mEmitterInstances.size()-1; i >= 0; --i) {
        if(mEmitterInstances[i].mId == instanceId) {
            stopEmitter(i, destroyAllParticles);
        }
    }
}

void GpuParticleSystemWorld::stopAll()
{
    for (int i = mEmitterInstances.size()-1; i >= 0; --i) {
        freeBuckets(mEmitterInstances[i]);
    }
    mEmitterInstances.clear();
}

void GpuParticleSystemWorld::init(uint32 maxParticles,
                                  uint16 maxEmitterInstances,
                                  uint16 maxEmitterCores,
                                  uint16 bucketSize,
                                  uint16 gpuThreadsPerGroup)
{
    if(gpuThreadsPerGroup == 0) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "gpuThreadsPerGroup cannot be 0."
                     " 64 is default value.",
                     "GpuParticleSystemWorld::init" );
    }

    if(bucketSize == 0) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "bucketSize cannot be 0."
                     " Multiplication of 64 should work well.",
                     "GpuParticleSystemWorld::init" );
    }

    if(bucketSize % gpuThreadsPerGroup != 0) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "bucketSize has to be multiplication of gpuThreadsPerGroup."
                     " Multiplication of 64 should work well.",
                     "GpuParticleSystemWorld::init" );
    }

    if(maxParticles < bucketSize) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "maxParticles cannot be lesser than bucketSize.",
                     "GpuParticleSystemWorld::init" );
    }

    if(maxEmitterCores == 0) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "maxEmitterCores cannot be 0.",
                     "GpuParticleSystemWorld::init" );
    }

    if(maxEmitterInstances == 0) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "maxEmitterInstances cannot be 0.",
                     "GpuParticleSystemWorld::init" );
    }

    mBucketSize = bucketSize;
    mBucketCount = (maxParticles + bucketSize - 1) / bucketSize;
    mMaxParticles = mBucketSize * mBucketCount;
    mThreadsPerGroup = gpuThreadsPerGroup;
    mGroupsPerBucket = bucketSize / gpuThreadsPerGroup;
    mMaxEmitterInstances = maxEmitterInstances;
    mMaxEmitterCores = maxEmitterCores;

    mAvailableBucketsStack.reserve(mBucketCount);
    for (Ogre::uint32 i = 0; i < mBucketCount; ++i) {
        mAvailableBucketsStack.push_back(mBucketCount-1-i);
    }

    mRegisteredEmitterCores.reserve(mMaxEmitterCores);
    mEmitterInstances.reserve(mMaxEmitterInstances);

    initBuffers();
}

void GpuParticleSystemWorld::createEmitterInstance(const GpuParticleEmitter* gpuParticleEmitterCore,
                                                   const Ogre::Matrix4& matParent,
                                                   Ogre::Node* parentNode,
                                                   Ogre::uint64 idCounter)
{
    mEmitterInstances.push_back(EmitterInstance());
    EmitterInstance& emitter = mEmitterInstances[mEmitterInstances.size()-1];
    emitter.mGpuParticleEmitter = gpuParticleEmitterCore;

    Ogre::Matrix4 matOffset;
    matOffset.makeTransform(gpuParticleEmitterCore->mPos, Ogre::Vector3::UNIT_SCALE, gpuParticleEmitterCore->mRot);

    Ogre::Matrix4 mat;
    mat = matParent * matOffset;

    Ogre::Vector3 scaleTemp;
    mat.decomposition(emitter.mPos, scaleTemp, emitter.mRot);

    emitter.mNode = parentNode;
    bool ok = requestBuckets(emitter);
    assert(ok); // already checked if we can allocate

    for (size_t j = 0; j < mRegisteredEmitterCores.size(); ++j) {
        if(emitter.mGpuParticleEmitter == mRegisteredEmitterCores[j]) {
            emitter.mGpuParticleEmitterIndex = j;
            break;
        }
    }

    for (size_t j = 0; j < mParticleRenderables.size(); ++j) {
        ParticleRenderable* particleRenderable = mParticleRenderables[j];
        if(particleRenderable->mDatablockName == gpuParticleEmitterCore->mDatablockName) {
            emitter.mParticleRenderable = particleRenderable;
            break;
        }
    }

    emitter.mId = idCounter;
}

Ogre::uint64 GpuParticleSystemWorld::start(const GpuParticleEmitter* emitterCore, Ogre::Node* parentNode, const Ogre::Vector3& parentPos, const Ogre::Quaternion& parentRot)
{
    if(mRegisteredEmitterCoresSet.find(emitterCore) == mRegisteredEmitterCoresSet.end()) {
        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                     "Used not registered particle system."
                     " Register GpuParticleEmitter by GpuParticleSystemWorld::registerEmitterCore.",
                     "GpuParticleSystemWorld::start" );
        return 0;
    }

    if(mEmitterInstances.size() >= mMaxEmitterInstances) {
        Ogre::LogManager::getSingletonPtr()->logMessage("GpuParticleSystemWorld warning: Could not add particle system instance: exceeded maximum number of emitters!", Ogre::LML_NORMAL);
        return 0;
    }

    Ogre::uint32 requiredBuckets = estimateRequiredBucketCount(emitterCore);
    if(!canAllocateBuckets(requiredBuckets)) {
        Ogre::LogManager::getSingletonPtr()->logMessage("GpuParticleSystemWorld warning: Could not add particle system instance: not enough particles available!", Ogre::LML_NORMAL);
        return 0;
    }

    Ogre::uint64 idCounter = getNextId();

    Ogre::Matrix4 matParent;
    matParent.makeTransform(parentPos, Ogre::Vector3::UNIT_SCALE, parentRot);

    createEmitterInstance(emitterCore, matParent, parentNode, idCounter);

    return idCounter;
}

uint64 GpuParticleSystemWorld::start(const std::vector<GpuParticleEmitter*>& emitters, Node* parentNode, const Ogre::Vector3& parentPos, const Ogre::Quaternion& parentRot)
{
    Ogre::uint32 requiredBuckets = 0;
    for (size_t i = 0; i < emitters.size(); ++i) {
        if(mRegisteredEmitterCoresSet.find(emitters[i]) == mRegisteredEmitterCoresSet.end()) {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Used not registered particle system."
                         " Register GpuParticleEmitter by GpuParticleSystemWorld::registerEmitterCore.",
                         "GpuParticleSystemWorld::start" );
            return 0;
        }

        uint32 buckets = estimateRequiredBucketCount(emitters[i]);
        requiredBuckets += buckets;
    }

    if(mEmitterInstances.size() + emitters.size() > mMaxEmitterInstances) {
        Ogre::LogManager::getSingletonPtr()->logMessage("GpuParticleSystemWorld warning: Could not add particle system instance: exceeded maximum number of emitters!", Ogre::LML_NORMAL);
        return 0;
    }

    if(!canAllocateBuckets(requiredBuckets)) {
        Ogre::LogManager::getSingletonPtr()->logMessage("GpuParticleSystemWorld warning: Could not add particle system instance: not enough particles available!", Ogre::LML_NORMAL);
        return 0;
    }

    Ogre::uint64 idCounter = getNextId();

    Ogre::Matrix4 matParent;
    matParent.makeTransform(parentPos, Ogre::Vector3::UNIT_SCALE, parentRot);

    for (size_t i = 0; i < emitters.size(); ++i) {
        const GpuParticleEmitter* gpuParticleEmitterCore = emitters[i];
        createEmitterInstance(gpuParticleEmitterCore, matParent, parentNode, idCounter);
    }

    return idCounter;
}

uint64 GpuParticleSystemWorld::start(const GpuParticleSystem* particleSystem, Node* parentNode, const Vector3& parentPos, const Quaternion& parentRot)
{
    return start(particleSystem->getEmitters(), parentNode, parentPos, parentRot);
}

void GpuParticleSystemWorld::stopEmitter(int instanceIndex, bool destroyAllParticles)
{
    if(instanceIndex < 0 || (uint32)instanceIndex >= mEmitterInstances.size()) {
        return;
    }

    mEmitterInstances[instanceIndex].mRun = false;
    if(destroyAllParticles || mEmitterInstances[instanceIndex].mParticleCount + mEmitterInstances[instanceIndex].mParticleAddedThisFrameCount == 0) {
        freeBuckets(mEmitterInstances[instanceIndex]);
        mEmitterInstances.erase(mEmitterInstances.begin()+instanceIndex);
    }
    else {
        freeUnusedBuckets(mEmitterInstances[instanceIndex]);
    }
}

void GpuParticleSystemWorld::updateInstances(float elapsedTime)
{
    for (int i = 0; i < (int)mEmitterInstances.size(); ++i) {
        EmitterInstance& emitterInstance = mEmitterInstances[i];
        const GpuParticleEmitter* emitterCore = emitterInstance.mGpuParticleEmitter;
        //        float oldTimeSinceStarted = emitterInstance.mTimeSinceStarted;

        emitterInstance.mParticleCount += emitterInstance.mParticleAddedThisFrameCount;
        emitterInstance.mParticleAddedThisFrameCount = 0;

        emitterInstance.mTimeSinceStarted += elapsedTime;
        if(!emitterInstance.mRun) {
            emitterInstance.mTimeSinceStopped += elapsedTime;

            // check if emitter stopped long enough time
            if(emitterInstance.mTimeSinceStopped >= emitterCore->getMaxParticleLifetime() || emitterInstance.mParticleCount == 0) {
                // remove
                freeBuckets(emitterInstance);
                mEmitterInstances.erase(mEmitterInstances.begin()+i);
                --i;
                continue;
            }

            freeUnusedBuckets(mEmitterInstances[i]);
        }

        float maxParticleLifetime = emitterCore->getMaxParticleLifetime();

        if(!emitterCore->mBurstMode || !emitterCore->isImmediate()) {

            // remove oldest particles
            if(emitterInstance.mTimeSinceStarted > maxParticleLifetime)
            {
                float emissionRate = emitterCore->getEmissionRate();
                float toFinishF = elapsedTime * emissionRate + emitterInstance.mFinishingParticleRemainder;
                Ogre::uint32 toFinish = (Ogre::uint32)toFinishF;
                emitterInstance.mFinishingParticleRemainder = toFinishF - toFinish;

                // cap number of particles to finish to be no greater than actual particle count.
                if(emitterInstance.mParticleCount < toFinish) {
                    toFinish = emitterInstance.mParticleCount;
                }

                emitterInstance.shiftParticleArrayIndex(toFinish);
                emitterInstance.mParticleCount -= toFinish;
                emitterInstance.mParticleCreatedCount -= toFinish;
            }
        }


        if(!emitterCore->mBurstMode) {

            // add new particles if emitter is still running
            if(emitterInstance.mRun) {

                float toCreateF = elapsedTime * emitterCore->mEmissionRate + emitterInstance.mParticleRemainder;
                Ogre::uint32 toCreate = (Ogre::uint32)toCreateF;
                emitterInstance.mParticleRemainder = toCreateF - toCreate;

                // cap number of particles to create to ensure maximum particle count won't be reached.
                Ogre::uint32 maxParticles = emitterCore->getMaxParticles();
                if(emitterInstance.mParticleCount + toCreate > maxParticles) {
                    toCreate = maxParticles - emitterInstance.mParticleCount;
                }

                emitterInstance.mParticleAddedThisFrameCount = toCreate;
            }
        }
        else {

            // Spawning phase
            if(emitterInstance.mRun) {
                if(emitterCore->isImmediate()) {
                    emitterInstance.mParticleAddedThisFrameCount = emitterCore->mBurstParticles;
                    emitterInstance.mRun = false;
                }
                else {
                    // since in burst mode we use particle only once, we can check sum of array index and particle count
                    // (if particle lifetime is short, some particles may die before spawning all of them).
                    Ogre::uint32 allParticleCount = emitterInstance.mParticleArrayStart+emitterInstance.mParticleCount;

                    float emissionRate = emitterCore->getEmissionRate();
                    float toCreateF = elapsedTime * emissionRate + emitterInstance.mParticleRemainder;
                    Ogre::uint32 toCreate = (Ogre::uint32)toCreateF;
                    emitterInstance.mParticleRemainder = toCreateF - toCreate;

                    if(allParticleCount + toCreate < emitterCore->mBurstParticles) {
                        emitterInstance.mParticleAddedThisFrameCount = toCreate;
                    }
                    else {
                        toCreate = emitterCore->mBurstParticles-allParticleCount;
                        emitterInstance.mParticleAddedThisFrameCount = toCreate;
                        emitterInstance.mRun = false;
                    }
                }
            }
        }
    }
}

Ogre::uint32 GpuParticleSystemWorld::getMaxEmitterCores() const
{
    return mMaxEmitterCores;
}

Ogre::uint32 GpuParticleSystemWorld::getMaxEmitterInstances() const
{
    return mMaxEmitterInstances;
}

GpuParticleSystemWorld::Info GpuParticleSystemWorld::getInfo() const
{
    Info info;
    for (size_t i = 0; i < mEmitterInstances.size(); ++i) {
        const EmitterInstance& emitter = mEmitterInstances[i];
        info.mAliveParticles += emitter.mParticleCount + emitter.mParticleAddedThisFrameCount;
        info.mParticlesCreated += emitter.mParticleCreatedCount;
        info.mParticlesAddedThisFrame += emitter.mParticleAddedThisFrameCount;
        info.mUsedBucketsParticleCapacity += emitter.mBucketIndexes.size()*mBucketSize;
    }
    return info;
}

const GpuParticleSystemWorld::AffectorList& GpuParticleSystemWorld::getRegisteredAffectorList() const
{
    return mRegisteredAffectorList;
}

Ogre::uint16 GpuParticleSystemWorld::getBucketSize() const
{
    return mBucketSize;
}

void GpuParticleSystemWorld::processTime(float elapsedTime)
{
    if(mEmitterInstances.empty() || mRegisteredEmitterCores.empty()) {
        for (size_t i = 0; i < mParticleRenderables.size(); ++i) {
            mParticleRenderables[i]->mRenderableVisible = false;
        }
        return;
    }

    // update time and remove old burst particle instances.
    updateInstances(elapsedTime);

    // upload emitter cores
    if(mEmitterCoreBufferDirty) {
        uploadToGpuEmitterCores();
        updateInstancesToCores();
    }

    uploadToGpuEmitterInstances();

    uploadToGpuParticleWorld(elapsedTime);

    // create particles
    emitParticleCreateGpu();

    // update particles
    Ogre::uint32 entriesCount = 0;
    emitParticleUpdateGpu(entriesCount);

    // TODO: bucket culling
    if(entriesCount != 0)
    {
        // Bucket culling: clear result buffer. It is important to do this in
        // separate job before checking culling.



        //  Bucket culling: ComputeJob to check if particle is in camera view (treat particle as sphere),
        //  and with InterlockedAdd count how many particles in bucket is visible.
        //  One is enough to show bucket (maybe InterlockedAdd is not necessary, only setting to 1).
    }

    prepareForRender();
}

Ogre::ReadOnlyBufferPacked* GpuParticleSystemWorld::getParticleBufferAsReadOnly() const
{
    return mParticleBufferAsReadOnly;
}

const String& GpuParticleSystemWorld::getMovableType() const
{
    return Ogre::BLANKSTRING;
}

Ogre::uint32 GpuParticleSystemWorld::getMaxParticles() const
{
    return mMaxParticles;
}

uint32 GpuParticleSystemWorld::getBucketGroupsCountToRender() const
{
    return mEntryBucketCount;
}

Ogre::ReadOnlyBufferPacked* GpuParticleSystemWorld::getEmitterCoreBufferAsReadOnly() const
{
    return mEmitterCoreBufferAsReadOnly;
}

Ogre::ReadOnlyBufferPacked* GpuParticleSystemWorld::getEntryBucketBufferAsReadOnly() const
{
    return mEntryBucketBufferAsReadOnly;
}

Ogre::ReadOnlyBufferPacked* GpuParticleSystemWorld::getParticleWorldBufferAsReadOnly() const
{
    return mParticleWorldBufferAsReadOnly;
}

void GpuParticleSystemWorld::initBuffers()
{
    uint32 maxTriangles = mMaxParticles * 2;

    // index buffer
    {
        bool hasShadow = false;

        Ogre::uint32 *cubeIndices = reinterpret_cast<Ogre::uint32*>( OGRE_MALLOC_SIMD(
                                                                         sizeof(Ogre::uint32) * 3 * maxTriangles,
                                                                         Ogre::MEMCATEGORY_GEOMETRY ) );

        for (uint32 i = 0; i < mMaxParticles; ++i) {
            int index = i*6;
            int vertex = i*4;
            cubeIndices[index+0] = vertex+0;
            cubeIndices[index+1] = vertex+1;
            cubeIndices[index+2] = vertex+2;
            cubeIndices[index+3] = vertex+2;
            cubeIndices[index+4] = vertex+3;
            cubeIndices[index+5] = vertex+0;
        }

        try
        {
            mIndexBuffer = mVaoManager->createIndexBuffer( IndexBufferPacked::IT_32BIT,
                                                           3 * maxTriangles,
                                                           BT_IMMUTABLE,
                                                           cubeIndices, hasShadow );
        }
        catch( Ogre::Exception &e )
        {
            // When keepAsShadow = true, the memory will be freed when the index buffer is destroyed.
            // However if for some weird reason there is an exception raised, the memory will
            // not be freed, so it is up to us to do so.
            // The reasons for exceptions are very rare. But we're doing this for correctness.
            OGRE_FREE_SIMD( mIndexBuffer, Ogre::MEMCATEGORY_GEOMETRY );
            mIndexBuffer = 0;
            throw e;
        }
    }

    Ogre::uint32 allAffectorsParticleSize = 0;
    Ogre::uint32 allAffectorsEmitterSize = 0;

    for (size_t i = 0; i < mRegisteredAffectorList.size(); ++i) {

        const GpuParticleAffector* affector = mRegisteredAffectorList[i];
        allAffectorsParticleSize += affector->getAffectorParticleBufferSize();
        allAffectorsEmitterSize += affector->getAffectorEmitterBufferSize();
    }

    mParticleDataStructFinalSize = ParticleDataStructSize+allAffectorsParticleSize;
    mEmitterCoreDataStructFinalSize = EmitterCoreDataStructSize+allAffectorsEmitterSize;


    // Particle buffer
    {
        mParticleBuffer = mVaoManager->createUavBuffer(mMaxParticles, mParticleDataStructFinalSize, BB_FLAG_UAV|BB_FLAG_READONLY, 0, false);
        mParticleBufferAsReadOnly = mParticleBuffer->getAsReadOnlyBufferView();
    }

    // Bucket buffer
    {
        mEntryBucketBuffer = mVaoManager->createUavBuffer(mBucketCount, EntryBucketDataStructSize, BB_FLAG_UAV|BB_FLAG_READONLY, 0, false);
        mEntryBucketBufferAsReadOnly = mEntryBucketBuffer->getAsReadOnlyBufferView();

        mCpuEntryBucketBuffer = reinterpret_cast<Ogre::uint32*>( OGRE_MALLOC_SIMD( mBucketCount * EntryBucketDataStructSize, MEMCATEGORY_GENERAL ) );
    }

    // Emitter core buffer
    {
        mEmitterCoreBuffer = mVaoManager->createUavBuffer(mMaxEmitterCores, mEmitterCoreDataStructFinalSize, BB_FLAG_UAV|BB_FLAG_READONLY, 0, false);
        mEmitterCoreBufferAsReadOnly = mEmitterCoreBuffer->getAsReadOnlyBufferView();

        mCpuEmitterCoreBuffer = reinterpret_cast<float*>( OGRE_MALLOC_SIMD( mMaxEmitterCores * mEmitterCoreDataStructFinalSize, MEMCATEGORY_GENERAL ) );
    }

    // Emitter instance buffer
    {
        mEmitterInstanceBuffer = mVaoManager->createUavBuffer(mMaxEmitterInstances, EmitterInstanceDataStructSize, BB_FLAG_UAV|BB_FLAG_READONLY, 0, false);
        mEmitterInstanceBufferAsReadOnly = mEmitterInstanceBuffer->getAsReadOnlyBufferView();

        mCpuEmitterInstanceBuffer = reinterpret_cast<float*>( OGRE_MALLOC_SIMD( mMaxEmitterInstances * EmitterInstanceDataStructSize, MEMCATEGORY_GENERAL ) );
    }

    // Particle world buffer
    {
        mParticleWorldBuffer = mVaoManager->createUavBuffer(1, ParticleWorldDataStructSize, BB_FLAG_UAV|BB_FLAG_READONLY, 0, false);
        mParticleWorldBufferAsReadOnly = mParticleWorldBuffer->getAsReadOnlyBufferView();

        mCpuParticleWorldBuffer = reinterpret_cast<float*>( OGRE_MALLOC_SIMD( 1 * ParticleWorldDataStructSize, MEMCATEGORY_GENERAL ) );
    }

    // Compute jobs
    {
        static int counter = 0;
        const String newId = StringConverter::toString( counter );
        counter++;

        {
            Ogre::HlmsComputeJob* originalCreateJob = getParticleCreateComputeJob();
            mCreateParticlesJob = originalCreateJob->clone(originalCreateJob->getNameStr() + "_" + newId);
            mCreateParticlesJob->setThreadsPerGroup(mThreadsPerGroup, 1, 1);

            ShaderParams::Param paramBucketSize;
            paramBucketSize.name	= "BucketSize";
            paramBucketSize.setManualValue(uint32(mBucketSize));

            ShaderParams::Param paramMaxParticles;
            paramMaxParticles.name	= "MaxParticles";
            paramMaxParticles.setManualValue(uint32(mMaxParticles));

            Ogre::ShaderParams &shaderParams = mCreateParticlesJob->getShaderParams( "default" );
            shaderParams.mParams.clear();
            shaderParams.mParams.push_back( paramBucketSize );
            shaderParams.mParams.push_back( paramMaxParticles );
            shaderParams.setDirty();
        }

        {
            Ogre::HlmsComputeJob* originalUpdateJob = getParticleUpdateComputeJob();
            mUpdateParticlesJob = originalUpdateJob->clone(originalUpdateJob->getNameStr() + "_" + newId);
            mUpdateParticlesJob->setThreadsPerGroup(mThreadsPerGroup, 1, 1);

            ShaderParams::Param paramBucketSize;
            paramBucketSize.name	= "BucketSize";
            paramBucketSize.setManualValue(uint32(mBucketSize));

            ShaderParams::Param paramMaxParticles;
            paramMaxParticles.name	= "MaxParticles";
            paramMaxParticles.setManualValue(uint32(mMaxParticles));

            Ogre::ShaderParams &shaderParams = mUpdateParticlesJob->getShaderParams( "default" );
            shaderParams.mParams.clear();
            shaderParams.mParams.push_back( paramBucketSize );
            shaderParams.mParams.push_back( paramMaxParticles );
            shaderParams.setDirty();
        }


    }

}

void GpuParticleSystemWorld::destroyBuffers()
{
    if(mParticleWorldBuffer) {
        mVaoManager->destroyUavBuffer(mParticleWorldBuffer);
        mParticleWorldBuffer = 0;
        mParticleWorldBufferAsReadOnly = 0;

        OGRE_FREE_SIMD( mCpuParticleWorldBuffer, MEMCATEGORY_GENERAL );
        mCpuParticleWorldBuffer = 0;
    }

    if(mEmitterInstanceBuffer) {
        mVaoManager->destroyUavBuffer(mEmitterInstanceBuffer);
        mEmitterInstanceBuffer = 0;
        mEmitterInstanceBufferAsReadOnly = 0;

        OGRE_FREE_SIMD( mCpuEmitterInstanceBuffer, MEMCATEGORY_GENERAL );
        mCpuEmitterInstanceBuffer = 0;
    }

    if(mEmitterCoreBuffer) {
        mVaoManager->destroyUavBuffer(mEmitterCoreBuffer);
        mEmitterCoreBuffer = 0;
        mEmitterCoreBufferAsReadOnly = 0;

        OGRE_FREE_SIMD( mCpuEmitterCoreBuffer, MEMCATEGORY_GENERAL );
        mCpuEmitterCoreBuffer = 0;
    }

    if(mEntryBucketBuffer) {
        mVaoManager->destroyUavBuffer(mEntryBucketBuffer);
        mEntryBucketBuffer = 0;
        mEntryBucketBufferAsReadOnly = 0;

        OGRE_FREE_SIMD( mCpuEntryBucketBuffer, MEMCATEGORY_GENERAL );
        mCpuEntryBucketBuffer = 0;
    }

    if(mParticleBuffer)
    {
        mVaoManager->destroyUavBuffer(mParticleBuffer);
        mParticleBuffer = 0;
        mParticleBufferAsReadOnly = 0;
    }

    if(mIndexBuffer)
    {
        mVaoManager->destroyIndexBuffer(mIndexBuffer);
    }

}

uint32 GpuParticleSystemWorld::estimateRequiredBucketCount(const GpuParticleEmitter* emitterCore) const
{
    uint32 particles = 0;
    if(emitterCore->mBurstMode) {
        particles = emitterCore->mBurstParticles;
    }
    else {
        //        float emissionRate = emitterCore->mEmissionRate;
        //        float maxParticleLifetime = emitterCore->getMaxParticleLifetime();
        //        float epsilonTime = 0.05f;
        //        float particlesF = emissionRate * (maxParticleLifetime+epsilonTime);
        particles = emitterCore->getMaxParticles();
    }

    uint32 buckets = getBucketsForNumber(particles);

    if(!emitterCore->mBurstMode) {
        /// We need additional bucket in case of looped particle system, as we create particles
        /// for whole bucket (imagine situation: |64|64|64|  | and we add one particle (so at the
        /// same time one particle will be finished), so it will be |63|64|64|01|
        buckets++;
    }

    return buckets;
}

Ogre::uint32 GpuParticleSystemWorld::getParticleDataStructFinalSize() const
{
    return mParticleDataStructFinalSize;
}

Ogre::uint32 GpuParticleSystemWorld::getEmitterCoreDataStructFinalSize() const
{
    return mEmitterCoreDataStructFinalSize;
}

bool GpuParticleSystemWorld::requestBuckets(GpuParticleSystemWorld::EmitterInstance& emitterInstance)
{
    uint32 buckets = estimateRequiredBucketCount(emitterInstance.mGpuParticleEmitter);

    if(buckets > (uint32)mAvailableBucketsStack.size()) {
        return false;
    }

    for (uint32 i = 0; i < buckets; ++i) {
        if(!mAvailableBucketsStack.empty()) {
            Ogre::uint32 bucketId = mAvailableBucketsStack[mAvailableBucketsStack.size()-1];
            emitterInstance.mBucketIndexes.push_back(bucketId);
            mAvailableBucketsStack.pop_back();
            emitterInstance.mEmitterParticleMaxCount += mBucketSize;
        }
    }

    return true;
}

void GpuParticleSystemWorld::freeBuckets(GpuParticleSystemWorld::EmitterInstance& emitterInstance)
{
    emitterInstance.mEmitterParticleMaxCount = 0;
    for (size_t i = 0; i < emitterInstance.mBucketIndexes.size(); ++i) {
        mAvailableBucketsStack.push_back(emitterInstance.mBucketIndexes[i]);
        emitterInstance.mEmitterParticleMaxCount += mBucketSize;
    }
    emitterInstance.mBucketIndexes.clear();
}

void GpuParticleSystemWorld::freeUnusedBuckets(GpuParticleSystemWorld::EmitterInstance& emitterInstance)
{
    //    if(emitterInstance.mRun) {
    //        // cannot free buckets with running emitter.
    //        return;
    //    }

    //    if(emitterInstance.mParticleCount + emitterInstance.mParticleAddedThisFrame == 0) {
    //        freeBuckets(emitterInstance);
    //        return;
    //    }

    std::vector<Ogre::uint32> list;
    list.reserve(emitterInstance.mBucketIndexes.size());

    size_t startingBucketIndex = emitterInstance.mParticleArrayStart / mBucketSize;
    size_t endingBucketIndex = (emitterInstance.mParticleArrayStart + emitterInstance.mParticleCount + emitterInstance.mParticleAddedThisFrameCount - 1) / mBucketSize;
    bool belong = true;
    for (size_t k = 0; k < emitterInstance.mBucketIndexes.size(); ++k) {
        size_t realK = (startingBucketIndex+k) % emitterInstance.mBucketIndexes.size();

        if(belong) {
            list.push_back(emitterInstance.mBucketIndexes[realK]);
        }
        else {
            mAvailableBucketsStack.push_back(emitterInstance.mBucketIndexes[realK]);
        }

        if(realK == endingBucketIndex) {
            belong = false;
        }
    }

    emitterInstance.mBucketIndexes = list;
    // now list is ordered - particle array start must be in list[0] bucket.
    emitterInstance.mParticleArrayStart = emitterInstance.mParticleArrayStart % mBucketSize;
}

uint32 GpuParticleSystemWorld::getBucketsForNumber(uint32 number) const
{
    return (number + mBucketSize-1) / mBucketSize;
}

void GpuParticleSystemWorld::uploadToGpuEmitterCores()
{
#define AS_U32PTR( x ) reinterpret_cast<uint32*RESTRICT_ALIAS>(x)

    float * RESTRICT_ALIAS buffer = reinterpret_cast<float*>( mCpuEmitterCoreBuffer );
    const float *bufferStart = buffer;

    for (size_t i = 0; i < mRegisteredEmitterCores.size(); ++i) {
        const GpuParticleEmitter* emitterCore = mRegisteredEmitterCores[i];
        ParticleRenderable* particleRenderable = getRenderableForEmitterCore(emitterCore);
        Ogre::Vector2 invSize(0.0f, 0.0f);
        Ogre::uint8 flipbookSizeX = 1;
        Ogre::uint8 flipbookSizeY = 1;
        float flipbookSizeInvX = 1.0f;
        float flipbookSizeInvY = 1.0f;

        if(particleRenderable) {
            invSize = particleRenderable->mParticleDatablock->getInvTextureSize();

            flipbookSizeX = particleRenderable->mParticleDatablock->getFlipbookSize().col;
            flipbookSizeY = particleRenderable->mParticleDatablock->getFlipbookSize().row;

            flipbookSizeInvX = 1.0f / (float)flipbookSizeX;
            flipbookSizeInvY = 1.0f / (float)flipbookSizeY;
        }

        *buffer++ = emitterCore->mColourA.r;
        *buffer++ = emitterCore->mColourA.g;
        *buffer++ = emitterCore->mColourA.b;
        *buffer++ = emitterCore->mColourA.a;

        *buffer++ = emitterCore->mColourB.r;
        *buffer++ = emitterCore->mColourB.g;
        *buffer++ = emitterCore->mColourB.b;
        *buffer++ = emitterCore->mColourB.a;

        *buffer++ = emitterCore->mSizeMin;
        *buffer++ = emitterCore->mSizeMax;

        *buffer++ = emitterCore->mSizeYMin;
        *buffer++ = emitterCore->mSizeYMax;

        *buffer++ = emitterCore->mSpotAngleMin;
        *buffer++ = emitterCore->mSpotAngleMax;

        *buffer++ = emitterCore->mDirectionVelocityMin;
        *buffer++ = emitterCore->mDirectionVelocityMax;

        *buffer++ = emitterCore->mParticleLifetimeMin;
        *buffer++ = emitterCore->mParticleLifetimeMax;

        float nextParticleSpawnTime = emitterCore->getTimeToSpawnParticle();
        *buffer++ = nextParticleSpawnTime;

        *buffer++ = emitterCore->mGravity.x;
        *buffer++ = emitterCore->mGravity.y;
        *buffer++ = emitterCore->mGravity.z;

        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, mUseDepthTexture && emitterCore->mUseDepthCollision ? 1u : 0u);

        Ogre::uint32 spriteCount = std::min<Ogre::uint32>(emitterCore->mSpriteFlipbookCoords.size(), GpuParticleEmitter::MaxSprites);
        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, emitterCore->mSpriteMode == GpuParticleEmitter::SpriteMode::SetWithStart ? (Ogre::uint32)spriteCount : (Ogre::uint32)0);
        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) (emitterCore->mSpriteMode == GpuParticleEmitter::SpriteMode::ChangeWithTrack) );

        float lastTimeValue = 0.0f;
        for (size_t i = 0; i < GpuParticleEmitter::MaxSprites; ++i) {
            if(i < emitterCore->mSpriteTimes.size()) {
                lastTimeValue = emitterCore->mSpriteTimes[i];
            }

            *buffer++ = lastTimeValue;
        }

        Ogre::Vector2 bottomLeft = Ogre::Vector2::ZERO;
        Ogre::Vector2 texCoordSize = Ogre::Vector2(1.0f, 1.0f);

        for (size_t i = 0; i < GpuParticleEmitter::MaxSprites; ++i) {
            // remember last value
            if(particleRenderable && i < emitterCore->mSpriteFlipbookCoords.size()) {

                const HlmsParticleDatablock::SpriteCoord& spriteCoord = emitterCore->mSpriteFlipbookCoords[i];
                if(particleRenderable->mParticleDatablock->getIsFlipbook()) {
                    float percentX = (float)spriteCoord.col / (float)flipbookSizeX;
                    float percentY = (float)spriteCoord.row / (float)flipbookSizeY;
                    bottomLeft = Ogre::Vector2(percentX, percentY);
                    texCoordSize = Ogre::Vector2(flipbookSizeInvX, flipbookSizeInvY);
                }
                else {
                    const HlmsParticleDatablock::Sprite* sprite = particleRenderable->mParticleDatablock->getSprite(spriteCoord.col);
                    if(sprite) {
                        bottomLeft = Ogre::Vector2(sprite->mLeft * invSize.x, sprite->mBottom * invSize.y);
                        texCoordSize = Ogre::Vector2(sprite->mSizeX * invSize.x, sprite->mSizeY * invSize.y);
                    }
                }
            }

            *buffer++ = bottomLeft.x;
            *buffer++ = bottomLeft.y;
            *buffer++ = texCoordSize.x;
            *buffer++ = texCoordSize.y;
        }

        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) emitterCore->mUseColourTrack);
        GpuParticleAffectorCommon::uploadVector3Track(buffer, emitterCore->mColourTrack);

        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) emitterCore->mUseAlphaTrack);
        GpuParticleAffectorCommon::uploadFloatTrack(buffer, emitterCore->mAlphaTrack, 1.0f);

        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) emitterCore->mFaderMode);
        *buffer++ = emitterCore->mParticleFaderStartTime;
        *buffer++ = emitterCore->mParticleFaderEndTime;

        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) emitterCore->mUseSizeTrack);
        GpuParticleAffectorCommon::uploadVector2Track(buffer, emitterCore->mSizeTrack);

        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32) emitterCore->mUseVelocityTrack);
        GpuParticleAffectorCommon::uploadFloatTrack(buffer, emitterCore->mVelocityTrack, 0.0f);



        //        for (size_t i = 0; i < GpuParticleEmitter::MaxSprites; ++i) {
        //            Ogre::uint32 spriteIndex = 0;
        //            if(i < emitterCore->mSpriteNames.size()) {

        //            }
        //            *AS_U32PTR( buffer ) = spriteIndex;      ++buffer;
        //        }

        // different size
        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, emitterCore->mUniformSize ? 1u : 0u);
        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32)emitterCore->mBillboardType);

        GpuParticleAffectorCommon::uploadU32ToFloatArray(buffer, (Ogre::uint32)emitterCore->mSpawnShape);
        *buffer++ = emitterCore->mSpawnShapeDimensions.x;
        *buffer++ = emitterCore->mSpawnShapeDimensions.y;
        *buffer++ = emitterCore->mSpawnShapeDimensions.z;

        const GpuParticleEmitter::AffectorMap& affectorMap = emitterCore->getAffectors();
        for (size_t i = 0; i < mRegisteredAffectorList.size(); ++i) {

            const GpuParticleAffector* affector = mRegisteredAffectorList[i];
            GpuParticleEmitter::AffectorMap::const_iterator itEmitterAffector = affectorMap.find(affector->getType());
            if(itEmitterAffector != affectorMap.end()) {
                affector = itEmitterAffector->second;
            }

            buffer = affector->prepareAffectorEmitterBuffer(buffer);
        }
    }

    OGRE_ASSERT_LOW( (size_t)(buffer - bufferStart) * sizeof(float) <=
                     mEmitterCoreBuffer->getTotalSizeBytes() );

    // TODO: is setting rest of the memory necessary?
    //Fill the remaining bytes with 0
    memset( buffer, 0, mEmitterCoreBuffer->getTotalSizeBytes() -
            (static_cast<size_t>(buffer - bufferStart) * sizeof(float)) );

    mEmitterCoreBuffer->upload( mCpuEmitterCoreBuffer, 0u, mEmitterCoreBuffer->getNumElements() );


    mEmitterCoreBufferDirty = false;

#undef AS_U32PTR
}

void GpuParticleSystemWorld::uploadToGpuEmitterInstances()
{
    float * RESTRICT_ALIAS emitterInstanceBuffer = reinterpret_cast<float*>( mCpuEmitterInstanceBuffer );
    const float *emitterInstanceBufferStart = emitterInstanceBuffer;

    for (size_t i = 0; i < mEmitterInstances.size(); ++i) {
        const EmitterInstance& emitter = mEmitterInstances[i];

        Ogre::Matrix4 mat;
        if(!emitter.mNode) {
            mat.makeTransform(emitter.mPos, Ogre::Vector3::UNIT_SCALE, emitter.mRot);
        }
        else {
            Ogre::Matrix4 matNode;
            matNode = emitter.mNode->_getFullTransformUpdated();
            Ogre::Matrix4 matOffset;
            matOffset.makeTransform(emitter.mPos, Ogre::Vector3::UNIT_SCALE, emitter.mRot);

            mat = matNode * matOffset;
        }

        for (int j = 0; j < 16; ++j) {
            *emitterInstanceBuffer++ = (float)mat[0][j];
        }

#define AS_U32PTR( x ) reinterpret_cast<uint32*RESTRICT_ALIAS>(x)

        *AS_U32PTR( emitterInstanceBuffer ) = emitter.mRun ? 1u : 0u;        ++emitterInstanceBuffer;

#undef AS_U32PTR

    }

    OGRE_ASSERT_LOW( (size_t)(emitterInstanceBuffer - emitterInstanceBufferStart) * sizeof(float) <=
                     mEmitterInstanceBuffer->getTotalSizeBytes() );

    // TODO: is setting rest of the memory necessary?
    //Fill the remaining bytes with 0
    memset( emitterInstanceBuffer, 0, mEmitterInstanceBuffer->getTotalSizeBytes() -
            (static_cast<size_t>(emitterInstanceBuffer - emitterInstanceBufferStart) * sizeof(float)) );

    mEmitterInstanceBuffer->upload( mCpuEmitterInstanceBuffer, 0u, mEmitterInstanceBuffer->getNumElements() );

}

void GpuParticleSystemWorld::uploadEntryBucketRow(Ogre::uint32*& RESTRICT_ALIAS entryBucketBuffer, const GpuParticleSystemWorld::BucketGroupData& bucketGroup)
{
    //    *entryBucketBuffer++ = bucketGroup.bucketId;
    *entryBucketBuffer++ = bucketGroup.emitterInstanceId;
    *entryBucketBuffer++ = bucketGroup.emitterCoreId;
    //    *entryBucketBuffer++ = bucketGroup.timeOffset;
    //    *entryBucketBuffer++ = bucketGroup.fromParticleInBucket;
    //    *entryBucketBuffer++ = bucketGroup.tillParticleInBucket;
    *entryBucketBuffer++ = bucketGroup.lastParticleIndex;
    *entryBucketBuffer++ = bucketGroup.nextBucketParticleIndex;
    *entryBucketBuffer++ = bucketGroup.particleCount;
}

void GpuParticleSystemWorld::uploadToGpuParticleWorld(float elapsedTime)
{
    float * RESTRICT_ALIAS particleWorldBuffer = reinterpret_cast<float*>( mCpuParticleWorldBuffer );
    const float *particleWorldBufferStart = particleWorldBuffer;

    {
        Ogre::Matrix4 prevCameraVP = mHlmsParticleListener->getPrevCameraVP();
        Ogre::Matrix4 prevCameraInvVP = prevCameraVP.inverse();
        for (int i = 0; i < 16; ++i) {
            *particleWorldBuffer++ = prevCameraVP[0][i];
        }
        for (int i = 0; i < 16; ++i) {
            *particleWorldBuffer++ = prevCameraInvVP[0][i];
        }

        Ogre::Vector2 cameraParamsAB = mHlmsParticleListener->getCameraProjectionAB();

        *particleWorldBuffer++ = cameraParamsAB.x;
        *particleWorldBuffer++ = cameraParamsAB.y;

        *particleWorldBuffer++ = elapsedTime;

#define AS_U32PTR( x ) reinterpret_cast<uint32*RESTRICT_ALIAS>(x)

        int randomNumber = rand();
        *AS_U32PTR( particleWorldBuffer ) = randomNumber;        ++particleWorldBuffer;

#undef AS_U32PTR

    }

    OGRE_ASSERT_LOW( (size_t)(particleWorldBuffer - particleWorldBufferStart) * sizeof(float) <=
                     mParticleWorldBuffer->getTotalSizeBytes() );

    // TODO: is setting rest of the memory necessary?
    //Fill the remaining bytes with 0
    memset( particleWorldBuffer, 0, mParticleWorldBuffer->getTotalSizeBytes() -
            (static_cast<size_t>(particleWorldBuffer - particleWorldBufferStart) * sizeof(float)) );

    mParticleWorldBuffer->upload( mCpuParticleWorldBuffer, 0u, mParticleWorldBuffer->getNumElements() );

}

uint32 GpuParticleSystemWorld::uploadBucketsForInstance(Ogre::uint32 *& RESTRICT_ALIAS entryBucketBuffer, size_t emitterInstanceIndex)
{
    EmitterInstance& emitterInstance = mEmitterInstances[emitterInstanceIndex];
    Ogre::uint32 particleCount = emitterInstance.mParticleCount + emitterInstance.mParticleAddedThisFrameCount;
    if(particleCount == 0) {
        return 0;
    }

    // from first to last particle included [emitterInstance.mParticleArrayStart; last]
    //    Ogre::uint32 firstParticle = emitterInstance.mParticleArrayStart;
    Ogre::uint32 lastParticle = (emitterInstance.mParticleArrayStart + particleCount - 1) % emitterInstance.mEmitterParticleMaxCount;

    return uploadBucketsForInstance(entryBucketBuffer, emitterInstanceIndex, lastParticle, particleCount);
}

uint32 GpuParticleSystemWorld::uploadBucketsForInstance(uint32*& entryBucketBuffer, size_t emitterInstanceIndex, uint32 lastParticle, uint32 particleCount)
{
    Ogre::uint32 instanceEntriesCount = 0;
    EmitterInstance& emitterInstance = mEmitterInstances[emitterInstanceIndex];

    Ogre::uint32 firstParticle = (lastParticle + emitterInstance.mEmitterParticleMaxCount - particleCount + 1) % emitterInstance.mEmitterParticleMaxCount;

    size_t firstBucketIndex = firstParticle / mBucketSize;
    size_t lastBucketIndex = lastParticle / mBucketSize;
    size_t emitterBucketsCount = emitterInstance.mBucketIndexes.size();

    size_t particleIndexInBucket = lastParticle % mBucketSize;

    for (size_t k = 0; k < emitterBucketsCount; ++k) {
        ++instanceEntriesCount;
        size_t shiftedK = (lastBucketIndex+emitterBucketsCount-k)%emitterBucketsCount;
        size_t shiftedNextK = (lastBucketIndex+emitterBucketsCount-k-1)%emitterBucketsCount;

        BucketGroupData bucket;
        //        bucket.bucketId = emitterInstance.mBucketIndexes[shiftedK];
        bucket.emitterInstanceId = emitterInstanceIndex;
        bucket.emitterCoreId = emitterInstance.mGpuParticleEmitterIndex;

        //        bucket.timeOffset = k;
        //        if(k == 0) {
        //            bucket.fromParticleInBucket = firstParticle % mBucketSize;
        //        }
        //        else {
        //            bucket.fromParticleInBucket = 0;
        //        }
        //        if(shiftedK == lastBucketIndex) {
        //            bucket.tillParticleInBucket = lastParticle % mBucketSize;
        //        }
        //        else {
        //            bucket.tillParticleInBucket = mBucketSize-1;
        //        }

        bucket.lastParticleIndex = (emitterInstance.mBucketIndexes[shiftedK]) * mBucketSize + particleIndexInBucket;
        bucket.nextBucketParticleIndex = (emitterInstance.mBucketIndexes[shiftedNextK] + 1) * mBucketSize;
        if(particleCount > mBucketSize) {
            bucket.particleCount = mBucketSize;
            particleCount -= mBucketSize;
        }
        else {
            bucket.particleCount = particleCount;
            particleCount = 0;
        }

        uploadEntryBucketRow(entryBucketBuffer, bucket);

        if(shiftedK == firstBucketIndex) {
            break;
        }
    }

    return instanceEntriesCount;
}

void GpuParticleSystemWorld::emitParticleCreateGpu()
{
    Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
    //    renderSystem->endRenderPassDescriptor(); // should be called only once before ComputeJob

    Ogre::HlmsCompute* hlmsCompute = Root::getSingleton().getHlmsManager()->getComputeHlms();

    // upload bucketGroup for particles to add
    Ogre::uint32 entriesCount = 0;
    {
        Ogre::uint32 * RESTRICT_ALIAS entryBucketBuffer = reinterpret_cast<Ogre::uint32*>( mCpuEntryBucketBuffer );
        const Ogre::uint32 *entryBucketBufferStart = entryBucketBuffer;

        // upload bucketGroup for existing particles
        for (size_t i = 0; i < mEmitterInstances.size(); ++i) {
            EmitterInstance& emitterInstance = mEmitterInstances[i];

            if(emitterInstance.mParticleAddedThisFrameCount == 0 ||
                    emitterInstance.mParticleCreatedCount >= emitterInstance.mParticleCount+emitterInstance.mParticleAddedThisFrameCount) {
                continue;
            }

            Ogre::uint32 particlesToCreate = emitterInstance.mParticleCount + emitterInstance.mParticleAddedThisFrameCount - emitterInstance.mParticleCreatedCount;
            Ogre::uint32 newParticleCreatedCount = emitterInstance.mParticleCreatedCount + particlesToCreate;

            // from first to last particle included [first; last]
            //            Ogre::uint32 firstParticle = (emitterInstance.mParticleArrayStart + emitterInstance.mParticleCount) % emitterInstance.mEmitterParticleMaxCount;
            //            Ogre::uint32 lastParticle = (emitterInstance.mParticleArrayStart + emitterInstance.mParticleCreatedCount + particlesToCreate - 1) % emitterInstance.mEmitterParticleMaxCount;

            if(emitterInstance.isStatic() || mInitLocationInUpdate) {

                if(!emitterInstance.mGpuParticleEmitter->mBurstMode) {
                    // Create whole bucket of particles ahead of time.

                    particlesToCreate = (((particlesToCreate-1) / mBucketSize) + 1) * mBucketSize;
                    newParticleCreatedCount = emitterInstance.mParticleCreatedCount + particlesToCreate;
                }
                else {
                    // In case of burst we never use particles in slots more than once.

                    // Create all necessary particles ahead of time for burst.
                    particlesToCreate = emitterInstance.mGpuParticleEmitter->mBurstParticles;
                    newParticleCreatedCount = emitterInstance.mGpuParticleEmitter->mBurstParticles;
                }
            }
            emitterInstance.mParticleCreatedCount = newParticleCreatedCount;
            Ogre::uint32 lastParticle = (emitterInstance.mParticleArrayStart + newParticleCreatedCount - 1) % emitterInstance.mEmitterParticleMaxCount;


            Ogre::uint32 entriesForInstance = uploadBucketsForInstance(entryBucketBuffer, i, lastParticle, particlesToCreate);
            entriesCount += entriesForInstance;
        }

        OGRE_ASSERT_LOW( (size_t)(entryBucketBuffer - entryBucketBufferStart) * sizeof(float) <=
                         mEntryBucketBuffer->getTotalSizeBytes() );

        // TODO: is setting rest of the memory necessary?
        //Fill the remaining bytes with 0
        memset( entryBucketBuffer, 0, mEntryBucketBuffer->getTotalSizeBytes() -
                (static_cast<size_t>(entryBucketBuffer - entryBucketBufferStart) * sizeof(float)) );

        mEntryBucketBuffer->upload( mCpuEntryBucketBuffer, 0u, mEntryBucketBuffer->getNumElements() );
    }

    if(entriesCount == 0) {
        return;
    }

    Ogre::uint32 groupsX = entriesCount * mGroupsPerBucket;
    mCreateParticlesJob->setNumThreadGroups(groupsX, 1, 1);

    {
        DescriptorSetUav::BufferSlot bufferSlot( DescriptorSetUav::BufferSlot::makeEmpty() );
        bufferSlot.buffer = mParticleBuffer;
        bufferSlot.access = Ogre::ResourceAccess::Write;
        mCreateParticlesJob->_setUavBuffer(0, bufferSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mEmitterCoreBufferAsReadOnly;
        mCreateParticlesJob->setTexBuffer( 0, texBufSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mEmitterInstanceBufferAsReadOnly;
        mCreateParticlesJob->setTexBuffer( 1, texBufSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mEntryBucketBufferAsReadOnly;
        mCreateParticlesJob->setTexBuffer( 2, texBufSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mParticleWorldBufferAsReadOnly;
        mCreateParticlesJob->setTexBuffer( 3, texBufSlot );
    }

    mCreateParticlesJob->analyzeBarriers(mResourceTransitions);
    renderSystem->executeResourceTransition(mResourceTransitions);
    hlmsCompute->dispatch( mCreateParticlesJob, 0, 0 );
}

void GpuParticleSystemWorld::emitParticleUpdateGpu(Ogre::uint32& resultEntriesCount)
{

    Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
    //    renderSystem->endRenderPassDescriptor(); // should be called only once before ComputeJob

    Ogre::HlmsCompute* hlmsCompute = Root::getSingleton().getHlmsManager()->getComputeHlms();
    ////    Ogre::HlmsComputeJob* job = mUpdateParticlesJob;

    Ogre::uint32 entriesCount = 0;
    {
        Ogre::uint32 * RESTRICT_ALIAS entryBucketBuffer = reinterpret_cast<Ogre::uint32*>( mCpuEntryBucketBuffer );
        const Ogre::uint32 *entryBucketBufferStart = entryBucketBuffer;

        // upload bucketGroup for existing particles
        for (size_t i = 0; i < mEmitterInstances.size(); ++i) {
            Ogre::uint32 entriesForInstance = uploadBucketsForInstance(entryBucketBuffer, i);
            entriesCount += entriesForInstance;
        }

        OGRE_ASSERT_LOW( (size_t)(entryBucketBuffer - entryBucketBufferStart) * sizeof(float) <=
                         mEntryBucketBuffer->getTotalSizeBytes() );

        // TODO: is setting rest of the memory necessary?
        //Fill the remaining bytes with 0
        memset( entryBucketBuffer, 0, mEntryBucketBuffer->getTotalSizeBytes() -
                (static_cast<size_t>(entryBucketBuffer - entryBucketBufferStart) * sizeof(float)) );

        mEntryBucketBuffer->upload( mCpuEntryBucketBuffer, 0u, mEntryBucketBuffer->getNumElements() );
    }
    resultEntriesCount = entriesCount;

    if(entriesCount == 0) {
        return;
    }

    Ogre::uint32 groupsX = entriesCount * mGroupsPerBucket;
    mUpdateParticlesJob->setNumThreadGroups(groupsX, 1, 1);

    {
        DescriptorSetUav::BufferSlot bufferSlot( DescriptorSetUav::BufferSlot::makeEmpty() );
        bufferSlot.buffer = mParticleBuffer;
        bufferSlot.access = Ogre::ResourceAccess::Write;
        mUpdateParticlesJob->_setUavBuffer(0, bufferSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mEmitterCoreBufferAsReadOnly;
        mUpdateParticlesJob->setTexBuffer( 0, texBufSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mEmitterInstanceBufferAsReadOnly;
        mUpdateParticlesJob->setTexBuffer( 1, texBufSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mEntryBucketBufferAsReadOnly;
        mUpdateParticlesJob->setTexBuffer( 2, texBufSlot );
    }

    {
        DescriptorSetTexture2::BufferSlot texBufSlot(DescriptorSetTexture2::BufferSlot::makeEmpty());
        texBufSlot.buffer = mParticleWorldBufferAsReadOnly;
        mUpdateParticlesJob->setTexBuffer( 3, texBufSlot );
    }

    if(mUseDepthTexture)
    {
        CompositorNode* node = mCompositorWorkspace->findNode(mDepthTextureCompositorNode);
        if(node) {
            TextureGpu* tex = node->getDefinedTexture(mDepthTextureName);

            DescriptorSetTexture2::TextureSlot texSlot(DescriptorSetTexture2::TextureSlot::makeEmpty());
            texSlot.texture = tex;
            mUpdateParticlesJob->setTexture( 4, texSlot );

            HlmsSamplerblock samplerBlock;
            mUpdateParticlesJob->setSamplerblock(4, samplerBlock );

            //            static int counter = 0;
            //            ++counter;

            //            if(counter > 500) {
            //                static int counter2 = 0;
            //                counter2++;

            //                Ogre::String path;
            //                Ogre::String ext = "exr";
            //                Ogre::Image2 img;
            //                img.convertFromTexture( tex, 0u, tex->getNumMipmaps() - 1u );
            //                img.save( path + "depthTexture" + Ogre::StringConverter::toString(counter2) + "." + ext, 0u,
            //                          tex->getNumMipmaps() );

            //                counter = 0;
            //            }
        }
    }

    mUpdateParticlesJob->analyzeBarriers(mResourceTransitions);
    renderSystem->executeResourceTransition(mResourceTransitions);
    hlmsCompute->dispatch( mUpdateParticlesJob, 0, 0 );

    //    clearJob();

    //    renderSystem->endRenderPassDescriptor();
    //    renderSystem->setUavStartingSlot( mDefinition->mStartingSlot );
    //    renderSystem->queueBindUAVs();
}

void GpuParticleSystemWorld::updateInstancesToCores()
{
    for (size_t i = 0; i < mEmitterInstances.size(); ++i) {
        EmitterInstance& emitterInstance = mEmitterInstances[i];
        for (size_t j = 0; j < mRegisteredEmitterCores.size(); ++j) {
            if(emitterInstance.mGpuParticleEmitter == mRegisteredEmitterCores[j]) {
                emitterInstance.mGpuParticleEmitterIndex = j;
                break;
            }
        }
    }
}

void GpuParticleSystemWorld::destroyParticleRenderable(const String& datablockName)
{
    // remove renderable from mParticleRenderables
    ParticleRenderable* particleRenderableToDel = nullptr;
    for (size_t i = 0; i < mParticleRenderables.size(); ++i) {
        ParticleRenderable* particleRenderable = mParticleRenderables[i];
        if(particleRenderable->mDatablockName == datablockName) {
            particleRenderableToDel = particleRenderable;
            mParticleRenderables.erase(mParticleRenderables.begin()+i);
            break;
        }
    }

    // remove renderable from mRenderables
    for (size_t i = 0; i < mRenderables.size(); ++i) {
        if(mRenderables[i] == particleRenderableToDel) {
            mRenderables.erase(mRenderables.begin()+i);
            break;
        }
    }

    delete particleRenderableToDel;
}

GpuParticleSystemWorld::ParticleRenderable* GpuParticleSystemWorld::getRenderableForEmitterCore(const GpuParticleEmitter* emitterCore) const
{
    for (size_t i = 0; i < mParticleRenderables.size(); ++i) {
        GpuParticleSystemWorld::ParticleRenderable* particleRenderable = mParticleRenderables[i];
        if(particleRenderable->mDatablockName == emitterCore->mDatablockName) {
            return particleRenderable;
        }
    }
    return nullptr;
}

uint64 GpuParticleSystemWorld::getNextId() const
{
    static Ogre::uint64 idCounter = 0;
    ++idCounter;
    return idCounter;
}

void GpuParticleSystemWorld::prepareForRender()
{
    // clear particle renderable data cached
    for (size_t i = 0; i < mParticleRenderables.size(); ++i) {
        ParticleRenderable* particleRenderable = mParticleRenderables[i];
        particleRenderable->clearCachedData();
    }

    // assign emitter instances to renderables
    for (size_t i = 0; i < mEmitterInstances.size(); ++i) {
        EmitterInstance& emitterInstance = mEmitterInstances[i];
        emitterInstance.mParticleRenderable->mCachedInstanceIndexes.push_back(i);
    }

    // upload buckets and set primitive range for each renderable.
    {
        Ogre::uint32 entriesCount = 0;
        Ogre::uint32 * RESTRICT_ALIAS entryBucketBuffer = reinterpret_cast<Ogre::uint32*>( mCpuEntryBucketBuffer );
        const Ogre::uint32 *entryBucketBufferStart = entryBucketBuffer;

        for (size_t i = 0; i < mParticleRenderables.size(); ++i) {
            ParticleRenderable* particleRenderable = mParticleRenderables[i];
            particleRenderable->mCachedStartBucketIndex = entriesCount;
            for (size_t j = 0; j < particleRenderable->mCachedInstanceIndexes.size(); ++j) {
                size_t emitterInstanceIndex = particleRenderable->mCachedInstanceIndexes[j];

                Ogre::uint32 entriesForInstance = uploadBucketsForInstance(entryBucketBuffer, emitterInstanceIndex);
                entriesCount += entriesForInstance;
                particleRenderable->mCachedBucketsCount += entriesForInstance;
            }

            particleRenderable->mRenderableVisible = particleRenderable->mCachedBucketsCount > 0;
            particleRenderable->prepareIndexBufferRange(particleRenderable->mCachedBucketsCount);
        }

        OGRE_ASSERT_LOW( (size_t)(entryBucketBuffer - entryBucketBufferStart) * sizeof(float) <=
                         mEntryBucketBuffer->getTotalSizeBytes() );

        // TODO: is setting rest of the memory necessary?
        //Fill the remaining bytes with 0
        memset( entryBucketBuffer, 0, mEntryBucketBuffer->getTotalSizeBytes() -
                (static_cast<size_t>(entryBucketBuffer - entryBucketBufferStart) * sizeof(float)) );

        mEntryBucketBuffer->upload( mCpuEntryBucketBuffer, 0u, mEntryBucketBuffer->getNumElements() );
    }

}

HlmsComputeJob* GpuParticleSystemWorld::getParticleCreateComputeJob()
{
    Ogre::HlmsComputeJob* job = nullptr;
    {
        Ogre::HlmsCompute* hlmsCompute = Root::getSingleton().getHlmsManager()->getComputeHlms();
        Ogre::StringVector pieceFiles;
        pieceFiles.push_back("ComputeParticleWorld_piece_all.any");
        job = hlmsCompute->createComputeJob("HlmsParticle/ParticleWorldCreate", "HlmsParticle/ParticleWorldCreate", "ComputeParticleWorldCreate_cs", pieceFiles);
        job->setThreadsPerGroup(64, 1, 1);
        job->setInformHlmsOfTextureData(false);
        job->setNumUavUnits(1);
        job->setNumTexUnits(4);
        if(mInitLocationInUpdate) {
            job->setProperty(Ogre::IdString("initLocationInUpdate"), 1);
        }

        for (size_t i = 0; i < mRegisteredAffectorList.size(); ++i) {
            const GpuParticleAffector* affector = mRegisteredAffectorList[i];
            job->setProperty(Ogre::IdString(affector->getAffectorProperty()), 1);
        }
    }
    return job;
}

HlmsComputeJob* GpuParticleSystemWorld::getParticleUpdateComputeJob()
{
    Ogre::HlmsComputeJob* job = nullptr;
    {
        Ogre::HlmsCompute* hlmsCompute = Root::getSingleton().getHlmsManager()->getComputeHlms();
        Ogre::StringVector pieceFiles;
        pieceFiles.push_back("ComputeParticleWorld_piece_all.any");
        job = hlmsCompute->createComputeJob("HlmsParticle/ParticleWorldUpdate", "HlmsParticle/ParticleWorldUpdate", "ComputeParticleWorldUpdate_cs", pieceFiles);
        job->setThreadsPerGroup(64, 1, 1);
        /// https://forums.ogre3d.org/viewtopic.php?t=95484
        /// You almost always want the divisor to be 1.
        /// The divisor should NOT be 32 if you have 32 threads per group.
        /// The divisor exists because oftentimes you want each thread (not threadgroup) in a compute shader to process more than one pixel. For example if each thread processes a block of 2x2, then you want the divisor to be set to 2:
        //        job->setNumThreadGroupsBasedOn(HlmsComputeJob::ThreadGroupsBasedOnUav, ComputeSimulationParticleUavSlot, 1, 1, 1);
        job->setInformHlmsOfTextureData(false);
        job->setNumUavUnits(1);
        //        job->setNumTexUnits(4);

        int texUnits = 4;
        if(mUseDepthTexture) {
            ++texUnits;
            job->setProperty(Ogre::IdString("useDepthTexture"), 1);
        }
        if(mInitLocationInUpdate) {
            job->setProperty(Ogre::IdString("initLocationInUpdate"), 1);
        }
        job->setNumTexUnits(texUnits);
        //        job->setConstBuffer();
        //        job->_setUavBuffer();
        //        job->clearTexBuffers();

        //        job = hlmsCompute->findComputeJobNoThrow( "HlmsParticle/Update" );
        //    job->setProperty();

        for (size_t i = 0; i < mRegisteredAffectorList.size(); ++i) {
            const GpuParticleAffector* affector = mRegisteredAffectorList[i];
            job->setProperty(Ogre::IdString(affector->getAffectorProperty()), 1);
        }
    }
    return job;
}

GpuParticleSystemWorld::ParticleRenderable::ParticleRenderable(GpuParticleSystemWorld* particleSystemWorld,
                                                               VaoManager* vaoManager,
                                                               IndexBufferPacked* indexBuffer,
                                                               const String& datablockName,
                                                               uint32 bucketSize)
    : mGpuParticleSystemWorld(particleSystemWorld)
    , mVaoManager(vaoManager)
    , mDatablockName(datablockName)
{
    //We use this magic value, to indicate this is a GpuParticleSystemWorld
    //and thus needs special shaders from HlmsParticle
    setCustomParameter( RenderableTypeId, Ogre::Vector4( 1.0f ) );
//    setCustomParameter( RenderableCustomParamBucketSize, Ogre::Vector4 ((Ogre::Real)bucketSize) );

    // init vao
    {
        mVao = vaoManager->createVertexArrayObject(VertexBufferPackedVec(), indexBuffer, OT_TRIANGLE_LIST );
        mVaoPerLod[0].push_back(mVao);
        mVaoPerLod[1].push_back(mVao);
    }

    setDatablock(datablockName);
    mParticleDatablock = dynamic_cast<HlmsParticleDatablock*>(getDatablock());
    if(!mParticleDatablock) {
        OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND,
                     "Casting to HlmsParticleDatablock failed in GpuParticleSystemWorld::ParticleRenderable.",
                     "GpuParticleSystemWorld::getRenderOperation" );
    }
}

GpuParticleSystemWorld::ParticleRenderable::~ParticleRenderable()
{
    mVaoPerLod[0].clear();
    mVaoPerLod[1].clear();
    mVaoManager->destroyVertexArrayObject(mVao);
    mVao = nullptr;
}

const LightList& GpuParticleSystemWorld::ParticleRenderable::getLights() const
{
    return mGpuParticleSystemWorld->queryLights(); //Return the data from our MovableObject base class.
}

void GpuParticleSystemWorld::ParticleRenderable::getRenderOperation( Ogre::v1::RenderOperation& op , bool casterPass )
{
    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                 "GpuParticleSystemWorld::ParticleRenderable do not implement getRenderOperation."
                 " You've put a v2 object in "
                 "the wrong RenderQueue ID (which is set to be compatible with "
                 "Ogre::v1::Entity). Do not mix v2 and v1 objects",
                 "GpuParticleSystemWorld::getRenderOperation" );
}
//-----------------------------------------------------------------------------------
void GpuParticleSystemWorld::ParticleRenderable::getWorldTransforms( Ogre::Matrix4* xform ) const
{
    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                 "GpuParticleSystemWorld::ParticleRenderable do not implement getWorldTransforms."
                 " You've put a v2 object in "
                 "the wrong RenderQueue ID (which is set to be compatible with "
                 "Ogre::v1::Entity). Do not mix v2 and v1 objects",
                 "GpuParticleSystemWorld::getRenderOperation" );
}
//-----------------------------------------------------------------------------------
bool GpuParticleSystemWorld::ParticleRenderable::getCastsShadows(void) const
{
    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                 "GpuParticleSystemWorld::ParticleRenderable do not implement getCastsShadows."
                 " You've put a v2 object in "
                 "the wrong RenderQueue ID (which is set to be compatible with "
                 "Ogre::v1::Entity). Do not mix v2 and v1 objects",
                 "GpuParticleSystemWorld::getRenderOperation" );
}

void GpuParticleSystemWorld::ParticleRenderable::prepareIndexBufferRange(uint32 entriesCount)
{
    // get visible buckets
    int buckets = entriesCount;

    int count = buckets * mGpuParticleSystemWorld->mBucketSize;
    int triangleCount = count*2;
    mVao->setPrimitiveRange(0, triangleCount*3);
}
