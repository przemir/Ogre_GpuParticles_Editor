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
class ParticleEditorData;

class GpuParticleSystemTreeWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleSystemTreeWidget(ParticleEditorData& _data);

    void setEditedObject(GpuParticleSystem* core);
    void particleSystemToGui();
    int getCurrentEmitterCoreIndex() const;

    GpuParticleSystem* getGpuParticleSystem() const;
    bool showOnlySelectedEmitters() const;

private:
    ParticleEditorData& data;
    GpuParticleSystem* mGpuParticleSystem = nullptr;

    QListWidget* mParticleEmittersList;
    QCheckBox* mShowOnlySelectedEmitters;

private slots:
    void createEmitterAction();
    void copyEmitterAction();
    void pasteToNewEmitterAction();
    void removeEmitterAction();
    void moveEmitterUpAction();
    void moveEmitterDownAction();

public slots:
    void updateEmitterInvalidIcons();

signals:
    void emitterChanged();
    void showOnlySelectedEmittersChanged();
};

#endif
