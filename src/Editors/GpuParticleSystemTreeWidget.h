/*
 * File: GpuParticleSystemTreeWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#ifndef PARTICLESYSTEMTREEWIDGET_H
#define PARTICLESYSTEMTREEWIDGET_H

#include <QWidget>
#include <QTreeWidget>

class QListWidget;
class QCheckBox;
class GpuParticleSystem;
class GpuParticleEmitter;
class GpuParticleAffector;
class ParticleEditorData;

class GpuParticleSystemTreeWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleSystemTreeWidget(ParticleEditorData& _data);

    void setEditedObject(GpuParticleSystem* core);
    void particleSystemToGui();
    int getCurrentEmitterCoreIndex() const;
    int getCurrentAffectorIndex() const;
    const GpuParticleAffector* getCurrentAffector();

    GpuParticleSystem* getGpuParticleSystem() const;
    bool showOnlySelectedEmitters() const;
    void createAffectorActions();

private:
    ParticleEditorData& data;
    GpuParticleSystem* mGpuParticleSystem = nullptr;

    QTreeWidget* mParticleEmittersTree;
    QCheckBox* mShowOnlySelectedEmitters;
    QMenu* mParticleEmittersTreeContextMenu;
    QMenu* mCreateAffectorMenu;

private:
    void setEmitterNames();
    void fillEmitterAffectorsToGui(QTreeWidgetItem* item, const GpuParticleEmitter* emitterCore);

private slots:
    void createEmitterAction();
    void copyAction();
    void pasteToNewEmitterAction();
    void pasteAffectorAction();
    void removeAction();
    void moveEmitterUpAction();
    void moveEmitterDownAction();
    void onContextMenuRequested(const QPoint& pos);
    void createAffectorAction(QAction* action);

public slots:
    void updateEmitterInvalidIcons();

signals:
    void emitterChanged();
    void showOnlySelectedEmittersChanged();
};

#endif
