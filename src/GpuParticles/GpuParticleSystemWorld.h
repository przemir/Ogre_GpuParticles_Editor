/*
 * File: GpuParticleSystemWorld.h
 * Author: Przemysław Bągard
 * Created: 2021-5-4
 *
 */

#ifndef GPUPARTICLESYSTEMWORLD_H
#define GPUPARTICLESYSTEMWORLD_H

#include <OgreMovableObject.h>
#include <OgreRenderable.h>
#include <OgreResourceTransition.h>

#include "GpuParticleEmitter.h"
#include "GpuParticleSystem.h"

class HlmsParticleDatablock;
class HlmsParticleListener;

/// Class which will manage all gpu particles.
class GpuParticleSystemWorld : public Ogre::MovableObject
{
public:

    /// ParticleRenderable is associated with one datablock.
    class ParticleRenderable : public Ogre::Renderable
    {
    private:
        GpuParticleSystemWorld* mGpuParticleSystemWorld;
        Ogre::VaoManager* mVaoManager = nullptr;
        Ogre::VertexArrayObject* mVao = nullptr;
        Ogre::String mDatablockName;
        HlmsParticleDatablock* mParticleDatablock = nullptr;

        std::vector<Ogre::uint32> mCachedInstanceIndexes;
        Ogre::uint32 mCachedStartBucketIndex = 0;
        Ogre::uint32 mCachedBucketsCount = 0;

    public:

        ParticleRenderable(GpuParticleSystemWorld* particleSystemWorld,
                           Ogre::VaoManager* vaoManager,
                           Ogre::IndexBufferPacked* indexBuffer,
                           const Ogre::String& datablockName,
                           Ogre::uint32 bucketSize);
        ~ParticleRenderable();

        //Overrides from Renderable
        virtual const Ogre::LightList& getLights(void) const;
        virtual void getRenderOperation( Ogre::v1::RenderOperation& op, bool casterPass );
        virtual void getWorldTransforms( Ogre::Matrix4* xform ) const;
        virtual bool getCastsShadows(void) const;
        //        virtual bool getUseIdentityWorldMatrix(void) const          { return true; }

        void clearCachedData()
        {
            mCachedInstanceIndexes.clear();
            mCachedStartBucketIndex = 0;
            mCachedBucketsCount = 0;
        }

        void prepareIndexBufferRange(Ogre::uint32 entriesCount);

        Ogre::uint32 getCachedStartBucketIndex() const { return mCachedStartBucketIndex; }
        Ogre::uint32 getCachedBucketsCount() const { return mCachedBucketsCount; }
        GpuParticleSystemWorld* getGpuParticleSystemWorld() const { return mGpuParticleSystemWorld; }

        friend class GpuParticleSystemWorld;
    };

    /// Instance using GpuParticleEmitter.
    class EmitterInstance
    {
    public:

        const GpuParticleEmitter* mGpuParticleEmitter = nullptr;
        ParticleRenderable* mParticleRenderable = nullptr;
        int mGpuParticleEmitterIndex = 0;
        Ogre::uint64 mId = 0;
        bool mRun = true;

        Ogre::Vector3 mPos = Ogre::Vector3::ZERO;
        Ogre::Quaternion mRot;

        /// If emitter is movable, set the mNode.
        /// Otherwise it will take mPos and mRot instead.
        Ogre::Node* mNode = nullptr;

        float mParticleRemainder = 0.0f;
        float mFinishingParticleRemainder = 0.0f;
        float mTimeSinceStarted = 0.0f;
        float mTimeSinceStopped = 0.0f;

        inline void shiftParticleArrayIndex(Ogre::uint32 indexToAdd)
        {
            mParticleArrayStart = (mParticleArrayStart + indexToAdd) % mEmitterParticleMaxCount;
        }

        /// If emitter is static (by static it means it have position and orientation but no Ogre::Node provided)
        /// we may make some assumptions like create whole bucket of particles instead of few.
        bool isStatic() const
        {
            return mNode == NULL;
        }

        Ogre::uint32 mParticleArrayStart = 0;

        /// mParticleCount does not contain particles added in this frame during processTime.
        Ogre::uint32 mParticleCount = 0;

        /// Number of alive particles after creation process.
        /// In static emitters we may create whole bucket of particles ahead of time.
        Ogre::uint32 mParticleCreatedCount = 0;

        Ogre::uint32 mParticleAddedThisFrameCount = 0;

        Ogre::uint32 mEmitterParticleMaxCount = 0;

        std::vector<Ogre::uint32> mBucketIndexes;
    };

