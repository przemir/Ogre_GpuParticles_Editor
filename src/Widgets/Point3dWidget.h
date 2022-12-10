/*
 * File: Point3dWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#ifndef POINT3DWIDGET_H
#define POINT3DWIDGET_H

#include <QWidget>
#include <OgreVector3.h>

class QDoubleSpinBox;

class Point3dWidget : public QWidget
{
    Q_OBJECT
public:
    Point3dWidget(QWidget* parent = nullptr);

    QDoubleSpinBox* mSpinX = nullptr;
    QDoubleSpinBox* mSpinY = nullptr;
    QDoubleSpinBox* mSpinZ = nullptr;

    void setSingleStep(double step);

    Ogre::Vector3 getValue() const;
    void setValue(const Ogre::Vector3& value, bool haveToEmit);

signals:
    void valueChanged(float valueX, float valueY, float valueZ);

private slots:
    void spinValueChanged(double value);
};

#endif
