/*
 * File: GpuParticleSystemTreeWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#include "GpuParticleSystemTreeWidget.h"
#include "ParticleEditorData.h"

#include <OgreRenderer.h>
#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QListWidget>

#include <GpuParticles/GpuParticleSystem.h>

GpuParticleSystemTreeWidget::GpuParticleSystemTreeWidget(ParticleEditorData& _data)
    : data(_data)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    mParticleEmittersList = new QListWidget();
    mParticleEmittersList->setContextMenuPolicy(Qt::ActionsContextMenu);
    {
        QAction* createAction = new QAction(tr("Create"));
        connect(createAction, SIGNAL(triggered(bool)), this, SLOT(createEmitterAction()));
        mParticleEmittersList->addAction(createAction);

        QAction* copyAction = new QAction(tr("Copy"));
        connect(copyAction, SIGNAL(triggered(bool)), this, SLOT(copyEmitterAction()));
        mParticleEmittersList->addAction(copyAction);

        QAction* pasteToNewAction = new QAction(tr("Paste to new emitter"));
        connect(pasteToNewAction, SIGNAL(triggered(bool)), this, SLOT(pasteToNewEmitterAction()));
        mParticleEmittersList->addAction(pasteToNewAction);

        QAction* removeAction = new QAction(tr("Remove"));
        connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(removeEmitterAction()));
        mParticleEmittersList->addAction(removeAction);

        QAction* moveUpAction = new QAction(tr("Move up"));
        connect(moveUpAction, SIGNAL(triggered(bool)), this, SLOT(moveEmitterUpAction()));
        mParticleEmittersList->addAction(moveUpAction);

        QAction* moveDownAction = new QAction(tr("Move down"));
        connect(moveDownAction, SIGNAL(triggered(bool)), this, SLOT(moveEmitterDownAction()));
        mParticleEmittersList->addAction(moveDownAction);
    }
    connect(mParticleEmittersList, SIGNAL(clicked(QModelIndex)), this, SIGNAL(emitterChanged()));
    mainLayout->addWidget(mParticleEmittersList);

    mShowOnlySelectedEmitters = new QCheckBox(tr("Show only selected emitter"));
    mShowOnlySelectedEmitters->setChecked(false);
    connect(mShowOnlySelectedEmitters, SIGNAL(clicked(bool)), this, SIGNAL(showOnlySelectedEmittersChanged()));
    mainLayout->addWidget(mShowOnlySelectedEmitters);

    setLayout(mainLayout);

    setEnabled(false);
}

void GpuParticleSystemTreeWidget::setEditedObject(GpuParticleSystem* core)
{
    mGpuParticleSystem = core;

    // refresh widget

    setEnabled(mGpuParticleSystem);

    particleSystemToGui();
}

void GpuParticleSystemTreeWidget::particleSystemToGui()
{
    QSignalBlocker bl(mParticleEmittersList);
    mParticleEmittersList->clear();

    if(mGpuParticleSystem) {
        for (unsigned int i = 0; i < mGpuParticleSystem->getEmitters().size(); ++i) {
//            const GpuParticleEmitter* emitterCore = mGpuParticleSystem->getEmitters()[i];
            mParticleEmittersList->addItem(tr("Emitter %1").arg(QString::number(i)));
        }
    }
}

void GpuParticleSystemTreeWidget::updateEmitterInvalidIcons()
{
    for (int i = 0; i < mParticleEmittersList->count(); ++i) {

        bool isValid = (i >= (int)data.mValidParticleEmitters.size() || data.mValidParticleEmitters[i]);
        QString tooltipStr = i < data.mParticleEmitterTooltips.size()
                ? data.mParticleEmitterTooltips[i] : QString();

        QListWidgetItem* item = mParticleEmittersList->item(i);
        item->setToolTip(tooltipStr);
        if(isValid) {
            item->setIcon(QIcon());
        }
        else {
            QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
            item->setIcon(icon);
        }
    }
}

int GpuParticleSystemTreeWidget::getCurrentEmitterCoreIndex() const
{
    return mParticleEmittersList->currentRow();
}

GpuParticleSystem* GpuParticleSystemTreeWidget::getGpuParticleSystem() const
{
    return mGpuParticleSystem;
}

bool GpuParticleSystemTreeWidget::showOnlySelectedEmitters() const
{
    return mShowOnlySelectedEmitters->isChecked();
}

void GpuParticleSystemTreeWidget::createEmitterAction()
{
    GpuParticleEmitter* emitter = OGRE_NEW GpuParticleEmitter();
    mGpuParticleSystem->addEmitter(emitter);

    particleSystemToGui();

    mParticleEmittersList->setCurrentRow(mParticleEmittersList->count()-1);

    emit emitterChanged();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::copyEmitterAction()
{
    int currentRow = mParticleEmittersList->currentRow();
    if(currentRow < 0 || currentRow >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    GpuParticleEmitter* emitter = mGpuParticleSystem->getEmitters()[currentRow];

    data.copyFromGpuParticleEmitter(emitter);
}

void GpuParticleSystemTreeWidget::pasteToNewEmitterAction()
{
    GpuParticleEmitter* emitter = OGRE_NEW GpuParticleEmitter();
    data.pasteToGpuParticleEmitter(emitter);
    mGpuParticleSystem->addEmitter(emitter);

    particleSystemToGui();

    mParticleEmittersList->setCurrentRow(mParticleEmittersList->count()-1);

    emit emitterChanged();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::removeEmitterAction()
{
    int currentRow = mParticleEmittersList->currentRow();
    if(currentRow < 0 || currentRow >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    data.mRenderer->stopParticleSystem();

    GpuParticleEmitter* emitter = mGpuParticleSystem->_takeEmitter(currentRow);

    particleSystemToGui();

    emit emitterChanged();

    OGRE_DELETE emitter;

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::moveEmitterUpAction()
{
    // up is previous

    int currentRow = mParticleEmittersList->currentRow();
    if(currentRow < 1 || currentRow >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    GpuParticleEmitter* emitter = mGpuParticleSystem->_takeEmitter(currentRow);
    mGpuParticleSystem->_insertEmitter(currentRow-1, emitter);

    particleSystemToGui();
    mParticleEmittersList->setCurrentRow(currentRow-1);

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::moveEmitterDownAction()
{
    // down is next

    int currentRow = mParticleEmittersList->currentRow();
    if(currentRow < 0 || currentRow+1 >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    GpuParticleEmitter* emitter = mGpuParticleSystem->_takeEmitter(currentRow);
    mGpuParticleSystem->_insertEmitter(currentRow+1, emitter);

    particleSystemToGui();
    mParticleEmittersList->setCurrentRow(currentRow+1);

    data.mRenderer->restartParticleSystem();
}
