/*
 * File: OgreQtSystem.cpp
 * Author: Przemysław Bągard
 * Created: 2020-6-3
 *
 */

#include "OgreQtSystem.h"

#include <QWindow>
#include <QDir>
#include <OgreRoot.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsCompute.h>
#include <OgreHlmsManager.h>
#include "OgreSDLGame.h"

OgreQtSystem::OgreQtSystem(OgreSDLGame* ogreSdlGame, Ogre::String resourcePath, Ogre::ColourValue backgroundColour)
    : OgreSDLSystem(ogreSdlGame, resourcePath, backgroundColour)
    , mQtWindow(nullptr)
    , mHlmsShadersOutputDirectoryName("QtOgreApp")
    , mPreferredRenderer(Renderer::None)
    , mRealRenderer(Renderer::None)
    , mEnableUnlitPbsHlmsShadersDebug(false)
    , mEnableCustomHlmsShadersDebug(false)
    , mEnableComputeHlmsShadersDebug(false)
{
    mUseCustomConfigHandle = true;
}

void OgreQtSystem::setWindow(QWindow* window)
{
    mQtWindow = window;
}

void OgreQtSystem::customConfigHandle()
{
    const Ogre::RenderSystemList& rsList = mRoot->getAvailableRenderers();
    if(rsList.empty()) {
        OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, "No render system available! Can't find render system plugins!\nThis can be caused by one or more reasons below:"
                    "\n1) No render system file in directory\n2) No plugin.cfg (release) / plugin_d.cfg (debug) file\n3) No entry or wrong entry in plugin/plugin_d.cfg file.\nException",
                    "OgreQtSystem::customConfigHandle");
        return;
    }

    static const Ogre::String OpenGL3PlusName = "OpenGL 3+";
    static const Ogre::String Direct3D11Name = "Direct3D11";
    static const Ogre::String VulkanName = "Vulkan";

    Ogre::String preferredRendererStr;
    if(mPreferredRenderer == Renderer::Direct3D) {
        preferredRendererStr = Direct3D11Name;
    }
    else if(mPreferredRenderer == Renderer::OpenGL) {
        preferredRendererStr = OpenGL3PlusName;
    }
    else if(mPreferredRenderer == Renderer::Vulkan) {
        preferredRendererStr = VulkanName;
    }

    Ogre::RenderSystem* rs = rsList[0];

    if(mPreferredRenderer != Renderer::Choose) {
        for (Ogre::RenderSystemList::const_iterator it = rsList.begin(); it != rsList.end(); it++)
        {
            if ((*it)->getName().find(preferredRendererStr) != Ogre::String::npos)
            {
                rs = *it;
                break;
            }
        }
    }
    else {
        if (!mRoot->showConfigDialog())
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Abort render system configuration", "QTOgreWindow::initialize");

        rs = mRoot->getRenderSystem();
    }

    QString dimensions = QString("%1 x %2").arg(mQtWindow->width()).arg(mQtWindow->height());
    rs->setConfigOption("Video Mode", dimensions.toStdString());
    rs->setConfigOption("Full Screen", "No");
    if(mPreferredRenderer != Renderer::Choose) {
        rs->setConfigOption("VSync", "Yes");
    }

    if(rs->getName().find(OpenGL3PlusName) != Ogre::String::npos) {
        mRealRenderer = Renderer::OpenGL;
    }
    else if(rs->getName().find(Direct3D11Name) != Ogre::String::npos) {
        mRealRenderer = Renderer::Direct3D;
    }
    else if(rs->getName().find(VulkanName) != Ogre::String::npos) {
        mRealRenderer = Renderer::Vulkan;
    }

//    rs->setConfigOption("reverse_depth", "Yes");
    mRoot->setRenderSystem(rs);
}

void OgreQtSystem::initMiscParamsListener(Ogre::NameValuePairList& params)
{
    Ogre::RenderSystem* rs = mRoot->getRenderSystem();

    // Flag within the params set so that Ogre3D initializes an OpenGL context on it's own.
    if (rs->getName().find("GL") <= rs->getName().size())
        params["currentGLContext"] = Ogre::String("false");

    // We need to supply the low level OS window handle to this QWindow so that Ogre3D knows where to draw the scene. Below is a cross-platform method on how to do this.
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)(mQtWindow->winId()));
    params["parentWindowHandle"] = Ogre::StringConverter::toString((size_t)(mQtWindow->winId()));
#else
    params["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(mQtWindow->winId()));
    params["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(mQtWindow->winId()));
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX && QT_VERSION_MAJOR >= 6
    // Without this there will be standalone Ogre window instead of embedded widget for Vulkan on linux.
    // It is not necessary for openGL/Direct3D. Vulkan on windows also works without those lines below.
    QNativeInterface::QX11Application* x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if(x11App) {
        x11.display = x11App->display();
        x11.window = (unsigned long)(mQtWindow->winId());
        params["SDL2x11"] = Ogre::StringConverter::toString( (uintptr_t)&x11 );
    }
    // QNativeInterface::QWaylandApplication* waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
#endif

#if defined(Q_OS_MAC)
    params["macAPI"] = "cocoa";
    params["macAPICocoaUseNSView"] = "true";
#endif
}

void OgreQtSystem::registerHlms()
{
    OgreSDLSystem::registerHlms();
    setHlmsShadersOutputFolder();
}

void OgreQtSystem::setHlmsShadersOutputFolder()
{
    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsCompute* hlmsCompute = hlmsManager->getComputeHlms();
    Ogre::String ogrePath = !mHlmsShadersOutputDirectoryName.empty() ? "./hlmsShadersDebug/" + mHlmsShadersOutputDirectoryName : "./hlmsShadersDebug";
    QString path = QString::fromStdString(ogrePath);

    if(mEnableUnlitPbsHlmsShadersDebug || mEnableCustomHlmsShadersDebug || mEnableComputeHlmsShadersDebug) {
        QDir().mkdir( "./hlmsShadersDebug/" );
        if(!mHlmsShadersOutputDirectoryName.empty()) {
            QDir().mkdir( path );
        }
    }

    if(mEnableComputeHlmsShadersDebug) {
        QDir().mkdir( path + "/compute/" );
        hlmsCompute->setDebugOutputPath( mEnableComputeHlmsShadersDebug, mEnableComputeHlmsShadersDebug, ogrePath + "/compute/" );
    }
    else {
        hlmsCompute->setDebugOutputPath( false, false );
    }

    for (int i = ((int)Ogre::HLMS_LOW_LEVEL)+1; i < Ogre::HLMS_MAX; ++i) {
        Ogre::Hlms* hlms = hlmsManager->getHlms(Ogre::HlmsTypes(i));
        if(!hlms) {
            continue;
        }
        bool enable = i == Ogre::HLMS_UNLIT || i == Ogre::HLMS_PBS
                ? mEnableUnlitPbsHlmsShadersDebug
                : mEnableCustomHlmsShadersDebug;

        if(enable) {
            Ogre::String typeNameStr = hlms->getTypeNameStr();
            QString typeNameQStr = QString::fromStdString(typeNameStr);
            QDir().mkdir( path + "/" + typeNameQStr + "/" );
            hlms->setDebugOutputPath( enable, enable, ogrePath + "/" + typeNameStr + "/" );
        }
        else {
            hlms->setDebugOutputPath( false, false );
        }
    }
}

OgreQtSystem::Renderer OgreQtSystem::getRealRenderer() const
{
    return mRealRenderer;
}
