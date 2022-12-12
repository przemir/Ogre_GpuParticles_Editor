/*
 * File: MainWindow.cpp
 * Author: Przemysław Bągard
 *
 */

#include "MainWindow.h"

#include <OgreHlms.h>
#include <OgreHlmsJson.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsUnlitDatablock.h>
#include <OgreFileSystem.h>

#include <QBoxLayout>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QStackedWidget>
#include <QSplitter>
#include <QLabel>
#include <QToolButton>
#include <QInputDialog>
#include <OgreImage2.h>
#include <OgreTextureGpuManager.h>
#include <QTextStream>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QDockWidget>

#include "OgreQtAppParticleEditorSystem.h"
#include "OgreRenderer.h"
#include "ParticleEditorAssets.h"
#include "ParticleEditorFunctions.h"
#include <Common/OgreSDLGame.h>
#include <GpuParticles/GpuParticleSystemResourceManager.h>
#include <GpuParticles/GpuParticleSystem.h>
#include <GpuParticles/GpuParticleSystemJsonManager.h>
#include <Editors/ParticleEditorData.h>
#include <Editors/GpuParticleEmitterWidget.h>
#include <Editors/GpuParticleSystemsListWidget.h>
#include <Editors/GpuParticleSystemTreeWidget.h>
#include <Editors/GpuParticleDatablocksListWidget.h>
#include <Editors/GpuParticleDatablockWidget.h>
#include <GpuParticles/Hlms/HlmsParticle.h>
#include <GpuParticles/Hlms/HlmsParticleDatablock.h>
#include <Widgets/ColourEditField.h>

