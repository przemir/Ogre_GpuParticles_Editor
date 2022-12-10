/*
 * File: ColourEditField.h
 * Author: Przemysław Bągard
 * Created: 2021-3-15
 *
 */

#ifndef COLOUREDITFIELD_H
#define COLOUREDITFIELD_H

#include <OgreColourValue.h>
#include <QWidget>

class ColourButton;
class QLineEdit;

class ColourEditField : public QWidget
{
    Q_OBJECT
public:
    ColourEditField(QWidget* parent = nullptr);

    ColourButton* mColourButton;
    QLineEdit* mLineEdit;
    bool mUseAlpha = false;

    void setColour(const Ogre::ColourValue& colour);
    Ogre::ColourValue getColour() const;

private:
    void updateLineEdit();

signals:
    void colorChanged();

private slots:
    void colourButtonPressed();

};

#endif
