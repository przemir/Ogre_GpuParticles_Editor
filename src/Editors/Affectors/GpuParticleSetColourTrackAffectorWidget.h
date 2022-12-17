/*
 * File: GpuParticleSetColourTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-15
 *
 */

#ifndef GPUPARTICLESETCOLOURTRACKAFFECTORWIDGET_H
#define GPUPARTICLESETCOLOURTRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidget.h"

class GpuParticleSetColourTrackAffector;
class ColourTrackTableWidget;
class QGroupBox;

class GpuParticleSetColourTrackAffectorWidget : public GpuParticleAffectorWidget
{
    Q_OBJECT
public:
    GpuParticleSetColourTrackAffectorWidget();

    void createGui();

    virtual void setEditedObject(GpuParticleAffector* affector) override;

private:
    void affectorToGui();

private slots:
    void onAffectorModified();

private:
    GpuParticleSetColourTrackAffector* mEditedObject = nullptr;

    QGroupBox* mColourTrackGroup;
//    QToolButton* mShowColourTrackButton;
    ColourTrackTableWidget* mColourTrackTableWidget;
};

#endif