namespace {
    class ScrollAreaWithSizeHint: public QScrollArea {
    public:
        virtual QSize sizeHint() const {
            QSize widgetSize = widget()->sizeHint();
            return QSize(widgetSize.width()+24,widgetSize.height()+24);
        }
    };
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mParticleEditorData(new ParticleEditorData())
{

    mResourcesCfg = "resources.cfg";

//    QFile file("darkStylesheet.css");
//    file.open(QFile::ReadOnly);
//    QString styleSheet = QLatin1String(file.readAll());

//    qApp->setStyleSheet(styleSheet);

    //    setCentralWidget(ogreWidget);

    {
        mMenuBar = new QMenuBar(this);

        {
            QMenu* fileMenu = new QMenu(tr("File"));

            QAction* newParticleSystemAction = new QAction(tr("New particle system"));
            connect(newParticleSystemAction, SIGNAL(triggered(bool)), this, SLOT(newParticleSystem()));
            fileMenu->addAction(newParticleSystemAction);

            QAction* newParticleDatablockAction = new QAction(tr("New particle datablock"));
            connect(newParticleDatablockAction, SIGNAL(triggered(bool)), this, SLOT(newParticleDatablock()));
            fileMenu->addAction(newParticleDatablockAction);

            fileMenu->addSeparator();

            QAction* loadParticleSystemAction = new QAction(tr("Load particle system"));
            connect(loadParticleSystemAction, SIGNAL(triggered(bool)), this, SLOT(loadParticleSystem()));
            fileMenu->addAction(loadParticleSystemAction);

            QAction* loadDatablockAction = new QAction(tr("Load particle/unlit datablock"));
            loadDatablockAction->setToolTip(tr("Loads datablocks. Particle type is preferred, unlit will be converted to particle."));
            connect(loadDatablockAction, SIGNAL(triggered(bool)), this, SLOT(loadDatablock()));
            fileMenu->addAction(loadDatablockAction);

//            QAction* loadTextureAction = new QAction(tr("Load texture"));
//            connect(loadTextureAction, SIGNAL(triggered(bool)), this, SLOT(loadTexture()));
//            fileMenu->addAction(loadTextureAction);

            fileMenu->addSeparator();

            QAction* saveParticleSystemAction = new QAction(tr("Save particle system"));
            connect(saveParticleSystemAction, SIGNAL(triggered(bool)), this, SLOT(saveParticleSystem()));
            fileMenu->addAction(saveParticleSystemAction);

            QAction* saveParticleDatablockAction = new QAction(tr("Save particle datablock"));
            connect(saveParticleDatablockAction, SIGNAL(triggered(bool)), this, SLOT(saveParticleDatablock()));
            fileMenu->addAction(saveParticleDatablockAction);

            fileMenu->addSeparator();

            QAction* exitAction = new QAction(tr("Exit"));
            connect(exitAction, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
            fileMenu->addAction(exitAction);

            mMenuBar->addMenu(fileMenu);
        }

        {
            QMenu* viewMenu = new QMenu(tr("View"));

            QAction* showPlaneAction = new QAction(tr("Show/hide plane"));
            showPlaneAction->setCheckable(true);
            showPlaneAction->setChecked(true);
            connect(showPlaneAction, SIGNAL(toggled(bool)), this, SLOT(showPlaneAction(bool)));
            viewMenu->addAction(showPlaneAction);

            viewMenu->addSeparator();

            QAction* setCameraFrontAction = new QAction(tr("Set camera: front"));
            connect(setCameraFrontAction, SIGNAL(triggered(bool)), this, SLOT(setCameraFromFrontAction()));
            viewMenu->addAction(setCameraFrontAction);

            QAction* setCameraBackAction = new QAction(tr("Set camera: back"));
            connect(setCameraBackAction, SIGNAL(triggered(bool)), this, SLOT(setCameraFromBackAction()));
            viewMenu->addAction(setCameraBackAction);

            QAction* setCameraLeftAction = new QAction(tr("Set camera: left"));
            connect(setCameraLeftAction, SIGNAL(triggered(bool)), this, SLOT(setCameraFromLeftAction()));
            viewMenu->addAction(setCameraLeftAction);

            QAction* setCameraRightAction = new QAction(tr("Set camera: right"));
            connect(setCameraRightAction, SIGNAL(triggered(bool)), this, SLOT(setCameraFromRightAction()));
            viewMenu->addAction(setCameraRightAction);

            QAction* setCameraTopAction = new QAction(tr("Set camera: top"));
            connect(setCameraTopAction, SIGNAL(triggered(bool)), this, SLOT(setCameraFromTopAction()));
            viewMenu->addAction(setCameraTopAction);

            QAction* setCameraBottomAction = new QAction(tr("Set camera: bottom"));
            connect(setCameraBottomAction, SIGNAL(triggered(bool)), this, SLOT(setCameraFromBottomAction()));
            viewMenu->addAction(setCameraBottomAction);

            mMenuBar->addMenu(viewMenu);
        }

        setMenuBar(mMenuBar);
    }


    mToolbar = new QToolBar(this);
    addToolBar(mToolbar);


    mInteriorWindow = new QMainWindow(0);
    mInteriorToolBar = new QToolBar(mInteriorWindow);
//    QComboBox* modeCombo = new QComboBox();
//    modeCombo->setMinimumWidth(200);
//    mInteriorToolBar->addWidget(modeCombo);
    mInteriorWindow->addToolBar(Qt::BottomToolBarArea, mInteriorToolBar);
    setCentralWidget(mInteriorWindow);

    mRenderer = new OgreRenderer(*mParticleEditorData);
    mParticleEditorData->mRenderer = mRenderer;
    OgreSDLGame* dummyGame = new OgreSDLGame();
    OgreQtAppParticleEditorSystem* ogreQtSystem = new OgreQtAppParticleEditorSystem(dummyGame, mResourcesCfg);
    QTOgreWindow* ogreWindow = new QTOgreWindow(ogreQtSystem, mRenderer);
//    mSkeletonData->mOgreWindow.setWindow(ogreWindow);
    QWidget* ogreWidget = QWidget::createWindowContainer(ogreWindow);
    mInteriorWindow->setCentralWidget(ogreWidget);

    mParticleEditorData->mWidgets.mOgreWidget = ogreWidget;
//    mLevelEditorData->mWidgets.mOgreMainWindow->setCentralWidget(ogreWidget);
    mParticleEditorData->mWidgets.mOgreMainWindow = mInteriorWindow;
    mParticleEditorData->mWidgets.mMainWindow = this;
    mParticleEditorData->mOgreQtAppSystem = ogreQtSystem;
    mParticleEditorData->mParticleEditorAssets = new ParticleEditorAssets();
    mParticleEditorData->mParticleEditorFunctions = new ParticleEditorFunctions(*mParticleEditorData);

    {
        mResourcesDock = new QDockWidget();
        mResourcesDock->setWindowTitle(tr("Resources"));

        QWidget* resourcesWidget = new QWidget();
        QVBoxLayout* resourcesLayout = new QVBoxLayout();
        resourcesLayout->setContentsMargins(0, 0, 0, 0);

        QToolBar* toolbar = new QToolBar();
        QAction* particleSystemAction = toolbar->addAction("ParticleSystem");
        particleSystemAction->setCheckable(true);
        particleSystemAction->setChecked(true);
        QAction* particleDatablockAction = toolbar->addAction("ParticleDatablock");
        particleDatablockAction->setCheckable(true);

        mResourcesTabActionGroup = new QActionGroup(this);
        mResourcesTabActionGroup->addAction(particleSystemAction);
        mResourcesTabActionGroup->addAction(particleDatablockAction);
        connect(mResourcesTabActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(resourceActionChoosen()));

        resourcesLayout->addWidget(toolbar);

        mResourcesStackedWidget = new QStackedWidget();

        {
            mParticleEditorData->mWidgets.mGpuParticleSystemsListWidget = new GpuParticleSystemsListWidget();
            connect(mParticleEditorData->mWidgets.mGpuParticleSystemsListWidget, SIGNAL(itemChanged(const QString&)),
                    this, SLOT(particleSystemChanged(const QString&)));
            mResourcesStackedWidget->addWidget(mParticleEditorData->mWidgets.mGpuParticleSystemsListWidget);
        }

        {
            mParticleEditorData->mWidgets.mGpuParticleDatablocksListWidget = new GpuParticleDatablocksListWidget();
            connect(mParticleEditorData->mWidgets.mGpuParticleDatablocksListWidget, SIGNAL(itemChanged(const QString&)),
                    this, SLOT(particleDatablockChanged(const QString&)));
            mResourcesStackedWidget->addWidget(mParticleEditorData->mWidgets.mGpuParticleDatablocksListWidget);
        }

        resourcesLayout->addWidget(mResourcesStackedWidget);
        resourcesWidget->setLayout(resourcesLayout);
        mResourcesDock->setWidget(resourcesWidget);

        addDockWidget(Qt::LeftDockWidgetArea, mResourcesDock);
    }

    {
        mParticleSystemDock = new QDockWidget();
        mParticleSystemDock->setWindowTitle(tr("Particle system"));

        mSelectedObjectStackedWidget = new QStackedWidget();

        {
            QSplitter* splitter = new QSplitter(Qt::Vertical);

            {
                QWidget* widget = new QWidget();
                QVBoxLayout* layout = new QVBoxLayout();

                {
                    QHBoxLayout* lineLayout = new QHBoxLayout();

                    QLineEdit* nameLabel = new QLineEdit();
                    mParticleEditorData->mWidgets.mGpuParticleSystemNameEdit = nameLabel;
                    nameLabel->setStyleSheet("font: bold 26px;");
                    nameLabel->setReadOnly(true);
                    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                    lineLayout->addWidget(nameLabel);

//                    lineLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));

                    QToolButton* actionButton = new QToolButton();
                    mParticleEditorData->mWidgets.mGpuParticleSystemActionButton = actionButton;
                    actionButton->setText(QString());
//                    actionButton->setArrowType(Qt::DownArrow);
                    actionButton->setContextMenuPolicy(Qt::ActionsContextMenu);
                    actionButton->setPopupMode(QToolButton::InstantPopup);

                    QAction* renameNameAction = new QAction(tr("Rename"));
                    connect(renameNameAction, SIGNAL(triggered(bool)), this, SLOT(particleSystemRenameAction()));
                    actionButton->addAction(renameNameAction);

                    QAction* cloneAction = new QAction(tr("Clone"));
                    connect(cloneAction, SIGNAL(triggered(bool)), this, SLOT(particleSystemCloneAction()));
                    actionButton->addAction(cloneAction);

                    QAction* removeAction = new QAction(tr("Remove"));
                    connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(particleSystemRemoveAction()));
                    actionButton->addAction(removeAction);

                    lineLayout->addWidget(actionButton);

                    layout->addLayout(lineLayout);
                }
                {
                    QFrame* line = new QFrame();
                    line->setFrameShape(QFrame::HLine);
                    line->setFrameShadow(QFrame::Sunken);
                    layout->addWidget(line);
                }
                {
                    mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget = new GpuParticleSystemTreeWidget(*mParticleEditorData);
                    layout->addWidget(mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget);
                    connect(mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget, SIGNAL(emitterChanged()),
                            this, SLOT(emitterChanged()));
                    connect(mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget, SIGNAL(showOnlySelectedEmittersChanged()),
                            this, SLOT(showOnlySelectedEmittersChanged()));
                    connect(mRenderer, SIGNAL(particleSystemRestarted()),
                            mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget, SLOT(updateEmitterInvalidIcons()));

                }

                widget->setLayout(layout);
                splitter->addWidget(widget);
                splitter->setStretchFactor(0, 1);
            }

            {
                QScrollArea* scrollArea = new ScrollAreaWithSizeHint();
                scrollArea->setWidgetResizable(true);
                mParticleEditorData->mWidgets.mGpuParticleEmitterWidget = new GpuParticleEmitterWidget(*mParticleEditorData);
                connect(mParticleEditorData->mWidgets.mGpuParticleEmitterWidget, SIGNAL(emitterCoreModified()), mRenderer, SLOT(updateEmitterCores()));
                scrollArea->setWidget(mParticleEditorData->mWidgets.mGpuParticleEmitterWidget);
                splitter->addWidget(scrollArea);
                splitter->setStretchFactor(1, 3);
            }

            mSelectedObjectStackedWidget->addWidget(splitter);
        }

        {
            QWidget* widget = new QWidget();
            QVBoxLayout* layout = new QVBoxLayout();

            {
                QHBoxLayout* lineLayout = new QHBoxLayout();

                QLineEdit* nameLabel = new QLineEdit();
                mParticleEditorData->mWidgets.mGpuParticleDatablockNameEdit = nameLabel;
                nameLabel->setStyleSheet("font: bold 26px;");
                nameLabel->setReadOnly(true);
                nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                lineLayout->addWidget(nameLabel);


//                lineLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));

                QToolButton* actionButton = new QToolButton();
                mParticleEditorData->mWidgets.mGpuParticleDatablockActionButton = actionButton;
                actionButton->setText(QString());
//                actionButton->setArrowType(Qt::DownArrow);
                actionButton->setContextMenuPolicy(Qt::ActionsContextMenu);
                actionButton->setPopupMode(QToolButton::InstantPopup);

                QAction* renameNameAction = new QAction(tr("Rename"));
                connect(renameNameAction, SIGNAL(triggered(bool)), this, SLOT(particleDatablockRenameAction()));
                actionButton->addAction(renameNameAction);

                QAction* cloneAction = new QAction(tr("Clone"));
                connect(cloneAction, SIGNAL(triggered(bool)), this, SLOT(particleDatablockCloneAction()));
                actionButton->addAction(cloneAction);

                QAction* removeAction = new QAction(tr("Remove"));
                connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(particleDatablockRemoveAction()));
                actionButton->addAction(removeAction);

                lineLayout->addWidget(actionButton);

                layout->addLayout(lineLayout);
            }

            {
                QFrame* line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                layout->addWidget(line);
            }

            {
                QScrollArea* scrollArea = new ScrollAreaWithSizeHint();
                scrollArea->setWidgetResizable(true);
                mParticleEditorData->mWidgets.mGpuParticleDatablockWidget = new GpuParticleDatablockWidget(*mParticleEditorData);
                connect(mParticleEditorData->mWidgets.mGpuParticleDatablockWidget, SIGNAL(particleDatablockModified()), mRenderer, SLOT(updateParticleDatablock()));
                scrollArea->setWidget(mParticleEditorData->mWidgets.mGpuParticleDatablockWidget);
                layout->addWidget(scrollArea);
            }

            widget->setLayout(layout);
            mSelectedObjectStackedWidget->addWidget(widget);
        }


        mParticleSystemDock->setWidget(mSelectedObjectStackedWidget);

        addDockWidget(Qt::RightDockWidgetArea, mParticleSystemDock);
    }

