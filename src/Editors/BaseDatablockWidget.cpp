/*
 * File: BaseDatablockWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#include "BaseDatablockWidget.h"

#include <ColourEditField.h>
#include <OgreHlmsDatablock.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>


const QStringList BaseDatablockWidget::CompareFunctionNames = QStringList()
  << QString("CMPF_ALWAYS_FAIL")
  << QString("CMPF_ALWAYS_PASS")
  << QString("CMPF_LESS")
  << QString("CMPF_LESS_EQUAL")
  << QString("CMPF_EQUAL")
  << QString("CMPF_NOT_EQUAL")
  << QString("CMPF_GREATER_EQUAL")
  << QString("CMPF_GREATER")
  << QString("NUM_COMPARE_FUNCTIONS");

BaseDatablockWidget::BaseDatablockWidget()
{
//    QGridLayout* gridLayout = new QGridLayout();
//    int row = 0;

//    createGui(gridLayout, row);

//    setLayout(gridLayout);
}

void BaseDatablockWidget::createGui(QGridLayout* gridLayout, int& row)
{
    {
        gridLayout->addWidget(new QLabel(tr("Alpha test compare function:")), row, 0, 1, 1, Qt::AlignRight);

        mCompareFunctionComboBox = new QComboBox();
        for (int i = 0; i < CompareFunctionNames.size(); ++i) {
            mCompareFunctionComboBox->addItem(CompareFunctionNames[i]);
        }
        connect(mCompareFunctionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBaseDatablockModified()));
        gridLayout->addWidget(mCompareFunctionComboBox, row++, 1);
    };

    {
        mAlphaTestShadowCasterOnlyCheckBox = new QCheckBox(tr("Alpha test shadow caster only"));
        connect(mAlphaTestShadowCasterOnlyCheckBox, SIGNAL(clicked(bool)), this, SLOT(onBaseDatablockModified()));
        gridLayout->addWidget(mAlphaTestShadowCasterOnlyCheckBox, row++, 1, 1, 1);
    }

    {
        gridLayout->addWidget(new QLabel("Alpha test threshold:"), row, 0, 1, 1, Qt::AlignRight);

        mAlphaTestTresholdSpinBox = new QDoubleSpinBox();
        mAlphaTestTresholdSpinBox->setRange(0.0, 1.0);
        mAlphaTestTresholdSpinBox->setSingleStep(0.01);
        connect(mAlphaTestTresholdSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBaseDatablockModified()));
        gridLayout->addWidget(mAlphaTestTresholdSpinBox, row++, 1);
    }

    {
        gridLayout->addWidget(new QLabel("Shadow constant bias:"), row, 0, 1, 1, Qt::AlignRight);

        mShadowConstantBiasSpinBox = new QDoubleSpinBox();
        mShadowConstantBiasSpinBox->setRange(0.0, 1.0);
        mShadowConstantBiasSpinBox->setSingleStep(0.01);
        connect(mShadowConstantBiasSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBaseDatablockModified()));
        gridLayout->addWidget(mShadowConstantBiasSpinBox, row++, 1);
    }

}

void BaseDatablockWidget::setEditedObject(Ogre::HlmsUnlitDatablock* editedDatablock)
{
    mEditedDatablock = editedDatablock;

    baseDatablockToGui();
}

void BaseDatablockWidget::baseDatablockToGui()
{
    {
        QSignalBlocker bl(mCompareFunctionComboBox);
        mCompareFunctionComboBox->setCurrentIndex(mEditedDatablock ? mEditedDatablock->getAlphaTest() : Ogre::CMPF_ALWAYS_PASS);
    }

    {
        QSignalBlocker bl(mAlphaTestShadowCasterOnlyCheckBox);
        mAlphaTestShadowCasterOnlyCheckBox->setChecked(mEditedDatablock ? mEditedDatablock->getAlphaTestShadowCasterOnly() : false);
    }

    {
        QSignalBlocker bl(mAlphaTestTresholdSpinBox);
        mAlphaTestTresholdSpinBox->setValue(mEditedDatablock ? mEditedDatablock->getAlphaTestThreshold() : 0.0f);
    }
    {
        QSignalBlocker bl(mShadowConstantBiasSpinBox);
        mShadowConstantBiasSpinBox->setValue(mEditedDatablock ? mEditedDatablock->mShadowConstantBias : 0.0f);
    }

}

void BaseDatablockWidget::onBaseDatablockModified()
{
    if(!mEditedDatablock) {
        return;
    }

    mEditedDatablock->mShadowConstantBias = mShadowConstantBiasSpinBox->value();
    mEditedDatablock->setAlphaTestThreshold(mAlphaTestTresholdSpinBox->value());
    mEditedDatablock->setAlphaTest((Ogre::CompareFunction)mCompareFunctionComboBox->currentIndex(), mAlphaTestShadowCasterOnlyCheckBox->isChecked(), true);

    emit baseDatablockModified();
}
