/*
 * File: GpuParticleSystemResourceManager.h
 * Author: Przemysław Bągard
 * Created: 2021-6-12
 *
 */

#ifndef BPARTICLESYSTEMMANAGER_H
#define BPARTICLESYSTEMMANAGER_H

#include <OgreSingleton.h>
#include <OgreCommon.h>
#include "OgreIdString.h"
#include <map>
#include <GpuParticles/GpuParticleAffectorCommon.h>

class GpuParticleSystem;
class GpuParticleAffector;

/// Class managing particle systems as resources.
class GpuParticleSystemResourceManager
		: public Ogre::Singleton<GpuParticleSystemResourceManager>
{
public:

    struct GpuParticleSystemEntry
    {
        GpuParticleSystem *gpuParticleSystem;
        Ogre::String       name;
        Ogre::String       srcFile;            ///Filename in which it was defined, if any
        Ogre::String       srcResourceGroup;   ///ResourceGroup in which it was defined, if any
        GpuParticleSystemEntry()
            : gpuParticleSystem( nullptr )
        {}

        GpuParticleSystemEntry( GpuParticleSystem *_gpuParticleSystem,
                                const Ogre::String &_name,
                                const Ogre::String &_srcFile,
                                const Ogre::String &_srcGroup )
            : gpuParticleSystem( _gpuParticleSystem )
            , name( _name )
            , srcFile( _srcFile )
            , srcResourceGroup( _srcGroup  )
        {}
    };

    typedef std::map<Ogre::IdString, GpuParticleSystemEntry> GpuParticleSystemMap;
    typedef std::map<Ogre::IdString, GpuParticleAffector*> AffectorByIdStringMap;

public:
    GpuParticleSystemResourceManager();
    ~GpuParticleSystemResourceManager();

    GpuParticleSystem* createParticleSystem( Ogre::IdString name, const Ogre::String &refName,
                                             const Ogre::String &filename=Ogre::BLANKSTRING,
                                             const Ogre::String &resourceGroup=Ogre::BLANKSTRING );

    const GpuParticleSystemMap& getGpuParticleSystems() const;
    const GpuParticleSystem* getGpuParticleSystem(const Ogre::String& name) const;

    Ogre::String getGpuParticleSystemName(const GpuParticleSystem* gpuParticleSystem) const;

    void destroyParticleSystem(Ogre::IdString gpuParticleSystemName);
    void destroyParticleSystem(const GpuParticleSystem* gpuParticleSystem);

    void registerCommonAffectors();

    /// Takes ownership
    void registerAffector(GpuParticleAffector* affector);
    const GpuParticleAffector* getAffectorByProperty(Ogre::IdString affectorProperty);
    const AffectorByIdStringMap& getAffectorByPropertyMap() const { return mAffectorByIdStringMap; }

private:
    GpuParticleSystemMap mGpuParticleSystemMap;
    AffectorByIdStringMap mAffectorByIdStringMap;
};

#endif
