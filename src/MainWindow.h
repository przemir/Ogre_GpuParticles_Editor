/*
 * File: MainWindow.h
 * Author: Przemysław Bągard
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DoubleValueController.h>
#include <QMainWindow>
#include "QTOgreWindow.h"
#include <GpuParticles/GpuParticleAffectorCommon.h>

class ParticleEditorData;
class ParticleProject;

class ColourEditField;
class ResizableStackedWidget;
class GpuParticleAffectorWidget;

class QStackedWidget;
class QActionGroup;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    ParticleEditorData* mParticleEditorData;
    Ogre::String mResourcesCfg;
    OgreRenderer* mRenderer = nullptr;

    QMainWindow* mInteriorWindow = nullptr;
    QToolBar* mInteriorToolBar = nullptr;

    QToolBar* mToolbar = nullptr;
    QMenuBar* mMenuBar = nullptr;

    QDockWidget* mResourcesDock = nullptr;
    QStackedWidget* mResourcesStackedWidget = nullptr;
    QActionGroup* mResourcesTabActionGroup = nullptr;
    QDockWidget* mParticleSystemDock = nullptr;
    QStackedWidget* mSelectedObjectStackedWidget = nullptr;
    ResizableStackedWidget* mEmitterStackedWidget = nullptr;
    typedef std::map<Ogre::String, GpuParticleAffectorWidget*> AffectorWidgetMap;
    AffectorWidgetMap mAffectorWidgetMap;

    ParticleProject* mParticleProject = nullptr;

    ColourEditField* mBackgroundColourEditField = nullptr;
    DoubleValueController mSpeedController;

private:

    void createAffectorWidgets();
    void createAffectorWidget(GpuParticleAffectorWidget* widget);

    /// Removes characters like / or \ from string, modifies input.
    /// It is only for file name, not for full path.
    static void cleanFileNameToSave(QString& fileName);

private slots:
    void onSceneInitialized();
    void particleSystemChanged(const QString&);
    void particleDatablockChanged(const QString &datablockName);
    void emitterChanged();
    void showOnlySelectedEmittersChanged();
    void backgroundColourChanged();
    void elapsedTimeSpeedChanged();
    void newParticleSystem();
    void newParticleDatablock();
//    void loadTexture();
    void loadParticleSystem();
    void loadDatablock();
    void saveParticleSystem();
    void saveParticleDatablock();
    void resourceActionChoosen();
    void particleSystemRenameAction();
    void particleSystemCloneAction();
    void particleSystemRemoveAction();
    void particleDatablockRenameAction();
    void particleDatablockCloneAction();
    void particleDatablockRemoveAction();

    void showPlaneAction(bool);
    void setCameraFromFrontAction();
    void setCameraFromBackAction();
    void setCameraFromLeftAction();
    void setCameraFromRightAction();
    void setCameraFromTopAction();
    void setCameraFromBottomAction();

    void reloadParticleShaders();
};

#endif