    /// Debug information to display.
    struct Info
    {
        /// No information from gpu is taken. Count is calculated from index range from-to.
        /// It should be equal when mParticleLifetimeMin == mParticleLifetimeMax for all particles, but
        /// otherwise some particles inside range could be already dead so the real number of
        /// alive particles can be even lesser.
        Ogre::uint32 mAliveParticles = 0;

        Ogre::uint32 mParticlesAddedThisFrame = 0;

        /// For static particle emitters particles are created ahead of time.
        Ogre::uint32 mParticlesCreated = 0;

        Ogre::uint32 mUsedBucketsParticleCapacity = 0;
    };

    struct BucketGroupData;

    /// Emitter list. It will be iterated each frame (by calling processTime method).
    typedef std::vector<EmitterInstance> EmitterInstanceList;
    EmitterInstanceList mEmitterInstances;

    /// Multimap to search for elements.
    typedef std::multimap<Ogre::uint64, int> EmitterInstanceIdToListIndex;
    std::multimap<Ogre::uint64, int> mEmitterInstanceIdToListIndex;

    /// ParticleRenderable is per datablock.
    typedef std::vector<ParticleRenderable*> ParticleRenderableList;
    ParticleRenderableList mParticleRenderables;

public:

    /// @param affectors - affectors handled by this GpuParticleSystemWorld.
    ///                    Also contains default values in case emitterCore have not have
    ///                    this affector. Takes ownership. Sorted inside by property names.
    /// @param useDepthTexture - depth texture is used for particle collisions.
    /// @param compositorWorkspace - only needed when useDepthTexture == true
    /// @param depthTextureCompositorNode - only needed when useDepthTexture == true
    /// @param depthTextureName - only needed when useDepthTexture == true
    GpuParticleSystemWorld(Ogre::IdType id,
                           Ogre::ObjectMemoryManager *objectMemoryManager,
                           Ogre::SceneManager* manager,
                           Ogre::uint8 renderQueueId,
                           HlmsParticleListener* hlmsParticleListener,
                           const std::vector<GpuParticleAffector*>& affectors,
                           bool useDepthTexture,
                           Ogre::CompositorWorkspace* compositorWorkspace = nullptr,
                           Ogre::IdString depthTextureCompositorNode = Ogre::IdString(),
                           Ogre::IdString depthTextureName = Ogre::IdString());

    ~GpuParticleSystemWorld();

public:

    /// Real maximum number of particles will be aligned to bucket size.
    /// @param bucketSize - number of particles per bucket.
    ///                     64 because this is default number of threads per group.
    /// @param maxEmitterCores - all emitter cores (recipes) are stored in uav buffer.
    ///                          This param is necessary to calculate size of this buffer.
    ///                          All systems handled by GpuParticleSystemWorld must share the same datablock.
    void init(Ogre::uint32 maxParticles,
              Ogre::uint16 maxEmitterInstances = 1024,
              Ogre::uint16 maxEmitterCores = 256,
              Ogre::uint16 bucketSize = 64,
              Ogre::uint16 gpuThreadsPerGroup = 64);


    void registerEmitterCore(const GpuParticleEmitter* particleEmitterCore);
    void registerEmitterCore(const std::vector<GpuParticleEmitter*>& emitters);

    /// Note that one GpuParticleSystem may have multiple GpuParticleEmitter.
    void registerEmitterCore(const GpuParticleSystem* particleSystem);

    void unregisterEmitterCore(const GpuParticleEmitter* particleEmitterCore);
    void unregisterEmitterCore(const std::vector<GpuParticleEmitter*>& emitters);
    void unregisterEmitterCore(const GpuParticleSystem* particleSystem);

    /// After emitter core modification.
    void makeEmitterCoresDirty();

    /// Checks if there is enough particle buckets and emitter instance count.
    /// It also check if this emitter is registered in this GpuParticleSystemWorld.
    bool canAdd(const GpuParticleEmitter* emitterCore) const;
    bool canAdd(const std::vector<GpuParticleEmitter*>& emitters) const;
    bool canAdd(const GpuParticleSystem* particleSystem) const;

    /// @param emitterNode - optional, if there is no node, offset will be used and
    ///                      emitter will be treated as static.
    /// @returns id
    Ogre::uint64 start(const GpuParticleEmitter* emitterCore, Ogre::Node* parentNode, const Ogre::Vector3& parentPos = Ogre::Vector3::ZERO, const Ogre::Quaternion& parentRot = Ogre::Quaternion());