    setFocusPolicy(Qt::WheelFocus);
    setFocusProxy(ogreWidget);

    {
        {
            mBackgroundColourEditField = new ColourEditField();
            mBackgroundColourEditField->setColour(OgreRenderer::DefaultBackgroundColour);
            mToolbar->addWidget(mBackgroundColourEditField);
            connect(mBackgroundColourEditField, SIGNAL(colorChanged()), this, SLOT(backgroundColourChanged()));
        }
        {
            QWidget* widget = new QWidget();
            QDoubleSpinBox* tSpin = new QDoubleSpinBox();
            QSlider* tSlider = new QSlider(Qt::Horizontal);
            tSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            mSpeedController.set(tSpin, tSlider);
            mSpeedController.setValueRange(0.0f, 1.0f);
            mSpeedController.setValue(1.0f);
            mSpeedController.setSingleStep(0.01);

            QHBoxLayout* layout = new QHBoxLayout();
            layout->addWidget(tSpin);
            layout->addWidget(tSlider);
            layout->setContentsMargins(0, 0, 0, 0);
            widget->setLayout(layout);

            mToolbar->addWidget(widget);
            connect(&mSpeedController, SIGNAL(valueChanged(double)), this, SLOT(elapsedTimeSpeedChanged()));
        }
    }

