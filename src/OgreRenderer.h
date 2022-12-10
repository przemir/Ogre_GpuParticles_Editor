/*
 * File:   OgreRenderer.h
 * Author: Przemysław Bągard
 *
 */

#ifndef OGRERENDERER_H
#define OGRERENDERER_H

#include <Ogre.h>
#include <OgreFrameListener.h>
#include <OgreTextAreaOverlayElement.h>

// Changed SdkCameraMan implementation to work with QKeyEvent, QMouseEvent, QWheelEvent
#include <Common/SdkQtCameraMan.h>

class QTOgreWindow;
class ParticleEditorData;
class HlmsParticleDatablock;

class OgreRenderer : public QObject, public Ogre::FrameListener
{
    Q_OBJECT
public:
    OgreRenderer(ParticleEditorData& _data, QObject* parent = 0);
    ~OgreRenderer();

    void initialize(QTOgreWindow* ogreWindow,
                    Ogre::SceneManager* ogreSceneMgr,
                    Ogre::Camera* ogreCamera);
    void createScene();
    void destroyGpuParticleSystemWorld();

    void setBackgroundColour(const Ogre::ColourValue& colour);

    void setCamera(const Ogre::Vector3& pos);

    static const Ogre::ColourValue DefaultBackgroundColour;

protected:

    void keyPressEvent(QKeyEvent * e);
    void keyReleaseEvent(QKeyEvent * e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseWheelEvent(QWheelEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

    virtual bool frameStarted(const Ogre::FrameEvent &evt);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool frameEnded(const Ogre::FrameEvent &evt);

    friend class QTOgreWindow;
private:
    QTOgreWindow* mOgreWindow = nullptr;
    Ogre::SceneManager* mSceneMgr = nullptr;
    Ogre::Camera* mCamera = nullptr;
    OgreQtBites::SdkQtCameraMan* mCameraMan = nullptr;

    Ogre::SceneNode* mPlaneNode = nullptr;
    Ogre::SceneNode* mParticleWorldNode = nullptr;
    Ogre::Item* mPlaneItem = nullptr;

    Ogre::v1::TextAreaOverlayElement *mDebugText = nullptr;
    Ogre::v1::TextAreaOverlayElement *mDebugTextShadow = nullptr;

private:

    void createDebugTextOverlay();
    void updateDebugTextColour(const Ogre::ColourValue& backgroundColour);
    void generateDebugText( float timeSinceLast, Ogre::String &outText );

public slots:
    void chooseParticleSystem();
    void updateEmitterCores();
    void updateParticleDatablock();
    void stopParticleSystem();
    void restartParticleSystem();
    void showPlane(bool);

signals:
    void sceneInitialized();
    void particleSystemRestarted();

private:
    ParticleEditorData& data;
};

#endif
