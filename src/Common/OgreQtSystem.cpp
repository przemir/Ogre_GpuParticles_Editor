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
    Ogre::RenderSystem* rs = rsList[0];
    std::vector<Ogre::String> renderOrder;
    if(mPreferredRenderer == Renderer::None) {
        renderOrder.push_back("OpenGL 3+");
        renderOrder.push_back("Direct3D11");
    }
    else if(mPreferredRenderer == Renderer::Direct3D) {
        renderOrder.push_back("Direct3D11");
        renderOrder.push_back("OpenGL 3+");
    }
    else if(mPreferredRenderer == Renderer::OpenGL) {
        renderOrder.push_back("OpenGL 3+");
        renderOrder.push_back("Direct3D11");
    }
    else if(mPreferredRenderer == Renderer::Vulkan) {
        renderOrder.push_back("Vulkan");
        renderOrder.push_back("OpenGL 3+");
        renderOrder.push_back("Direct3D11");
    }

    for (std::vector<Ogre::String>::iterator iter = renderOrder.begin(); iter != renderOrder.end(); iter++)
    {
        for (Ogre::RenderSystemList::const_iterator it = rsList.begin(); it != rsList.end(); it++)
        {
            if ((*it)->getName().find(*iter) != Ogre::String::npos)
            {
                rs = *it;
                break;
            }
        }
        if (rs != NULL) break;
    }
    if (rs == NULL)
    {
        if (!mRoot->restoreConfig())
        {
            if (!mRoot->showConfigDialog())
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Abort render system configuration", "QTOgreWindow::initialize");
        }
    }
    QString dimensions = QString("%1 x %2").arg(mQtWindow->width()).arg(mQtWindow->height());
    rs->setConfigOption("Video Mode", dimensions.toStdString());
    rs->setConfigOption("Full Screen", "No");
    rs->setConfigOption("VSync", "Yes");

//    rs->setConfigOption("reverse_depth", "Yes");
    mRoot->setRenderSystem(rs);
}

void OgreQtSystem::initMiscParamsListener(Ogre::NameValuePairList& params)
{
    Ogre::RenderSystem* rs = mRoot->getRenderSystem();

    /*
    Flag within the params set so that Ogre3D initializes an OpenGL context on it's own.
    */
    if (rs->getName().find("GL") <= rs->getName().size())
        params["currentGLContext"] = Ogre::String("false");

    // We need to supply the low level OS window handle to this QWindow so that Ogre3D knows where to draw the scene. Below is a cross-platform method on how to do this.
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)(mQtWindow->winId()));
    params["parentWindowHandle"] = Ogre::StringConverter::toString((size_t)(mQtWindow->winId()));
#else
    params["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(mWindow->winId()));
    params["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(mWindow->winId()));
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
    QString path = "./hlmsShadersDebug";
    Ogre::String ogrePath = "./hlmsShadersDebug";
#ifdef SHOW_OUTPUT_SHADERS
    //Enable shader dump, folder must be created first
    QDir().mkdir( "./hlmsShadersDebug/" );
    QDir().mkdir( path );
    QDir().mkdir( path + "/compute/" );
    hlmsCompute->setDebugOutputPath( true, true, ogrePath + "/compute/" );
#else
    hlmsCompute->setDebugOutputPath( false, false );
#endif

    for (int i = ((int)Ogre::HLMS_LOW_LEVEL)+1; i < Ogre::HLMS_MAX; ++i) {
        Ogre::Hlms* hlms = hlmsManager->getHlms(Ogre::HlmsTypes(i));
        if(!hlms) {
            continue;
        }
        bool enable = i >= Ogre::HLMS_USER0 && i <= Ogre::HLMS_USER3;
#ifdef SHOW_OUTPUT_SHADERS
        Ogre::String typeNameStr = hlms->getTypeNameStr();
        QString typeNameQStr = QString::fromStdString(typeNameStr);
        QDir().mkdir( path + "/" + typeNameQStr + "/" );
        hlms->setDebugOutputPath( enable, enable, ogrePath + "/" + typeNameStr + "/" );
#else
        hlms->setDebugOutputPath( false, false );
#endif
    }
}
