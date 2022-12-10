/*
 * File: Point3dWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#include "Point3dWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

Point3dWidget::Point3dWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* xLabel = new QLabel("X");
    xLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(xLabel);
    mSpinX = new QDoubleSpinBox();
    mSpinX->setRange(-1000.0, 1000.0);
    connect(mSpinX, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinX);

    QLabel* yLabel = new QLabel("Y");
    yLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(yLabel);
    mSpinY = new QDoubleSpinBox();
    mSpinY->setRange(-1000.0, 1000.0);
    connect(mSpinY, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinY);

    QLabel* zLabel = new QLabel("Z");
    zLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(zLabel);
    mSpinZ = new QDoubleSpinBox();
    mSpinZ->setRange(-1000.0, 1000.0);
    connect(mSpinZ, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinZ);

    setSingleStep(0.01);

    setLayout(layout);
}

void Point3dWidget::setSingleStep(double step)
{
    mSpinX->setSingleStep(step);
    mSpinY->setSingleStep(step);
    mSpinZ->setSingleStep(step);
}

Ogre::Vector3 Point3dWidget::getValue() const
{
    return Ogre::Vector3(mSpinX->value(), mSpinY->value(), mSpinZ->value());
}

void Point3dWidget::setValue(const Ogre::Vector3& value, bool haveToEmit)
{
    bool old = mSpinX->blockSignals(true);
    mSpinX->setValue(value.x);
    mSpinX->blockSignals(old);

    old = mSpinY->blockSignals(true);
    mSpinY->setValue(value.y);
    mSpinY->blockSignals(old);

    old = mSpinZ->blockSignals(true);
    mSpinZ->setValue(value.z);
    mSpinZ->blockSignals(old);

    if(haveToEmit)
        emit valueChanged(value.x, value.y, value.z);
}

void Point3dWidget::spinValueChanged(double value)
{
    emit valueChanged(mSpinX->value(), mSpinY->value(), mSpinZ->value());
}
