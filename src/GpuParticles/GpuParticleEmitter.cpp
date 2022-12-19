/*
 * File: GpuParticleEmitter.cpp
 * Author: Przemysław Bągard
 * Created: 2021-5-4
 *
 */

#include "GpuParticles/GpuParticleEmitter.h"
#include <OgreStringConverter.h>

const float GpuParticleEmitter::Epsilon = 0.001f;

GpuParticleEmitter::GpuParticleEmitter()
{

}

GpuParticleEmitter::~GpuParticleEmitter()
{
    for(AffectorMap::const_iterator it = mAffectors.begin(); it != mAffectors.end(); ++it) {
        delete it->second;
    }
}

GpuParticleEmitter::GpuParticleEmitter(const GpuParticleEmitter& other)
    : mPos(other.mPos)
    , mRot(other.mRot)
    , mDatablockName(other.mDatablockName)
    , mSpriteMode(other.mSpriteMode)
    , mSpriteFlipbookCoords(other.mSpriteFlipbookCoords)
    , mSpriteTimes(other.mSpriteTimes)
    , mEmitterLifetime(other.mEmitterLifetime)
    , mEmissionRate(other.mEmissionRate)
    , mBurstParticles(other.mBurstParticles)
    , mBurstMode(other.mBurstMode)
    , mSpawnShape(other.mSpawnShape)
    , mSpawnShapeDimensions(other.mSpawnShapeDimensions)
    , mFaderMode(other.mFaderMode)
    , mParticleFaderStartTime(other.mParticleFaderStartTime)
    , mParticleFaderEndTime(other.mParticleFaderEndTime)
    , mUniformSize(other.mUniformSize)
    , mBillboardType(other.mBillboardType)
    , mColourA(other.mColourA)
    , mColourB(other.mColourB)
    , mSizeMin(other.mSizeMin)
    , mSizeMax(other.mSizeMax)
    , mSizeYMin(other.mSizeYMin)
    , mSizeYMax(other.mSizeYMax)
    , mParticleLifetimeMin(other.mParticleLifetimeMin)
    , mParticleLifetimeMax(other.mParticleLifetimeMax)
    , mDirection(other.mDirection)
    , mSpotAngleMin(other.mSpotAngleMin)
    , mSpotAngleMax(other.mSpotAngleMax)
    , mDirectionVelocityMin(other.mDirectionVelocityMin)
    , mDirectionVelocityMax(other.mDirectionVelocityMax)
{
    for(AffectorMap::const_iterator it = other.mAffectors.begin(); it != other.mAffectors.end(); ++it) {
        addAffector(it->second->clone());
    }
}

GpuParticleEmitter& GpuParticleEmitter::operator=(const GpuParticleEmitter& other)
{
    mPos = other.mPos;
    mRot = other.mRot;
    mDatablockName = other.mDatablockName;
    mSpriteMode = other.mSpriteMode;
    mSpriteFlipbookCoords = other.mSpriteFlipbookCoords;
    mSpriteTimes = other.mSpriteTimes;
    mEmitterLifetime = other.mEmitterLifetime;
    mEmissionRate = other.mEmissionRate;
    mBurstParticles = other.mBurstParticles;
    mBurstMode = other.mBurstMode;
    mSpawnShape = other.mSpawnShape;
    mSpawnShapeDimensions = other.mSpawnShapeDimensions;
    mFaderMode = other.mFaderMode;
    mParticleFaderStartTime = other.mParticleFaderStartTime;
    mParticleFaderEndTime = other.mParticleFaderEndTime;
    mUniformSize = other.mUniformSize;
    mBillboardType = other.mBillboardType;
    mColourA = other.mColourA;
    mColourB = other.mColourB;
    mSizeMin = other.mSizeMin;
    mSizeMax = other.mSizeMax;
    mSizeYMin = other.mSizeYMin;
    mSizeYMax = other.mSizeYMax;
    mParticleLifetimeMin = other.mParticleLifetimeMin;
    mParticleLifetimeMax = other.mParticleLifetimeMax;
    mDirection = other.mDirection;
    mSpotAngleMin = other.mSpotAngleMin;
    mSpotAngleMax = other.mSpotAngleMax;
    mDirectionVelocityMin = other.mDirectionVelocityMin;
    mDirectionVelocityMax = other.mDirectionVelocityMax;

    // delete old affectors
    for(AffectorMap::const_iterator it = mAffectors.begin(); it != mAffectors.end(); ++it) {
        delete it->second;
    }

    // add new affectors
    for(AffectorMap::const_iterator it = other.mAffectors.begin(); it != other.mAffectors.end(); ++it) {
        addAffector(it->second->clone());
    }

    return *this;
}

Ogre::String GpuParticleEmitter::spriteModeToStr(GpuParticleEmitter::SpriteMode value)
{
    switch (value) {
    case SpriteMode::ChangeWithTrack: return "ChangeWithTrack";
    case SpriteMode::SetWithStart: return "SetWithStart";
    case SpriteMode::None: return Ogre::String();
    }
    return Ogre::String();
}

GpuParticleEmitter::SpriteMode GpuParticleEmitter::strToSpriteMode(const Ogre::String& str)
{
    if(str == "ChangeWithTrack") {
        return SpriteMode::ChangeWithTrack;
    }
    else if(str == "SetWithStart") {
        return SpriteMode::SetWithStart;
    }

    return SpriteMode::None;
}

