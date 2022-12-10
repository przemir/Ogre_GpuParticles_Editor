/*
 * File: ParticleEditorData.cpp
 * Author: Przemysław Bągard
 * Created: 2020-10-11
 *
 */

#include "ParticleEditorData.h"
#include <MainWindow.h>
#include <OgreQtAppParticleEditorSystem.h>
#include <QLineEdit>
#include <GpuParticles/GpuParticleEmitter.h>

ParticleEditorData::ParticleEditorData()
{
    mGpuParticleEmitterClipboard = OGRE_NEW GpuParticleEmitter();
}

ParticleEditorData::~ParticleEditorData()
{
    OGRE_DELETE mGpuParticleEmitterClipboard;
}

QString ParticleEditorData::currentParticleSystemName() const
{
    return mWidgets.mGpuParticleSystemNameEdit->text();
}

QString ParticleEditorData::currentParticleDatablockName() const
{
    return mWidgets.mGpuParticleDatablockNameEdit->text();
}

void ParticleEditorData::copyFromGpuParticleEmitter(const GpuParticleEmitter* emitter)
{
    *mGpuParticleEmitterClipboard = *emitter;
}

void ParticleEditorData::pasteToGpuParticleEmitter(GpuParticleEmitter* emitter) const
{
    *emitter = *mGpuParticleEmitterClipboard;
}

Ogre::Root* ParticleEditorData::getRoot() const
{
    return mOgreQtAppSystem->getRoot();
}

Ogre::RenderSystem* ParticleEditorData::getRenderSystem() const
{
    return mOgreQtAppSystem->getRoot()->getRenderSystem();
}

QMainWindow* ParticleEditorData::WidgetData::getQMainWindow()
{
    return mMainWindow;
}