    connect(mRenderer, SIGNAL(sceneInitialized()), this, SLOT(onSceneInitialized()));
}

MainWindow::~MainWindow()
{
    mRenderer->destroyGpuParticleSystemWorld();
}

void MainWindow::cleanFileNameToSave(QString& fileName)
{
    QString pattern = "[" + QRegExp::escape("/\\.,") + "]";
    fileName = fileName.remove(QRegExp(pattern));
}

void MainWindow::onSceneInitialized()
{
    mParticleEditorData->mParticleEditorFunctions->updateParticleSystemsWidgets();
    mParticleEditorData->mParticleEditorFunctions->refreshParticleDatablocks();
    mParticleEditorData->mParticleEditorFunctions->updateParticleDatablocksWidgets();

    mParticleEditorData->mWidgets.mGpuParticleDatablockWidget->refreshLoadFromResourcesActions();
}

void MainWindow::particleSystemChanged(const QString& name)
{
    const GpuParticleSystem* core = GpuParticleSystemResourceManager::getSingleton().getGpuParticleSystem(name.toStdString());
    GpuParticleSystem* coreNonConst = const_cast<GpuParticleSystem*>(core);
    mParticleEditorData->mChoosenGpuParticleSystem = coreNonConst;
    mParticleEditorData->mWidgets.mGpuParticleSystemNameEdit->setText(name);
    mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget->setEditedObject(coreNonConst);
    mParticleEditorData->mWidgets.mGpuParticleEmitterWidget->setEditedObject(nullptr, -1);

    mRenderer->chooseParticleSystem();
}

void MainWindow::particleDatablockChanged(const QString& datablockName)
{
    Ogre::String datablockNameStr = datablockName.toStdString();

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));
    HlmsParticleDatablock* particleDatablock = static_cast<HlmsParticleDatablock*>(hlmsParticle->getDatablock(datablockNameStr));

    mParticleEditorData->mChoosenGpuParticleDatablock = particleDatablock;

//    mRenderer->chooseParticleSystem();

    mParticleEditorData->mWidgets.mGpuParticleDatablockNameEdit->setText(datablockName);
    mParticleEditorData->mWidgets.mGpuParticleDatablockWidget->setEditedObject(particleDatablock);
}

void MainWindow::emitterChanged()
{
    int emitterIndex = mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget->getCurrentEmitterCoreIndex();
    GpuParticleSystem* core = mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget->getGpuParticleSystem();
    mParticleEditorData->mWidgets.mGpuParticleEmitterWidget->setEditedObject(core, emitterIndex);

    if(mParticleEditorData->mWidgets.mGpuParticleSystemTreeWidget->showOnlySelectedEmitters()) {
        mRenderer->restartParticleSystem();
    }
}

void MainWindow::showOnlySelectedEmittersChanged()
{
    mRenderer->restartParticleSystem();
}

void MainWindow::backgroundColourChanged()
{
    mRenderer->setBackgroundColour(mBackgroundColourEditField->getColour());
}

void MainWindow::elapsedTimeSpeedChanged()
{
    mParticleEditorData->mElapsedTimeSpeed = mSpeedController.getValue();
}

void MainWindow::newParticleSystem()
{
    QString currentParticleSystemName = tr("New particle system");

    bool ok = true;
    QString particleSystemNameCandidateStr = QInputDialog::getText(this, tr("Particle system name"),
                                                                   tr("Choose name:"), QLineEdit::Normal,
                                                                   currentParticleSystemName, &ok);
    if(!ok) {
        return;
    }

    GpuParticleSystemResourceManager& manager = GpuParticleSystemResourceManager::getSingleton();
    Ogre::String particleSystemNameCandidate = particleSystemNameCandidateStr.toStdString();
    if(manager.getGpuParticleSystem(particleSystemNameCandidate)) {
        QMessageBox::critical(this, tr("Can't create particle system!"), tr("Particle system with such name already exists!"));
        return;
    }

    GpuParticleSystem* newParticleSystem = manager.createParticleSystem(particleSystemNameCandidate, particleSystemNameCandidate, Ogre::BLANKSTRING, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    mParticleEditorData->mParticleEditorFunctions->updateParticleSystemsWidgets();
}

void MainWindow::newParticleDatablock()
{
    QString currentParticleDatablockName = tr("New particle datablock");

    bool ok = true;
    QString particleDatablockNameCandidateStr = QInputDialog::getText(this, tr("Particle datablock name"),
                                                                   tr("Choose name:"), QLineEdit::Normal,
                                                                   currentParticleDatablockName, &ok);
    if(!ok) {
        return;
    }

    Ogre::String particleDatablockNameCandidate = particleDatablockNameCandidateStr.toStdString();

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));

    if(hlmsParticle->getDatablock(particleDatablockNameCandidate)) {
        QMessageBox::critical(this, tr("Can't create particle datablock!"), tr("Particle datablock with such name already exists!"));
        return;
    }

    Ogre::HlmsMacroblock macroblock;
    macroblock.mDepthWrite = false;
    Ogre::HlmsBlendblock blendBlock;

    Ogre::HlmsDatablock *datablock = hlmsParticle->createDatablock( particleDatablockNameCandidate,
                                          particleDatablockNameCandidate,
                                          macroblock,
                                          blendBlock,
                                          Ogre::HlmsParamVec() );


    mParticleEditorData->mParticleEditorFunctions->refreshParticleDatablocks();
    mParticleEditorData->mParticleEditorFunctions->updateParticleDatablocksWidgets();
}