Ogre::String GpuParticleEmitter::faderModeToStr(GpuParticleEmitter::FaderMode value)
{
    switch (value) {
    case FaderMode::Enabled: return "Enabled";
    case FaderMode::AlphaOnly: return "AlphaOnly";
    case FaderMode::None: return Ogre::String();
    }
    return Ogre::String();
}

GpuParticleEmitter::FaderMode GpuParticleEmitter::strToFaderMode(const Ogre::String& str)
{
    if(str == "Enabled") {
        return FaderMode::Enabled;
    }
    else if(str == "AlphaOnly") {
        return FaderMode::AlphaOnly;
    }

    return FaderMode::None;
}

Ogre::String GpuParticleEmitter::spawnShapeToStr(GpuParticleEmitter::SpawnShape value)
{
    switch (value) {
    case SpawnShape::Point: return "Point";
    case SpawnShape::Box: return "Box";
    case SpawnShape::Sphere: return "Sphere";
    case SpawnShape::Disc: return "Disc";
    }
    return Ogre::String();
}

GpuParticleEmitter::SpawnShape GpuParticleEmitter::strToSpawnShape(const Ogre::String& str)
{
    if(str == "Point") {
        return SpawnShape::Point;
    }
    else if(str == "Box") {
        return SpawnShape::Box;
    }
    else if(str == "Sphere") {
        return SpawnShape::Sphere;
    }
    else if(str == "Disc") {
        return SpawnShape::Disc;
    }

    return SpawnShape::Point;
}

Ogre::String GpuParticleEmitter::billboardTypeToStr(Ogre::v1::BillboardType value)
{
    switch (value) {
    case Ogre::v1::BBT_POINT: return "Point";
    case Ogre::v1::BBT_ORIENTED_COMMON: return "OrientedCommon";
    case Ogre::v1::BBT_ORIENTED_SELF: return "OrientedSelf";
    case Ogre::v1::BBT_PERPENDICULAR_COMMON: return "PerpendicularCommon";
    case Ogre::v1::BBT_PERPENDICULAR_SELF: return "PerpendicularSelf";
    }
    return "Point";
}

Ogre::v1::BillboardType GpuParticleEmitter::strToBillboardType(const Ogre::String& str)
{
    if(str == "Point") {
        return Ogre::v1::BBT_POINT;
    }
    else if(str == "OrientedCommon") {
        return Ogre::v1::BBT_ORIENTED_COMMON;
    }
    else if(str == "OrientedSelf") {
        return Ogre::v1::BBT_ORIENTED_SELF;
    }
    else if(str == "PerpendicularCommon") {
        return Ogre::v1::BBT_PERPENDICULAR_COMMON;
    }
    else if(str == "PerpendicularSelf") {
        return Ogre::v1::BBT_PERPENDICULAR_SELF;
    }

    return Ogre::v1::BBT_POINT;
}

float GpuParticleEmitter::getTimeToSpawnParticle() const
{
    if(mBurstMode) {
        return !isImmediate() ? 1.0f / (mEmitterLifetime * mBurstParticles) : 0.0f;
    }
    else {
        return 1.0f / mEmissionRate;
    }
}

float GpuParticleEmitter::getEmissionRate() const
{
    if(mBurstMode) {
        return !isImmediate() ? mBurstParticles / mEmitterLifetime : 0.0f;
    }
    return mEmissionRate;
}

bool GpuParticleEmitter::isImmediateBurst() const
{
    return mBurstMode && isImmediate();
}

GpuParticleEmitter* GpuParticleEmitter::clone()
{
    return OGRE_NEW GpuParticleEmitter(*this);
}

const GpuParticleAffector* GpuParticleEmitter::getAffectorNoThrow(AffectorType type) const
{
    AffectorMap::const_iterator it = mAffectors.find(type);
    if(it != mAffectors.end()) {
        return it->second;
    }
    return nullptr;
}

void GpuParticleEmitter::addAffector(GpuParticleAffector* affector)
{
    if(getAffectorNoThrow(affector->getType())) {
        OGRE_EXCEPT( Ogre::Exception::ERR_DUPLICATE_ITEM,
                     "emitter already contains affector of such type "
                     + Ogre::StringConverter::toString((int)affector->getType())
                     + ". Affector property '" + affector->getAffectorProperty() + "' collide with '"
                     + mAffectors[affector->getType()]->getAffectorProperty() + "'.",
                     "GpuParticleEmitter::addAffector" );
        return;
    }

    mAffectors[affector->getType()] = affector;
}

void GpuParticleEmitter::removeAndDestroyAffector(AffectorType type)
{
    AffectorMap::const_iterator it = mAffectors.find(type);
    if(it != mAffectors.end()) {
        delete it->second;
        mAffectors.erase(it);
    }
    else {
        OGRE_EXCEPT( Ogre::Exception::ERR_ITEM_NOT_FOUND,
                     "emitter does not have affector of such type "
                     + Ogre::StringConverter::toString((int)type) + ".",
                     "GpuParticleEmitter::addAffector" );
        return;
    }
}

const std::map<AffectorType, GpuParticleAffector*>& GpuParticleEmitter::getAffectors() const
{
    return mAffectors;
}
