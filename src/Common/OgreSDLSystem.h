
#ifndef _OgreSDLSystem_H_
#define _OgreSDLSystem_H_

#include "OgreOverlayPrerequisites.h"
#include "OgrePrerequisites.h"

#include "OgreColourValue.h"
#include "OgreOverlaySystem.h"
//#include "SdlEmulationLayer.h"
//#include "System/StaticPluginLoader.h"
#include "Threading/OgreUniformScalableTask.h"


#if OGRE_USE_SDL2
  #if OGRE_VERSION_MAJOR >= 3
#    if defined( __clang__ )
#        pragma clang diagnostic push
#        pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#    elif defined( __GNUC__ )
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#    endif
  #endif
#    include <SDL.h>
  #if OGRE_VERSION_MAJOR >= 3
#    if defined( __clang__ )
#        pragma clang diagnostic pop
#    elif defined( __GNUC__ )
#        pragma GCC diagnostic pop
#    endif
  #endif
#endif

//namespace Demo
//{
    class OgreSDLInputHandler;
    class OgreSDLGame;

    class OgreSDLSystem
    {
    protected:
        OgreSDLGame* mOgreSDLGame;

    #if OGRE_USE_SDL2
        SDL_Window *     mSdlWindow;
        OgreSDLInputHandler *mInputHandler;
    #endif

        Ogre::Root *               mRoot;
        Ogre::Window *             mRenderWindow;
        Ogre::SceneManager *       mSceneManager;
        Ogre::Camera *             mCamera;
        Ogre::CompositorWorkspace *mWorkspace;
        Ogre::String               mPluginsFolder;
        Ogre::String               mWriteAccessFolder;
        Ogre::String               mResourcePath;
        Ogre::String                mResourcesCfg;
        Ogre::String                mOgreCfg;
        Ogre::String                mPluginsCfg;
        Ogre::String                mProjectPrefix; // may be like 'cache/project_'

        Ogre::v1::OverlaySystem *mOverlaySystem;

//        StaticPluginLoader          mStaticPluginLoader;

        /// Tracks the amount of elapsed time since we last
        /// heard from the LogicSystem finishing a frame
        float                mAccumTimeSinceLastLogicFrame;
        Ogre::uint32         mCurrentTransformIdx;

        bool mQuit;
        bool mAlwaysAskForConfig;
        bool mUseHlmsDiskCache;
        bool mUseMicrocodeCache;
        bool                mUseCustomConfigHandle;

        Ogre::ColourValue mBackgroundColour;

#if OGRE_USE_SDL2
        void handleWindowEvent( const SDL_Event &evt );
#endif

        bool isWriteAccessFolder( const Ogre::String &folderPath, const Ogre::String &fileToSave );

        static void addResourceLocation( const Ogre::String &archName, const Ogre::String &typeName,
                                         const Ogre::String &secName );

        void loadTextureCache();
        void saveTextureCache();
        void loadHlmsDiskCache();
        void saveHlmsDiskCache();

        virtual void setupResources();
        virtual void registerHlms();
        /// Optional override method where you can perform resource group loading
        /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        virtual void loadResources();
        virtual void chooseSceneManager();
        virtual void createCamera();
        /// Virtual so that advanced samples such as Sample_Compositor can override this
        /// method to change the default behavior if setupCompositor() is overridden, be
        /// aware @mBackgroundColour will be ignored
        virtual Ogre::CompositorWorkspace *setupCompositor();

        /// Called right before initializing Ogre's first window, so the params can be customized
        virtual void initMiscParamsListener( Ogre::NameValuePairList &params );

        /// Optional override method where you can create resource listeners (e.g. for loading screens)
        virtual void createResourceListener() {}

        virtual void customConfigHandle() {}

    public:
        OgreSDLSystem( OgreSDLGame *ogreSdlGame,
                        Ogre::String resourcePath = Ogre::String(""),
                        Ogre::ColourValue backgroundColour = Ogre::ColourValue( 0.2f, 0.4f, 0.6f ) );
        virtual ~OgreSDLSystem();

        virtual void initialize( const Ogre::String &windowTitle );
        virtual void deinitialize();

        void update( float timeSinceLast );

#if OGRE_USE_SDL2
        OgreSDLInputHandler *getInputHandler() { return mInputHandler; }
#endif

#if OGRE_VERSION_MAJOR >= 3
        /// Creates an atmosphere and binds it to the SceneManager
        /// You can use SceneManager::getAtmosphere to retrieve it.
        ///
        /// The input light will be bound to the atmosphere component.
        /// Can be nullptr.
        void createAtmosphere( Ogre::Light *sunLight );
#endif
        void setQuit() { mQuit = true; }
        bool getQuit() const { return mQuit; }

        float getAccumTimeSinceLastLogicFrame() const { return mAccumTimeSinceLastLogicFrame; }

        Ogre::Root *               getRoot() const { return mRoot; }
        Ogre::Window *             getRenderWindow() const { return mRenderWindow; }
        Ogre::SceneManager *       getSceneManager() const { return mSceneManager; }
        Ogre::Camera *             getCamera() const { return mCamera; }
        Ogre::CompositorWorkspace *getCompositorWorkspace() const { return mWorkspace; }
        Ogre::v1::OverlaySystem *  getOverlaySystem() const { return mOverlaySystem; }

        const Ogre::String &getPluginsFolder() const { return mPluginsFolder; }
        const Ogre::String &getWriteAccessFolder() const { return mWriteAccessFolder; }
        const Ogre::String &getResourcePath() const { return mResourcePath; }

        virtual void stopCompositor();
        virtual void restartCompositor();
    };
//}

#endif