//void MainWindow::loadTexture()
//{
////    mParticleEditorData->mParticleEditorFunctions->loadTexture(Ogre::String());

//    QString initPathStr;

//    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Load image file"), initPathStr, "*.*");
//    if(QFile::exists(fileName)) {
//        QFileInfo fileInfo(fileName);
//        Ogre::String dirPathStr = fileInfo.dir().absolutePath().toStdString();
//        Ogre::String fname = fileInfo.fileName().toStdString();

//        Ogre::TextureGpuManager *textureManager =
//                mParticleEditorData->getRenderSystem()->getTextureGpuManager();

//        if(textureManager->findTextureNoThrow(fname)) {
//            QMessageBox::critical(this, tr("Loading texture failed!"), tr("Texture with such name already exists!"));
//            return;
//        }

//        // Test overlapping reads from same archive
//        Ogre::FileSystemArchive arch(dirPathStr, "FileSystem", true);
//        arch.load();

//        // File 1
//        Ogre::DataStreamPtr stream = arch.open(fname);

//        Ogre::Image2 *imagePtr = new Ogre::Image2();
//        try {
//            imagePtr->load(stream);
//        }
//        catch( Ogre::Exception &e )
//        {
//            QString msg = tr("Couldn't load image file! Ogre Exception occured!\n");
//            msg += QString(e.what());
//            QMessageBox::critical(this, tr("Loading texture failed!"), msg);
//            delete imagePtr;
//            return;
//        }
//        catch (...) {
//            QMessageBox::critical(this, tr("Loading texture failed!"), tr("Couldn't load image file! Ogre Exception occured!"));
//            delete imagePtr;
//            return;
//        }

//        Ogre::TextureGpu *texture = textureManager->createTexture(
//            fname, fname, Ogre::GpuPageOutStrategy::Discard,
//            Ogre::TextureFlags::AutomaticBatching | Ogre::TextureFlags::PrefersLoadingFromFileAsSRGB,
//            Ogre::TextureTypes::Type2D );

//        // Ogre will call "delete imagePtr" when done, because we're passing
//        // true to autoDeleteImage argument in scheduleTransitionTo
//        texture->scheduleTransitionTo( Ogre::GpuResidency::Resident, imagePtr, true );

//        texture->waitForData();

//        QMessageBox::information(this, tr("Loading texture completed!"), tr("Added texture: %1.").arg(QString::fromStdString(fname)));
//    }
//}

void MainWindow::loadParticleSystem()
{
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Load particle system"), QString(), "Json file (*.json)");
    if(QFile::exists(fileName)) {
        QFileInfo fileInfo(fileName);
        Ogre::String dirPathStr = fileInfo.dir().absolutePath().toStdString();
        Ogre::String fname = fileInfo.fileName().toStdString();

        // Test overlapping reads from same archive
        Ogre::FileSystemArchive arch(dirPathStr, "FileSystem", true);
        arch.load();

        // File 1
        Ogre::DataStreamPtr stream = arch.open(fname);

        std::set<Ogre::String> particleSystemsNamesBefore;
        mParticleEditorData->mParticleEditorFunctions->fillParticleSystemNames(particleSystemsNamesBefore);

        GpuParticleSystemJsonManager::getSingleton().parseScript(stream, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        std::set<Ogre::String> particleSystemsNamesAfter;
        mParticleEditorData->mParticleEditorFunctions->fillParticleSystemNames(particleSystemsNamesAfter);

        QStringList addedParticleSystems;

        for(std::set<Ogre::String>::const_iterator it = particleSystemsNamesAfter.begin();
            it != particleSystemsNamesAfter.end(); ++it) {

            if(particleSystemsNamesBefore.find(*it) == particleSystemsNamesBefore.end()) {
                addedParticleSystems.push_back(QString::fromStdString(*it));
            }
        }

        mParticleEditorData->mParticleEditorFunctions->updateParticleSystemsWidgets();

        if(addedParticleSystems.empty()) {
            QMessageBox::warning(this, tr("Loading particle systems failed!"), tr("No particle system was loaded!"));
        }
        else {
            QMessageBox::information(this, tr("Loading particle systems completed!"), tr("Added particle systems: %1.").arg(addedParticleSystems.join(", ")));
        }
    }
}

void MainWindow::loadDatablock()
{
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Load particle/unlit datablock"), QString(), "Json file (*.json)");
    if(QFile::exists(fileName)) {
        Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
        Ogre::HlmsUnlit *hlmsUnlit = static_cast<Ogre::HlmsUnlit*>( hlmsManager->getHlms(Ogre::HLMS_UNLIT) );
        HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));

        QFile file(fileName);
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream readFile(&file);
        QString jsonString = readFile.readAll();
        QByteArray ba = jsonString.toLatin1();
        char* jsonChar = ba.data();
        Ogre::String fname = fileName.toStdString();
        Ogre::HlmsJson hlmsJson(hlmsManager, 0);

        std::set<Ogre::String> particleDatablocksBefore;
        mParticleEditorData->mParticleEditorFunctions->fillDatablockNames(particleDatablocksBefore, HlmsParticle::ParticleHlmsType);

        std::set<Ogre::String> unlitDatablocksBefore;
        mParticleEditorData->mParticleEditorFunctions->fillDatablockNames(unlitDatablocksBefore, Ogre::HLMS_UNLIT);

        hlmsJson.loadMaterials(fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, jsonChar, "");
        file.close();

        std::set<Ogre::String> particleDatablocksAfter;
        mParticleEditorData->mParticleEditorFunctions->fillDatablockNames(particleDatablocksAfter, HlmsParticle::ParticleHlmsType);

        std::set<Ogre::String> unlitDatablocksAfter;
        mParticleEditorData->mParticleEditorFunctions->fillDatablockNames(unlitDatablocksAfter, Ogre::HLMS_UNLIT);

        QStringList addedParticleDatablocks;
        QStringList convertedParticleDatablocks;

        for(std::set<Ogre::String>::const_iterator it = particleDatablocksAfter.begin();
            it != particleDatablocksAfter.end(); ++it) {

            if(particleDatablocksBefore.find(*it) == particleDatablocksBefore.end()) {
                addedParticleDatablocks.push_back(QString::fromStdString(*it));
            }
        }

        for(std::set<Ogre::String>::const_iterator it = unlitDatablocksAfter.begin();
            it != unlitDatablocksAfter.end(); ++it) {

            if(unlitDatablocksBefore.find(*it) == unlitDatablocksBefore.end() &&
               particleDatablocksBefore.find(*it) == particleDatablocksBefore.end()) {

                // convert unlit datablock to particle datablock

                convertedParticleDatablocks.push_back(QString::fromStdString(*it));

                Ogre::String unlitDatablockName = *it;
                Ogre::HlmsUnlitDatablock* unlitDatablock = static_cast<Ogre::HlmsUnlitDatablock*>(hlmsUnlit->getDatablock(unlitDatablockName));

                HlmsParticleDatablock* particleDatablock = hlmsParticle->cloneFromUnlitDatablock(unlitDatablock, unlitDatablockName + "_converted");

                hlmsUnlit->destroyDatablock(*it);

                particleDatablock->clone(unlitDatablockName);

                Ogre::String tmpName = *particleDatablock->getNameStr();
                hlmsParticle->destroyDatablock(tmpName);
            }
        }

        mParticleEditorData->mParticleEditorFunctions->refreshParticleDatablocks();
        mParticleEditorData->mParticleEditorFunctions->updateParticleDatablocksWidgets();

        if(addedParticleDatablocks.empty() && convertedParticleDatablocks.empty()) {
            QMessageBox::warning(this, tr("Loading datablocks failed!"), tr("No particle datablock was loaded!"));
        }
        else {
            QString msg;
            if(!addedParticleDatablocks.empty()) {
                msg += tr("Added particle datablocks: %1.").arg(addedParticleDatablocks.join(", "));
            }
            if(!convertedParticleDatablocks.empty()) {
                if(!msg.isEmpty()) {
                    msg += "\n";
                }
                msg += tr("Converted unlit to particle datablocks: %1.").arg(convertedParticleDatablocks.join(", "));
            }
            QMessageBox::information(this, tr("Loading datablocks completed!"), msg);
        }

    }
}

