/*
 * File: ParticleEditorData.h
 * Author: Przemysław Bągard
 * Created: 2020-10-11
 *
 */

#ifndef PARTICLEEDITORDATA_H
#define PARTICLEEDITORDATA_H

#include <OgrePrerequisites.h>
#include <QMap>
#include <QPair>
#include <QString>
#include <QVector>

class QStatusBar;
class QToolBar;
class QWidget;
class QMainWindow;
class QLineEdit;
class QToolButton;

class GpuParticleSystemWorld;
class OgreQtAppParticleEditorSystem;
class GpuParticleSystemsListWidget;
class GpuParticleSystemTreeWidget;
class GpuParticleEmitterWidget;
class GpuParticleSystem;
class ParticleEditorAssets;
class OgreRenderer;
class GpuParticleEmitter;
class MainWindow;
class HlmsParticleDatablock;
class GpuParticleDatablocksListWidget;
class GpuParticleDatablockWidget;
class ParticleEditorFunctions;

class ParticleEditorData
{
public:

    class WidgetData {
    public:

        MainWindow* mMainWindow = nullptr;
        QMainWindow* getQMainWindow();

        GpuParticleSystemsListWidget* mGpuParticleSystemsListWidget = nullptr;
        GpuParticleDatablocksListWidget* mGpuParticleDatablocksListWidget = nullptr;
        GpuParticleSystemTreeWidget* mGpuParticleSystemTreeWidget = nullptr;
        QLineEdit* mGpuParticleSystemNameEdit = nullptr;
        QToolButton* mGpuParticleSystemActionButton = nullptr;
        GpuParticleEmitterWidget* mGpuParticleEmitterWidget = nullptr;
        GpuParticleDatablockWidget* mGpuParticleDatablockWidget = nullptr;
        QLineEdit* mGpuParticleDatablockNameEdit = nullptr;
        QToolButton* mGpuParticleDatablockActionButton = nullptr;

        QStatusBar* mStatusBar = nullptr;
        QWidget* mOgreWidget = nullptr;
        QMainWindow* mOgreMainWindow = nullptr;
        QToolBar* mOgreMainWindowToolBar = nullptr;

    } mWidgets;

    class DisplayOptions
    {
    public:
        bool mRunParticleSystems = true;
    } mDisplayOptions;

public:
    ParticleEditorData();
    ~ParticleEditorData();

    QString mProjectPath;
    QString mProjectFolderPath;

    /// DatablockName, TextureName pair
    QVector<QPair<QString, QString> > mDatablockList;
    /// Needed to check if file was already read.
    QMap<QString, QString> mCustomTextures;

    GpuParticleSystemWorld* mGpuParticleSystemWorld = nullptr;
    GpuParticleSystem* mChoosenGpuParticleSystem = nullptr;
    GpuParticleSystem* mStartedGpuParticleSystem = nullptr;
    GpuParticleEmitter* mGpuParticleEmitterClipboard; // owned, created and destroyed by ParticleEditorData.
    HlmsParticleDatablock* mChoosenGpuParticleDatablock = nullptr;
    Ogre::uint64 mParticleInstanceId = 0;
    Ogre::Node* mParticleInstanceNode = nullptr;
    Ogre::Node* mGpuParticleSystemWorldNode = nullptr;

    OgreRenderer* mRenderer = nullptr;
    Ogre::Window* mWindow = nullptr;
    Ogre::SceneManager* mSceneManager = nullptr;
    Ogre::Camera* mCamera = nullptr;
    OgreQtAppParticleEditorSystem* mOgreQtAppSystem = nullptr;

    ParticleEditorAssets* mParticleEditorAssets = nullptr;
    ParticleEditorFunctions* mParticleEditorFunctions = nullptr;

    float mElapsedTimeSpeed = 1.0f;

    std::vector<bool> mValidParticleEmitters;
    QStringList mParticleEmitterTooltips;

public:
    QString currentParticleSystemName() const;
    QString currentParticleDatablockName() const;
    void copyFromGpuParticleEmitter(const GpuParticleEmitter* emitter);
    void pasteToGpuParticleEmitter(GpuParticleEmitter* emitter) const;
    Ogre::Root* getRoot() const;
    Ogre::RenderSystem* getRenderSystem() const;
};

#endif
