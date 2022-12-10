
TARGET = OgreGpuParticlesEditor
TEMPLATE = app

# SET PATHS inside OgreGpuParticlesEditorPaths.pri: OGRE_ROOT, OGRE_BUILD, OGRE_DEPENDENCIES
include (OgreGpuParticlesEditorPaths.pri)
#

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += OGRE_ENABLED
DEFINES += SHOW_OUTPUT_SHADERS

# Common settings, compiler flags etc.
DEFINES -= UNICODE
DEFINES += _USE_MATH_DEFINES

debug {
    DEFINES += OGRE_GPUPARTICLE_EDITOR_DEBUG
}

win32-msvc* {
    QMAKE_CFLAGS_RELEASE += -Zi
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF
    #QMAKE_CFLAGS += /GL
    QMAKE_LFLAGS_RELEASE -= /GL
    QMAKE_LFLAGS += /ignore:4100
    #/NODEFAULTLIB:MSVCRTD

    QMAKE_CXXFLAGS += -wd4100 -wd4251 -wd4193 -wd4275 -we4715

# 4100 - unreferenced formal parameter
#
    QMAKE_CXXFLAGS_WARN_ON -= -w34100
    QMAKE_CXXFLAGS_WARN_OFF += -wd4100
}

linux-g++ {
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -Wall -Wno-comment -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-parameter
    QMAKE_CXXFLAGS += -msse -msse2 -msse3

}

OGRE_ENABLED {
    isEmpty(OGRE_ROOT) {
      error( "No 'OGRE_ROOT' path specified!" );
    }
    isEmpty(OGRE_BUILD) {
      error( "No 'OGRE_BUILD' path specified!" );
    }
    isEmpty(OGRE_DEPENDENCIES) {
      error( "No 'OGRE_DEPENDENCIES' path specified!" );
    }

    OGRE_COMPONENTS = $${OGRE_ROOT}/Components
    INCLUDEPATH += $${OGRE_ROOT}/OgreMain/include/Compositor
    INCLUDEPATH += $${OGRE_COMPONENTS}/Overlay/include
    INCLUDEPATH += $${OGRE_COMPONENTS}/Hlms/Common/include
    INCLUDEPATH += $${OGRE_COMPONENTS}/Hlms/Unlit/include
    INCLUDEPATH += $${OGRE_COMPONENTS}/Hlms/Pbs/include

    LIBS += -L$${OGRE_BUILD}/lib

#linux-g++ {
#    LIBS += -L$${OGRE_DEPENDENCIES}/lib
#}

    release:LIBS += -L$${OGRE_DEPENDENCIES}/lib/Release
    debug:LIBS += -L$${OGRE_DEPENDENCIES}/lib/Debug

    INCLUDEPATH += $${OGRE_BUILD}/include
    INCLUDEPATH += $${OGRE_ROOT}/OgreMain/include
    INCLUDEPATH += $${OGRE_DEPENDENCIES}/include
    INCLUDEPATH += $${OGRE_ROOT}/Samples/Common/include

    release:LIBS *= -lOgreMain -lOgreOverlay -lOgreHlmsPbs -lOgreHlmsUnlit
    debug:LIBS *= -lOgreMain_d -lOgreOverlay_d -lOgreHlmsPbs_d -lOgreHlmsUnlit_d

}

INCLUDEPATH += src \
               src/Utils \
               src/Widgets

DEPENDPATH  += src/ \
               src/Utils/ \
               src/Widgets

VPATH = src/

