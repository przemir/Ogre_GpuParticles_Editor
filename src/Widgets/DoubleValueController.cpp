/*
 * File:   FloatValueController.cpp
 * Author: Przemysław Bągard
 * Created: 2015-03-29
 *
 */

#include "DoubleValueController.h"

DoubleValueController::DoubleValueController(QObject* parent)
    : QObject(parent)
    , spin(NULL)
    , slider(NULL)
{
}

void DoubleValueController::setValueRange(double timeFrom, double timeTo)
{
    Q_ASSERT(spin);
    Q_ASSERT(slider);

    spin->setMinimum(timeFrom);
    spin->setMaximum(timeTo);

    slider->setMinimum(valueToSliderValue(timeFrom));
    slider->setMaximum(valueToSliderValue(timeTo));
}

void DoubleValueController::setValue(double value, bool haveToEmit)
{
    bool old = spin->blockSignals(true);
    spin->setValue(value);
    spin->blockSignals(old);

    old = slider->blockSignals(true);
    int sliderValue = valueToSliderValue(value);
    slider->setValue(sliderValue);
    slider->blockSignals(old);

    if(haveToEmit)
        emit valueChanged(value);
}

void DoubleValueController::setSingleStep(double step)
{
    spin->setSingleStep(step);
}

void DoubleValueController::setEnabled(bool enabled)
{
    spin->setEnabled(enabled);
    slider->setEnabled(enabled);
}

QDoubleSpinBox*DoubleValueController::getSpinBox()
{
    return spin;
}

double DoubleValueController::getValue() const
{
    return spin->value();
}

void DoubleValueController::set(QDoubleSpinBox *spin, QSlider *slider)
{
    Q_ASSERT(spin);
    Q_ASSERT(slider);

    this->spin = spin;
    this->slider = slider;

    connect(spin, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
}

void DoubleValueController::sliderValueChanged(int value)
{
    double time = ((double)value)/100.0;

    bool oldState = spin->blockSignals(true);
    spin->setValue(time);
    spin->blockSignals(oldState);

    emit valueChanged(time);
}

void DoubleValueController::spinValueChanged(double value)
{
    int sliderValue = valueToSliderValue(value);

    bool oldState = slider->blockSignals(true);
    slider->setValue(sliderValue);
    slider->blockSignals(oldState);

    emit valueChanged(value);
}

