/*
 * File: GpuParticleEmitterWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#include "ParticleEditorData.h"
#include "GpuParticleEmitterWidget.h"

#include <ColourEditField.h>
#include <EulerDegreeHVSWidget.h>
#include <Point3dWidget.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <RangeWidget.h>
#include <QToolButton>
#include <QGroupBox>
#include <ColourButton.h>
#include <QTableView>
#include <SpriteTrackTableWidget.h>
#include <QPushButton>
#include <OgreHlmsManager.h>
#include <OgreRoot.h>

#include <GpuParticles/GpuParticleEmitter.h>
#include <GpuParticles/GpuParticleSystem.h>

#include <GpuParticles/Hlms/HlmsParticleDatablock.h>

#include "ParticleEditorAssets.h"

GpuParticleEmitterWidget::GpuParticleEmitterWidget(ParticleEditorData& _data)
    : data(_data)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();

    QGridLayout* grid = new QGridLayout();

    int row = 0;

    GpuParticleEmitter dummy;

    {
        grid->addWidget(new QLabel(tr("Mode:")), row, 0, 1, 1, Qt::AlignRight);
        mModeCombo = new QComboBox();
        mModeCombo->addItem("Looped");
        mModeCombo->addItem("Burst");
        mModeCombo->setCurrentIndex(0);
        connect(mModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(emitterModeChanged()));
        grid->addWidget(mModeCombo, row++, 1, 1, 1);
    }

    {
        mBurstButton = new QPushButton(tr("Burst"));
        connect(mBurstButton, SIGNAL(clicked(bool)), this, SLOT(burstButtonClicked()));
        grid->addWidget(mBurstButton, row++, 1, 1, 1);
    }

    {
        mEmissionRateLabel = new QLabel("Emission rate:");
        grid->addWidget(mEmissionRateLabel, row, 0, 1, 1, Qt::AlignRight);

        mEmissionRateEdit = new QDoubleSpinBox();
        mEmissionRateEdit->setRange(0.0, 2048.0);
        mEmissionRateEdit->setSingleStep(1.0);
        mEmissionRateEdit->setValue(dummy.mEmissionRate);
        connect(mEmissionRateEdit, SIGNAL(valueChanged(double)), this, SLOT(emitterModified()));
        grid->addWidget(mEmissionRateEdit, row++, 1, 1, 1);
    }

    {
        mBurstParticlesLabel = new QLabel("Burst particles:");
        grid->addWidget(mBurstParticlesLabel, row, 0, 1, 1, Qt::AlignRight);

        mBurstParticlesEdit = new QSpinBox();
        mBurstParticlesEdit->setRange(0, 10000);
        mBurstParticlesEdit->setSingleStep(1);
        mBurstParticlesEdit->setValue(dummy.mBurstParticles);
        connect(mBurstParticlesEdit, SIGNAL(valueChanged(int)), this, SLOT(emitterModified()));
        grid->addWidget(mBurstParticlesEdit, row++, 1, 1, 1);
    }

    {
        mEmitterLifetimeLabel = new QLabel("Emitter lifetime:");
        grid->addWidget(mEmitterLifetimeLabel, row, 0, 1, 1, Qt::AlignRight);

        mEmitterLifetimeEdit = new QDoubleSpinBox();
        mEmitterLifetimeEdit->setRange(0.0, 100.0);
        mEmitterLifetimeEdit->setSingleStep(0.01);
        mEmitterLifetimeEdit->setValue(dummy.mEmitterLifetime);
        connect(mEmitterLifetimeEdit, SIGNAL(valueChanged(double)), this, SLOT(emitterModified()));
        grid->addWidget(mEmitterLifetimeEdit, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Particle lifetime:")), row, 0, 1, 1, Qt::AlignRight);

        mParticleLifetimeRangeWidget = new RangeWidget();
        mParticleLifetimeRangeWidget->setValue(Geometry::Range(dummy.mParticleLifetimeMin, dummy.mParticleLifetimeMax), false);
        connect(mParticleLifetimeRangeWidget, SIGNAL(valueChanged(float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mParticleLifetimeRangeWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Pos:")), row, 0, 1, 1, Qt::AlignRight);
        mPos3dWidget = new Point3dWidget();
        connect(mPos3dWidget, SIGNAL(valueChanged(float,float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mPos3dWidget, row++, 1, 1, 1);
    }
    {
        grid->addWidget(new QLabel(tr("Rot (YXZ):")), row, 0, 1, 1, Qt::AlignRight);
        mEulerRotWidget = new EulerDegreeHVSWidget(true);
        connect(mEulerRotWidget, SIGNAL(valueChanged(float,float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mEulerRotWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Datablock:")), row, 0, 1, 1, Qt::AlignRight);
        mDatablockCombo = new QComboBox();
        connect(mDatablockCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(datablockChanged()));
        mDatablockCombo->setIconSize(QSize(64, 64));
        mDatablockCombo->setMinimumHeight(64);
//        refreshDatablocks();
        grid->addWidget(mDatablockCombo, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Sprite mode:")), row, 0, 1, 1, Qt::AlignRight);
        mSpriteModeCombo = new QComboBox();
        mSpriteModeCombo->addItem("None", QVariant((int)GpuParticleEmitter::SpriteMode::None));
        mSpriteModeCombo->addItem("ChangeWithTrack", QVariant((int)GpuParticleEmitter::SpriteMode::ChangeWithTrack));
        mSpriteModeCombo->addItem("SetWithStart", QVariant((int)GpuParticleEmitter::SpriteMode::SetWithStart));
        mSpriteModeCombo->setCurrentIndex(0);
        connect(mSpriteModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(spriteModeChanged()));
        grid->addWidget(mSpriteModeCombo, row++, 1, 1, 1);
    }

    const int MinimumTrackHeight = 200;

    {
        mSpriteTrackGroup = new QGroupBox();
        mSpriteTrackGroup->setTitle("Sprite track");
        connect(mSpriteTrackGroup, SIGNAL(clicked(bool)), this, SLOT(emitterModified()));

        {
            QVBoxLayout* layout = new QVBoxLayout();

//            mShowSpriteTrackButton = new QToolButton();
//            mShowSpriteTrackButton->setArrowType(Qt::DownArrow);
//            layout->addWidget(mShowSpriteTrackButton);

            {
                mSpriteTrackTableWidget = new SpriteTrackTableWidget();
                mSpriteTrackTableWidget->setMaxCount(GpuParticleEmitter::MaxTrackValues);
                mSpriteTrackTableWidget->setMinimumHeight(MinimumTrackHeight);

                connect(mSpriteTrackTableWidget, SIGNAL(dataModified()), this, SLOT(emitterModified()));
                layout->addWidget(mSpriteTrackTableWidget);
            }

            mSpriteTrackGroup->setLayout(layout);
        }

        grid->addWidget(mSpriteTrackGroup, row++, 0, 1, 2);
    }

    {
        grid->addWidget(new QLabel(tr("Billboard type:")), row, 0, 1, 1, Qt::AlignRight);
        mBillboardTypeCombo = new QComboBox();
        mBillboardTypeCombo->addItem("Point (default)", QVariant(Ogre::v1::BBT_POINT));
        mBillboardTypeCombo->addItem("OrientedCommon (velocity dir)", QVariant(Ogre::v1::BBT_ORIENTED_COMMON));
        mBillboardTypeCombo->addItem("OrientedSelf", QVariant(Ogre::v1::BBT_ORIENTED_SELF));
        mBillboardTypeCombo->addItem("PerpendicularCommon", QVariant(Ogre::v1::BBT_PERPENDICULAR_COMMON));
        mBillboardTypeCombo->addItem("PerpendicularSelf", QVariant(Ogre::v1::BBT_PERPENDICULAR_SELF));
        mBillboardTypeCombo->setCurrentIndex(0);
        connect(mBillboardTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(emitterModified()));
        grid->addWidget(mBillboardTypeCombo, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Spawn shape:")), row, 0, 1, 1, Qt::AlignRight);
        mSpawnShapeCombo = new QComboBox();
        mSpawnShapeCombo->addItem("Point", QVariant((int)GpuParticleEmitter::SpawnShape::Point));
        mSpawnShapeCombo->addItem("Box", QVariant((int)GpuParticleEmitter::SpawnShape::Box));
        mSpawnShapeCombo->addItem("Sphere", QVariant((int)GpuParticleEmitter::SpawnShape::Sphere));
        mSpawnShapeCombo->addItem("Disc", QVariant((int)GpuParticleEmitter::SpawnShape::Disc));
        mSpawnShapeCombo->setCurrentIndex(0);
        connect(mSpawnShapeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(emitterModified()));
        grid->addWidget(mSpawnShapeCombo, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Spawn dimensions:")), row, 0, 1, 1, Qt::AlignRight);
        mSpawnShapeDimensionsWidget = new Point3dWidget();
        mSpawnShapeDimensionsWidget->setValue(dummy.mSpawnShapeDimensions, false);
        connect(mSpawnShapeDimensionsWidget, SIGNAL(valueChanged(float,float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mSpawnShapeDimensionsWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("ColourA:")), row, 0, 1, 1, Qt::AlignRight);
        mColourAEditField = new ColourEditField();
        mColourAEditField->setColour(dummy.mColourA);
        connect(mColourAEditField, SIGNAL(colorChanged()), this, SLOT(emitterModified()));
        grid->addWidget(mColourAEditField, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("ColourB:")), row, 0, 1, 1, Qt::AlignRight);
        mColourBEditField = new ColourEditField();
        mColourBEditField->setColour(dummy.mColourB);
        connect(mColourBEditField, SIGNAL(colorChanged()), this, SLOT(emitterModified()));
        grid->addWidget(mColourBEditField, row++, 1, 1, 1);
    }

    {
        mUniformSizeCheckBox = new QCheckBox("Uniform size");
        mUniformSizeCheckBox->setChecked(dummy.mUniformSize);
        connect(mUniformSizeCheckBox, SIGNAL(clicked(bool)), this, SLOT(emitterModified()));
        grid->addWidget(mUniformSizeCheckBox, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Size:")), row, 0, 1, 1, Qt::AlignRight);

        mSizeXRangeWidget = new RangeWidget();
        mSizeXRangeWidget->setValue(Geometry::Range(dummy.mSizeMin, dummy.mSizeMax), false);
        connect(mSizeXRangeWidget, SIGNAL(valueChanged(float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mSizeXRangeWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("SizeY:")), row, 0, 1, 1, Qt::AlignRight);

        mSizeYRangeWidget = new RangeWidget();
        mSizeYRangeWidget->setValue(Geometry::Range(dummy.mSizeYMin, dummy.mSizeYMax), false);
        connect(mSizeYRangeWidget, SIGNAL(valueChanged(float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mSizeYRangeWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Direction:")), row, 0, 1, 1, Qt::AlignRight);
        mDirectionWidget = new Point3dWidget();
        mDirectionWidget->setValue(dummy.mDirection, false);
        connect(mDirectionWidget, SIGNAL(valueChanged(float,float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mDirectionWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Spot angle:")), row, 0, 1, 1, Qt::AlignRight);

        mSpotAngleRangeWidget = new RangeWidget();
        mSpotAngleRangeWidget->setValue(Geometry::Range(dummy.mSpotAngleMin, dummy.mSpotAngleMax), false);
        connect(mSpotAngleRangeWidget, SIGNAL(valueChanged(float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mSpotAngleRangeWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Direction velocity:")), row, 0, 1, 1, Qt::AlignRight);

        mDirectionVelocityRangeWidget = new RangeWidget();
        mDirectionVelocityRangeWidget->setValue(Geometry::Range(dummy.mDirectionVelocityMin, dummy.mDirectionVelocityMax), false);
        connect(mDirectionVelocityRangeWidget, SIGNAL(valueChanged(float,float)), this, SLOT(emitterModified()));
        grid->addWidget(mDirectionVelocityRangeWidget, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel(tr("Fader mode:")), row, 0, 1, 1, Qt::AlignRight);
        mFaderModeCombo = new QComboBox();
        mFaderModeCombo->addItem("None", QVariant((int)GpuParticleEmitter::FaderMode::None));
        mFaderModeCombo->addItem("Enabled", QVariant((int)GpuParticleEmitter::FaderMode::Enabled));
        mFaderModeCombo->addItem("AlphaOnly", QVariant((int)GpuParticleEmitter::FaderMode::AlphaOnly));
        mFaderModeCombo->setCurrentIndex(0);
        connect(mFaderModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(emitterModified()));
        grid->addWidget(mFaderModeCombo, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel("Fader start:"), row, 0, 1, 1, Qt::AlignRight);

        mFaderStartEdit = new QDoubleSpinBox();
        mFaderStartEdit->setRange(0.0, 100.0);
        mFaderStartEdit->setSingleStep(0.01);
        mFaderStartEdit->setValue(dummy.mParticleFaderStartTime);
        connect(mFaderStartEdit, SIGNAL(valueChanged(double)), this, SLOT(emitterModified()));
        grid->addWidget(mFaderStartEdit, row++, 1, 1, 1);
    }

    {
        grid->addWidget(new QLabel("Fader end:"), row, 0, 1, 1, Qt::AlignRight);

        mFaderEndEdit = new QDoubleSpinBox();
        mFaderEndEdit->setRange(0.0, 100.0);
        mFaderEndEdit->setSingleStep(0.01);
        mFaderEndEdit->setValue(dummy.mParticleFaderEndTime);
        connect(mFaderEndEdit, SIGNAL(valueChanged(double)), this, SLOT(emitterModified()));
        grid->addWidget(mFaderEndEdit, row++, 1, 1, 1);
    }

    mainLayout->addLayout(grid);
    QSpacerItem* spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addItem(spacerItem);

    setLayout(mainLayout);

    setModeFieldEnabled();

    setEnabled(false);
}

void GpuParticleEmitterWidget::setEditedObject(GpuParticleSystem* core, int index)
{
    mEditedCore = core;
    mEditedIndex = index;
    if(core && index >= 0 && (size_t)index < core->getEmitters().size()) {
        mEditedObject = const_cast<GpuParticleEmitter*>(core->getEmitters()[index]);
    }
    else {
        mEditedObject = nullptr;
    }

    setEnabled(mEditedObject != nullptr);

    emitterToGui();
}

void GpuParticleEmitterWidget::updateSpriteTrackGui()
{
    HlmsParticleDatablock::SpriteCoord flipbookSize(0, 0);
    QVector<QPixmap> icons;
    QStringList spriteNameList;
    if(!mEditedObject) {
        mSpriteTrackTableWidget->setSpriteList(flipbookSize, icons, spriteNameList);
        return;
    }

    QString str = QString::fromStdString(mEditedObject->mDatablockName);
    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsDatablock* datablock = hlmsManager->getDatablockNoDefault(mEditedObject->mDatablockName);

    HlmsParticleDatablock* particleDatablock = dynamic_cast<HlmsParticleDatablock*>(datablock);
    if(particleDatablock) {

        flipbookSize = particleDatablock->getFlipbookSize();
        if(!particleDatablock->getIsFlipbook()) {
            flipbookSize.row = 1;
            flipbookSize.col = (Ogre::uint8)particleDatablock->getSprites().size();
        }

        size_t count = flipbookSize.row*flipbookSize.col;
        spriteNameList.reserve(count);
        if(particleDatablock->getIsFlipbook()) {
            for(size_t i = 0; i < count; ++i) {
                spriteNameList.push_back(QString());
            }
        }
        else {
            const HlmsParticleDatablock::SpriteList& spriteList = particleDatablock->getSprites();
            for(HlmsParticleDatablock::SpriteList::const_iterator it = spriteList.begin();
                it != spriteList.end(); ++it) {

                Ogre::String spriteName = it->mName;
                QString spriteNameStr = QString::fromStdString(spriteName);
                spriteNameList.push_back(spriteNameStr);
            }
        }

        if(data.mParticleEditorAssets->mDatablocks.contains(str)) {
            ParticleEditorAssets::DatablockData& textureData = data.mParticleEditorAssets->mDatablocks[str];
            icons = textureData.mSprites;
        }
    }
    mSpriteTrackTableWidget->setSpriteList(flipbookSize, icons, spriteNameList);
    mSpriteTrackTableWidget->refreshSpriteColumn();
}

void GpuParticleEmitterWidget::updateEnableSpriteTrackGui()
{
    bool isEnabled = mEditedObject &&
            (mEditedObject->mSpriteMode == GpuParticleEmitter::SpriteMode::ChangeWithTrack ||
             mEditedObject->mSpriteMode == GpuParticleEmitter::SpriteMode::SetWithStart);
    mSpriteTrackGroup->setEnabled(isEnabled);
}

void GpuParticleEmitterWidget::emitterToGui()
{
    GpuParticleEmitter* object = mEditedObject;
    GpuParticleEmitter dummy;
    if(!object) {
        object = &dummy;
    }

    {
        QSignalBlocker bl(mModeCombo);
        int modeIndex = object->mBurstMode ? 1 : 0;
        mModeCombo->setCurrentIndex(modeIndex);
        setModeFieldEnabled();
    }

    { QSignalBlocker bl(mPos3dWidget); mPos3dWidget->setValue(object->mPos, false); }
    {
        QSignalBlocker bl(mEulerRotWidget);
        EulerDegreeHVS euler;
        euler.fromQuaternion(object->mRot);
        mEulerRotWidget->setValue(euler, false);
    }

    { QSignalBlocker bl(mBurstParticlesEdit); mBurstParticlesEdit->setValue(object->mBurstParticles); }
    { QSignalBlocker bl(mEmitterLifetimeEdit); mEmitterLifetimeEdit->setValue(object->mEmitterLifetime); }
    { QSignalBlocker bl(mEmissionRateEdit); mEmissionRateEdit->setValue(object->mEmissionRate); }
    { QSignalBlocker bl(mColourAEditField); mColourAEditField->setColour(object->mColourA); }
    { QSignalBlocker bl(mColourBEditField); mColourBEditField->setColour(object->mColourB); }
    { QSignalBlocker bl(mUniformSizeCheckBox); mUniformSizeCheckBox->setChecked(object->mUniformSize); }
    { QSignalBlocker bl(mSizeXRangeWidget); mSizeXRangeWidget->setValue(Geometry::Range(object->mSizeMin, object->mSizeMax), false); }
    { QSignalBlocker bl(mSizeYRangeWidget); mSizeYRangeWidget->setValue(Geometry::Range(object->mSizeYMin, object->mSizeYMax), false); }
    { QSignalBlocker bl(mDirectionWidget); mDirectionWidget->setValue(object->mDirection, false); }
    { QSignalBlocker bl(mParticleLifetimeRangeWidget); mParticleLifetimeRangeWidget->setValue(Geometry::Range(object->mParticleLifetimeMin, object->mParticleLifetimeMax), false); }
    { QSignalBlocker bl(mSpotAngleRangeWidget); mSpotAngleRangeWidget->setValue(Geometry::Range(object->mSpotAngleMin, object->mSpotAngleMax), false); }
    { QSignalBlocker bl(mDirectionVelocityRangeWidget); mDirectionVelocityRangeWidget->setValue(Geometry::Range(object->mDirectionVelocityMin, object->mDirectionVelocityMax), false); }

    {
        QSignalBlocker bl(mBillboardTypeCombo);
        int modeIndex = (int)(object->mBillboardType);
        int index = mBillboardTypeCombo->findData(QVariant(modeIndex));
        mBillboardTypeCombo->setCurrentIndex(index);
    }

    {
        QSignalBlocker bl(mDatablockCombo);
        QString str = QString::fromStdString(object->mDatablockName);
        int index = mDatablockCombo->findText(str);
        mDatablockCombo->setCurrentIndex(index);

        updateEnableSpriteTrackGui();
        updateSpriteTrackGui();
    }

    {
        QSignalBlocker bl(mFaderModeCombo);
        int modeIndex = (int)(object->mFaderMode);
        int index = mFaderModeCombo->findData(QVariant(modeIndex));
        mFaderModeCombo->setCurrentIndex(index);
    }
    { QSignalBlocker bl(mFaderStartEdit); mFaderStartEdit->setValue(object->mParticleFaderStartTime); }
    { QSignalBlocker bl(mFaderEndEdit); mFaderEndEdit->setValue(object->mParticleFaderEndTime); }

    {
        QSignalBlocker bl(mSpriteModeCombo);
        int modeIndex = (int)(object->mSpriteMode);
        int index = mSpriteModeCombo->findData(QVariant(modeIndex));
        mSpriteModeCombo->setCurrentIndex(index);
    }

    {
        QSignalBlocker bl(mSpawnShapeCombo);
        int modeIndex = (int)(object->mSpawnShape);
        int index = mSpawnShapeCombo->findData(QVariant(modeIndex));
        mSpawnShapeCombo->setCurrentIndex(index);
    }
    { QSignalBlocker bl(mSpawnShapeDimensionsWidget); mSpawnShapeDimensionsWidget->setValue(object->mSpawnShapeDimensions, true); }

    {
        QVector<QPair<float, HlmsParticleDatablock::SpriteCoord> > values;

        for (size_t i = 0; i < object->mSpriteFlipbookCoords.size(); ++i) {
            values.push_back(qMakePair(object->mSpriteTimes[i], object->mSpriteFlipbookCoords[i]));
        }

        QSignalBlocker bl(mSpriteTrackTableWidget);
        mSpriteTrackTableWidget->mTrackTableModel.setValues(values);
    }

}

void GpuParticleEmitterWidget::setModeFieldEnabled()
{
    int index = mModeCombo->currentIndex();
    bool burstMode = (index == 1);

    mEmissionRateLabel->setEnabled(!burstMode);
    mEmissionRateEdit->setEnabled(!burstMode);

    mBurstParticlesLabel->setEnabled(burstMode);
    mBurstParticlesEdit->setEnabled(burstMode);

    mEmitterLifetimeLabel->setEnabled(burstMode);
    mEmitterLifetimeEdit->setEnabled(burstMode);

    mBurstButton->setEnabled(burstMode);
}

void GpuParticleEmitterWidget::setDatablockList(const QStringList& datablockList, const QVector<QPixmap>& icons)
{
    QSignalBlocker bl(mDatablockCombo);
    QString currentText = mDatablockCombo->currentText();
    mDatablockCombo->clear();

    for (int i = 0; i < datablockList.size(); ++i) {
        mDatablockCombo->addItem(icons[i], datablockList[i]);
    }

    int index = mDatablockCombo->findText(currentText);
    mDatablockCombo->setCurrentIndex(index);
}

void GpuParticleEmitterWidget::emitterModeChanged()
{
    setModeFieldEnabled();

    emitterModified();
}

void GpuParticleEmitterWidget::emitterModified()
{
    if(!mEditedObject) {
        return;
    }

    int modeIndex = mModeCombo->currentIndex();
    bool isBurst = modeIndex == 1;

    if(isBurst) {
        mEditedObject->setBurst(mBurstParticlesEdit->value(), mEmitterLifetimeEdit->value());
    }
    else {
        mEditedObject->setLooped(mEmissionRateEdit->value());
    }

    mEditedObject->mPos = mPos3dWidget->getValue();
    mEditedObject->mRot = mEulerRotWidget->getValue().toQuaternion();

    mEditedObject->mColourA = mColourAEditField->getColour();
    mEditedObject->mColourB = mColourBEditField->getColour();
    mEditedObject->mUniformSize = mUniformSizeCheckBox->isChecked();
    mEditedObject->mSizeMin = mSizeXRangeWidget->getValue().getMin();
    mEditedObject->mSizeMax = mSizeXRangeWidget->getValue().getMax();
    mEditedObject->mSizeYMin = mSizeYRangeWidget->getValue().getMin();
    mEditedObject->mSizeYMax = mSizeYRangeWidget->getValue().getMax();
    mEditedObject->mDirection = mDirectionWidget->getValue();
    mEditedObject->mParticleLifetimeMin = mParticleLifetimeRangeWidget->getValue().getMin();
    mEditedObject->mParticleLifetimeMax = mParticleLifetimeRangeWidget->getValue().getMax();
    mEditedObject->mSpotAngleMin = mSpotAngleRangeWidget->getValue().getMin();
    mEditedObject->mSpotAngleMax = mSpotAngleRangeWidget->getValue().getMax();
    mEditedObject->mDirectionVelocityMin = mDirectionVelocityRangeWidget->getValue().getMin();
    mEditedObject->mDirectionVelocityMax = mDirectionVelocityRangeWidget->getValue().getMax();

//    {
//        QString str = mDatablockCombo->currentText();
//        mEditedObject->mDatablockName = str.toStdString();
//    }

    QVariant billboardData = mBillboardTypeCombo->currentData();
    if(billboardData.isValid()) {
        mEditedObject->mBillboardType = (Ogre::v1::BillboardType)(billboardData.toInt());
    }
    else {
        mEditedObject->mBillboardType = Ogre::v1::BBT_POINT;
    }

    QVariant faderModeData = mFaderModeCombo->currentData();
    if(faderModeData.isValid()) {
        mEditedObject->mFaderMode = (GpuParticleEmitter::FaderMode)(faderModeData.toInt());
    }
    else {
        mEditedObject->mFaderMode = GpuParticleEmitter::FaderMode::None;
    }
    mEditedObject->mParticleFaderStartTime = mFaderStartEdit->value();
    mEditedObject->mParticleFaderEndTime = mFaderEndEdit->value();

    QVariant spawnShapeData = mSpawnShapeCombo->currentData();
    if(spawnShapeData.isValid()) {
        mEditedObject->mSpawnShape = (GpuParticleEmitter::SpawnShape)(spawnShapeData.toInt());
    }
    else {
        mEditedObject->mSpawnShape = GpuParticleEmitter::SpawnShape::Point;
    }
    mEditedObject->mSpawnShapeDimensions = mSpawnShapeDimensionsWidget->getValue();

    {
        QVector<QPair<float, HlmsParticleDatablock::SpriteCoord> > values = mSpriteTrackTableWidget->mTrackTableModel.getValues();
        mEditedObject->mSpriteTimes.clear();
        mEditedObject->mSpriteFlipbookCoords.clear();
        for (int i = 0; i < values.size(); ++i) {
            float time = values[i].first;
            const HlmsParticleDatablock::SpriteCoord& coord = values[i].second;

            mEditedObject->mSpriteTimes.push_back(time);
            mEditedObject->mSpriteFlipbookCoords.push_back(coord);
        }
    }

    emit emitterCoreModified();
}

void GpuParticleEmitterWidget::datablockChanged()
{
    QString str = mDatablockCombo->currentText();
    mEditedObject->mDatablockName = str.toStdString();

    updateSpriteTrackGui();

//    emitterModified();

    emit emitterCoreModified();
}

void GpuParticleEmitterWidget::burstButtonClicked()
{
    emit emitterCoreModified();
}

void GpuParticleEmitterWidget::spriteModeChanged()
{
    QVariant spriteModeData = mSpriteModeCombo->currentData();
    if(spriteModeData.isValid()) {
        mEditedObject->mSpriteMode = (GpuParticleEmitter::SpriteMode)(spriteModeData.toInt());
    }
    else {
        mEditedObject->mSpriteMode = GpuParticleEmitter::SpriteMode::None;
    }

    updateEnableSpriteTrackGui();

//    emitterModified();

    emit emitterCoreModified();
}
