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
#include <QMenu>
#include <QMessageBox>

#include <GpuParticles/GpuParticleSystem.h>
#include <GpuParticles/GpuParticleSystemResourceManager.h>

GpuParticleSystemTreeWidget::GpuParticleSystemTreeWidget(ParticleEditorData& _data)
    : data(_data)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    mParticleEmittersTree = new QTreeWidget();
    mParticleEmittersTree->setHeaderHidden(true);
//    mParticleEmittersTree->setRootIsDecorated(false);
    mParticleEmittersTree->setContextMenuPolicy(Qt::CustomContextMenu);

    // create actions
    {
        mParticleEmittersTreeContextMenu = new QMenu(this);

        QAction* createAction = new QAction(tr("Create"));
        connect(createAction, SIGNAL(triggered(bool)), this, SLOT(createEmitterAction()));
        mParticleEmittersTreeContextMenu->addAction(createAction);

        QAction* copyAction = new QAction(tr("Copy"));
        connect(copyAction, SIGNAL(triggered(bool)), this, SLOT(copyAction()));
        mParticleEmittersTreeContextMenu->addAction(copyAction);

        QAction* pasteToNewAction = new QAction(tr("Paste to new emitter"));
        connect(pasteToNewAction, SIGNAL(triggered(bool)), this, SLOT(pasteToNewEmitterAction()));
        mParticleEmittersTreeContextMenu->addAction(pasteToNewAction);

        QAction* pasteAffectorAction = new QAction(tr("Paste affector"));
        connect(pasteAffectorAction, SIGNAL(triggered(bool)), this, SLOT(pasteAffectorAction()));
        mParticleEmittersTreeContextMenu->addAction(pasteAffectorAction);

        mCreateAffectorMenu = new QMenu(tr("Create affector"));
        connect(mCreateAffectorMenu, SIGNAL(triggered(QAction*)), this, SLOT(createAffectorAction(QAction*)));
        mParticleEmittersTreeContextMenu->addMenu(mCreateAffectorMenu);

        QAction* removeAction = new QAction(tr("Remove"));
        connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(removeAction()));
        mParticleEmittersTreeContextMenu->addAction(removeAction);

        QAction* moveUpAction = new QAction(tr("Move up"));
        connect(moveUpAction, SIGNAL(triggered(bool)), this, SLOT(moveEmitterUpAction()));
        mParticleEmittersTreeContextMenu->addAction(moveUpAction);

        QAction* moveDownAction = new QAction(tr("Move down"));
        connect(moveDownAction, SIGNAL(triggered(bool)), this, SLOT(moveEmitterDownAction()));
        mParticleEmittersTreeContextMenu->addAction(moveDownAction);
    }
    connect(mParticleEmittersTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(mParticleEmittersTree, SIGNAL(clicked(QModelIndex)), this, SIGNAL(emitterChanged()));
    mainLayout->addWidget(mParticleEmittersTree);

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
    QSignalBlocker bl(mParticleEmittersTree);
    mParticleEmittersTree->clear();

    if(mGpuParticleSystem) {
        for (unsigned int i = 0; i < mGpuParticleSystem->getEmitters().size(); ++i) {
            const GpuParticleEmitter* emitterCore = mGpuParticleSystem->getEmitters()[i];

            QTreeWidgetItem* item = new QTreeWidgetItem();
            mParticleEmittersTree->addTopLevelItem(item);
            fillEmitterAffectorsToGui(item, emitterCore);
        }
        setEmitterNames();
    }
}

void GpuParticleSystemTreeWidget::updateEmitterInvalidIcons()
{
    for (int i = 0; i < mParticleEmittersTree->topLevelItemCount(); ++i) {

        bool isValid = (i >= (int)data.mValidParticleEmitters.size() || data.mValidParticleEmitters[i]);
        QString tooltipStr = i < data.mParticleEmitterTooltips.size()
                ? data.mParticleEmitterTooltips[i] : QString();

        QTreeWidgetItem* item = mParticleEmittersTree->topLevelItem(i);
        item->setToolTip(0, tooltipStr);
        if(isValid) {
            item->setIcon(0, QIcon());
        }
        else {
            QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
            item->setIcon(0, icon);
        }
    }
}

int GpuParticleSystemTreeWidget::getCurrentEmitterCoreIndex() const
{
    if(!mParticleEmittersTree->currentItem()) {
        return -1;
    }

    if(!mParticleEmittersTree->currentItem()->parent()) {
        // top level
        return mParticleEmittersTree->currentIndex().row();
    }

    // affector, so show parent's index.
    return mParticleEmittersTree->currentIndex().parent().row();
}

