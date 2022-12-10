/*
 * File: ParticleEditorFunctions.h
 * Author: Przemysław Bągard
 * Created: 2022-12-2
 *
 */

#ifndef PARTICLEEDITORFUNCTIONS_H
#define PARTICLEEDITORFUNCTIONS_H

#include <OgreHlmsCommon.h>
#include <set>
#include <QSet>

class ParticleEditorData;
class HlmsParticleDatablock;
class GpuParticleSystem;

class ParticleEditorFunctions
{
public:
    ParticleEditorFunctions(ParticleEditorData& _data);

    void updateParticleSystemsWidgets();

    void refreshParticleDatablocks();
    void updateParticleDatablocksWidgets();

    void loadAllUnloadedTextures();

    /// Returns all resource paths containing textures.
    void getAllTexturePaths(std::set<Ogre::String>& pathSet);

    void loadTexture(const Ogre::String& initPath);

    bool checkIfTextureFileExists(const Ogre::TextureGpu* texture) const;

    void recalculateParticleDatablockSpriteIcons(const HlmsParticleDatablock* particleDatablock);

    void fillParticleSystemNames(std::set<Ogre::String>& result);
    void fillDatablockNames(std::set<Ogre::String>& result, Ogre::HlmsTypes hlmsType);
    void fillParticleSystemsUsingDatablock(std::map<Ogre::String, const GpuParticleSystem*>& result, const Ogre::String& datablockName);


private:
    ParticleEditorData& data;
};

#endif
