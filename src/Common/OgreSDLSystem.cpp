
#include "OgreSDLSystem.h"
#include "OgreSDLGame.h"
#if OGRE_USE_SDL2
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
    #include "OgreSDLInputHandler.h"
#endif
#endif

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreConfigFile.h"

#include "OgreCamera.h"
#include "OgreItem.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreArchiveManager.h"

#include "Compositor/OgreCompositorManager2.h"

#include "OgreOverlaySystem.h"
#include "OgreOverlayManager.h"

#include "OgreTextureGpuManager.h"

#include "OgreWindowEventUtilities.h"
#include "OgreWindow.h"

#include "OgreFileSystemLayer.h"

#include "OgreHlmsDiskCache.h"
#include "OgreGpuProgramManager.h"

#include "OgreLogManager.h"

#include "OgrePlatformInformation.h"

#include <fstream>

#if OGRE_USE_SDL2
    #include <SDL_syswm.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    #include "OSX/macUtils.h"
    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        #include "System/iOS/iOSUtils.h"
    #else
        #include "System/OSX/OSXUtils.h"
    #endif
#endif

//namespace Demo
//{
    OgreSDLSystem::OgreSDLSystem( OgreSDLGame * ogreSdlGame,
                                    Ogre::String resourcePath ,
                                    Ogre::ColourValue backgroundColour ) :
        mOgreSDLGame( ogreSdlGame ),
    #if OGRE_USE_SDL2
        mSdlWindow( 0 ),
        mInputHandler( 0 ),
    #endif
        mRoot( 0 ),
        mRenderWindow( 0 ),
        mSceneManager( 0 ),
        mCamera( 0 ),
        mWorkspace( 0 ),
        mPluginsFolder( "./" ),
        mResourcePath( resourcePath ),
        mResourcesCfg( "resources2.cfg" ),
        mOgreCfg( "ogre.cfg" ),
        mPluginsCfg( "plugins.cfg" ),
        mOverlaySystem( 0 ),
        mAccumTimeSinceLastLogicFrame( 0 ),
        mCurrentTransformIdx( 0 ),
        mQuit( false ),
        mAlwaysAskForConfig( true ),
        mUseHlmsDiskCache( true ),
        mUseMicrocodeCache( true ),
        mUseCustomConfigHandle( false ),
        mBackgroundColour( backgroundColour )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        // Note:  macBundlePath works for iOS too. It's misnamed.
        mResourcePath = Ogre::macBundlePath() + "/Contents/Resources/";
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        mResourcePath = Ogre::macBundlePath() + "/";
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        mPluginsFolder = mResourcePath;
#endif
        if( isWriteAccessFolder( mPluginsFolder, "Ogre.log" ) )
            mWriteAccessFolder = mPluginsFolder;
        else
        {
            Ogre::FileSystemLayer filesystemLayer( OGRE_VERSION_NAME );
            mWriteAccessFolder = filesystemLayer.getWritablePath( "" );
        }
    }
    //-----------------------------------------------------------------------------------
    OgreSDLSystem::~OgreSDLSystem()
    {
        if( mRoot )
        {
            Ogre::LogManager::getSingleton().logMessage(
                        "WARNING: OgreSDLSystem::deinitialize() not called!!!", Ogre::LML_CRITICAL );
        }
    }
    //-----------------------------------------------------------------------------------
    bool OgreSDLSystem::isWriteAccessFolder( const Ogre::String &folderPath,
                                              const Ogre::String &fileToSave )
    {
        if( !Ogre::FileSystemLayer::createDirectory( folderPath ) )
            return false;

        std::ofstream of( (folderPath + fileToSave).c_str(),
                          std::ios::out | std::ios::binary | std::ios::app );
        if( !of )
            return false;

        return true;
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::initialize( const Ogre::String &windowTitle )
    {
    #if OGRE_USE_SDL2
        //if( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
        if( SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                      SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS ) != 0 )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_INTERNAL_ERROR, "Cannot initialize SDL2!",
                         "OgreSDLSystem::initialize" );
        }
    #endif

        Ogre::String pluginsPath;
        // only use plugins.cfg if not static
    #ifndef OGRE_STATIC_LIB
    #if OGRE_DEBUG_MODE && !((OGRE_PLATFORM == OGRE_PLATFORM_APPLE) || (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS))
        pluginsPath = mPluginsFolder + "plugins_d.cfg";
    #else
        pluginsPath = mPluginsFolder + mPluginsCfg;
    #endif
    #endif

        mRoot = OGRE_NEW Ogre::Root( pluginsPath,
                                     mWriteAccessFolder + mOgreCfg,
                                     mWriteAccessFolder + "Ogre.log" );

