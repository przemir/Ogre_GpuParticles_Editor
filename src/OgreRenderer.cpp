/*
 * File:   OgreRenderer.cpp
 * Author: Przemysław Bągard
 *
 */

#include "OgreRenderer.h"
#include "QTOgreWindow.h"

#if OGRE_VERSION >= ((2 << 16) | (1 << 8) | 0)
#include <OgreFrameStats.h>
#include <OgreHlmsManager.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreImage2.h>
#include <OgreItem.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayManager.h>
#include <OgreQtImageHelper.h>
#endif

#include <OgreWindow.h>

#include <Editors/ParticleEditorData.h>
#include "OgreQtAppParticleEditorSystem.h"

#include <GpuParticles/Hlms/HlmsParticle.h>
#include <GpuParticles/Hlms/HlmsParticleDatablock.h>

#include <GpuParticles/GpuParticleSystemResourceManager.h>
#include <GpuParticles/GpuParticleSystem.h>

#include <GpuParticles/GpuParticleSystemWorld.h>

#include "Editors/GpuParticleEmitterWidget.h"
#include "OgreQtAppParticleEditorSystem.h"

#include "ParticleEditorAssets.h"
#include "ParticleEditorFunctions.h"

#include <Editors/GpuParticleSystemTreeWidget.h>

const Ogre::ColourValue OgreRenderer::DefaultBackgroundColour = Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f);

OgreRenderer::OgreRenderer(ParticleEditorData& _data, QObject *parent)
    : QObject(parent)
    , data(_data)
{
}

OgreRenderer::~OgreRenderer()
{
    if (mCameraMan) delete mCameraMan;
}

void OgreRenderer::initialize(QTOgreWindow *ogreWindow,
                              Ogre::SceneManager *ogreSceneMgr,
                              Ogre::Camera *ogreCamera)
{
    mOgreWindow = ogreWindow;
    mSceneMgr = ogreSceneMgr;
    mCamera = ogreCamera;
    mCamera->setDirection(0.0f, -2.0f, 5.0f);
    mCamera->setPosition(0.0f, 2.0f, -5.0f);

    mCameraMan = new OgreQtBites::SdkQtCameraMan(mCamera);
    mCameraMan->setTopSpeed(6, 30);

    data.mCamera = mCamera;
    data.mWindow = mOgreWindow->getOgreSystem()->getRenderWindow();
}