int GpuParticleSystemTreeWidget::getCurrentAffectorIndex() const
{
    if(!mParticleEmittersTree->currentItem()) {
        return -1;
    }

    if(!mParticleEmittersTree->currentItem()->parent()) {
        // top level
        return -1;
    }

    return mParticleEmittersTree->currentIndex().row();
}

const GpuParticleAffector* GpuParticleSystemTreeWidget::getCurrentAffector()
{
    if(!mGpuParticleSystem) {
        return nullptr;
    }

    if(!mParticleEmittersTree->currentItem()) {
        return nullptr;
    }

    if(!mParticleEmittersTree->currentItem()->parent()) {
        // top level
        return nullptr;
    }

    int emitterIndex = getCurrentEmitterCoreIndex();
    if(emitterIndex < 0 || emitterIndex >= (int)mGpuParticleSystem->getEmitters().size()) {
        return nullptr;
    }

//    int affectorIndex = mParticleEmittersTree->currentIndex().row();
    QTreeWidgetItem* affectorItem = mParticleEmittersTree->currentItem();

    Ogre::String type = affectorItem->data(0, Qt::UserRole).toString().toStdString();

    const GpuParticleEmitter* emitter = mGpuParticleSystem->getEmitters()[emitterIndex];
    const GpuParticleAffector* affector = emitter->getAffectorNoThrow(type);
    return affector;
}

GpuParticleSystem* GpuParticleSystemTreeWidget::getGpuParticleSystem() const
{
    return mGpuParticleSystem;
}

bool GpuParticleSystemTreeWidget::showOnlySelectedEmitters() const
{
    return mShowOnlySelectedEmitters->isChecked();
}

void GpuParticleSystemTreeWidget::createAffectorActions()
{
    std::map<Ogre::String, const GpuParticleAffector*> affectorNamesMap;
    std::map<Ogre::String, const GpuParticleAffector*> customAffectors;

    GpuParticleSystemResourceManager& gpuParticleSystemResourceManager = GpuParticleSystemResourceManager::getSingleton();
    const GpuParticleSystemResourceManager::AffectorByIdStringMap& registeredAffectors = gpuParticleSystemResourceManager.getAffectorByPropertyMap();
    for(GpuParticleSystemResourceManager::AffectorByIdStringMap::const_iterator it = registeredAffectors.begin();
        it != registeredAffectors.end(); ++it) {

        const GpuParticleAffector* affector = it->second;
        affectorNamesMap.insert(std::make_pair(affector->getAffectorProperty(), affector));
    }

    for(std::map<Ogre::String, const GpuParticleAffector*>::const_iterator it = affectorNamesMap.begin();
        it != affectorNamesMap.end(); ++it) {

        const GpuParticleAffector* affector = it->second;
        QAction* action = new QAction(QString::fromStdString(affector->getAffectorProperty()));
        action->setData(QString::fromStdString(affector->getAffectorProperty()));
//        connect(action, SIGNAL(triggered(bool)), this, SLOT(createAffectorAction()));
        mCreateAffectorMenu->addAction(action);
    }
}

void GpuParticleSystemTreeWidget::setEmitterNames()
{
    for (int i = 0; i < mParticleEmittersTree->topLevelItemCount(); ++i) {
        mParticleEmittersTree->topLevelItem(i)->setText(0, tr("Emitter %1").arg(QString::number(i)));
    }
}

void GpuParticleSystemTreeWidget::fillEmitterAffectorsToGui(QTreeWidgetItem* item, const GpuParticleEmitter* emitterCore)
{
    for(GpuParticleEmitter::AffectorByNameMap::const_iterator it = emitterCore->getAffectorByNameMap().begin();
        it != emitterCore->getAffectorByNameMap().end(); ++it) {

        const GpuParticleAffector* affector = it->second;
        QTreeWidgetItem* affectorItem = new QTreeWidgetItem();
        affectorItem->setText(0, QString::fromStdString(affector->getAffectorProperty()));
        affectorItem->setData(0, Qt::UserRole, QString::fromStdString(affector->getAffectorProperty()));
        item->addChild(affectorItem);
    }
    item->sortChildren(0, Qt::AscendingOrder);
}