//        mStaticPluginLoader.install( mRoot );
        // enable sRGB Gamma Conversion mode by default for all renderers,
        // but still allow to override it via config dialog
        Ogre::RenderSystemList::const_iterator itor = mRoot->getAvailableRenderers().begin();
        Ogre::RenderSystemList::const_iterator endt = mRoot->getAvailableRenderers().end();

        while( itor != endt )
        {
            Ogre::RenderSystem *rs = *itor;
            rs->setConfigOption( "sRGB Gamma Conversion", "Yes" );
            ++itor;
        }
        if( !mUseCustomConfigHandle ) {
        if( mAlwaysAskForConfig || !mRoot->restoreConfig() )
        {
            if( !mRoot->showConfigDialog() )
            {
                mQuit = true;
                return;
            }
        }
        }
        else {
            customConfigHandle();
        }

    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        if(!mRoot->getRenderSystem())
        {
            Ogre::RenderSystem *renderSystem =
                    mRoot->getRenderSystemByName( "Metal Rendering Subsystem" );
            mRoot->setRenderSystem( renderSystem );
        }
    #endif

        mRoot->initialise( false, windowTitle );

        Ogre::ConfigOptionMap& cfgOpts = mRoot->getRenderSystem()->getConfigOptions();

        int width   = 1280;
        int height  = 720;

    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        {
            Ogre::Vector2 screenRes = iOSUtils::getScreenResolutionInPoints();
            width = static_cast<int>( screenRes.x );
            height = static_cast<int>( screenRes.y );
        }
    #endif

        Ogre::ConfigOptionMap::iterator opt = cfgOpts.find( "Video Mode" );
        if( opt != cfgOpts.end() )
        {
            //Ignore leading space
            const Ogre::String::size_type start = opt->second.currentValue.find_first_of("012356789");
            //Get the width and height
            Ogre::String::size_type widthEnd = opt->second.currentValue.find(' ', start);
            // we know that the height starts 3 characters after the width and goes until the next space
            Ogre::String::size_type heightEnd = opt->second.currentValue.find(' ', widthEnd+3);
            // Now we can parse out the values
            width   = Ogre::StringConverter::parseInt( opt->second.currentValue.substr( 0, widthEnd ) );
            height  = Ogre::StringConverter::parseInt( opt->second.currentValue.substr(
                                                           widthEnd+3, heightEnd ) );
        }

        Ogre::NameValuePairList params;
        bool fullscreen = Ogre::StringConverter::parseBool( cfgOpts["Full Screen"].currentValue );
    #if OGRE_USE_SDL2
        int screen = 0;
        int posX = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);
        int posY = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);

        if(fullscreen)
        {
            posX = SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen);
            posY = SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen);
        }

        mSdlWindow = SDL_CreateWindow(
                    windowTitle.c_str(),    // window title
                    posX,               // initial x position
                    posY,               // initial y position
                    width,              // width, in pixels
                    height,             // height, in pixels
                    SDL_WINDOW_SHOWN
                      | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0) | SDL_WINDOW_RESIZABLE );

        //Get the native whnd
        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );

        if( SDL_GetWindowWMInfo( mSdlWindow, &wmInfo ) == SDL_FALSE )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_INTERNAL_ERROR,
                         "Couldn't get WM Info! (SDL2)",
                         "OgreSDLSystem::initialize" );
        }

        Ogre::String winHandle;
        switch( wmInfo.subsystem )
        {
        #if defined(SDL_VIDEO_DRIVER_WINDOWS)
        case SDL_SYSWM_WINDOWS:
            // Windows code
            winHandle = Ogre::StringConverter::toString( (uintptr_t)wmInfo.info.win.window );
            break;
        #endif
        #if defined(SDL_VIDEO_DRIVER_WINRT)
        case SDL_SYSWM_WINRT:
            // Windows code
            winHandle = Ogre::StringConverter::toString( (uintptr_t)wmInfo.info.winrt.window );
            break;
        #endif
        #if defined(SDL_VIDEO_DRIVER_COCOA)
        case SDL_SYSWM_COCOA:
            winHandle  = Ogre::StringConverter::toString(WindowContentViewHandle(wmInfo));
            break;
        #endif
        #if defined(SDL_VIDEO_DRIVER_X11)
        case SDL_SYSWM_X11:
            winHandle = Ogre::StringConverter::toString( (uintptr_t)wmInfo.info.x11.window );
            break;
        #endif
        default:
            OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED,
                         "Unexpected WM! (SDL2)",
                         "OgreSDLSystem::initialize" );
            break;
        }

        #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
            params.insert( std::make_pair("externalWindowHandle",  winHandle) );
        #else
            params.insert( std::make_pair("parentWindowHandle",  winHandle) );
        #endif
    #endif

        params.insert( std::make_pair("title", windowTitle) );
        params.insert( std::make_pair("gamma", cfgOpts["sRGB Gamma Conversion"].currentValue) );
        params.insert( std::make_pair("FSAA", cfgOpts["FSAA"].currentValue) );
        params.insert( std::make_pair("vsync", cfgOpts["VSync"].currentValue) );
        params.insert( std::make_pair("reverse_depth", "Yes" ) );
        initMiscParamsListener( params );

        mRenderWindow = Ogre::Root::getSingleton().createRenderWindow( windowTitle, width, height,
                                                                       fullscreen, &params );

        mOverlaySystem = OGRE_NEW Ogre::v1::OverlaySystem();

        setupResources();
        loadResources();
        chooseSceneManager();
        createCamera();
        mWorkspace = setupCompositor();

    #if OGRE_USE_SDL2
        mInputHandler = new OgreSDLInputHandler( mSdlWindow, mOgreSDLGame );
    #endif

        mOgreSDLGame->initialize();