void OgreRenderer::createScene()
{
    // create your scene here :)

    auto importMeshV1 = [&](const QString& name)->Ogre::MovableObject* {
        QString filename = name + ".mesh";
        Ogre::v1::MeshPtr meshV1 = Ogre::v1::MeshManager::getSingleton().load(filename.toStdString().c_str(), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(name.toStdString().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
//        mesh->importV1( meshV1.get(), true, true, true );
        return mSceneMgr->createEntity(meshV1);
//        return mSceneMgr->createEntity(filename.toStdString().c_str(), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
//        return mSceneMgr->createItem(mesh);
    };

    auto importMeshV2 = [&](const QString& name)->Ogre::MovableObject* {
        QString filename = name + ".mesh";
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(filename.toStdString().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
        return mSceneMgr->createItem(mesh);
    };

    auto importMeshV2_ver2 = [&](const QString& name)->Ogre::MovableObject* {
        QString filename = name + ".mesh";

        Ogre::Item *item = mSceneMgr->createItem( filename.toStdString().c_str(),
                                                  Ogre::ResourceGroupManager::
                                                  AUTODETECT_RESOURCE_GROUP_NAME,
                                                  Ogre::SCENE_DYNAMIC );

        //Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(filename.toStdString().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
        return item;
    };

    {
//        Ogre::SceneNode* rootNode = mSceneMgr->getRootSceneNode( Ogre::SCENE_STATIC );

        Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane( "Plane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 1.0f ), 50.0f, 50.0f,
                                            1, 1, true, 1, 4.0f, 4.0f, Ogre::Vector3::UNIT_Z,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC );

        Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createByImportingV1(
                    "Plane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    planeMeshV1.get(), true, true, true );

        {
            mPlaneItem = mSceneMgr->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
            mPlaneNode = mSceneMgr->getRootSceneNode( Ogre::SCENE_DYNAMIC )->createChildSceneNode( Ogre::SCENE_DYNAMIC );
            mPlaneNode->setPosition( 0, -3.0f, 0 );
            mPlaneNode->attachObject( mPlaneItem );
        }
    }

    data.mParticleInstanceNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

    // global light
//    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.6f, 0.5f, 0.5f), Ogre::ColourValue(0.2f, 0.2f, 0.1f), Ogre::Vector3(0.0f, -1.0f, 0.0f));
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.2f), Ogre::ColourValue(0.2f, 0.2f, 0.1f), Ogre::Vector3(0.0f, -1.0f, 0.0f));
    // Create a Light and set its position
    Ogre::Light* light = mSceneMgr->createLight();
    Ogre::SceneNode * lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    lightNode->setPosition(20.0f, 80.0f, 50.0f);
//    lightNode->setPosition(2.0f, 8.0f, 5.0f);
    lightNode->attachObject(light);
    light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
    light->setType( Ogre::Light::LT_DIRECTIONAL );
    light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

    createDebugTextOverlay();

    setBackgroundColour(DefaultBackgroundColour);

    {
        Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();

        assert( dynamic_cast<const HlmsParticle*>(hlmsManager->getHlms(HlmsParticle::ParticleHlmsType)) );
        HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType) );
        HlmsParticleListener* hlmsParticleListener = hlmsParticle->getParticleListener();

        std::vector<GpuParticleAffector*> affectors;

        GpuParticleSystemResourceManager& gpuParticleSystemResourceManager = GpuParticleSystemResourceManager::getSingleton();
        const GpuParticleSystemResourceManager::AffectorByTypeMap& registeredAffectors = gpuParticleSystemResourceManager.getAffectorByTypeMap();
        for(GpuParticleSystemResourceManager::AffectorByTypeMap::const_iterator it = registeredAffectors.begin();
            it != registeredAffectors.end(); ++it) {

            affectors.push_back(it->second->clone());
        }

        bool useDepthTexture = true;

        // Compositor is needed only in case of useDepthTexture == true.
        Ogre::IdString depthTextureCompositorNode = "ParticleEditorNode";
        Ogre::IdString depthTextureId = "depthTextureCopy";

        data.mGpuParticleSystemWorld = OGRE_NEW GpuParticleSystemWorld(
                    Ogre::Id::generateNewId<Ogre::MovableObject>(),
                    &mSceneMgr->_getEntityMemoryManager( Ogre::SCENE_DYNAMIC ),
                    mSceneMgr, 15, hlmsParticleListener, affectors,
                    useDepthTexture, data.mOgreQtAppSystem->getCompositorWorkspace(),
                    depthTextureCompositorNode, depthTextureId);

        data.mGpuParticleSystemWorld->init(65536, 128, 1024, 64, 64);

//        // register all known particle systems
//        {
//            const GpuParticleSystemResourceManager::GpuParticleSystemMap& particleSystemCoreMap = GpuParticleSystemResourceManager::getSingleton().getGpuParticleSystems();
//            for(GpuParticleSystemResourceManager::GpuParticleSystemMap::const_iterator it = particleSystemCoreMap.begin();
//                it != particleSystemCoreMap.end(); ++it) {

//                const GpuParticleSystem* core = it->second.gpuParticleSystem;
//                data.mGpuParticleSystemWorld->registerEmitterCore(core->getEmitters());
//            }
//        }

        mParticleWorldNode = mSceneMgr->getRootSceneNode( Ogre::SCENE_DYNAMIC )->createChildSceneNode( Ogre::SCENE_DYNAMIC );
        mParticleWorldNode->attachObject(data.mGpuParticleSystemWorld);
        data.mGpuParticleSystemWorldNode = mParticleWorldNode;
    }

    emit sceneInitialized();
}

