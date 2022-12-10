/*
 * File: ColourEditField.cpp
 * Author: Przemysław Bągard
 * Created: 2021-3-15
 *
 */

#include "ColourButton.h"
#include "ColourEditField.h"

#include <QBoxLayout>
#include <QLineEdit>

ColourEditField::ColourEditField(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout();

    mColourButton = new ColourButton();
    layout->addWidget(mColourButton);
    connect(mColourButton, SIGNAL(colorChanged()), this, SLOT(colourButtonPressed()));

    mLineEdit = new QLineEdit();
    mLineEdit->setReadOnly(true);
    layout->addWidget(mLineEdit);

    setLayout(layout);
}

void ColourEditField::setColour(const Ogre::ColourValue& colour)
{
    QSignalBlocker bl(mColourButton);
    mColourButton->setColour(colour);
    updateLineEdit();

    emit colorChanged();
}

Ogre::ColourValue ColourEditField::getColour() const
{
    return mColourButton->getOgreColour();
}

void ColourEditField::updateLineEdit()
{
    Ogre::ColourValue colour = getColour();
    if(mUseAlpha) {
        QString str = QString("(%1, %2, %3, %4)").arg(colour.r).arg(colour.g).arg(colour.b).arg(colour.a);
        mLineEdit->setText(str);
    }
    else {
        QString str = QString("(%1, %2, %3)").arg(colour.r).arg(colour.g).arg(colour.b);
        mLineEdit->setText(str);
    }
}

void ColourEditField::colourButtonPressed()
{
    updateLineEdit();

    emit colorChanged();
}