#if OGRE_PROFILING
        Ogre::Profiler::getSingleton().setEnabled( true );
    #if OGRE_PROFILING == OGRE_PROFILING_INTERNAL
        Ogre::Profiler::getSingleton().endProfile( "" );
    #endif
    #if OGRE_PROFILING == OGRE_PROFILING_INTERNAL_OFFLINE
        Ogre::Profiler::getSingleton().getOfflineProfiler().setDumpPathsOnShutdown(
                    mWriteAccessFolder + "ProfilePerFrame",
                    mWriteAccessFolder + "ProfileAccum" );
    #endif
#endif
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::deinitialize(void)
    {
        mOgreSDLGame->deinitialize();

        saveTextureCache();
        saveHlmsDiskCache();

        if( mSceneManager )
            mSceneManager->removeRenderQueueListener( mOverlaySystem );

        OGRE_DELETE mOverlaySystem;
        mOverlaySystem = 0;

    #if OGRE_USE_SDL2
        delete mInputHandler;
        mInputHandler = 0;
    #endif

        OGRE_DELETE mRoot;
        mRoot = 0;

    #if OGRE_USE_SDL2
        if( mSdlWindow )
        {
            // Restore desktop resolution on exit
            SDL_SetWindowFullscreen( mSdlWindow, 0 );
            SDL_DestroyWindow( mSdlWindow );
            mSdlWindow = 0;
        }

        SDL_Quit();
    #endif
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::update( float timeSinceLast )
    {
        Ogre::WindowEventUtilities::messagePump();

    #if OGRE_USE_SDL2
        SDL_Event evt;
        while( SDL_PollEvent( &evt ) )
        {
            switch( evt.type )
            {
            case SDL_WINDOWEVENT:
                handleWindowEvent( evt );
                break;
            case SDL_QUIT:
                mQuit = true;
                break;
            default:
                break;
            }

            mInputHandler->_handleSdlEvents( evt );
        }
    #endif

        mOgreSDLGame->update( timeSinceLast );

        if( mRenderWindow->isVisible() )
            mQuit |= !mRoot->renderOneFrame();

        mAccumTimeSinceLastLogicFrame += timeSinceLast;

        //SDL_SetWindowPosition( mSdlWindow, 0, 0 );
        /*SDL_Rect rect;
        SDL_GetDisplayBounds( 0, &rect );
        SDL_GetDisplayBounds( 0, &rect );*/
    }
    //-----------------------------------------------------------------------------------
    #if OGRE_USE_SDL2
    void OgreSDLSystem::handleWindowEvent( const SDL_Event& evt )
    {
        switch( evt.window.event )
        {
            /*case SDL_WINDOWEVENT_MAXIMIZED:
                SDL_SetWindowBordered( mSdlWindow, SDL_FALSE );
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
                SDL_SetWindowBordered( mSdlWindow, SDL_TRUE );
                break;*/
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                int w,h;
                SDL_GetWindowSize( mSdlWindow, &w, &h );
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
                mRenderWindow->requestResolution( w, h );
#endif
                mRenderWindow->windowMovedOrResized();
                break;
            case SDL_WINDOWEVENT_RESIZED:
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
                mRenderWindow->requestResolution( evt.window.data1, evt.window.data2 );
#endif
                mRenderWindow->windowMovedOrResized();
                break;
            case SDL_WINDOWEVENT_CLOSE:
                break;
        case SDL_WINDOWEVENT_SHOWN:
            mRenderWindow->_setVisible( true );
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            mRenderWindow->_setVisible( false );
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            mRenderWindow->setFocused( true );
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            mRenderWindow->setFocused( false );
            break;
        }
    }
    #endif
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::addResourceLocation( const Ogre::String &archName, const Ogre::String &typeName,
                                              const Ogre::String &secName )
    {
#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) || (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
        // OS X does not set the working directory relative to the app,
        // In order to make things portable on OS X we need to provide
        // the loading with it's own bundle path location
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    Ogre::String( Ogre::macBundlePath() + "/" + archName ), typeName, secName );
#else
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
#endif
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::loadTextureCache(void)
    {
#if !OGRE_NO_JSON
        Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();
        Ogre::Archive *rwAccessFolderArchive = archiveManager.load( mWriteAccessFolder,
                                                                    "FileSystem", true );
        try
        {
            const Ogre::String filename = mProjectPrefix + "textureMetadataCache.json";
            if( rwAccessFolderArchive->exists( filename ) )
            {
                Ogre::DataStreamPtr stream = rwAccessFolderArchive->open( filename );
                std::vector<char> fileData;
                fileData.resize( stream->size() + 1 );
                if( !fileData.empty() )
                {
                    stream->read( &fileData[0], stream->size() );
                    //Add null terminator just in case (to prevent bad input)
                    fileData.back() = '\0';
                    Ogre::TextureGpuManager *textureManager =
                            mRoot->getRenderSystem()->getTextureGpuManager();
                    textureManager->importTextureMetadataCache( stream->getName(), &fileData[0], false );
                }
            }
            else
            {
                Ogre::LogManager::getSingleton().logMessage(
                            "[INFO] Texture cache not found at " + mWriteAccessFolder +
                            filename );
            }
        }
        catch( Ogre::Exception &e )
        {
            Ogre::LogManager::getSingleton().logMessage( e.getFullDescription() );
        }

        archiveManager.unload( rwAccessFolderArchive );
#endif
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::saveTextureCache(void)
    {
        if( mRoot->getRenderSystem() )
        {
            Ogre::TextureGpuManager *textureManager = mRoot->getRenderSystem()->getTextureGpuManager();
            if( textureManager )
            {
                Ogre::String jsonString;
                textureManager->exportTextureMetadataCache( jsonString );
                const Ogre::String path = mWriteAccessFolder + "/" + mProjectPrefix + "textureMetadataCache.json";
                std::ofstream file( path.c_str(), std::ios::binary | std::ios::out );
                if( file.is_open() )
                    file.write( jsonString.c_str(), static_cast<std::streamsize>( jsonString.size() ) );
                file.close();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::loadHlmsDiskCache(void)
    {
        if( !mUseMicrocodeCache && !mUseHlmsDiskCache )
            return;

        Ogre::HlmsManager *hlmsManager = mRoot->getHlmsManager();
        Ogre::HlmsDiskCache diskCache( hlmsManager );

        Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

        Ogre::Archive *rwAccessFolderArchive = archiveManager.load( mWriteAccessFolder,
                                                                    "FileSystem", true );

        if( mUseMicrocodeCache )
        {
            //Make sure the microcode cache is enabled.
            Ogre::GpuProgramManager::getSingleton().setSaveMicrocodesToCache( true );
            const Ogre::String filename = mProjectPrefix + "microcodeCodeCache.cache";
            if( rwAccessFolderArchive->exists( filename ) )
            {
                Ogre::DataStreamPtr shaderCacheFile = rwAccessFolderArchive->open( filename );
                Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache( shaderCacheFile );
            }
        }

        if( mUseHlmsDiskCache )
        {
            for( size_t i=Ogre::HLMS_LOW_LEVEL + 1u; i<Ogre::HLMS_MAX; ++i )
            {
                Ogre::Hlms *hlms = hlmsManager->getHlms( static_cast<Ogre::HlmsTypes>( i ) );
                if( hlms )
                {
                    Ogre::String filename = mProjectPrefix + "hlmsDiskCache" +
                                            Ogre::StringConverter::toString( i ) + ".bin";

                    try
                    {
                        if( rwAccessFolderArchive->exists( filename ) )
                        {
                            Ogre::DataStreamPtr diskCacheFile = rwAccessFolderArchive->open( filename );
                            diskCache.loadFrom( diskCacheFile );
                            diskCache.applyTo( hlms );
                        }
                    }
                    catch( Ogre::Exception& )
                    {
                        Ogre::LogManager::getSingleton().logMessage(
                                    "Error loading cache from " + mWriteAccessFolder + "/" +
                                    filename + "! If you have issues, try deleting the file "
                                    "and restarting the app" );
                    }
                }
            }
        }

        archiveManager.unload( mWriteAccessFolder );
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::saveHlmsDiskCache(void)
    {
        if( mRoot->getRenderSystem() && Ogre::GpuProgramManager::getSingletonPtr() &&
            (mUseMicrocodeCache || mUseHlmsDiskCache) )
        {
            Ogre::HlmsManager *hlmsManager = mRoot->getHlmsManager();
            Ogre::HlmsDiskCache diskCache( hlmsManager );

            Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

            Ogre::Archive *rwAccessFolderArchive = archiveManager.load( mWriteAccessFolder,
                                                                        "FileSystem", false );

            if( mUseHlmsDiskCache )
            {
                for( size_t i=Ogre::HLMS_LOW_LEVEL + 1u; i<Ogre::HLMS_MAX; ++i )
                {
                    Ogre::Hlms *hlms = hlmsManager->getHlms( static_cast<Ogre::HlmsTypes>( i ) );
                    if( hlms )
                    {
                        diskCache.copyFrom( hlms );

                        Ogre::DataStreamPtr diskCacheFile =
                                rwAccessFolderArchive->create( mProjectPrefix + "hlmsDiskCache" +
                                                               Ogre::StringConverter::toString( i ) +
                                                               ".bin" );
                        diskCache.saveTo( diskCacheFile );
                    }
                }
            }

            if( Ogre::GpuProgramManager::getSingleton().isCacheDirty() && mUseMicrocodeCache )
            {
                const Ogre::String filename = mProjectPrefix + "microcodeCodeCache.cache";
                Ogre::DataStreamPtr shaderCacheFile = rwAccessFolderArchive->create( filename );
                Ogre::GpuProgramManager::getSingleton().saveMicrocodeCache( shaderCacheFile );
            }

            archiveManager.unload( mWriteAccessFolder );
        }
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::setupResources(void)
    {
        // Load resource paths from config file
        Ogre::ConfigFile cf;
        cf.load(mResourcePath + mResourcesCfg);

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, archName;
        while( seci.hasMoreElements() )
        {
            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();

            if( secName != "Hlms" && secName != "Dev" && secName != "Settings" && secName != "Paths" )
            {
                Ogre::ConfigFile::SettingsMultiMap::iterator i;
                for (i = settings->begin(); i != settings->end(); ++i)
                {
                    typeName = i->first;
                    archName = i->second;
                    addResourceLocation( archName, typeName, secName );
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::registerHlms(void)
    {
        Ogre::ConfigFile cf;
        cf.load( mResourcePath + mResourcesCfg );

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        Ogre::String rootHlmsFolder = Ogre::macBundlePath() + '/' +
                                  cf.getSetting( "DoNotUseAsResource", "Hlms", "" );
#else
        Ogre::String rootHlmsFolder = mResourcePath + cf.getSetting( "DoNotUseAsResource", "Hlms", "" );
#endif

        if( rootHlmsFolder.empty() )
            rootHlmsFolder = "./";
        else if( *(rootHlmsFolder.end() - 1) != '/' )
            rootHlmsFolder += "/";

        //At this point rootHlmsFolder should be a valid path to the Hlms data folder

        Ogre::HlmsUnlit *hlmsUnlit = 0;
        Ogre::HlmsPbs *hlmsPbs = 0;

        //For retrieval of the paths to the different folders needed
        Ogre::String mainFolderPath;
        Ogre::StringVector libraryFoldersPaths;
        Ogre::StringVector::const_iterator libraryFolderPathIt;
        Ogre::StringVector::const_iterator libraryFolderPathEn;

        Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();
        
        {
            //Create & Register HlmsUnlit
            //Get the path to all the subdirectories used by HlmsUnlit
            Ogre::HlmsUnlit::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
            Ogre::Archive *archiveUnlit = archiveManager.load( rootHlmsFolder + mainFolderPath,
                                                               "FileSystem", true );
            Ogre::ArchiveVec archiveUnlitLibraryFolders;
            libraryFolderPathIt = libraryFoldersPaths.begin();
            libraryFolderPathEn = libraryFoldersPaths.end();
            while( libraryFolderPathIt != libraryFolderPathEn )
            {
                Ogre::Archive *archiveLibrary =
                        archiveManager.load( rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true );
                archiveUnlitLibraryFolders.push_back( archiveLibrary );
                ++libraryFolderPathIt;
            }

            //Create and register the unlit Hlms
            hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &archiveUnlitLibraryFolders );
            Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );
        }

        {
            //Create & Register HlmsPbs
            //Do the same for HlmsPbs:
            Ogre::HlmsPbs::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
            Ogre::Archive *archivePbs = archiveManager.load( rootHlmsFolder + mainFolderPath,
                                                             "FileSystem", true );

            //Get the library archive(s)
            Ogre::ArchiveVec archivePbsLibraryFolders;
            libraryFolderPathIt = libraryFoldersPaths.begin();
            libraryFolderPathEn = libraryFoldersPaths.end();
            while( libraryFolderPathIt != libraryFolderPathEn )
            {
                Ogre::Archive *archiveLibrary =
                        archiveManager.load( rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true );
                archivePbsLibraryFolders.push_back( archiveLibrary );
                ++libraryFolderPathIt;
            }

            //Create and register
            hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &archivePbsLibraryFolders );
            Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
        }


        Ogre::RenderSystem *renderSystem = mRoot->getRenderSystem();
        if( renderSystem->getName() == "Direct3D11 Rendering Subsystem" )
        {
            //Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
            //and below to avoid saturating AMD's discard limit (8MB) or
            //saturate the PCIE bus in some low end machines.
            bool supportsNoOverwriteOnTextureBuffers;
            renderSystem->getCustomAttribute( "MapNoOverwriteOnDynamicBufferSRV",
                                              &supportsNoOverwriteOnTextureBuffers );

            if( !supportsNoOverwriteOnTextureBuffers )
            {
                hlmsPbs->setTextureBufferDefaultSize( 512 * 1024 );
                hlmsUnlit->setTextureBufferDefaultSize( 512 * 1024 );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::loadResources(void)
    {
        registerHlms();

        loadTextureCache();
        loadHlmsDiskCache();

        // Initialise, parse scripts etc
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups( true );

        // Initialize resources for LTC area lights and accurate specular reflections (IBL)
        Ogre::Hlms *hlms = mRoot->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
        OGRE_ASSERT_HIGH( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlms );
        try
        {
            hlmsPbs->loadLtcMatrix();
        }
        catch( Ogre::FileNotFoundException &e )
        {
            Ogre::LogManager::getSingleton().logMessage( e.getFullDescription(), Ogre::LML_CRITICAL );
            Ogre::LogManager::getSingleton().logMessage(
                "WARNING: LTC matrix textures could not be loaded. Accurate specular IBL reflections "
                "and LTC area lights won't be available or may not function properly!",
                Ogre::LML_CRITICAL );
        }
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::chooseSceneManager(void)
    {
#if OGRE_DEBUG_MODE
        //Debugging multithreaded code is a PITA, disable it.
        const size_t numThreads = 1;
#else
        //getNumLogicalCores() may return 0 if couldn't detect
        const size_t numThreads = std::max<size_t>( 1, Ogre::PlatformInformation::getNumLogicalCores() );
#endif
        // Create the SceneManager, in this case a generic one
        mSceneManager = mRoot->createSceneManager( Ogre::ST_GENERIC,
                                                   numThreads,
                                                   "ExampleSMInstance" );

        mSceneManager->addRenderQueueListener( mOverlaySystem );
        mSceneManager->getRenderQueue()->setSortRenderQueue(
                    Ogre::v1::OverlayManager::getSingleton().mDefaultRenderQueueId,
                    Ogre::RenderQueue::StableSort );

        //Set sane defaults for proper shadow mapping
        mSceneManager->setShadowDirectionalLightExtrusionDistance( 500.0f );
        mSceneManager->setShadowFarDistance( 500.0f );
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::createCamera(void)
    {
        mCamera = mSceneManager->createCamera( "Main Camera" );

        // Position it at 500 in Z direction
        mCamera->setPosition( Ogre::Vector3( 0, 5, 15 ) );
        // Look back along -Z
        mCamera->lookAt( Ogre::Vector3( 0, 0, 0 ) );
        mCamera->setNearClipDistance( 0.2f );
        mCamera->setFarClipDistance( 1000.0f );
        mCamera->setAutoAspectRatio( true );
    }
    //-----------------------------------------------------------------------------------
    Ogre::CompositorWorkspace* OgreSDLSystem::setupCompositor(void)
    {
        Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

        const Ogre::String workspaceName( "Demo Workspace" );
        if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
        {
            compositorManager->createBasicWorkspaceDef( workspaceName, mBackgroundColour,
                                                        Ogre::IdString() );
        }

        return compositorManager->addWorkspace( mSceneManager, mRenderWindow->getTexture(), mCamera,
                                                workspaceName, true );
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::initMiscParamsListener( Ogre::NameValuePairList &params )
    {
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::stopCompositor(void)
    {
        if( mWorkspace )
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            compositorManager->removeWorkspace( mWorkspace );
            mWorkspace = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void OgreSDLSystem::restartCompositor(void)
    {
        stopCompositor();
        mWorkspace = setupCompositor();
    }
//}