bool OgreRenderer::frameStarted(const Ogre::FrameEvent &evt)
{
//    if( mDisplayHelpMode != 0 )
    {
        //Show FPS
        Ogre::String finalText;
        generateDebugText( evt.timeSinceLastFrame, finalText );
        mDebugText->setCaption( finalText );
        mDebugTextShadow->setCaption( finalText );
    }

    if(data.mGpuParticleSystemWorld && data.mDisplayOptions.mRunParticleSystems) {
        float elapsedTime = evt.timeSinceLastFrame * data.mElapsedTimeSpeed;

        if(data.mElapsedTimeSpeed >= 0.005f) {
            data.mGpuParticleSystemWorld->processTime(elapsedTime);
        }
    }

    return true;
}

bool OgreRenderer::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    mCameraMan->frameRenderingQueued(evt);
    return true;
}

bool OgreRenderer::frameEnded(const Ogre::FrameEvent &evt)
{
    return true;
}

void OgreRenderer::createDebugTextOverlay()
{
    Ogre::v1::OverlayManager &overlayManager = Ogre::v1::OverlayManager::getSingleton();
    Ogre::v1::Overlay *overlay = overlayManager.create( "DebugText" );

    Ogre::v1::OverlayContainer *panel = static_cast<Ogre::v1::OverlayContainer*>(
        overlayManager.createOverlayElement("Panel", "DebugPanel"));
    mDebugText = static_cast<Ogre::v1::TextAreaOverlayElement*>(
                overlayManager.createOverlayElement( "TextArea", "DebugText" ) );
    mDebugText->setFontName( "DebugFont" );
    mDebugText->setCharHeight( 0.025f );
    mDebugText->setPosition( 0.002f, 0.002f );

    mDebugTextShadow= static_cast<Ogre::v1::TextAreaOverlayElement*>(
                overlayManager.createOverlayElement( "TextArea", "0DebugTextShadow" ) );
    mDebugTextShadow->setFontName( "DebugFont" );
    mDebugTextShadow->setCharHeight( 0.025f );
    mDebugTextShadow->setColour( Ogre::ColourValue::Black );
    mDebugTextShadow->setPosition( 0.004f, 0.002f );

    panel->addChild( mDebugTextShadow );
    panel->addChild( mDebugText );
    overlay->add2D( panel );
    overlay->show();

    updateDebugTextColour(Ogre::ColourValue::Black);
}

void OgreRenderer::updateDebugTextColour(const Ogre::ColourValue& backgroundColour)
{
    float backgroudGray = (backgroundColour.r + backgroundColour.g + backgroundColour.b) / 3.0f;

    Ogre::ColourValue textColour = Ogre::ColourValue::White;
    if(backgroudGray > 0.5f) {
        textColour = Ogre::ColourValue::Black;
    }

    mDebugText->setColour( textColour );
    mDebugTextShadow->setColour( backgroundColour );
}

void OgreRenderer::generateDebugText(float timeSinceLast, Ogre::String& outText)
{
//    if( mDisplayHelpMode == 0 )
//    {
//        outText = mHelpDescription;
//        outText += "\n\nPress F1 to toggle help";
//        outText += "\n\nProtip: Ctrl+F1 will reload PBS shaders (for real time template editing).\n"
//                   "Ctrl+F2 reloads Unlit shaders.\n"
//                   "Ctrl+F3 reloads Compute shaders.\n"
//                   "Note: If the modified templates produce invalid shader code, "
//                   "crashes or exceptions can happen.\n";
//        return;
//    }

    char tmp[128];
    Ogre::LwString str( Ogre::LwString::FromEmptyPointer(tmp, sizeof(tmp)) );
    Ogre::Vector3 camPos = mCamera->getPosition();

    Ogre::String finalText;
    finalText.reserve( 128 );
    finalText  = "Frame time:\t";
    finalText += Ogre::StringConverter::toString( timeSinceLast * 1000.0f );
    finalText += " ms\n";
    finalText += "Frame FPS:\t";
    finalText += Ogre::StringConverter::toString( 1.0f / timeSinceLast );
    finalText += "\n";

    GpuParticleSystemWorld::Info info = data.mGpuParticleSystemWorld->getInfo();
    finalText += "\nParticles alive: ";
    finalText += Ogre::StringConverter::toString(info.mAliveParticles).c_str();
    finalText += "\nParticles allocated: ";
    finalText += Ogre::StringConverter::toString(info.mUsedBucketsParticleCapacity).c_str();
//    finalText += "\nParticles created (alive or created ahead of time): ";
//    finalText += Ogre::StringConverter::toString(info.mParticlesCreated).c_str();
//    finalText += "\nParticles added this frame: ";
//    finalText += Ogre::StringConverter::toString(info.mParticlesAddedThisFrame).c_str();

    finalText += "\n";
    finalText += "\nCamera: ";
    str.a( "[", Ogre::LwString::Float( camPos.x, 2, 2 ), ", ",
                Ogre::LwString::Float( camPos.y, 2, 2 ), ", ",
                Ogre::LwString::Float( camPos.z, 2, 2 ), "]" );
    finalText += str.c_str();

    outText.swap( finalText );

    mDebugText->setCaption( finalText );
    mDebugTextShadow->setCaption( finalText );
}

