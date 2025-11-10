/*
 * File: OgreQtSystem.h
 * Author: Przemysław Bągard
 * Created: 2020-6-3
 *
 */

#ifndef OGREQTSYSTEM_H
#define OGREQTSYSTEM_H

#include "OgreSDLSystem.h"

#include <QGuiApplication>

class QWindow;

/// Customize OgreSDLSystem to allow Qt usage.
class OgreQtSystem : public OgreSDLSystem
{
public:

    enum class Renderer
    {
        /// It does not matter.
        /// Chooses first available renderer (order from plugin.cfg/plugin_d.cfg).
        None,

        /// Open renderer chooser dialog.
        /// Some parameters like "Video Mode" or "Full Screen" won't be used.
        Choose,

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

    Renderer getRealRenderer() const;

protected:
    QWindow* mQtWindow;

    /// Subfolder of "./hlmsShadersDebug" where debug shaders will be print.
    /// May be empty, then shaders will be generated in "./hlmsShadersDebug/<type>/"
    /// instead of "./hlmsShadersDebug/<mHlmsShadersOutputDirectoryName>/<type>/"
    Ogre::String mHlmsShadersOutputDirectoryName;

    /// If preferred renderer is in available renderer list, chooses it.
    /// Otherwise first available renderer is choosen.
    Renderer mPreferredRenderer;

    /// What renderer was really selected by the application.
    Renderer mRealRenderer;

    /// Should print shaders to "./hlmsShadersDebug/<mHlmsShadersOutputDirectoryName>/<type>/" folder?
    bool mEnableUnlitPbsHlmsShadersDebug;
    bool mEnableCustomHlmsShadersDebug;
    bool mEnableComputeHlmsShadersDebug;

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX && QT_VERSION_MAJOR >= 6
    struct {
        Display *display;           // The X11 display
        unsigned long window;       // The X11 window
    } x11;
#endif
};

#endif
