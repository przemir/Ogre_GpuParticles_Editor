/*
 * File: RangeWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

#include <QWidget>
#include <Utils/Range.h>

class QDoubleSpinBox;

class RangeWidget : public QWidget
{
    Q_OBJECT
public:
    RangeWidget();

    QDoubleSpinBox* mSpinX = nullptr;
    QDoubleSpinBox* mSpinY = nullptr;

    void setSingleStep(double step);
    void setPossibleRange(double minValue, double maxValue);

    Geometry::Range getValue() const;
    void setValue(const Geometry::Range& value, bool haveToEmit);

signals:
    void valueChanged(float valueX, float valueY);

private slots:
    void spinValueChanged(double value);
};

#endif
