/*
 * File: Point2dWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#include "Point2dWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

Point2dWidget::Point2dWidget(QWidget* parent)
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

    setSingleStep(0.01);

    setLayout(layout);
}

void Point2dWidget::setSingleStep(double step)
{
    mSpinX->setSingleStep(step);
    mSpinY->setSingleStep(step);
}

Ogre::Vector2 Point2dWidget::getValue() const
{
    return Ogre::Vector2(mSpinX->value(), mSpinY->value());
}

void Point2dWidget::setValue(const Ogre::Vector2& value, bool haveToEmit)
{
    bool old = mSpinX->blockSignals(true);
    mSpinX->setValue(value.x);
    mSpinX->blockSignals(old);

    old = mSpinY->blockSignals(true);
    mSpinY->setValue(value.y);
    mSpinY->blockSignals(old);

    if(haveToEmit)
        emit valueChanged(value.x, value.y);
}

void Point2dWidget::spinValueChanged(double value)
{
    emit valueChanged(mSpinX->value(), mSpinY->value());
}
