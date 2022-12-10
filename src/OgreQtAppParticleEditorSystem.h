/*
 * File: OgreQtAppParticleEditorSystem.h
 * Author: Przemysław Bągard
 *
 */

#ifndef OGREQTAPPSKELETONCOREEDITORSYSTEM_H
#define OGREQTAPPSKELETONCOREEDITORSYSTEM_H

#include <Common/OgreQtSystem.h>

class QWindow;
class OgreSDLGame;

class OgreQtAppParticleEditorSystem : public OgreQtSystem
{
public:
    OgreQtAppParticleEditorSystem(OgreSDLGame *ogreSdlGame, const Ogre::String& resourcesCfg);

    virtual void initialize( const Ogre::String &windowTitle ) override;
    virtual void deinitialize(void) override;

    virtual void setupResources(void) override;
    virtual void createCamera(void) override;
    virtual Ogre::CompositorWorkspace* setupCompositor(void) override;

    virtual void registerHlms(void) override;
};

#endif
