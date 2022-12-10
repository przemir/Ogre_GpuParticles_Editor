/*
 * File: EulerDegreeHVSWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#ifndef EULERDEGREEWIDGET_H
#define EULERDEGREEWIDGET_H

#include <QWidget>
#include <Utils/EulerDegreeHVS.h>

class QDoubleSpinBox;

class EulerDegreeHVSWidget : public QWidget
{
    Q_OBJECT
public:
    EulerDegreeHVSWidget(bool yawPitchRollNotation = false, QWidget* parent = nullptr);

    QDoubleSpinBox* mSpinH = nullptr;
    QDoubleSpinBox* mSpinV = nullptr;
    QDoubleSpinBox* mSpinS = nullptr;

    void setSingleStep(double step);
    void setDecimals(int precision);

    EulerDegreeHVS getValue() const;
    void setValue(const EulerDegreeHVS& value, bool haveToEmit);

signals:
    void valueChanged(float valueX, float valueY, float valueZ);

private slots:
    void spinValueChanged(double value);
};

#endif