void MainWindow::saveParticleSystem()
{
    if(!mParticleEditorData->mChoosenGpuParticleSystem) {

        QMessageBox::information(this, tr("Save failed!"), tr("No choosen particle system to save!"));

        return;
    }

    QString defaultParticleSystemName = mParticleEditorData->currentParticleSystemName();
    cleanFileNameToSave(defaultParticleSystemName);

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save particle system"), defaultParticleSystemName, "Json file (*.gpuparticle.json)");
    if(!fileName.isEmpty()) {

        Ogre::String outStr;
        GpuParticleSystemJsonManager::getSingleton().saveGpuParticleSystem(mParticleEditorData->mChoosenGpuParticleSystem, outStr);

        QFile file(fileName);
        file.open(QFile::WriteOnly | QFile::Text);

        file.write(outStr.c_str(), outStr.length());

        file.close();

    }
}

void MainWindow::saveParticleDatablock()
{
    if(!mParticleEditorData->mChoosenGpuParticleDatablock) {

        QMessageBox::information(this, tr("Save failed!"), tr("No choosen particle datablock to save!"));

        return;
    }

    QString defaultParticleDatablockName = mParticleEditorData->currentParticleDatablockName();
    cleanFileNameToSave(defaultParticleDatablockName);

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save particle datablock"), defaultParticleDatablockName, "Json file (*.json)");
    if(!fileName.isEmpty()) {

        Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();

        Ogre::String outStr;
        Ogre::HlmsJson hlmsJson( hlmsManager, nullptr );
        hlmsJson.saveMaterial( mParticleEditorData->mChoosenGpuParticleDatablock, outStr, Ogre::String() );

        QFile file(fileName);
        file.open(QFile::WriteOnly | QFile::Text);

        file.write(outStr.c_str(), outStr.length());

        file.close();
    }
}

void MainWindow::resourceActionChoosen()
{
    int index = mResourcesTabActionGroup->actions().indexOf(mResourcesTabActionGroup->checkedAction());
    if(index < 0) {
        return;
    }

    mResourcesStackedWidget->setCurrentIndex(index);
    mSelectedObjectStackedWidget->setCurrentIndex(index);

    if(index == 0) {
        mParticleSystemDock->setWindowTitle(tr("Particle system"));
    }
    else if(index == 1) {
        mParticleSystemDock->setWindowTitle(tr("Particle datablock"));
    }
}