void GpuParticleSystemTreeWidget::createEmitterAction()
{
    GpuParticleEmitter* emitter = new GpuParticleEmitter();
    mGpuParticleSystem->addEmitter(emitter);

    QSignalBlocker bl(mParticleEmittersTree);
    QTreeWidgetItem* item = new QTreeWidgetItem();
    mParticleEmittersTree->addTopLevelItem(item);

    setEmitterNames();

    mParticleEmittersTree->setCurrentItem(mParticleEmittersTree->topLevelItem(mParticleEmittersTree->topLevelItemCount()-1));

    emit emitterChanged();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::copyAction()
{
    if(!mParticleEmittersTree->currentItem()) {
        return;
    }

    if(!mParticleEmittersTree->currentItem()->parent()) {
        // top level item

        int currentRow = mParticleEmittersTree->currentIndex().row();
        if(currentRow < 0 || currentRow >= (int)mGpuParticleSystem->getEmitters().size()) {
            return;
        }

        GpuParticleEmitter* emitter = mGpuParticleSystem->getEmitters()[currentRow];

        data.copyFromGpuParticleEmitter(emitter);
    }
    else {

        int affectorIndex = mParticleEmittersTree->currentIndex().row();
        QTreeWidgetItem* affectorItem = mParticleEmittersTree->currentItem();

        Ogre::String type = affectorItem->data(0, Qt::UserRole).toString().toStdString();

        int emitterIndex = getCurrentEmitterCoreIndex();
        const GpuParticleEmitter* emitter = mGpuParticleSystem->getEmitters()[emitterIndex];
        const GpuParticleAffector* affector = emitter->getAffectorNoThrow(type);

        data.copyFromGpuParticleAffector(affector);
    }
}

void GpuParticleSystemTreeWidget::pasteToNewEmitterAction()
{
    GpuParticleEmitter* emitter = new GpuParticleEmitter();
    data.pasteToGpuParticleEmitter(emitter);
    mGpuParticleSystem->addEmitter(emitter);

    QSignalBlocker bl(mParticleEmittersTree);
    QTreeWidgetItem* item = new QTreeWidgetItem();
    mParticleEmittersTree->addTopLevelItem(item);
    fillEmitterAffectorsToGui(item, emitter);

    setEmitterNames();

    mParticleEmittersTree->setCurrentItem(mParticleEmittersTree->topLevelItem(mParticleEmittersTree->topLevelItemCount()-1));

    emit emitterChanged();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::pasteAffectorAction()
{
    if(!mParticleEmittersTree->currentItem()) {
        return;
    }

    int emitterIndex = getCurrentEmitterCoreIndex();
    if(emitterIndex < 0 || emitterIndex >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    const GpuParticleAffector* clipboardAffector = data.mGpuParticleAffectorClipboard;
    if(!clipboardAffector) {
        QMessageBox::critical(this, tr("Creating affector failed!"), tr("No affector in clipboard!"));
        return;
    }

    GpuParticleEmitter* emitter = const_cast<GpuParticleEmitter*>(mGpuParticleSystem->getEmitters()[emitterIndex]);
    if(emitter->getAffectorNoThrow(clipboardAffector->getAffectorProperty())) {
        emitter->removeAndDestroyAffector(clipboardAffector->getAffectorProperty());
    }

    GpuParticleAffector* affector = data.pasteToGpuParticleAffector();
    emitter->addAffector(affector);

    {
        QSignalBlocker bl(mParticleEmittersTree);

        QTreeWidgetItem* item = mParticleEmittersTree->topLevelItem(emitterIndex);

        bool found = false;
        for (int i = 0; i < item->childCount(); ++i) {
            Ogre::String type = item->child(i)->data(0, Qt::UserRole).toString().toStdString();

            if(type == affector->getAffectorProperty()) {
                found = true;
                break;
            }
        }

        if(!found) {
            QTreeWidgetItem* affectorItem = new QTreeWidgetItem();
            affectorItem->setText(0, QString::fromStdString(affector->getAffectorProperty()));
            affectorItem->setData(0, Qt::UserRole, QString::fromStdString(affector->getAffectorProperty()));
            item->addChild(affectorItem);
            item->sortChildren(0, Qt::AscendingOrder);
        }
    }

    emit emitterChanged();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::removeAction()
{
    if(!mParticleEmittersTree->currentItem()) {
        return;
    }

    int emitterIndex = getCurrentEmitterCoreIndex();
    if(emitterIndex < 0 || emitterIndex >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    data.mRenderer->stopParticleSystem();

    if(!mParticleEmittersTree->currentItem()->parent()) {

        // remove emitter
        GpuParticleEmitter* emitter = mGpuParticleSystem->_takeEmitter(emitterIndex);
        OGRE_DELETE emitter;

        QSignalBlocker bl(mParticleEmittersTree);
        delete mParticleEmittersTree->takeTopLevelItem(emitterIndex);

        setEmitterNames();
    }
    else {

        // remove affector

        int affectorIndex = mParticleEmittersTree->currentIndex().row();
        QTreeWidgetItem* affectorItem = mParticleEmittersTree->currentItem();

        Ogre::String type = affectorItem->data(0, Qt::UserRole).toString().toStdString();

        GpuParticleEmitter* emitter = const_cast<GpuParticleEmitter*>(mGpuParticleSystem->getEmitters()[emitterIndex]);
        emitter->removeAndDestroyAffector(type);

        QSignalBlocker bl(mParticleEmittersTree);
        delete mParticleEmittersTree->topLevelItem(emitterIndex)->takeChild(affectorIndex);
    }

    emit emitterChanged();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::moveEmitterUpAction()
{
    // up is previous
    if(!mParticleEmittersTree->currentItem() || mParticleEmittersTree->currentItem()->parent() ) {
        return;
    }

    QModelIndex currentIndex = mParticleEmittersTree->currentIndex();
    int currentRow = currentIndex.row();
    if(currentRow < 1 || currentRow >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    GpuParticleEmitter* emitter = mGpuParticleSystem->_takeEmitter(currentRow);
    mGpuParticleSystem->_insertEmitter(currentRow-1, emitter);

    QSignalBlocker bl(mParticleEmittersTree);

    QTreeWidgetItem* item = mParticleEmittersTree->takeTopLevelItem(currentRow);
    mParticleEmittersTree->insertTopLevelItem(currentRow-1, item);
    mParticleEmittersTree->setCurrentItem(mParticleEmittersTree->topLevelItem(currentRow-1));

    setEmitterNames();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::moveEmitterDownAction()
{
    // down is next
    if(!mParticleEmittersTree->currentItem() || mParticleEmittersTree->currentItem()->parent() ) {
        return;
    }

    QModelIndex currentIndex = mParticleEmittersTree->currentIndex();
    int currentRow = currentIndex.row();
    if(currentRow < 0 || currentRow+1 >= (int)mGpuParticleSystem->getEmitters().size()) {
        return;
    }

    GpuParticleEmitter* emitter = mGpuParticleSystem->_takeEmitter(currentRow);
    mGpuParticleSystem->_insertEmitter(currentRow+1, emitter);

    QSignalBlocker bl(mParticleEmittersTree);

    QTreeWidgetItem* item = mParticleEmittersTree->takeTopLevelItem(currentRow);
    mParticleEmittersTree->insertTopLevelItem(currentRow+1, item);
    mParticleEmittersTree->setCurrentItem(mParticleEmittersTree->topLevelItem(currentRow+1));

    setEmitterNames();

    data.mRenderer->restartParticleSystem();
}

void GpuParticleSystemTreeWidget::onContextMenuRequested(const QPoint& pos)
{
    int emitterIndex = getCurrentEmitterCoreIndex();
    mCreateAffectorMenu->setEnabled(emitterIndex >= 0);

    mParticleEmittersTreeContextMenu->exec(mapToGlobal(pos));
}

void GpuParticleSystemTreeWidget::createAffectorAction(QAction* action)
{
    int emitterIndex = getCurrentEmitterCoreIndex();
    if(!mGpuParticleSystem || emitterIndex < 0 || action->isSeparator()) {
        return;
    }

    Ogre::String actionType = action->data().toString().toStdString();

    GpuParticleSystemResourceManager& gpuParticleSystemResourceManager = GpuParticleSystemResourceManager::getSingleton();
    const GpuParticleSystemResourceManager::AffectorByIdStringMap& registeredAffectors = gpuParticleSystemResourceManager.getAffectorByPropertyMap();
    GpuParticleSystemResourceManager::AffectorByIdStringMap::const_iterator it = registeredAffectors.find(actionType);
    if(it == registeredAffectors.end()) {
        return;
    }

    const GpuParticleAffector* affector = it->second;
    GpuParticleEmitter* emitter = const_cast<GpuParticleEmitter*>(mGpuParticleSystem->getEmitters()[emitterIndex]);

    if(emitter->getAffectorNoThrow(affector->getAffectorProperty())) {
        QMessageBox::critical(this, tr("Creating affector failed!"), tr("Emitter already contains this affector!"));
        return;
    }

    emitter->addAffector(affector->clone());

    {
        QSignalBlocker bl(mParticleEmittersTree);

        QTreeWidgetItem* item = mParticleEmittersTree->topLevelItem(emitterIndex);

        QTreeWidgetItem* affectorItem = new QTreeWidgetItem();
        affectorItem->setText(0, QString::fromStdString(affector->getAffectorProperty()));
        affectorItem->setData(0, Qt::UserRole, QString::fromStdString(affector->getAffectorProperty()));
        item->addChild(affectorItem);
        item->sortChildren(0, Qt::AscendingOrder);
    }
}