    /// Adds multiple emitters. If there is not enough bucket, no particle emitter will be added.
    /// @param emitters - each emitter may have different offset.
    /// @param parentNode - node is the same for all added emitters
    /// @param parentPos, parentRot - they are parent of each emitter offsets. They are child transform of parentNode.
    /// @returns id - it is the same for all added emitters
    Ogre::uint64 start(const std::vector<GpuParticleEmitter*>& emitters, Ogre::Node* parentNode = NULL, const Ogre::Vector3& parentPos = Ogre::Vector3::ZERO, const Ogre::Quaternion& parentRot = Ogre::Quaternion());
    Ogre::uint64 start(const GpuParticleSystem* particleSystem, Ogre::Node* parentNode = NULL, const Ogre::Vector3& parentPos = Ogre::Vector3::ZERO, const Ogre::Quaternion& parentRot = Ogre::Quaternion());

    /// Note that burst particles don't need this.
    /// @param destroyAllParticles - if true, particles from emitter instance will dissapear immediately,
    ///                              otherwise emitter just stop adding new particles.
    void stop(Ogre::uint64 instanceId, bool destroyAllParticles);

    /// Stops all emitters (with destroying all particles)
    void stopAll();

    void processTime(float elapsedTime);

public:

    Ogre::ReadOnlyBufferPacked* getParticleBufferAsReadOnly() const;
    Ogre::ReadOnlyBufferPacked* getEntryBucketBufferAsReadOnly() const;
    Ogre::ReadOnlyBufferPacked* getParticleWorldBufferAsReadOnly() const;
    Ogre::ReadOnlyBufferPacked* getEmitterCoreBufferAsReadOnly() const;

    Ogre::uint32 getMaxParticles() const;
    Ogre::uint32 getBucketGroupsCountToRender() const;
    Ogre::uint32 getMaxEmitterCores() const;
    Ogre::uint32 getMaxEmitterInstances() const;

    /// Get diagnostic info like alive particle count etc. It iterate trough instances.
    Info getInfo() const;

    inline int getTotalBuckets() const { return mBucketCount; }
    inline int getAvailableBuckets() const { return mAvailableBucketsStack.size(); }
    bool canAllocateBuckets(int buckets) const { return buckets <= (int)mAvailableBucketsStack.size(); }

    typedef std::vector<const GpuParticleAffector*> AffectorList;

private:
    Ogre::IndexBufferPacked* mIndexBuffer = nullptr;

    /// Each particle data (read/modified on gpu side only)
    Ogre::UavBufferPacked* mParticleBuffer = nullptr;
    Ogre::ReadOnlyBufferPacked* mParticleBufferAsReadOnly = nullptr;

    /// Each particle bucket data (cpu write, gpu read)
    Ogre::UavBufferPacked* mEntryBucketBuffer = nullptr;
    Ogre::ReadOnlyBufferPacked* mEntryBucketBufferAsReadOnly = nullptr;
    Ogre::uint32* mCpuEntryBucketBuffer = nullptr;
    Ogre::uint32 mEntryBucketCount = 0;

    /// Particle system types (cpu write (only when dirty), gpu read)
    Ogre::UavBufferPacked* mEmitterCoreBuffer = nullptr;
    Ogre::ReadOnlyBufferPacked* mEmitterCoreBufferAsReadOnly = nullptr;
    float* mCpuEmitterCoreBuffer = nullptr;
    bool mEmitterCoreBufferDirty = false;

    /// Emitters instances (cpu write, gpu read)
    Ogre::UavBufferPacked* mEmitterInstanceBuffer = nullptr;
    Ogre::ReadOnlyBufferPacked* mEmitterInstanceBufferAsReadOnly = nullptr;
    float* mCpuEmitterInstanceBuffer = nullptr;

    /// ParticleWorld data (only 1 element with data like elapsed time)
    Ogre::UavBufferPacked* mParticleWorldBuffer = nullptr;
    Ogre::ReadOnlyBufferPacked* mParticleWorldBufferAsReadOnly = nullptr;
    float* mCpuParticleWorldBuffer = nullptr;

    /// Emitter cores must be registered as they will be send to buffer.
    std::vector<const GpuParticleEmitter*> mRegisteredEmitterCores;

    /// The number is count of emitter registered. GpuParticleEmitter may be registered
    /// multiple times (some systems may have multiple instances of the same GpuParticleEmitter).
    typedef std::map<const GpuParticleEmitter*, Ogre::uint16> GpuParticleEmitterMap;
    GpuParticleEmitterMap mRegisteredEmitterCoresSet;

    std::vector<const GpuParticleAffector*> mRegisteredAffectorList;
    /// Contains 'Ogre::IdString(mRegisteredAffectorList[k]->getAffectorProperty())' for
    /// fast search inside GpuParticleEmitter.
    std::vector<Ogre::IdString> mRegisteredAffectorIdStringList;

    /// Bucket indexes stack.
    std::vector<Ogre::uint32> mAvailableBucketsStack;

    HlmsParticleListener* mHlmsParticleListener;

