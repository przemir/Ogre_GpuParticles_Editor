/*
 * File: RangeWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#include "RangeWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

RangeWidget::RangeWidget()
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    mSpinX = new QDoubleSpinBox();
    mSpinX->setRange(-1000.0, 1000.0);
    connect(mSpinX, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinX);

    QLabel* label = new QLabel("-");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    mSpinY = new QDoubleSpinBox();
    mSpinY->setRange(-1000.0, 1000.0);
    connect(mSpinY, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    layout->addWidget(mSpinY);

    setSingleStep(0.01);

    setLayout(layout);
}

void RangeWidget::setSingleStep(double step)
{
    mSpinX->setSingleStep(step);
    mSpinY->setSingleStep(step);
}

void RangeWidget::setPossibleRange(double minValue, double maxValue)
{
    mSpinX->setRange(minValue, maxValue);
    mSpinY->setRange(minValue, maxValue);
}

Geometry::Range RangeWidget::getValue() const
{
    return Geometry::Range(mSpinX->value(), mSpinY->value());
}

void RangeWidget::setValue(const Geometry::Range& value, bool haveToEmit)
{
    bool old = mSpinX->blockSignals(true);
    mSpinX->setValue(value.getMin());
    mSpinX->blockSignals(old);

    old = mSpinY->blockSignals(true);
    mSpinY->setValue(value.getMax());
    mSpinY->blockSignals(old);

    if(haveToEmit)
        emit valueChanged(value.getMin(), value.getMax());
}

void RangeWidget::spinValueChanged(double value)
{
    Geometry::Range range(mSpinX->value(), mSpinY->value());
    setValue(range, false);

    emit valueChanged(mSpinX->value(), mSpinY->value());
}