void OgreRenderer::chooseParticleSystem()
{
    restartParticleSystem();
}

void OgreRenderer::updateEmitterCores()
{
    data.mGpuParticleSystemWorld->makeEmitterCoresDirty();
    restartParticleSystem();
}

void OgreRenderer::updateParticleDatablock()
{
    data.mWidgets.mGpuParticleEmitterWidget->updateSpriteTrackGui();
    restartParticleSystem();
}

void OgreRenderer::stopParticleSystem()
{
    data.mGpuParticleSystemWorld->stopAll();
//    data.mGpuParticleSystemWorld->stop(data.mParticleInstanceId, true);
    data.mParticleInstanceId = 0;

    if(data.mStartedGpuParticleSystem) {
        data.mGpuParticleSystemWorld->unregisterEmitterCore(data.mStartedGpuParticleSystem);
        data.mStartedGpuParticleSystem = nullptr;
    }

    data.mValidParticleEmitters.clear();
    data.mParticleEmitterTooltips.clear();
}

void OgreRenderer::restartParticleSystem()
{
    stopParticleSystem();
    data.mStartedGpuParticleSystem = data.mChoosenGpuParticleSystem;

    if(data.mChoosenGpuParticleSystem) {

        bool ok = true;
        // validity check if all particle datablocks used by emitters exists
        for (size_t i = 0; i < data.mStartedGpuParticleSystem->getEmitters().size(); ++i) {
            Ogre::String datablockName = data.mStartedGpuParticleSystem->getEmitters()[i]->mDatablockName;

            Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
            HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));
            HlmsParticleDatablock* particleDatablock = dynamic_cast<HlmsParticleDatablock*>(hlmsParticle->getDatablock(datablockName));

            if(!particleDatablock) {
                ok = false;

                static const QString missingDatablock = tr("Missing datablock!");
                data.mValidParticleEmitters.push_back(false);
                data.mParticleEmitterTooltips.push_back(missingDatablock);
            }
            else {
                Ogre::TextureGpu* texture = particleDatablock->getEmissiveTexture();
                if(!texture) {
                    ok = false;

                    static const QString noTexture = tr("No texture assigned to particle datablock!");
                    data.mValidParticleEmitters.push_back(false);
                    data.mParticleEmitterTooltips.push_back(noTexture);
                }
                else if(!data.mParticleEditorFunctions->checkIfTextureFileExists(texture)) {
                    ok = false;

                    static const QString missingTexture = tr("Missing texture inside particle datablock!");
                    data.mValidParticleEmitters.push_back(false);
                    data.mParticleEmitterTooltips.push_back(missingTexture);
                }
                else {
                    data.mValidParticleEmitters.push_back(true);
                    data.mParticleEmitterTooltips.push_back(QString());
                }
            }
        }

        // Don't start particle system if something went wrong to avoid crash.
        if(ok) {
            data.mGpuParticleSystemWorld->registerEmitterCore(data.mStartedGpuParticleSystem);

            bool showOnlySelectedEmitters = data.mWidgets.mGpuParticleSystemTreeWidget->showOnlySelectedEmitters();
            int currentEmitterIndex = data.mWidgets.mGpuParticleSystemTreeWidget->getCurrentEmitterCoreIndex();

            if(!showOnlySelectedEmitters || currentEmitterIndex < 0 || currentEmitterIndex >= (int)data.mChoosenGpuParticleSystem->getEmitters().size()) {
                data.mParticleInstanceId = data.mGpuParticleSystemWorld->start(data.mChoosenGpuParticleSystem, data.mParticleInstanceNode);
            }
            else {
                const GpuParticleEmitter* emitter = data.mChoosenGpuParticleSystem->getEmitters()[currentEmitterIndex];
                data.mParticleInstanceId = data.mGpuParticleSystemWorld->start(emitter, data.mParticleInstanceNode);
            }
        }
    }

    emit particleSystemRestarted();
}