    /// Depth texture is used to particle depth buffer collision.
    bool mUseDepthTexture = true;

    /// Some initialization steps will be in update shader.
    /// In case where we create whole bucket particles ahead of time,
    /// there is no knowing where emitter will for those instances
    /// attached to Ogre::SceneNode.
    bool mInitLocationInUpdate = true;

    /// Compositor contains depth buffer needed by depth collisions.
    /// Only needed if mUseDepthTexture == true
    Ogre::CompositorWorkspace* mCompositorWorkspace;

    /// Only needed if mUseDepthTexture == true
    Ogre::IdString mDepthTextureCompositorNode;

    /// Only needed if mUseDepthTexture == true
    Ogre::IdString mDepthTextureName;

public:

    //Overrides from MovableObject
    virtual const Ogre::String& getMovableType(void) const;

    static const int RenderableTypeId;
    static const int EntryBucketDataStructSize;
    static const int EmitterInstanceDataStructSize;
    static const int ParticleWorldDataStructSize;

    Ogre::uint32 estimateRequiredBucketCount(const GpuParticleEmitter* emitterCore) const;

    Ogre::uint32 getParticleDataStructFinalSize() const;
    Ogre::uint32 getEmitterCoreDataStructFinalSize() const;

    const AffectorList& getRegisteredAffectorList() const;

    Ogre::uint16 getBucketSize() const;

private:
    static const int ParticleDataStructSize;
    static const int EmitterCoreDataStructSize;

private:
    Ogre::VaoManager* mVaoManager;
    Ogre::uint32 mMaxParticles;
    Ogre::uint32 mMaxEmitterInstances;
    Ogre::uint32 mMaxEmitterCores;
    Ogre::uint32 mBucketCount;
    Ogre::uint16 mBucketSize;
    Ogre::uint16 mThreadsPerGroup;
    Ogre::uint16 mGroupsPerBucket;

    Ogre::uint32 mParticleDataStructFinalSize;
    Ogre::uint32 mEmitterCoreDataStructFinalSize;

    Ogre::HlmsComputeJob* mCreateParticlesJob = nullptr;
    Ogre::HlmsComputeJob* mUpdateParticlesJob = nullptr;
    Ogre::ResourceTransitionArray mResourceTransitions;

    void updateInstances(float elapsedTime);

    void initBuffers();
    void destroyBuffers();
    /// @returns false if there is no bucket available
    bool requestBuckets(EmitterInstance& emitterInstance);
    void freeBuckets(EmitterInstance& emitterInstance);

    /// For example when emitter instance stopped, we may already free some unused buckets.
    /// Method assume, that there will be at least one used bucket.
    void freeUnusedBuckets(EmitterInstance& emitterInstance);
    Ogre::uint32 getBucketsForNumber(Ogre::uint32 number) const;
    void uploadToGpuEmitterCores();

    void uploadToGpuEmitterInstances();
    void uploadEntryBucketRow(Ogre::uint32 *& RESTRICT_ALIAS entryBucketBuffer, const BucketGroupData& bucketGroup);
    void uploadToGpuParticleWorld(float elapsedTime);
    /// @returns number of entries generated for instance
    Ogre::uint32 uploadBucketsForInstance(Ogre::uint32 *& RESTRICT_ALIAS entryBucketBuffer, size_t emitterInstanceIndex);
    Ogre::uint32 uploadBucketsForInstance(Ogre::uint32 *& RESTRICT_ALIAS entryBucketBuffer, size_t emitterInstanceIndex, Ogre::uint32 lastParticle, Ogre::uint32 particleCount);

    void emitParticleCreateGpu();
    void emitParticleUpdateGpu(Ogre::uint32& resultEntriesCount);
    void updateInstancesToCores();
    void destroyParticleRenderable(const Ogre::String& datablockName);

    /// Can assert if not enough buckets.
    void createEmitterInstance(const GpuParticleEmitter* gpuParticleEmitterCore, const Ogre::Matrix4& matParent, Ogre::Node* parentNode, Ogre::uint64 idCounter);

    /// Removes i-th element then swaps last element to fill the gap.
    void destroyEmitterInstance(int instanceIndex);

    ParticleRenderable* getRenderableForEmitterCore(const GpuParticleEmitter* emitterCore) const;

    Ogre::uint64 getNextId() const;

    /// Uploads buckets offsets and primitive range for renderable objects.
    void prepareForRender();

    EmitterInstanceIdToListIndex::iterator findEmitterInstanceIt(int listIndex);

    Ogre::HlmsComputeJob* getParticleCreateComputeJob();
    Ogre::HlmsComputeJob* getParticleUpdateComputeJob();
};

#endif
