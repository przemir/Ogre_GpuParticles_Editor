/*
 * File: SamplerblockWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#include "SamplerblockWidget.h"

#include <ColourEditField.h>
#include <OgreHlmsDatablock.h>
#include <OgreHlmsSamplerblock.h>
#include <OgreHlmsUnlitDatablock.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "BaseDatablockWidget.h"


const QStringList SamplerblockWidget::FilterOptionsNames = QStringList()
  << QString("FO_NONE")
  << QString("FO_POINT")
  << QString("FO_LINEAR")
  << QString("FO_ANISOTROPIC");

const QStringList SamplerblockWidget::TextureAddressingModeNames = QStringList()
  << QString("TAM_WRAP")
  << QString("TAM_MIRROR")
  << QString("TAM_CLAMP")
  << QString("TAM_BORDER")
  << QString("TAM_UNKNOWN");

SamplerblockWidget::SamplerblockWidget()
{
//    QGridLayout* gridLayout = new QGridLayout();
//    int row = 0;

//    createGui(gridLayout, row);

//    setLayout(gridLayout);
}

void SamplerblockWidget::createGui(QGridLayout* gridLayout, int& row)
{
    auto addFilterOptionsCombo = [&](QComboBox*& comboBox, const QString& text)->void
    {
        gridLayout->addWidget(new QLabel(text), row, 0, 1, 1, Qt::AlignRight);

        comboBox = new QComboBox();
        for (int i = 0; i < FilterOptionsNames.size(); ++i) {
            comboBox->addItem(FilterOptionsNames[i]);
        }
        gridLayout->addWidget(comboBox, row++, 1);
    };

    addFilterOptionsCombo(mMinFilterComboBox, tr("Min filter:"));
    connect(mMinFilterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSamplerblockModified()));

    addFilterOptionsCombo(mMagFilterComboBox, tr("Mag filter:"));
    connect(mMagFilterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSamplerblockModified()));

    addFilterOptionsCombo(mMipFilterComboBox, tr("Mip filter:"));
    connect(mMipFilterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSamplerblockModified()));

    auto addTextureAddressingModeCombo = [&](QComboBox*& comboBox, const QString& text)->void
    {
        gridLayout->addWidget(new QLabel(text), row, 0, 1, 1, Qt::AlignRight);

        comboBox = new QComboBox();
        for (int i = 0; i < TextureAddressingModeNames.size(); ++i) {
            comboBox->addItem(TextureAddressingModeNames[i]);
        }
        gridLayout->addWidget(comboBox, row++, 1);
    };

    addTextureAddressingModeCombo(mUComboBox, tr("U:"));
    connect(mUComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSamplerblockModified()));

    addTextureAddressingModeCombo(mVComboBox, tr("V:"));
    connect(mVComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSamplerblockModified()));

    addTextureAddressingModeCombo(mWComboBox, tr("W:"));
    connect(mWComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSamplerblockModified()));

    {
        gridLayout->addWidget(new QLabel("Mip Lod bias:"), row, 0, 1, 1, Qt::AlignRight);

        mMipLodBiasSpinBox = new QDoubleSpinBox();
        mMipLodBiasSpinBox->setRange(0.0, 1.0);
        mMipLodBiasSpinBox->setSingleStep(0.01);
        connect(mMipLodBiasSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onSamplerblockModified()));
        gridLayout->addWidget(mMipLodBiasSpinBox, row++, 1);
    }

    {
        gridLayout->addWidget(new QLabel("Max anisotropy:"), row, 0, 1, 1, Qt::AlignRight);

        mMaxAnisotropySpinBox = new QDoubleSpinBox();
        mMaxAnisotropySpinBox->setRange(0.0, 1.0);
        mMaxAnisotropySpinBox->setSingleStep(0.01);
        connect(mMaxAnisotropySpinBox, SIGNAL(valueChanged(double)), this, SLOT(onSamplerblockModified()));
        gridLayout->addWidget(mMaxAnisotropySpinBox, row++, 1);
    }

    {
        gridLayout->addWidget(new QLabel(tr("Compare function:")), row, 0, 1, 1, Qt::AlignRight);

        mCompareFunctionComboBox = new QComboBox();
        mCompareFunctionComboBox->setEnabled(false);
        for (int i = 0; i < BaseDatablockWidget::CompareFunctionNames.size(); ++i) {
            mCompareFunctionComboBox->addItem(BaseDatablockWidget::CompareFunctionNames[i]);
        }
        connect(mCompareFunctionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSamplerblockModified()));
        gridLayout->addWidget(mCompareFunctionComboBox, row++, 1);
    };

    {
        gridLayout->addWidget(new QLabel(tr("Border colour:")), row, 0, 1, 1, Qt::AlignRight);
        mBorderColourEditField = new ColourEditField();
        connect(mBorderColourEditField, SIGNAL(colorChanged()), this, SLOT(onSamplerblockModified()));
        gridLayout->addWidget(mBorderColourEditField, row++, 1, 1, 1);
    }

    {
        gridLayout->addWidget(new QLabel("Min Lod:"), row, 0, 1, 1, Qt::AlignRight);

        mMinLodSpinBox = new QLineEdit();
        mMinLodSpinBox->setDisabled(true);
//        connect(mMinLodSpinBox, SIGNAL(editingFinished()), this, SLOT(onSamplerblockModified()));
        gridLayout->addWidget(mMinLodSpinBox, row++, 1);
    }
    {
        gridLayout->addWidget(new QLabel("Max Lod:"), row, 0, 1, 1, Qt::AlignRight);

        mMaxLodSpinBox = new QLineEdit();
        mMaxLodSpinBox->setDisabled(true);
//        connect(mMaxLodSpinBox, SIGNAL(editingFinished()), this, SLOT(onSamplerblockModified()));
        gridLayout->addWidget(mMaxLodSpinBox, row++, 1);
    }
}

void SamplerblockWidget::setEditedObject(Ogre::HlmsUnlitDatablock* editedDatablock, Ogre::uint8 textureSlotIndex)
{
    mEditedDatablock = editedDatablock;
    mEditedDatablockTextureSlotIndex = textureSlotIndex;

    samplerblockToGui();
}

void SamplerblockWidget::samplerblockToGui()
{
    Ogre::HlmsSamplerblock dummy;
    const Ogre::HlmsSamplerblock* srcSamplerblock = mEditedDatablock ? mEditedDatablock->getSamplerblock(mEditedDatablockTextureSlotIndex) : nullptr;
    const Ogre::HlmsSamplerblock* samplerblock = srcSamplerblock ? srcSamplerblock : &dummy;

    {
        QSignalBlocker bl(mMinFilterComboBox);
        mMinFilterComboBox->setCurrentIndex(samplerblock->mMinFilter);
    }
    {
        QSignalBlocker bl(mMagFilterComboBox);
        mMagFilterComboBox->setCurrentIndex(samplerblock->mMagFilter);
    }
    {
        QSignalBlocker bl(mMipFilterComboBox);
        mMipFilterComboBox->setCurrentIndex(samplerblock->mMipFilter);
    }

    {
        QSignalBlocker bl(mUComboBox);
        mUComboBox->setCurrentIndex(samplerblock->mU);
    }
    {
        QSignalBlocker bl(mVComboBox);
        mVComboBox->setCurrentIndex(samplerblock->mV);
    }
    {
        QSignalBlocker bl(mWComboBox);
        mWComboBox->setCurrentIndex(samplerblock->mW);
    }

    {
        QSignalBlocker bl(mMipLodBiasSpinBox);
        mMipLodBiasSpinBox->setValue(samplerblock->mMipLodBias);
    }
    {
        QSignalBlocker bl(mMaxAnisotropySpinBox);
        mMaxAnisotropySpinBox->setValue(samplerblock->mMaxAnisotropy);
    }

    {
        QSignalBlocker bl(mCompareFunctionComboBox);
        mCompareFunctionComboBox->setCurrentIndex(samplerblock->mCompareFunction);
    }

    {
        QSignalBlocker bl(mBorderColourEditField);
        mBorderColourEditField->setColour(samplerblock->mBorderColour);
    }

    {
        QSignalBlocker bl(mMinLodSpinBox);
        mMinLodSpinBox->setText(QString::number(samplerblock->mMinLod));
    }
    {
        QSignalBlocker bl(mMaxLodSpinBox);
        mMaxLodSpinBox->setText(QString::number(samplerblock->mMaxLod));
    }

}

void SamplerblockWidget::onSamplerblockModified()
{
    if(!mEditedDatablock) {
        return;
    }

    const Ogre::HlmsSamplerblock* oldSamplerblock = mEditedDatablock->getSamplerblock(mEditedDatablockTextureSlotIndex);

    Ogre::HlmsSamplerblock samplerblock;
    if(oldSamplerblock) {
        samplerblock = *oldSamplerblock;
    }

    samplerblock.mMinFilter = (Ogre::FilterOptions)mMinFilterComboBox->currentIndex();
    samplerblock.mMagFilter = (Ogre::FilterOptions)mMagFilterComboBox->currentIndex();
    samplerblock.mMipFilter = (Ogre::FilterOptions)mMipFilterComboBox->currentIndex();
    samplerblock.mU = (Ogre::TextureAddressingMode)mUComboBox->currentIndex();
    samplerblock.mV = (Ogre::TextureAddressingMode)mVComboBox->currentIndex();
    samplerblock.mW = (Ogre::TextureAddressingMode)mWComboBox->currentIndex();

    samplerblock.mMipLodBias = mMipLodBiasSpinBox->value();
    samplerblock.mMaxAnisotropy = mMaxAnisotropySpinBox->value();

    samplerblock.mCompareFunction = (Ogre::CompareFunction)mCompareFunctionComboBox->currentIndex();

    samplerblock.mBorderColour = mBorderColourEditField->getColour();

    mEditedDatablock->setSamplerblock(mEditedDatablockTextureSlotIndex, samplerblock);

    emit samplerblockModified();
}
