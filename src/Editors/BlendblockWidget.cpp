/*
 * File: BlendblockWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#include "BlendblockWidget.h"

#include <OgreHlmsDatablock.h>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>

const QStringList BlendblockWidget::SceneBlendFactorNames = QStringList()
  << QString("SBF_ONE")
  << QString("SBF_ZERO")
  << QString("SBF_DEST_COLOUR")
  << QString("SBF_SOURCE_COLOUR")
  << QString("SBF_ONE_MINUS_DEST_COLOUR")
  << QString("SBF_ONE_MINUS_SOURCE_COLOUR")
  << QString("SBF_DEST_ALPHA")
  << QString("SBF_SOURCE_ALPHA")
  << QString("SBF_ONE_MINUS_DEST_ALPHA")
  << QString("SBF_ONE_MINUS_SOURCE_ALPHA");


const QStringList BlendblockWidget::SceneBlendOperationNames = QStringList()
  << QString("SBO_ADD")
  << QString("SBO_SUBTRACT")
  << QString("SBO_REVERSE_SUBTRACT")
  << QString("SBO_MIN")
  << QString("SBO_MAX");

const QStringList BlendblockWidget::SceneBlendTypeNames = QStringList()
  << QString("SBT_TRANSPARENT_ALPHA")
  << QString("SBT_TRANSPARENT_COLOUR")
  << QString("SBT_ADD")
  << QString("SBT_MODULATE")
  << QString("SBT_REPLACE");

BlendblockWidget::BlendblockWidget()
{
//    QGridLayout* gridLayout = new QGridLayout();
//    int row = 0;

//    createGui(gridLayout, row);

//    setLayout(gridLayout);
}

void BlendblockWidget::createGui(QGridLayout* gridLayout, int& row)
{
    {
        gridLayout->addWidget(new QLabel(tr("Alpha to coverage")), row, 0);
        mAlphaToCoverageComboBox = new QComboBox();

        mAlphaToCoverageComboBox->addItem("A2cDisabled", Ogre::HlmsBlendblock::A2cDisabled);
        mAlphaToCoverageComboBox->addItem("A2cEnabled", Ogre::HlmsBlendblock::A2cEnabled);
        mAlphaToCoverageComboBox->addItem("A2cEnabledMsaaOnly", Ogre::HlmsBlendblock::A2cEnabledMsaaOnly);

        connect(mAlphaToCoverageComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBlendblockModified()));
        gridLayout->addWidget(mAlphaToCoverageComboBox, row++, 1, 1, 1);
    }

    {
        gridLayout->addWidget(new QLabel(tr("Blend channel mask:")), row, 0, 1, 1, Qt::AlignRight);

        QHBoxLayout* maskLayout = new QHBoxLayout();
        mBlendChannelMaskCheckBox[0] = new QCheckBox("R");
        mBlendChannelMaskCheckBox[1] = new QCheckBox("G");
        mBlendChannelMaskCheckBox[2] = new QCheckBox("B");
        mBlendChannelMaskCheckBox[3] = new QCheckBox("A");
        for (int i = 0; i < 4; ++i) {
            mBlendChannelMaskCheckBox[i]->setChecked(true);
            connect(mBlendChannelMaskCheckBox[i], SIGNAL(clicked(bool)), this, SLOT(onBlendblockModified()));
            maskLayout->addWidget(mBlendChannelMaskCheckBox[i]);
        }
        gridLayout->addLayout(maskLayout, row++, 1);
    }

    {
        mIsTransparentMaskCheckBox[0] = new QCheckBox(tr("Is transparent (auto)"));
        mIsTransparentMaskCheckBox[0]->setEnabled(false);
        connect(mIsTransparentMaskCheckBox[0], SIGNAL(clicked(bool)), this, SLOT(onBlendblockModified()));
        gridLayout->addWidget(mIsTransparentMaskCheckBox[0], row++, 1, 1, 1);
    }

    {
        mIsTransparentMaskCheckBox[1] = new QCheckBox(tr("Is transparent (Force transparent render order)"));
        connect(mIsTransparentMaskCheckBox[1], SIGNAL(clicked(bool)), this, SLOT(onBlendblockModified()));
        gridLayout->addWidget(mIsTransparentMaskCheckBox[1], row++, 1, 1, 1);
    }

    {
        QHBoxLayout* lineLayout = new QHBoxLayout();

        mSeparateBlendCheckBox = new QCheckBox(tr("Separate blend"));
        connect(mSeparateBlendCheckBox, SIGNAL(clicked(bool)), this, SLOT(onBlendblockModified()));
        lineLayout->addWidget(mSeparateBlendCheckBox);

//        gridLayout->addWidget(mSeparateBlendCheckBox, row, 0);

        mPresetsButton = new QToolButton();
        mPresetsButton->setText("Presets");
        mPresetsButton->setContextMenuPolicy(Qt::ActionsContextMenu);
        mPresetsButton->setPopupMode(QToolButton::InstantPopup);
        for (int i = 0; i < SceneBlendTypeNames.size(); ++i) {
            QAction* action = new QAction(SceneBlendTypeNames[i]);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(sceneBlendTypeClicked()));
            mPresetsButton->addAction(action);
        }
        lineLayout->addWidget(mPresetsButton);

//        gridLayout->addWidget(mPresetsButton, row++, 1);
        gridLayout->addLayout(lineLayout, row++, 1);
    }

    auto addSceneBlendFactoryCombo = [&](QComboBox*& comboBox, const QString& text)->void
    {
        gridLayout->addWidget(new QLabel(text), row, 0, 1, 1, Qt::AlignRight);

        comboBox = new QComboBox();
        for (int i = 0; i < SceneBlendFactorNames.size(); ++i) {
            comboBox->addItem(SceneBlendFactorNames[i]);
        }
        gridLayout->addWidget(comboBox, row++, 1);
    };

    addSceneBlendFactoryCombo(mSourceBlendFactorComboBox, tr("Source blend factor:"));
    connect(mSourceBlendFactorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBlendblockModified()));

    addSceneBlendFactoryCombo(mDestBlendFactorComboBox, tr("Dest blend factor:"));
    connect(mDestBlendFactorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBlendblockModified()));

    addSceneBlendFactoryCombo(mSourceBlendFactorAlphaComboBox, tr("Source blend factor alpha:"));
    connect(mSourceBlendFactorAlphaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBlendblockModified()));

    addSceneBlendFactoryCombo(mDestBlendFactorAlphaComboBox, tr("Dest blend factor alpha:"));
    connect(mDestBlendFactorAlphaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBlendblockModified()));

    auto addSceneOperationCombo = [&](QComboBox*& comboBox, const QString& text)->void
    {
        gridLayout->addWidget(new QLabel(text), row, 0, 1, 1, Qt::AlignRight);

        comboBox = new QComboBox();
        for (int i = 0; i < SceneBlendOperationNames.size(); ++i) {
            comboBox->addItem(SceneBlendOperationNames[i]);
        }
        gridLayout->addWidget(comboBox, row++, 1);
    };

    addSceneOperationCombo(mBlendOperationComboBox, tr("Blend operation:"));
    connect(mBlendOperationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBlendblockModified()));

    addSceneOperationCombo(mBlendOperationAlphaComboBox, tr("Blend operation alpha:"));
    connect(mBlendOperationAlphaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBlendblockModified()));

}

void BlendblockWidget::setEditedObject(Ogre::HlmsDatablock* editedDatablock, bool caster)
{
    mEditedDatablock = editedDatablock;
    mEditedDatablockCaster = caster;

    blendblockToGui();
}

void BlendblockWidget::blendblockToGui()
{
    Ogre::HlmsBlendblock dummy;
    const Ogre::HlmsBlendblock* srcBlendblock = mEditedDatablock ? mEditedDatablock->getBlendblock(mEditedDatablockCaster) : nullptr;
    const Ogre::HlmsBlendblock* blendblock = srcBlendblock ? srcBlendblock : &dummy;

    {
        QSignalBlocker bl(mAlphaToCoverageComboBox);
        int index = mAlphaToCoverageComboBox->findData(blendblock->mAlphaToCoverage);
        mAlphaToCoverageComboBox->setCurrentIndex(index);
    }

    {
        QSignalBlocker bl(mBlendChannelMaskCheckBox[0]);
        mBlendChannelMaskCheckBox[0]->setChecked(blendblock->mBlendChannelMask & Ogre::HlmsBlendblock::BlendChannelRed);
    }
    {
        QSignalBlocker bl(mBlendChannelMaskCheckBox[1]);
        mBlendChannelMaskCheckBox[1]->setChecked(blendblock->mBlendChannelMask & Ogre::HlmsBlendblock::BlendChannelGreen);
    }
    {
        QSignalBlocker bl(mBlendChannelMaskCheckBox[2]);
        mBlendChannelMaskCheckBox[2]->setChecked(blendblock->mBlendChannelMask & Ogre::HlmsBlendblock::BlendChannelBlue);
    }
    {
        QSignalBlocker bl(mBlendChannelMaskCheckBox[3]);
        mBlendChannelMaskCheckBox[3]->setChecked(blendblock->mBlendChannelMask & Ogre::HlmsBlendblock::BlendChannelAlpha);
    }

    updateIsTransparentAutoToGui();

    {
        QSignalBlocker bl(mIsTransparentMaskCheckBox[1]);
        mIsTransparentMaskCheckBox[1]->setChecked(blendblock->isForcedTransparent());
    }

    {
        QSignalBlocker bl(mSeparateBlendCheckBox);
        mSeparateBlendCheckBox->setChecked(blendblock->mSeparateBlend);
    }

    {
        QSignalBlocker bl(mSourceBlendFactorComboBox);
        mSourceBlendFactorComboBox->setCurrentIndex(blendblock->mSourceBlendFactor);
    }
    {
        QSignalBlocker bl(mDestBlendFactorComboBox);
        mDestBlendFactorComboBox->setCurrentIndex(blendblock->mDestBlendFactor);
    }
    {
        QSignalBlocker bl(mSourceBlendFactorAlphaComboBox);
        mSourceBlendFactorAlphaComboBox->setCurrentIndex(blendblock->mSourceBlendFactorAlpha);
    }
    {
        QSignalBlocker bl(mDestBlendFactorAlphaComboBox);
        mDestBlendFactorAlphaComboBox->setCurrentIndex(blendblock->mDestBlendFactorAlpha);
    }

    {
        QSignalBlocker bl(mBlendOperationComboBox);
        mBlendOperationComboBox->setCurrentIndex(blendblock->mBlendOperation);
    }
    {
        QSignalBlocker bl(mBlendOperationAlphaComboBox);
        mBlendOperationAlphaComboBox->setCurrentIndex(blendblock->mBlendOperationAlpha);
    }

}

void BlendblockWidget::updateIsTransparentAutoToGui()
{
    const Ogre::HlmsBlendblock* blendblock = mEditedDatablock ? mEditedDatablock->getBlendblock(mEditedDatablockCaster) : nullptr;
    QSignalBlocker bl(mIsTransparentMaskCheckBox[0]);
    mIsTransparentMaskCheckBox[0]->setChecked(blendblock ? blendblock->isAutoTransparent() : false);

}

void BlendblockWidget::sceneBlendTypeClicked()
{
    if(!mEditedDatablock) {
        return;
    }

    QObject* s = sender();
    int index = -1;
    for (int i = 0; i < mPresetsButton->actions().size(); ++i) {
        if(s == mPresetsButton->actions()[i]) {
            index = i;
            break;
        }
    }

    if(index < 0) {
        return;
    }

    const Ogre::HlmsBlendblock* oldBlendblock = mEditedDatablock->getBlendblock(mEditedDatablockCaster);

    Ogre::HlmsBlendblock blendblock;
    if(oldBlendblock) {
        blendblock = *oldBlendblock;
    }

    blendblock.setBlendType((Ogre::SceneBlendType)index);

    mEditedDatablock->setBlendblock(blendblock, mEditedDatablockCaster);

    blendblockToGui();

    emit blendblockModified();
}

void BlendblockWidget::onBlendblockModified()
{
    if(!mEditedDatablock) {
        return;
    }

    const Ogre::HlmsBlendblock* oldBlendblock = mEditedDatablock->getBlendblock(mEditedDatablockCaster);

    Ogre::HlmsBlendblock blendblock;
    if(oldBlendblock) {
        blendblock = *oldBlendblock;
    }

    blendblock.mAlphaToCoverage = static_cast<Ogre::HlmsBlendblock::A2CSetting>(mAlphaToCoverageComboBox->currentData().toInt());

    {
        Ogre::uint8 blendChannelMask = 0;
        if(mBlendChannelMaskCheckBox[0]->isChecked()) {
            blendChannelMask |= Ogre::HlmsBlendblock::BlendChannelRed;
        }
        if(mBlendChannelMaskCheckBox[1]->isChecked()) {
            blendChannelMask |= Ogre::HlmsBlendblock::BlendChannelGreen;
        }
        if(mBlendChannelMaskCheckBox[2]->isChecked()) {
            blendChannelMask |= Ogre::HlmsBlendblock::BlendChannelBlue;
        }
        if(mBlendChannelMaskCheckBox[3]->isChecked()) {
            blendChannelMask |= Ogre::HlmsBlendblock::BlendChannelAlpha;
        }
        blendblock.mBlendChannelMask = blendChannelMask;
    }

    {
//        Ogre::uint8 isTransparent = 0;
//        if(mIsTransparentMaskCheckBox[0]->isChecked()) {
//            isTransparent |= 0x01;
//        }
//        if(mIsTransparentMaskCheckBox[1]->isChecked()) {
//            isTransparent |= 0x02;
//        }
//        blendblock.mIsTransparent = isTransparent;

        blendblock.setForceTransparentRenderOrder(mIsTransparentMaskCheckBox[1]->isChecked());
    }

    blendblock.mSeparateBlend = mSeparateBlendCheckBox->isChecked();

    blendblock.mSourceBlendFactor = (Ogre::SceneBlendFactor)mSourceBlendFactorComboBox->currentIndex();
    blendblock.mDestBlendFactor = (Ogre::SceneBlendFactor)mDestBlendFactorComboBox->currentIndex();
    blendblock.mSourceBlendFactorAlpha = (Ogre::SceneBlendFactor)mSourceBlendFactorAlphaComboBox->currentIndex();
    blendblock.mDestBlendFactorAlpha = (Ogre::SceneBlendFactor)mDestBlendFactorAlphaComboBox->currentIndex();

    blendblock.mBlendOperation = (Ogre::SceneBlendOperation)mBlendOperationComboBox->currentIndex();
    blendblock.mBlendOperationAlpha = (Ogre::SceneBlendOperation)mBlendOperationAlphaComboBox->currentIndex();

    mEditedDatablock->setBlendblock(blendblock, mEditedDatablockCaster);

    updateIsTransparentAutoToGui();

    emit blendblockModified();
}