void MainWindow::particleSystemRenameAction()
{
    QString currentParticleSystemName = mParticleEditorData->currentParticleSystemName();

    bool ok = true;
    QString particleSystemNameCandidateStr = QInputDialog::getText(this, tr("Particle system name"),
                                         tr("Choose new name:"), QLineEdit::Normal,
                                         currentParticleSystemName, &ok);
    if(!ok || particleSystemNameCandidateStr == currentParticleSystemName) {
        return;
    }

    GpuParticleSystemResourceManager& manager = GpuParticleSystemResourceManager::getSingleton();
    Ogre::String particleSystemNameCandidate = particleSystemNameCandidateStr.toStdString();
    if(manager.getGpuParticleSystem(particleSystemNameCandidate)) {
        QMessageBox::critical(this, tr("Can't rename particle system!"), tr("Particle system with such name already exists!"));
        return;
    }

    mRenderer->stopParticleSystem();

    const GpuParticleSystem* oldParticleSystem = manager.getGpuParticleSystem(currentParticleSystemName.toStdString());
    GpuParticleSystem* oldParticleSystemNonConst = const_cast<GpuParticleSystem*>(oldParticleSystem);
    std::vector<GpuParticleEmitter*> emitters = oldParticleSystemNonConst->_takeEmitters();

    GpuParticleSystem* newParticleSystem = manager.createParticleSystem(particleSystemNameCandidate, particleSystemNameCandidate, Ogre::BLANKSTRING, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    for (size_t i = 0; i < emitters.size(); ++i) {
        newParticleSystem->addEmitter(emitters[i]);
    }

    manager.destroyParticleSystem(currentParticleSystemName.toStdString());

    mParticleEditorData->mParticleEditorFunctions->updateParticleSystemsWidgets();
    mParticleEditorData->mWidgets.mGpuParticleSystemsListWidget->chooseItem(particleSystemNameCandidateStr);
    particleSystemChanged(particleSystemNameCandidateStr);
}

void MainWindow::particleSystemCloneAction()
{
    QString currentParticleSystemName = mParticleEditorData->currentParticleSystemName();

    bool ok = true;
    QString particleSystemNameCandidateStr = QInputDialog::getText(this, tr("Particle system name"),
                                                                   tr("Choose name:"), QLineEdit::Normal,
                                                                   currentParticleSystemName, &ok);
    if(!ok) {
        return;
    }

    GpuParticleSystemResourceManager& manager = GpuParticleSystemResourceManager::getSingleton();
    Ogre::String particleSystemNameCandidate = particleSystemNameCandidateStr.toStdString();
    if(manager.getGpuParticleSystem(particleSystemNameCandidate)) {
        QMessageBox::critical(this, tr("Can't copy particle system!"), tr("Particle system with such name already exists!"));
        return;
    }

    const GpuParticleSystem* oldParticleSystem = manager.getGpuParticleSystem(currentParticleSystemName.toStdString());
    GpuParticleSystem* newParticleSystem = manager.createParticleSystem(particleSystemNameCandidate, particleSystemNameCandidate, Ogre::BLANKSTRING, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    for (size_t i = 0; i < oldParticleSystem->getEmitters().size(); ++i) {
        newParticleSystem->addEmitter(oldParticleSystem->getEmitters()[i]->clone());
    }

    mParticleEditorData->mParticleEditorFunctions->updateParticleSystemsWidgets();
}

void MainWindow::particleSystemRemoveAction()
{
    QString currentParticleSystemName = mParticleEditorData->currentParticleSystemName();
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Removing particle system"), tr("Are you sure you want to remove particle system '%1'").arg(currentParticleSystemName));
    if(result != QMessageBox::Yes) {
        return;
    }

    mRenderer->stopParticleSystem();

    GpuParticleSystemResourceManager& manager = GpuParticleSystemResourceManager::getSingleton();

    manager.destroyParticleSystem(currentParticleSystemName.toStdString());

    mParticleEditorData->mParticleEditorFunctions->updateParticleSystemsWidgets();
    particleSystemChanged(QString());
}

void MainWindow::particleDatablockRenameAction()
{
    QString currentParticleDatablockNameStr = mParticleEditorData->currentParticleDatablockName();
    Ogre::String currentParticleDatablockName = currentParticleDatablockNameStr.toStdString();

    bool ok = true;
    QString particleDatablockNameCandidateStr = QInputDialog::getText(this, tr("Particle datablock name"),
                                                                   tr("Choose new name:"), QLineEdit::Normal,
                                                                   currentParticleDatablockNameStr, &ok);
    Ogre::String particleDatablockNameCandidate = particleDatablockNameCandidateStr.toStdString();

    if(!ok) {
        return;
    }

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));
    HlmsParticleDatablock* particleDatablock = static_cast<HlmsParticleDatablock*>(hlmsParticle->getDatablock(particleDatablockNameCandidate));

    if(particleDatablock) {
        QMessageBox::critical(this, tr("Can't copy particle datablock!"), tr("Datablock with such name already exists!"));
        return;
    }

    bool replace = false;
    std::map<Ogre::String, const GpuParticleSystem*> particleSystemsUsingDatablock;
    mParticleEditorData->mParticleEditorFunctions->fillParticleSystemsUsingDatablock(particleSystemsUsingDatablock, currentParticleDatablockName);
    if(!particleSystemsUsingDatablock.empty()) {
        QString msgText = tr("Are you sure you want to rename particle datablock '%1'?").arg(currentParticleDatablockNameStr);
        msgText += "\n" + tr("Following %1 particle systems are using this datablock:\n").arg(particleSystemsUsingDatablock.size());
        for(std::map<Ogre::String, const GpuParticleSystem*>::const_iterator it = particleSystemsUsingDatablock.begin();
            it != particleSystemsUsingDatablock.end(); ++it) {
            if(it != particleSystemsUsingDatablock.begin()) {
                msgText += ",";
            }
            msgText += QString::fromStdString(it->first);
        }
        int code = QMessageBox::question(this, tr("Particle datablock name"), msgText, tr("Continue"), tr("Replace occurences"), tr("Cancel"), 0, 2);
        if(code == 2) {
            // cancel
            return;
        }
        else if(code == 1) {
            // replace occurences
            replace = true;
        }
    }

    mRenderer->stopParticleSystem();

    if(replace) {
        for(std::map<Ogre::String, const GpuParticleSystem*>::const_iterator it = particleSystemsUsingDatablock.begin();
            it != particleSystemsUsingDatablock.end(); ++it) {

            const GpuParticleSystem* particleSystem = it->second;
            for (size_t i = 0; i < particleSystem->getEmitters().size(); ++i) {
                const GpuParticleEmitter* emitter = particleSystem->getEmitters()[i];
                if(emitter->mDatablockName == currentParticleDatablockName) {
                    GpuParticleEmitter* emitterNonConst = const_cast<GpuParticleEmitter*>(emitter);
                    emitterNonConst->mDatablockName = particleDatablockNameCandidate;
                }
            }
        }
    }

    HlmsParticleDatablock* originalParticleDatablock = static_cast<HlmsParticleDatablock*>(hlmsParticle->getDatablock(currentParticleDatablockName));

    Ogre::HlmsDatablock* datablock = originalParticleDatablock->clone(particleDatablockNameCandidate);

    hlmsParticle->destroyDatablock(currentParticleDatablockName);

    mParticleEditorData->mParticleEditorFunctions->refreshParticleDatablocks();
    mParticleEditorData->mParticleEditorFunctions->updateParticleDatablocksWidgets();
    particleDatablockChanged(particleDatablockNameCandidateStr);

    mRenderer->restartParticleSystem();
}

