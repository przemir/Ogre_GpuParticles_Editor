/*
 * File: ColourButton.cpp
 * Author: Przemysław Bągard
 * Created: 2021-3-13
 *
 */

#include "ColourButton.h"

#include <QColorDialog>

ColourButton::ColourButton(QWidget* parent)
    : QToolButton(parent)
{
    connect(this, SIGNAL(pressed()), this, SLOT(changeColor()));
}

QColor ColourButton::ogreColourToQt(const Ogre::ColourValue& colour)
{
    return QColor(colour.r*255.0f, colour.g*255.0f, colour.b*255.0f, colour.a*255.0f);
}

Ogre::ColourValue ColourButton::qtColourToOgre(const QColor& colour)
{
    return Ogre::ColourValue(colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF());
}

void ColourButton::setColour(const Ogre::ColourValue& colour)
{
    setColour(ogreColourToQt(colour));
}

void ColourButton::setColour(const QColor& color)
{
    mColor = color;
    updateColor();
    emit colorChanged();
}

Ogre::ColourValue ColourButton::getOgreColour() const
{
    return qtColourToOgre(mColor);
}

const QColor& ColourButton::getQColor() const
{
    return mColor;
}

void ColourButton::updateColor()
{
    setStyleSheet( "background-color: " + mColor.name() );
}

void ColourButton::changeColor()
{
    QColor newColor = QColorDialog::getColor(mColor, parentWidget());
    if ( !newColor.isValid() ) {

        // canceled, no colour change
        return;
    }

    if ( newColor != mColor )
    {
        setColour( newColor );
    }
}
