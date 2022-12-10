/*
 * File:   FloatValueController.h
 * Author: Przemysław Bągard
 * Created: 2015-03-29
 *
 */

#ifndef DOUBLEVALUECONTROLLER_H
#define DOUBLEVALUECONTROLLER_H

#include <QObject>
#include <QDoubleSpinBox>
#include <QSlider>

class DoubleValueController : public QObject
{
    Q_OBJECT
public:
    DoubleValueController(QObject* parent = NULL);

    void set(QDoubleSpinBox* spin, QSlider* slider);
    void setValueRange(double timeFrom, double timeTo);
    void setValue(double value, bool haveToEmit = true);
    void setSingleStep(double step);
    void setEnabled(bool enabled);

    QDoubleSpinBox* getSpinBox();
    double getValue() const;

signals:
    void valueChanged(double);

public slots:
    void sliderValueChanged(int value);
    void spinValueChanged(double value);

private:
    QDoubleSpinBox* spin;
    QSlider* slider;

    inline int valueToSliderValue(double value) {
        return (int)(value*100.0);
    }
};

#endif // DOUBLEVALUECONTROLLER_H
