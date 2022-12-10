/*
 * File: ColourButton.h
 * Author: Przemysław Bągard
 * Created: 2021-3-13
 *
 */

#ifndef COLOURBUTTON_H
#define COLOURBUTTON_H

// From site:
// https://stackoverflow.com/questions/18257281/qt-color-picker-widget

#include <QToolButton>
#include <OgreColourValue.h>

class ColourButton : public QToolButton
{
    Q_OBJECT
public:
    ColourButton(QWidget* parent = nullptr);

    static QColor ogreColourToQt(const Ogre::ColourValue& colour);
    static Ogre::ColourValue qtColourToOgre(const QColor& colour);

    void setColour(const Ogre::ColourValue& colour);
    void setColour(const QColor& color);

    Ogre::ColourValue getOgreColour() const;
    const QColor& getQColor() const;


public slots:
    void updateColor();
    void changeColor();

signals:
    void colorChanged();

private:
    QColor mColor;
};

#endif
