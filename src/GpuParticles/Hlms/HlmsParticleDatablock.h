/*
 * File: HlmsParticleDatablock.h
 * Author: Przemysław Bągard
 * Created: 2021-5-15
 *
 */

#ifndef HLMSPARTICLEDATABLOCK_H
#define HLMSPARTICLEDATABLOCK_H

#include <OgreHlmsUnlitDatablock.h>

class HlmsParticleDatablock : public Ogre::HlmsUnlitDatablock
{
    friend class HlmsParticle;

public:

    /// Used in flipbook mode.
    struct SpriteCoord
    {
        SpriteCoord()
        {}

        SpriteCoord(Ogre::uint8 _row, Ogre::uint8 _col)
            : row(_row)
            , col(_col)
        {}

        Ogre::uint8 row = 0;
        Ogre::uint8 col = 0;
    };

    /// Used in atlas mode
    class Sprite
    {
    public:
        /// Optional, just comment to identify
        Ogre::String mName;

        Ogre::uint32 mLeft;
        Ogre::uint32 mBottom;
        Ogre::uint32 mSizeX;
        Ogre::uint32 mSizeY;
    };
    typedef std::vector<Sprite> SpriteList;

public:
    HlmsParticleDatablock(Ogre::IdString name,
                          Ogre::HlmsUnlit *creator,
                          const Ogre::HlmsMacroblock *macroblock,
                          const Ogre::HlmsBlendblock *blendblock,
                          const Ogre::HlmsParamVec &params );

    void addSprite(const Sprite& sprite);
    const Sprite* getSprite(const Ogre::String& spriteName) const;
    const Sprite* getSprite(int index) const;
    const SpriteList& getSprites() const;

    void recalculateInvTextureSize();

    Ogre::Vector2 getInvTextureSize() const;

    bool getIsFlipbook() const;
    void setIsFlipbook(bool isFlipbook);

    void setFlipbookSize(const Ogre::uint8& flipbookSizeX, const Ogre::uint8& flipbookSizeY);
    SpriteCoord getFlipbookSize() const;

    /// Called from HlmsParticle.
    void copyFromUnlitDatablockImpl(const Ogre::HlmsUnlitDatablock* srcDatablock);

    virtual void cloneImpl( Ogre::HlmsDatablock *datablock ) const override;

private:
    bool mIsFlipbook;
    SpriteCoord mFlipbookSize;
    Ogre::Vector2 mInvTextureSize;
    SpriteList mSprites;
};

#endif