SOURCES += main.cpp \
    src/MainWindow.cpp \
    src/OgreQtAppParticleEditorSystem.cpp \
    src/OgreRenderer.cpp \
    src/ParticleEditorAssets.cpp \
    src/ParticleEditorFunctions.cpp \
    src/QTOgreWindow.cpp \
    src/GpuParticles/GpuParticleSystemJsonManager.cpp \
    src/GpuParticles/GpuParticleSystemWorld.cpp \
    src/GpuParticles/Hlms/HlmsJsonParticleAtlas.cpp \
    src/GpuParticles/Hlms/HlmsParticle.cpp \
    src/GpuParticles/Hlms/HlmsParticleDatablock.cpp \
    src/GpuParticles/GpuParticleSystemResourceManager.cpp \
    src/GpuParticles/GpuParticleEmitter.cpp \
    src/GpuParticles/GpuParticleSystem.cpp \
    src/Utils/EulerDegreeHVS.cpp \
    src/Utils/OgreQtImageHelper.cpp \
    src/Utils/Range.cpp \
    src/Widgets/ColourButton.cpp \
    src/Widgets/ColourEditField.cpp \
    src/Widgets/ColourTrackTableWidget.cpp \
    src/Widgets/DoubleValueController.cpp \
    src/Widgets/EulerDegreeHVSWidget.cpp \
    src/Widgets/FloatTrackTableWidget.cpp \
    src/Widgets/Point2dTrackTableWidget.cpp \
    src/Widgets/Point2dWidget.cpp \
    src/Widgets/Point3dWidget.cpp \
    src/Widgets/RangeWidget.cpp \
    src/Widgets/SpriteTrackTableWidget.cpp \
    src/Widgets/TrackTableModel.cpp \
    src/Widgets/TrackTableModelBase.cpp \
    src/Widgets/TrackTableWidget.cpp \
    src/Widgets/TrackTableWidgetBase.cpp \
    src/Common/OgreQtSystem.cpp \
    src/Common/OgreSDLSystem.cpp \
    src/Common/OgreSDLGame.cpp \
    src/Editors/BaseDatablockWidget.cpp \
    src/Editors/BlendblockWidget.cpp \
    src/Editors/GpuParticleDatablockWidget.cpp \
    src/Editors/GpuParticleDatablocksListWidget.cpp \
    src/Editors/ImageChooserDialog.cpp \
    src/Editors/MacroblockWidget.cpp \
    src/Editors/ParticleEditorData.cpp \
    src/Editors/GpuParticleEmitterWidget.cpp \
    src/Editors/GpuParticleSystemsListWidget.cpp \
    src/Editors/GpuParticleSystemTreeWidget.cpp \
    src/Editors/SamplerblockWidget.cpp \
    src/Editors/UnlitDatablockWidget.cpp

HEADERS  += \
    src/MainWindow.h \
    src/OgreQtAppParticleEditorSystem.h \
    src/OgreRenderer.h \
    src/ParticleEditorAssets.h \
    src/ParticleEditorFunctions.h \
    src/QTOgreWindow.h \
    src/GpuParticles/GpuParticleSystemJsonManager.h \
    src/GpuParticles/GpuParticleSystemWorld.h \
    src/GpuParticles/Hlms/HlmsJsonParticleAtlas.h \
    src/GpuParticles/Hlms/HlmsParticle.h \
    src/GpuParticles/Hlms/HlmsParticleDatablock.h \
    src/GpuParticles/GpuParticleSystemResourceManager.h \
    src/GpuParticles/GpuParticleEmitter.h \
    src/GpuParticles/GpuParticleSystem.h \
    src/Utils/EulerDegreeHVS.h \
    src/Utils/OgreQtImageHelper.h \
    src/Utils/Range.h \
    src/Widgets/ColourButton.h \
    src/Widgets/ColourEditField.h \
    src/Widgets/ColourTrackTableWidget.h \
    src/Widgets/DoubleValueController.h \
    src/Widgets/EulerDegreeHVSWidget.h \
    src/Widgets/FloatTrackTableWidget.h \
    src/Widgets/Point2dTrackTableWidget.h \
    src/Widgets/Point2dWidget.h \
    src/Widgets/Point3dWidget.h \
    src/Widgets/RangeWidget.h \
    src/Widgets/SpriteTrackTableWidget.h \
    src/Widgets/TrackTableModel.h \
    src/Widgets/TrackTableModelBase.h \
    src/Widgets/TrackTableWidget.h \
    src/Widgets/TrackTableWidgetBase.h \
    src/Common/OgreQtSystem.h \
    src/Common/OgreSDLSystem.h \
    src/Common/OgreSDLGame.h \
    src/Common/SdkQtCameraMan.h \
    src/Editors/BaseDatablockWidget.h \
    src/Editors/BlendblockWidget.h \
    src/Editors/GpuParticleDatablockWidget.h \
    src/Editors/GpuParticleDatablocksListWidget.h \
    src/Editors/ImageChooserDialog.h \
    src/Editors/MacroblockWidget.h \
    src/Editors/ParticleEditorData.h \
    src/Editors/GpuParticleEmitterWidget.h \
    src/Editors/GpuParticleSystemsListWidget.h \
    src/Editors/GpuParticleSystemTreeWidget.h \
    src/Editors/SamplerblockWidget.h \
    src/Editors/UnlitDatablockWidget.h

CONFIG += console

DIST = .

debug:DESTDIR = $$PWD/bin/Debug/
release:DESTDIR = $$PWD/bin/Release/
