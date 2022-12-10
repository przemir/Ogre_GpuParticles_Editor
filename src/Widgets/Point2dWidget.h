/*
 * File: Point2dWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-6-23
 *
 */

#ifndef POINT2DWIDGET_H
#define POINT2DWIDGET_H

#include <QWidget>
#include <OgreVector2.h>

class QDoubleSpinBox;

class Point2dWidget : public QWidget
{
    Q_OBJECT
public:
    Point2dWidget(QWidget* parent = nullptr);

    QDoubleSpinBox* mSpinX = nullptr;
    QDoubleSpinBox* mSpinY = nullptr;

    void setSingleStep(double step);

    Ogre::Vector2 getValue() const;
    void setValue(const Ogre::Vector2& value, bool haveToEmit);

signals:
    void valueChanged(float valueX, float valueY);

private slots:
    void spinValueChanged(double value);
};

#endif
