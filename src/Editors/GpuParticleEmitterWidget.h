/*
 * File: GpuParticleEmitterWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#ifndef PARTICLEEMITTERWIDGET_H
#define PARTICLEEMITTERWIDGET_H

#include <QWidget>
#include <TrackTableModel.h>

#include <GpuParticles/GpuParticleEmitter.h>

class QLabel;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QToolButton;
class QGroupBox;

class Point3dWidget;
class EulerDegreeHVSWidget;
class ColourEditField;
class RangeWidget;
class ColourTrackTableWidget;
class FloatTrackTableWidget;
class Point2dTrackTableWidget;
class SpriteTrackTableWidget;
class QPushButton;

class GpuParticleSystem;
class GpuParticleEmitter;
class ParticleEditorData;

class GpuParticleEmitterWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleEmitterWidget(ParticleEditorData& _data);

    void setEditedObject(GpuParticleSystem* core, int index);
    void emitterToGui();
    void setModeFieldEnabled();

private:

    GpuParticleSystem* mEditedCore;
    int mEditedIndex;
    GpuParticleEmitter* mEditedObject = nullptr;

    QComboBox* mModeCombo;
    QComboBox* mDatablockCombo;

    QComboBox* mSpriteModeCombo;

    QLabel* mEmissionRateLabel;
    QDoubleSpinBox* mEmissionRateEdit;

    QLabel* mBurstParticlesLabel;
    QSpinBox* mBurstParticlesEdit;

    QLabel* mEmitterLifetimeLabel;
    QDoubleSpinBox* mEmitterLifetimeEdit;

    QPushButton* mBurstButton;

    QComboBox* mBillboardTypeCombo;


    QComboBox* mSpawnShapeCombo;
    Point3dWidget* mSpawnShapeDimensionsWidget;

    // TODO:
    // mSpriteMode, mSpriteNames, mSpriteTimes

    Point3dWidget* mPos3dWidget;
    EulerDegreeHVSWidget* mEulerRotWidget;
    ColourEditField* mColourAEditField;
    ColourEditField* mColourBEditField;

    QCheckBox* mUniformSizeCheckBox;
    RangeWidget* mSizeXRangeWidget;
    RangeWidget* mSizeYRangeWidget;

    QCheckBox* mUseDepthCollisionsCheckBox;


    Point3dWidget* mDirectionWidget;
    Point3dWidget* mGravityWidget;

    RangeWidget* mParticleLifetimeRangeWidget;
    RangeWidget* mSpotAngleRangeWidget;
    RangeWidget* mDirectionVelocityRangeWidget;

    QComboBox* mFaderModeCombo;
    QDoubleSpinBox* mFaderStartEdit;
    QDoubleSpinBox* mFaderEndEdit;

    QGroupBox* mSpriteTrackGroup;
//    QToolButton* mShowSpriteTrackButton;
    SpriteTrackTableWidget* mSpriteTrackTableWidget;

    QGroupBox* mColourTrackGroup;
//    QToolButton* mShowColourTrackButton;
    ColourTrackTableWidget* mColourTrackTableWidget;

    QGroupBox* mAlphaTrackGroup;
//    QToolButton* mShowAlphaTrackButton;
    FloatTrackTableWidget* mAlphaTrackTableWidget;

    QGroupBox* mSizeTrackGroup;
//    QToolButton* mShowSizeTrackButton;
    Point2dTrackTableWidget* mSizeTrackTableWidget;

    QGroupBox* mVelocityTrackGroup;
//    QToolButton* mShowVelocityTrackButton;
    FloatTrackTableWidget* mVelocityTrackTableWidget;

    ParticleEditorData& data;

public:

    void setDatablockList(const QStringList& datablockList, const QVector<QPixmap>& icons);
    void updateSpriteTrackGui();
    void updateEnableSpriteTrackGui();

private slots:
    void emitterModeChanged();
    void emitterModified();
    void datablockChanged();
    void burstButtonClicked();
    void spriteModeChanged();

signals:
    void emitterCoreModified();
};

#endif