void OgreRenderer::showPlane(bool show)
{
    mPlaneItem->setVisible(show);
}

/*
How we handle keyboard and mouse events.
*/
void OgreRenderer::keyPressEvent(QKeyEvent * e)
{
    if(mCameraMan)
        mCameraMan->injectKeyDown(*e);
}

void OgreRenderer::keyReleaseEvent(QKeyEvent * e)
{
    if(mCameraMan)
        mCameraMan->injectKeyUp(*e);
}

void OgreRenderer::mouseMoveEvent( QMouseEvent* e )
{
    static int lastX = e->x();
    static int lastY = e->y();
    int relX = e->x() - lastX;
    int relY = e->y() - lastY;
    lastX = e->x();
    lastY = e->y();

    if(mCameraMan && (e->buttons() & Qt::LeftButton))
        mCameraMan->injectMouseMove(relX, relY);
}

void OgreRenderer::mouseWheelEvent(QWheelEvent *e)
{
    if(mCameraMan)
        mCameraMan->injectWheelMove(*e);
}

void OgreRenderer::mousePressEvent( QMouseEvent* e )
{
    if(mCameraMan)
        mCameraMan->injectMouseDown(*e);
}

void OgreRenderer::mouseReleaseEvent( QMouseEvent* e )
{
    if(mCameraMan)
        mCameraMan->injectMouseUp(*e);
}

void OgreRenderer::destroyGpuParticleSystemWorld()
{
    if(data.mGpuParticleSystemWorld) {
        mParticleWorldNode->detachObject(data.mGpuParticleSystemWorld);

        delete data.mGpuParticleSystemWorld;
        data.mGpuParticleSystemWorld = nullptr;

// Don't use  getParent()->removeChild(mParticleWorldNode) and delete later as this won't remove it from SceneManager!
        mParticleWorldNode->getParentSceneNode()->removeAndDestroyChild(mParticleWorldNode);
//        delete mParticleWorldNode;
        mParticleWorldNode = nullptr;
    }
}

void OgreRenderer::setBackgroundColour(const Ogre::ColourValue& colour)
{
    Ogre::String strParamName = "colour";
    Ogre::MaterialPtr d2Material = Ogre::MaterialManager::getSingletonPtr()->getByName("Postprocess/FillColour/Background");
    if(d2Material.isNull() == false)
    {
        Ogre::Pass* pass = d2Material->getTechnique(0)->getPass(0);

        // Retrieve the shader parameters
        Ogre::GpuProgramParametersSharedPtr pParams = pass->getFragmentProgramParameters();
        if ( pParams.isNull() )
        {
           assert( false );
        }

        if ( pParams->_findNamedConstantDefinition( strParamName ) )
        {
            pParams->setNamedConstant( strParamName, colour );
        }
    }

    updateDebugTextColour(colour);
}

void OgreRenderer::setCamera(const Ogre::Vector3& pos)
{
    mCamera->setPosition(pos);
    Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;
    Ogre::Vector2 posXZ(pos.x, pos.z);
    if(posXZ.isZeroLength()) {
        lookAt.z = 0.001f;
    }
    mCamera->lookAt(lookAt);
}
