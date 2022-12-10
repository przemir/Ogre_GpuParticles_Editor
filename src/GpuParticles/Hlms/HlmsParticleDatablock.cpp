/*
 * File: HlmsParticleDatablock.cpp
 * Author: Przemysław Bągard
 * Created: 2021-5-15
 *
 */

#include "GpuParticles/Hlms/HlmsParticleDatablock.h"

#include <OgreTextureGpu.h>

HlmsParticleDatablock::HlmsParticleDatablock(Ogre::IdString name,
                                             Ogre::HlmsUnlit* creator,
                                             const Ogre::HlmsMacroblock* macroblock,
                                             const Ogre::HlmsBlendblock* blendblock,
                                             const Ogre::HlmsParamVec& params)
    : HlmsUnlitDatablock(name, creator, macroblock, blendblock, params)
    , mIsFlipbook(true)
    , mFlipbookSize(1, 1)
    , mInvTextureSize(Ogre::Vector2::ZERO)
{

}

void HlmsParticleDatablock::addSprite(const HlmsParticleDatablock::Sprite& sprite)
{
    mSprites.push_back(sprite);
}

const HlmsParticleDatablock::Sprite* HlmsParticleDatablock::getSprite(const Ogre::String& spriteName) const
{
    for(SpriteList::const_iterator it = mSprites.begin(); it != mSprites.end(); ++it) {
        const Sprite& sprite = *it;
        if(sprite.mName == spriteName) {
            return &sprite;
        }
    }
    return nullptr;
}

const HlmsParticleDatablock::Sprite* HlmsParticleDatablock::getSprite(int index) const
{
    return &mSprites[index];
}

const HlmsParticleDatablock::SpriteList& HlmsParticleDatablock::getSprites() const
{
    return mSprites;
}

void HlmsParticleDatablock::recalculateInvTextureSize()
{
    Ogre::TextureGpu* texture = getTexture(0);
    if(texture) {

        texture->scheduleTransitionTo( Ogre::GpuResidency::OnSystemRam );
//        texture->scheduleTransitionTo( Ogre::GpuResidency::Resident );

        texture->waitForMetadata();
        mInvTextureSize.x = 1.0f / texture->getWidth();
        mInvTextureSize.y = 1.0f / texture->getHeight();
    }
}

Ogre::Vector2 HlmsParticleDatablock::getInvTextureSize() const
{
    return mInvTextureSize;
}

bool HlmsParticleDatablock::getIsFlipbook() const
{
    return mIsFlipbook;
}

void HlmsParticleDatablock::setIsFlipbook(bool isFlipbook)
{
    mIsFlipbook = isFlipbook;
}

void HlmsParticleDatablock::setFlipbookSize(const Ogre::uint8& flipbookSizeX, const Ogre::uint8& flipbookSizeY)
{
    mFlipbookSize.col = flipbookSizeX;
    if(mFlipbookSize.col < 1) {
        mFlipbookSize.col = 1;
    }
    mFlipbookSize.row = flipbookSizeY;
    if(mFlipbookSize.row < 1) {
        mFlipbookSize.row = 1;
    }
}

void HlmsParticleDatablock::copyFromUnlitDatablockImpl(const Ogre::HlmsUnlitDatablock* srcDatablock)
{
    // Directly const cast macroblocks to keep their mRefCount consistent
    setMacroblock( const_cast<Ogre::HlmsMacroblock*>( srcDatablock->getMacroblock(false) ), false );
    setMacroblock( const_cast<Ogre::HlmsMacroblock*>( srcDatablock->getMacroblock(true) ), true );

    // Directly const cast blendblocks to keep their mRefCount consistent
    setBlendblock( const_cast<Ogre::HlmsBlendblock*>( srcDatablock->getBlendblock(false) ), false );
    setBlendblock( const_cast<Ogre::HlmsBlendblock*>( srcDatablock->getBlendblock(true) ), true );

    mAlphaTestCmp = srcDatablock->getAlphaTest();
    mAlphaTestShadowCasterOnly = srcDatablock->getAlphaTestShadowCasterOnly();
    mAlphaTestThreshold = srcDatablock->getAlphaTestThreshold();

    mShadowConstantBias = srcDatablock->mShadowConstantBias;

    // hack to access HlmsUnlitDatablock::cloneImpl which is protected member.
    {
        class HlmsUnlitDatablockProtectedAccessor : Ogre::HlmsUnlitDatablock {
        public:
            virtual void cloneImpl( HlmsDatablock *datablock ) const override
            {
                Ogre::HlmsUnlitDatablock::cloneImpl(datablock);
            }
        };

        const HlmsUnlitDatablockProtectedAccessor* hlmsUnlitDatablockProtectedAccessor = static_cast<const HlmsUnlitDatablockProtectedAccessor*>(srcDatablock);
        hlmsUnlitDatablockProtectedAccessor->cloneImpl( this );
    }

    calculateHash();
}

void HlmsParticleDatablock::cloneImpl( Ogre::HlmsDatablock *datablock ) const
{
    Ogre::HlmsUnlitDatablock::cloneImpl( datablock );
    HlmsParticleDatablock *datablockImpl = static_cast<HlmsParticleDatablock*>( datablock );

    datablockImpl->mIsFlipbook = mIsFlipbook;
    datablockImpl->mFlipbookSize = mFlipbookSize;
    datablockImpl->mInvTextureSize = mInvTextureSize;
    datablockImpl->mSprites = mSprites;
}

HlmsParticleDatablock::SpriteCoord HlmsParticleDatablock::getFlipbookSize() const
{
    return mFlipbookSize;
}
