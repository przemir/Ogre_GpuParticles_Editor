/*
 * File: HlmsJsonParticleAtlas.h
 * Author: Przemysław Bągard
 * Created: 2021-5-15
 *
 */

#if !OGRE_NO_JSON
#ifndef HLMSJSONPARTICLEATLAS_H
#define HLMSJSONPARTICLEATLAS_H

//#include "OgrePrerequisites.h"
#include "OgreHlmsJson.h"
#include "HlmsParticleDatablock.h"
#include "OgreHeaderPrefix.h"

class HlmsJsonParticleAtlas
{
    Ogre::HlmsManager         *mHlmsManager;
    Ogre::TextureGpuManager   *mTextureManager;

    void loadAtlas( const rapidjson::Value &json,
                    const Ogre::HlmsJson::NamedBlocks &blocks,
                    HlmsParticleDatablock* datablock,
                    const Ogre::String &resourceGroup );

    void loadFlipbookSize( const rapidjson::Value &json,
                           const Ogre::HlmsJson::NamedBlocks &blocks,
                           HlmsParticleDatablock* datablock,
                           const Ogre::String &resourceGroup );

public:
    HlmsJsonParticleAtlas( Ogre::HlmsManager *hlmsManager, Ogre::TextureGpuManager *textureManager );

    static Ogre::String quote( const Ogre::String& value );

    void loadMaterial( const rapidjson::Value &json, const Ogre::HlmsJson::NamedBlocks &blocks,
                       Ogre::HlmsDatablock *datablock, const Ogre::String &resourceGroup );
    void saveMaterial( const Ogre::HlmsDatablock *datablock, Ogre::String &outString );
};

#endif

#endif
