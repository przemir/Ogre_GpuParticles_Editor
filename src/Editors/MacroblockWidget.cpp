/*
 * File: MacroblockWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#include "MacroblockWidget.h"

#include <OgreHlmsDatablock.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include "BaseDatablockWidget.h"

const QStringList MacroblockWidget::CullModeNames = QStringList()
  << QString("CULL_NONE")
  << QString("CULL_CLOCKWISE")
  << QString("CULL_ANTICLOCKWISE");

const QStringList MacroblockWidget::PolygonModeNames = QStringList()
  << QString("PM_POINTS")
  << QString("PM_WIREFRAME")
  << QString("PM_SOLID");

MacroblockWidget::MacroblockWidget()
{
//    QGridLayout* gridLayout = new QGridLayout();
//    int row = 0;

//    createGui(gridLayout, row);

//    setLayout(gridLayout);
}

void MacroblockWidget::createGui(QGridLayout* gridLayout, int& row)
{
    {
        mScissorTestCheckbox = new QCheckBox(tr("Scissor test"));
        connect(mScissorTestCheckbox, SIGNAL(clicked(bool)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mScissorTestCheckbox, row++, 1, 1, 1);
    }

    {
        mDepthClampCheckbox = new QCheckBox(tr("Depth clamp"));
        connect(mDepthClampCheckbox, SIGNAL(clicked(bool)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mDepthClampCheckbox, row++, 1, 1, 1);
    }

    {
        mDepthCheckCheckbox = new QCheckBox(tr("Depth check"));
        connect(mDepthCheckCheckbox, SIGNAL(clicked(bool)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mDepthCheckCheckbox, row++, 1, 1, 1);
    }

    {
        mDepthWriteCheckbox = new QCheckBox(tr("Depth write"));
        connect(mDepthWriteCheckbox, SIGNAL(clicked(bool)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mDepthWriteCheckbox, row++, 1, 1, 1);
    }

    {
        gridLayout->addWidget(new QLabel(tr("Depth func:")), row, 0, 1, 1, Qt::AlignRight);

        mDepthFuncComboBox = new QComboBox();
        for (int i = 0; i < BaseDatablockWidget::CompareFunctionNames.size(); ++i) {
            mDepthFuncComboBox->addItem(BaseDatablockWidget::CompareFunctionNames[i]);
        }
        connect(mDepthFuncComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mDepthFuncComboBox, row++, 1);
    };

    {
        gridLayout->addWidget(new QLabel("Depth bias constant:"), row, 0, 1, 1, Qt::AlignRight);

        mDepthBiasConstantSpinBox = new QDoubleSpinBox();
        mDepthBiasConstantSpinBox->setRange(0.0, 1.0);
        mDepthBiasConstantSpinBox->setSingleStep(0.01);
        connect(mDepthBiasConstantSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mDepthBiasConstantSpinBox, row++, 1);
    }

    {
        gridLayout->addWidget(new QLabel("Depth bias slope scale:"), row, 0, 1, 1, Qt::AlignRight);

        mDepthBiasSlopeScaleSpinBox = new QDoubleSpinBox();
        mDepthBiasSlopeScaleSpinBox->setRange(0.0, 1.0);
        mDepthBiasSlopeScaleSpinBox->setSingleStep(0.01);
        connect(mDepthBiasSlopeScaleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mDepthBiasSlopeScaleSpinBox, row++, 1);
    }

    {
        gridLayout->addWidget(new QLabel(tr("Cull mode:")), row, 0, 1, 1, Qt::AlignRight);

        mCullModeComboBox = new QComboBox();
        for (int i = 0; i < CullModeNames.size(); ++i) {
            mCullModeComboBox->addItem(CullModeNames[i]);
        }
        connect(mCullModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mCullModeComboBox, row++, 1);
    };

    {
        gridLayout->addWidget(new QLabel(tr("Polygon mode:")), row, 0, 1, 1, Qt::AlignRight);

        mPolygonModeComboBox = new QComboBox();
        for (int i = 0; i < PolygonModeNames.size(); ++i) {
            mPolygonModeComboBox->addItem(PolygonModeNames[i]);
        }
        connect(mPolygonModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onMacroblockModified()));
        gridLayout->addWidget(mPolygonModeComboBox, row++, 1);
    };
}

void MacroblockWidget::setEditedObject(Ogre::HlmsDatablock* editedDatablock, bool caster)
{
    mEditedDatablock = editedDatablock;
    mEditedDatablockCaster = caster;

    macroblockToGui();
}

void MacroblockWidget::macroblockToGui()
{
    Ogre::HlmsMacroblock dummy;
    const Ogre::HlmsMacroblock* srcMacroblock = mEditedDatablock ? mEditedDatablock->getMacroblock(mEditedDatablockCaster) : nullptr;
    const Ogre::HlmsMacroblock* macroblock = srcMacroblock ? srcMacroblock : &dummy;

    {
        QSignalBlocker bl(mScissorTestCheckbox);
        mScissorTestCheckbox->setChecked(macroblock->mScissorTestEnabled);
    }
    {
        QSignalBlocker bl(mDepthClampCheckbox);
        mDepthClampCheckbox->setChecked(macroblock->mDepthClamp);
    }
    {
        QSignalBlocker bl(mDepthCheckCheckbox);
        mDepthCheckCheckbox->setChecked(macroblock->mDepthCheck);
    }
    {
        QSignalBlocker bl(mDepthWriteCheckbox);
        mDepthWriteCheckbox->setChecked(macroblock->mDepthWrite);
    }

    {
        QSignalBlocker bl(mDepthFuncComboBox);
        mDepthFuncComboBox->setCurrentIndex(macroblock->mDepthFunc);
    }

    {
        QSignalBlocker bl(mDepthBiasConstantSpinBox);
        mDepthBiasConstantSpinBox->setValue(macroblock->mDepthBiasConstant);
    }
    {
        QSignalBlocker bl(mDepthBiasSlopeScaleSpinBox);
        mDepthBiasSlopeScaleSpinBox->setValue(macroblock->mDepthBiasSlopeScale);
    }

    {
        QSignalBlocker bl(mCullModeComboBox);
        mCullModeComboBox->setCurrentIndex(macroblock->mCullMode-1);
    }
    {
        QSignalBlocker bl(mPolygonModeComboBox);
        mPolygonModeComboBox->setCurrentIndex(macroblock->mPolygonMode-1);
    }
}

void MacroblockWidget::onMacroblockModified()
{
    if(!mEditedDatablock) {
        return;
    }

    const Ogre::HlmsMacroblock* oldMacroblock = mEditedDatablock->getMacroblock(mEditedDatablockCaster);

    Ogre::HlmsMacroblock macroblock;
    if(oldMacroblock) {
        macroblock = *oldMacroblock;
    }

    macroblock.mScissorTestEnabled = mScissorTestCheckbox->isChecked();
    macroblock.mDepthClamp = mDepthClampCheckbox->isChecked();
    macroblock.mDepthCheck = mDepthCheckCheckbox->isChecked();
    macroblock.mDepthWrite = mDepthWriteCheckbox->isChecked();

    macroblock.mDepthFunc = (Ogre::CompareFunction)mDepthFuncComboBox->currentIndex();

    macroblock.mDepthBiasConstant = mDepthBiasConstantSpinBox->value();
    macroblock.mDepthBiasSlopeScale = mDepthBiasSlopeScaleSpinBox->value();

    macroblock.mCullMode = (Ogre::CullingMode)(mCullModeComboBox->currentIndex()+1);
    macroblock.mPolygonMode = (Ogre::PolygonMode)(mPolygonModeComboBox->currentIndex()+1);

    mEditedDatablock->setMacroblock(macroblock, mEditedDatablockCaster);

    emit macroblockModified();
}
