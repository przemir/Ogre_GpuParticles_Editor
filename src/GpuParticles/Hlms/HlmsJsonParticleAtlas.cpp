/*
 * File: HlmsJsonParticleAtlas.cpp
 * Author: Przemysław Bągard
 * Created: 2021-5-15
 *
 */

#if !OGRE_NO_JSON

#include "GpuParticles/Hlms/HlmsJsonParticleAtlas.h"

#include "rapidjson/document.h"

#include <OgreStringConverter.h>
#include <OgreTextureGpu.h>

HlmsJsonParticleAtlas::HlmsJsonParticleAtlas(Ogre::HlmsManager* hlmsManager,
                                             Ogre::TextureGpuManager* textureManager)
    : mHlmsManager( hlmsManager )
    , mTextureManager( textureManager )
{

}

void HlmsJsonParticleAtlas::loadMaterial(const rapidjson::Value& json,
                                         const Ogre::HlmsJson::NamedBlocks& blocks,
                                         Ogre::HlmsDatablock* datablock,
                                         const Ogre::String& resourceGroup)
{
    assert(dynamic_cast<HlmsParticleDatablock*>(datablock));
    HlmsParticleDatablock* particleDatablock = static_cast<HlmsParticleDatablock*>(datablock);

    rapidjson::Value::ConstMemberIterator itor = json.FindMember("isFlipbook");
    if( itor != json.MemberEnd() && itor->value.IsBool() ) {
        particleDatablock->setIsFlipbook(itor->value.GetBool());
    }

    itor = json.FindMember("atlas");
    if( itor != json.MemberEnd() && itor->value.IsObject() )
    {
        const rapidjson::Value &subobj = itor->value;
        loadAtlas( subobj, blocks, particleDatablock, resourceGroup );
    }

    itor = json.FindMember("flipbookSize");
    if( itor != json.MemberEnd() && itor->value.IsArray() )
    {
        const rapidjson::Value &subobj = itor->value;
        loadFlipbookSize(subobj, blocks, particleDatablock, resourceGroup );
    }

    if(!particleDatablock->getIsFlipbook()) {
        particleDatablock->recalculateInvTextureSize();
    }
}

void HlmsJsonParticleAtlas::loadAtlas(const rapidjson::Value& json,
                                      const Ogre::HlmsJson::NamedBlocks& blocks,
                                      HlmsParticleDatablock* datablock,
                                      const Ogre::String& resourceGroup)
{
    for (rapidjson::Value::ConstMemberIterator itor = json.MemberBegin(); itor != json.MemberEnd(); ++itor) {
        if (!itor->value.IsArray()) {
            continue;
        }

        const rapidjson::Value& array = itor->value;
        if(array.Size() != 4) {
            continue;
        }


        HlmsParticleDatablock::Sprite sprite;
        sprite.mName = itor->name.GetString();
        sprite.mLeft = static_cast<int>(array[0].GetInt());
        sprite.mBottom = static_cast<int>(array[1].GetInt());
        sprite.mSizeX = static_cast<int>(array[2].GetInt());
        sprite.mSizeY = static_cast<int>(array[3].GetInt());

        datablock->addSprite(sprite);
    }
}

void HlmsJsonParticleAtlas::loadFlipbookSize(const rapidjson::Value& json, const Ogre::HlmsJson::NamedBlocks& blocks, HlmsParticleDatablock* datablock, const Ogre::String& resourceGroup)
{
    if (!json.IsArray()) {
        return;
    }

    if(json.Size() != 2) {
        return;
    }

    HlmsParticleDatablock::SpriteCoord spriteCoord;
    spriteCoord.col = static_cast<int>(json[0].GetInt());
    spriteCoord.row = static_cast<int>(json[1].GetInt());

    datablock->setFlipbookSize(spriteCoord.col, spriteCoord.row);
}


void HlmsJsonParticleAtlas::saveMaterial(const Ogre::HlmsDatablock* datablock,
                                         Ogre::String& outString)
{
    assert( dynamic_cast<const HlmsParticleDatablock*>(datablock) );
    const HlmsParticleDatablock *particleDatablock = static_cast<const HlmsParticleDatablock*>(datablock);

    outString += "\n\t\t\t" + quote("isFlipbook") + ": " + (particleDatablock->getIsFlipbook() ? "true" : "false") + ",";

    if(particleDatablock->getIsFlipbook())
    {
        outString += "\n\t\t\t" + quote("flipbookSize") + ": [";

        Ogre::uint8 rowCount = particleDatablock->getFlipbookSize().row;
        Ogre::uint8 colCount = particleDatablock->getFlipbookSize().col;

        outString += Ogre::StringConverter::toString(colCount);
        outString += ", ";
        outString += Ogre::StringConverter::toString(rowCount);
        outString += "]";
    }
    else
    {
        outString += "\n\t\t\t" + quote("atlas") + ": [";

        for(HlmsParticleDatablock::SpriteList::const_iterator it = particleDatablock->getSprites().begin();
            it != particleDatablock->getSprites().end(); ++it) {

            const HlmsParticleDatablock::Sprite& sprite = *it;

            if(it != particleDatablock->getSprites().begin()) {
                outString += ",";
            }

            outString += "\n\t\t\t\t[ name : " + quote(sprite.mName) + ", geometry : [";
            outString += Ogre::StringConverter::toString(sprite.mLeft);
            outString += ", ";
            outString += Ogre::StringConverter::toString(sprite.mBottom);
            outString += ", ";
            outString += Ogre::StringConverter::toString(sprite.mSizeX);
            outString += ", ";
            outString += Ogre::StringConverter::toString(sprite.mSizeY);
            outString += "]]";
        }

        outString += "\n\t\t\t]";
    }
}

Ogre::String HlmsJsonParticleAtlas::quote(const Ogre::String& value)
{
    return "\"" + value + "\"";
}

#endif
