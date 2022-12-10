
#ifndef _OgreSDLSystem_H_
#define _OgreSDLSystem_H_

//#include "System/StaticPluginLoader.h"
#include "OgrePrerequisites.h"
#include "OgreColourValue.h"
#include "OgreOverlayPrerequisites.h"

#include "Threading/OgreUniformScalableTask.h"
//#include "SdlEmulationLayer.h"
#include "OgreOverlaySystem.h"

#if OGRE_USE_SDL2
    #include <SDL.h>
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
        SDL_Window          *mSdlWindow;
        OgreSDLInputHandler     *mInputHandler;
    #endif

        Ogre::Root                  *mRoot;
        Ogre::Window                *mRenderWindow;
        Ogre::SceneManager          *mSceneManager;
        Ogre::Camera                *mCamera;
        Ogre::CompositorWorkspace   *mWorkspace;
        Ogre::String                mPluginsFolder;
        Ogre::String                mWriteAccessFolder;
        Ogre::String                mResourcePath;
        Ogre::String                mResourcesCfg;
        Ogre::String                mOgreCfg;
        Ogre::String                mPluginsCfg;
        Ogre::String                mProjectPrefix; // may be like 'cache/project_'

        Ogre::v1::OverlaySystem     *mOverlaySystem;

//        StaticPluginLoader          mStaticPluginLoader;

        /// Tracks the amount of elapsed time since we last
        /// heard from the LogicSystem finishing a frame
        float               mAccumTimeSinceLastLogicFrame;
        Ogre::uint32        mCurrentTransformIdx;

        bool                mQuit;
        bool                mAlwaysAskForConfig;
        bool                mUseHlmsDiskCache;
        bool                mUseMicrocodeCache;
        bool                mUseCustomConfigHandle;

        Ogre::ColourValue   mBackgroundColour;

    #if OGRE_USE_SDL2
        void handleWindowEvent( const SDL_Event& evt );
    #endif

        bool isWriteAccessFolder( const Ogre::String &folderPath, const Ogre::String &fileToSave );

        static void addResourceLocation( const Ogre::String &archName, const Ogre::String &typeName,
                                         const Ogre::String &secName );

        void loadTextureCache(void);
        void saveTextureCache(void);
        void loadHlmsDiskCache(void);
        void saveHlmsDiskCache(void);
        virtual void setupResources(void);
        virtual void registerHlms(void);
        /// Optional override method where you can perform resource group loading
        /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        virtual void loadResources(void);
        virtual void chooseSceneManager(void);
        virtual void createCamera(void);
        /// Virtual so that advanced samples such as Sample_Compositor can override this
        /// method to change the default behavior if setupCompositor() is overridden, be
        /// aware @mBackgroundColour will be ignored
        virtual Ogre::CompositorWorkspace* setupCompositor(void);

        /// Called right before initializing Ogre's first window, so the params can be customized
        virtual void initMiscParamsListener( Ogre::NameValuePairList &params );

        /// Optional override method where you can create resource listeners (e.g. for loading screens)
        virtual void createResourceListener(void) {}

        virtual void customConfigHandle() {}

    public:
        OgreSDLSystem( OgreSDLGame *ogreSdlGame,
                        Ogre::String resourcePath = Ogre::String(""),
                        Ogre::ColourValue backgroundColour = Ogre::ColourValue( 0.2f, 0.4f, 0.6f ) );
        virtual ~OgreSDLSystem();

        virtual void initialize( const Ogre::String &windowTitle );
        virtual void deinitialize(void);

        void update( float timeSinceLast );

    #if OGRE_USE_SDL2
        OgreSDLInputHandler* getInputHandler(void)                  { return mInputHandler; }
    #endif

        void setQuit(void)                                      { mQuit = true; }
        bool getQuit(void) const                                { return mQuit; }

        float getAccumTimeSinceLastLogicFrame(void) const       { return mAccumTimeSinceLastLogicFrame; }

        Ogre::Root* getRoot(void) const                         { return mRoot; }
        Ogre::Window* getRenderWindow(void) const               { return mRenderWindow; }
        Ogre::SceneManager* getSceneManager(void) const         { return mSceneManager; }
        Ogre::Camera* getCamera(void) const                     { return mCamera; }
        Ogre::CompositorWorkspace* getCompositorWorkspace(void) const { return mWorkspace; }
        Ogre::v1::OverlaySystem* getOverlaySystem(void) const   { return mOverlaySystem; }

        const Ogre::String& getPluginsFolder(void) const        { return mPluginsFolder; }
        const Ogre::String& getWriteAccessFolder(void) const    { return mWriteAccessFolder; }
        const Ogre::String& getResourcePath(void) const         { return mResourcePath; }

        virtual void stopCompositor(void);
        virtual void restartCompositor(void);
    };
//}

#endif