void MainWindow::particleDatablockCloneAction()
{
    QString currentParticleDatablockName = mParticleEditorData->currentParticleDatablockName();

    bool ok = true;
    QString particleDatablockNameCandidateStr = QInputDialog::getText(this, tr("Particle datablock name"),
                                                                   tr("Choose name:"), QLineEdit::Normal,
                                                                   currentParticleDatablockName, &ok);
    Ogre::String particleDatablockNameCandidate = particleDatablockNameCandidateStr.toStdString();

    if(!ok) {
        return;
    }

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));
    HlmsParticleDatablock* particleDatablock = static_cast<HlmsParticleDatablock*>(hlmsParticle->getDatablock(particleDatablockNameCandidate));

    if(particleDatablock) {
        QMessageBox::critical(this, tr("Can't copy particle datablock!"), tr("Datablock with such name already exists!"));
        return;
    }

    HlmsParticleDatablock* originalParticleDatablock = static_cast<HlmsParticleDatablock*>(hlmsParticle->getDatablock(currentParticleDatablockName.toStdString()));

    Ogre::HlmsDatablock* datablock = originalParticleDatablock->clone(particleDatablockNameCandidate);

    mParticleEditorData->mParticleEditorFunctions->refreshParticleDatablocks();
    mParticleEditorData->mParticleEditorFunctions->updateParticleDatablocksWidgets();
}

void MainWindow::particleDatablockRemoveAction()
{
    QString currentParticleDatablockName = mParticleEditorData->currentParticleDatablockName();

    std::map<Ogre::String, const GpuParticleSystem*> particleSystemsUsingDatablock;
    mParticleEditorData->mParticleEditorFunctions->fillParticleSystemsUsingDatablock(particleSystemsUsingDatablock, currentParticleDatablockName.toStdString());

    QString msgText = tr("Are you sure you want to remove particle datablock '%1'?").arg(currentParticleDatablockName);
    if(!particleSystemsUsingDatablock.empty()) {
        msgText += "\n" + tr("Following %1 particle systems are using this datablock:\n").arg(particleSystemsUsingDatablock.size());
        for(std::map<Ogre::String, const GpuParticleSystem*>::const_iterator it = particleSystemsUsingDatablock.begin();
            it != particleSystemsUsingDatablock.end(); ++it) {
            if(it != particleSystemsUsingDatablock.begin()) {
                msgText += ",";
            }
            msgText += QString::fromStdString(it->first);
        }
    }

    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Removing particle datablock"), msgText);
    if(result != QMessageBox::Yes) {
        return;
    }

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    HlmsParticle* hlmsParticle = static_cast<HlmsParticle*>( hlmsManager->getHlms(HlmsParticle::ParticleHlmsType));
    HlmsParticleDatablock* particleDatablock = static_cast<HlmsParticleDatablock*>(hlmsParticle->getDatablock(currentParticleDatablockName.toStdString()));

    if(!particleDatablock) {
        return;
    }


    mRenderer->stopParticleSystem();

    hlmsParticle->destroyDatablock(currentParticleDatablockName.toStdString());

    mParticleEditorData->mParticleEditorFunctions->refreshParticleDatablocks();
    mParticleEditorData->mParticleEditorFunctions->updateParticleDatablocksWidgets();
    particleDatablockChanged(QString());

    mRenderer->restartParticleSystem();

}

void MainWindow::showPlaneAction(bool show)
{
    mRenderer->showPlane(show);
}

void MainWindow::setCameraFromFrontAction()
{
    mRenderer->setCamera(Ogre::Vector3(0.0f, 0.0f, 5.0f));
}

void MainWindow::setCameraFromBackAction()
{
    mRenderer->setCamera(Ogre::Vector3(0.0f, 0.0f, -5.0f));
}

void MainWindow::setCameraFromLeftAction()
{
    mRenderer->setCamera(Ogre::Vector3(-5.0f, 0.0f, 0.0f));
}

void MainWindow::setCameraFromRightAction()
{
    mRenderer->setCamera(Ogre::Vector3(5.0f, 0.0f, 0.0f));
}

void MainWindow::setCameraFromTopAction()
{
    mRenderer->setCamera(Ogre::Vector3(0.0f, 5.0f, 0.0f));
}

void MainWindow::setCameraFromBottomAction()
{
    mRenderer->setCamera(Ogre::Vector3(0.0f, -5.0f, 0.0f));
}
