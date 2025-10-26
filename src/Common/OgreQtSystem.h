/*
 * File: OgreQtSystem.h
 * Author: Przemysław Bągard
 * Created: 2020-6-3
 *
 */

#ifndef OGREQTSYSTEM_H
#define OGREQTSYSTEM_H

#include "OgreSDLSystem.h"

class QWindow;

/// Customize OgreSDLSystem to allow Qt usage.
class OgreQtSystem : public OgreSDLSystem
{
public:

    enum class Renderer
    {
        /// It does not matter
        None,

        OpenGL,

        Direct3D,

        Vulkan
    };


public:

    OgreQtSystem(OgreSDLGame *ogreSdlGame,
                 Ogre::String resourcePath = Ogre::String(""),
                 Ogre::ColourValue backgroundColour = Ogre::ColourValue( 0.2f, 0.4f, 0.6f ));

    void setWindow(QWindow* window);

    virtual void customConfigHandle() override;

    /// Called right before initializing Ogre's first window, so the params can be customized
    virtual void initMiscParamsListener( Ogre::NameValuePairList &params ) override;

    virtual void registerHlms(void) override;
    virtual void setHlmsShadersOutputFolder();

protected:
    QWindow* mQtWindow;
    Ogre::String mHlmsShadersOutputDirectoryName;
    Renderer mPreferredRenderer;
};

#endif
