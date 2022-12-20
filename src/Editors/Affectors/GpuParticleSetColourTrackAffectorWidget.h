/*
 * File: GpuParticleSetColourTrackAffectorWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-15
 *
 */

#ifndef GPUPARTICLESETCOLOURTRACKAFFECTORWIDGET_H
#define GPUPARTICLESETCOLOURTRACKAFFECTORWIDGET_H

#include "../GpuParticleAffectorWidgetTempl.h"

#include <GpuParticles/Affectors/GpuParticleSetColourTrackAffector.h>

class ColourTrackTableWidget;
class QGroupBox;

class GpuParticleSetColourTrackAffectorWidget : public GpuParticleAffectorWidgetTempl<GpuParticleSetColourTrackAffector>
{
    Q_OBJECT
public:
    GpuParticleSetColourTrackAffectorWidget();

    void createGui();

private:
    virtual void affectorToGui() override;

private slots:
    void onAffectorModified();

private:
    QGroupBox* mColourTrackGroup;
    ColourTrackTableWidget* mColourTrackTableWidget;
};

#endif
