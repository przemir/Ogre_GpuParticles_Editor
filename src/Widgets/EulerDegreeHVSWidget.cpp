/*
 * File: EulerDegreeHVSWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#include "EulerDegreeHVSWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

EulerDegreeHVSWidget::EulerDegreeHVSWidget(bool yawPitchRollNotation, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* hLabel = new QLabel("H");
    if(yawPitchRollNotation) {
        hLabel->setText("Yaw");
    }
    hLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hLabel->setToolTip("Horizontal (Yaw; around Y axis)\nIn degrees.");
    layout->addWidget(hLabel);
    mSpinH = new QDoubleSpinBox();
    mSpinH->setRange(-180.0, 180.0);
    connect(mSpinH, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinH);

    QLabel* vLabel = new QLabel("V");
    if(yawPitchRollNotation) {
        vLabel->setText("Pitch");
    }
    vLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    vLabel->setToolTip("Vertical (Pitch; around X axis)\nIn degrees.");
    layout->addWidget(vLabel);
    mSpinV = new QDoubleSpinBox();
    mSpinV->setRange(-90.0, 90.0);
    connect(mSpinV, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinV);

    QLabel* sLabel = new QLabel("S");
    if(yawPitchRollNotation) {
        sLabel->setText("Roll");
    }
    sLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sLabel->setToolTip("Side (Roll; around Z axis)\nIn degrees.");
    layout->addWidget(sLabel);
    mSpinS = new QDoubleSpinBox();
    mSpinS->setRange(-180.0, 180.0);
    connect(mSpinS, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinS);

//    setDecimals(0);
    setSingleStep(1);

    setLayout(layout);
}

void EulerDegreeHVSWidget::setSingleStep(double step)
{
    mSpinH->setSingleStep(step);
    mSpinV->setSingleStep(step);
    mSpinS->setSingleStep(step);
}

void EulerDegreeHVSWidget::setDecimals(int precision)
{
    mSpinH->setDecimals(precision);
    mSpinV->setDecimals(precision);
    mSpinS->setDecimals(precision);
}

EulerDegreeHVS EulerDegreeHVSWidget::getValue() const
{
    EulerDegreeHVS eulerDegree;
    eulerDegree.mHorizontal = Ogre::Degree(mSpinH->value());
    eulerDegree.mVertical = Ogre::Degree(mSpinV->value());
    eulerDegree.mSide = Ogre::Degree(mSpinS->value());
    return eulerDegree;
}

void EulerDegreeHVSWidget::setValue(const EulerDegreeHVS& value, bool haveToEmit)
{
    bool old = mSpinH->blockSignals(true);
    mSpinH->setValue(value.mHorizontal.valueDegrees());
    mSpinH->blockSignals(old);

    old = mSpinV->blockSignals(true);
    mSpinV->setValue(value.mVertical.valueDegrees());
    mSpinV->blockSignals(old);

    old = mSpinS->blockSignals(true);
    mSpinS->setValue(value.mSide.valueDegrees());
    mSpinS->blockSignals(old);

    if(haveToEmit)
        emit valueChanged(value.mHorizontal.valueDegrees(), value.mVertical.valueDegrees(), value.mSide.valueDegrees());
}

void EulerDegreeHVSWidget::spinValueChanged(double value)
{
    emit valueChanged(mSpinH->value(), mSpinV->value(), mSpinS->value());
}
