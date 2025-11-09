/*
 * File: OgreQtAppParticleEditorSystem.cpp
 * Author: Przemysław Bągard
 *
 */

#include "OgreQtAppParticleEditorSystem.h"

#include <QWindow>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreCompositorManager2.h>
#include <OgreConfigFile.h>
#include <OgreWindow.h>
#include <Common/OgreSDLGame.h>

#include <GpuParticles/GpuParticleSystemJsonManager.h>
#include <GpuParticles/GpuParticleSystemResourceManager.h>

#include <GpuParticles/Hlms/HlmsParticle.h>

OgreQtAppParticleEditorSystem::OgreQtAppParticleEditorSystem(OgreSDLGame* ogreSdlGame, const Ogre::String& resourcesCfg)
    : OgreQtSystem(ogreSdlGame, "../", Ogre::ColourValue(0.0f, 0.5f, 1.0f))
{
#ifdef OGRE_GPUPARTICLE_EDITOR_DEBUG
    mPluginsCfg = "plugins_d.cfg";
#else
    mPluginsCfg = "plugins.cfg";
#endif

    mResourcesCfg = resourcesCfg;

    mProjectPrefix = "cache/OgreGpuParticleEditor_";

    mHlmsShadersOutputDirectoryName = "OgreGpuParticleEditor";

    // mPreferredRenderer = Renderer::Choose;
    mPreferredRenderer = Renderer::Direct3D;
//    mPreferredRenderer = Renderer::OpenGL;
//    mPreferredRenderer = Renderer::Vulkan;
}

void OgreQtAppParticleEditorSystem::initialize(const Ogre::String& windowTitle)
{
    OgreQtSystem::initialize(windowTitle);
}

void OgreQtAppParticleEditorSystem::deinitialize()
{
    OgreQtSystem::deinitialize();
}

void OgreQtAppParticleEditorSystem::setupResources()
{
    GpuParticleSystemResourceManager* gpuParticleSystemResourceManager = new GpuParticleSystemResourceManager();
    gpuParticleSystemResourceManager->registerCommonAffectors();
    new GpuParticleSystemJsonManager();

    OgreSDLSystem::setupResources();
}

void OgreQtAppParticleEditorSystem::createCamera()
{
    OgreQtSystem::createCamera();

    mCamera->setFOVy(Ogre::Degree(60.0));
    mCamera->setAspectRatio(Ogre::Real(mRenderWindow->getWidth()) / Ogre::Real(mRenderWindow->getHeight()));
    mCamera->setAutoAspectRatio(true);
}

Ogre::CompositorWorkspace* OgreQtAppParticleEditorSystem::setupCompositor()
{
//    return OgreQtSystem::setupCompositor();

    Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

    const Ogre::String workspaceName( "ParticleEditorWorkspace" );
    if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
    {
        compositorManager->createBasicWorkspaceDef( workspaceName, mBackgroundColour,
                                                    Ogre::IdString() );
    }

    return compositorManager->addWorkspace( mSceneManager, mRenderWindow->getTexture(), mCamera,
                                            workspaceName, true );
}

void OgreQtAppParticleEditorSystem::registerHlms()
{
    OgreSDLSystem::registerHlms();

    {
        Ogre::ConfigFile cf;
        cf.load( mResourcePath + mResourcesCfg );

        auto getPath = [&](const Ogre::String& key, const Ogre::String& section)->Ogre::String
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            Ogre::String rootHlmsFolder = Ogre::macBundlePath() + '/' +
                                  cf.getSetting( key, section, "" );
#else
            Ogre::String rootHlmsFolder = mResourcePath + cf.getSetting( key, section, "" );
#endif

            if( rootHlmsFolder.empty() )
                rootHlmsFolder = "./";
            else if( *(rootHlmsFolder.end() - 1) != '/' )
                rootHlmsFolder += "/";

            return rootHlmsFolder;
        };

        Ogre::String rootHlmsFolder = getPath("DoNotUseAsResource", "Hlms");
        Ogre::String rootHlms2Folder = getPath("DoNotUseAsResourceParticle", "Hlms");

        HlmsParticle::registerHlms(rootHlmsFolder, rootHlms2Folder, true);
    }

    setHlmsShadersOutputFolder();
}
