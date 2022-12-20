/*
 * File: GpuParticleSystemResourceManager.cpp
 * Author: Przemysław Bągard
 * Created: 2021-6-12
 *
 */

#include <GpuParticles/GpuParticleSystemResourceManager.h>
#include <OgreException.h>
#include <OgreId.h>
#include <OgreStringConverter.h>
#include <GpuParticles/GpuParticleSystem.h>
#include <GpuParticles/Affectors/GpuParticleDepthCollisionAffector.h>
#include <GpuParticles/Affectors/GpuParticleGlobalGravityAffector.h>
#include <GpuParticles/Affectors/GpuParticleSetAlphaTrackAffector.h>
#include <GpuParticles/Affectors/GpuParticleSetColourTrackAffector.h>
#include <GpuParticles/Affectors/GpuParticleSetSizeTrackAffector.h>
#include <GpuParticles/Affectors/GpuParticleSetVelocityTrackAffector.h>

template<> GpuParticleSystemResourceManager *Ogre::Singleton<GpuParticleSystemResourceManager>::msSingleton = 0;

GpuParticleSystemResourceManager::GpuParticleSystemResourceManager()
{

}

GpuParticleSystemResourceManager::~GpuParticleSystemResourceManager()
{
    // destroy particle systems
    GpuParticleSystemMap::const_iterator it = mGpuParticleSystemMap.begin();
    for(; it != mGpuParticleSystemMap.end(); ++it) {
        OGRE_DELETE it->second.gpuParticleSystem;
    }

    // destroy affectors
    for(AffectorByIdStringMap::const_iterator it = mAffectorByIdStringMap.begin(); it != mAffectorByIdStringMap.end(); ++it) {
        delete it->second;
    }
    mAffectorByIdStringMap.clear();
}

GpuParticleSystem* GpuParticleSystemResourceManager::createParticleSystem(Ogre::IdString name,
                                                                const Ogre::String& refName,
                                                                const Ogre::String& filename,
                                                                const Ogre::String& resourceGroup)
{
    if (refName.empty())
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_DUPLICATE_ITEM,
            "BGpuParticleSystem has empty name.",
            "GpuParticleSystemResourceManager::addTemplate");
    }

    if( mGpuParticleSystemMap.find( name ) != mGpuParticleSystemMap.end() )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_DUPLICATE_ITEM, "A particle system with name '" +
                     name.getFriendlyText() + "' already exists.",
                     "GpuParticleSystemResourceManager::createParticleSystem" );
    }

    GpuParticleSystem* retVal =
            OGRE_NEW GpuParticleSystem();

    mGpuParticleSystemMap[name] = GpuParticleSystemEntry( retVal, refName, filename, resourceGroup );

    return retVal;
}

const GpuParticleSystemResourceManager::GpuParticleSystemMap& GpuParticleSystemResourceManager::getGpuParticleSystems() const
{
    return mGpuParticleSystemMap;
}

const GpuParticleSystem* GpuParticleSystemResourceManager::getGpuParticleSystem(const Ogre::String& name) const
{
    GpuParticleSystemMap::const_iterator it = mGpuParticleSystemMap.find(name);
    if(it != mGpuParticleSystemMap.end()) {
        return it->second.gpuParticleSystem;
    }
    return nullptr;
}

Ogre::String GpuParticleSystemResourceManager::getGpuParticleSystemName(const GpuParticleSystem* gpuParticleSystem) const
{
    GpuParticleSystemMap::const_iterator it = mGpuParticleSystemMap.begin();
    for(; it != mGpuParticleSystemMap.end(); ++it) {
        if(gpuParticleSystem == it->second.gpuParticleSystem) {
            return it->second.name;
        }
    }

    return Ogre::String();
}

void GpuParticleSystemResourceManager::destroyParticleSystem(Ogre::IdString gpuParticleSystemName)
{
    GpuParticleSystemMap::const_iterator it = mGpuParticleSystemMap.find(gpuParticleSystemName);
    if(it == mGpuParticleSystemMap.end()) {
        OGRE_EXCEPT( Ogre::Exception::ERR_ITEM_NOT_FOUND, "A particle system with name '" +
                     gpuParticleSystemName.getFriendlyText() + "' was not found in mGpuParticleSystemMap.",
                     "GpuParticleSystemResourceManager::destroyParticleSystem" );
        return;
    }

    OGRE_DELETE it->second.gpuParticleSystem;
    mGpuParticleSystemMap.erase(it);
}

void GpuParticleSystemResourceManager::destroyParticleSystem(const GpuParticleSystem* gpuParticleSystem)
{
    GpuParticleSystemMap::const_iterator it = mGpuParticleSystemMap.begin();
    for(; it != mGpuParticleSystemMap.end(); ++it) {
        if(gpuParticleSystem == it->second.gpuParticleSystem) {

            OGRE_DELETE it->second.gpuParticleSystem;
            mGpuParticleSystemMap.erase(it);

            return;
        }
    }

    OGRE_EXCEPT( Ogre::Exception::ERR_ITEM_NOT_FOUND, "A particle system to delete was not found in mGpuParticleSystemMap.",
                 "GpuParticleSystemResourceManager::destroyParticleSystem" );
}

void GpuParticleSystemResourceManager::registerCommonAffectors()
{
    registerAffector(new GpuParticleGlobalGravityAffector());
    registerAffector(new GpuParticleSetColourTrackAffector());
    registerAffector(new GpuParticleSetAlphaTrackAffector());
    registerAffector(new GpuParticleSetSizeTrackAffector());
    registerAffector(new GpuParticleSetVelocityTrackAffector());
    registerAffector(new GpuParticleDepthCollisionAffector());
}

void GpuParticleSystemResourceManager::registerAffector(GpuParticleAffector* affector)
{
    if(affector->getAffectorProperty().empty()) {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "A particle affector cannot have empty affector property name",
                     "GpuParticleSystemResourceManager::registerAffector" );
        return;
    }

    AffectorByIdStringMap::iterator itByIdString = mAffectorByIdStringMap.find(affector->getAffectorProperty());
    if(itByIdString != mAffectorByIdStringMap.end()) {

        OGRE_EXCEPT( Ogre::Exception::ERR_DUPLICATE_ITEM, "A particle affector with property name '" +
                     affector->getAffectorProperty() + "' already exists.",
                     "GpuParticleSystemResourceManager::registerAffector" );
        return;
    }

    mAffectorByIdStringMap[affector->getAffectorProperty()] = affector;
}

const GpuParticleAffector* GpuParticleSystemResourceManager::getAffectorByProperty(Ogre::IdString affectorProperty)
{
    AffectorByIdStringMap::const_iterator it = mAffectorByIdStringMap.find(affectorProperty);
    if(it != mAffectorByIdStringMap.end()) {
        return it->second;
    }
    return nullptr;
}
