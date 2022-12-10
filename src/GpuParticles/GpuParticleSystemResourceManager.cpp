/*
 * File: GpuParticleSystemResourceManager.cpp
 * Author: Przemysław Bągard
 * Created: 2021-6-12
 *
 */

#include <GpuParticles/GpuParticleSystemResourceManager.h>
#include <OgreException.h>
#include <OgreId.h>
#include <GpuParticles/GpuParticleSystem.h>

template<> GpuParticleSystemResourceManager *Ogre::Singleton<GpuParticleSystemResourceManager>::msSingleton = 0;

GpuParticleSystemResourceManager::GpuParticleSystemResourceManager()
{

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
                     name.getFriendlyText() + "' already exists.", "GpuParticleSystemJsonManager::createParticleSystem" );
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
                     gpuParticleSystemName.getFriendlyText() + "' was not found in mGpuParticleSystemMap.", "GpuParticleSystemJsonManager::destroyParticleSystem" );
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

    OGRE_EXCEPT( Ogre::Exception::ERR_ITEM_NOT_FOUND, "A particle system to delete was not found in mGpuParticleSystemMap.", "GpuParticleSystemJsonManager::destroyParticleSystem" );
}
