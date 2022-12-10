/*
 * File: UnlitDatablockWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#include "UnlitDatablockWidget.h"

#include <ColourEditField.h>
#include <OgreHlmsDatablock.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>


UnlitDatablockWidget::UnlitDatablockWidget()
{
//    QGridLayout* gridLayout = new QGridLayout();
//    int row = 0;

//    createGui(gridLayout, row);

//    setLayout(gridLayout);
}

void UnlitDatablockWidget::createGui(QGridLayout* gridLayout, int& row)
{
    {
        mUseColourCheckBox = new QCheckBox(tr("Use colour"));
        connect(mUseColourCheckBox, SIGNAL(clicked(bool)), this, SLOT(onUnlitDatablockModified()));
        gridLayout->addWidget(mUseColourCheckBox, row++, 1, 1, 1);
    }

    {
        gridLayout->addWidget(new QLabel(tr("Colour:")), row, 0, 1, 1, Qt::AlignRight);
        mColourEditField = new ColourEditField();
        connect(mColourEditField, SIGNAL(colorChanged()), this, SLOT(onUnlitDatablockModified()));
        gridLayout->addWidget(mColourEditField, row++, 1, 1, 1);
    }
}

void UnlitDatablockWidget::setEditedObject(Ogre::HlmsUnlitDatablock* editedDatablock)
{
    mEditedDatablock = editedDatablock;

    unlitDatablockToGui();
}

void UnlitDatablockWidget::unlitDatablockToGui()
{
    {
        QSignalBlocker bl(mUseColourCheckBox);
        mUseColourCheckBox->setChecked(mEditedDatablock ? mEditedDatablock->hasColour() : false);
    }
    {
        QSignalBlocker bl(mColourEditField);
        mColourEditField->setColour(mEditedDatablock ? mEditedDatablock->getColour() : Ogre::ColourValue::White);
    }
}

void UnlitDatablockWidget::onUnlitDatablockModified()
{
    if(!mEditedDatablock) {
        return;
    }

    mEditedDatablock->setUseColour(mUseColourCheckBox->isChecked());
    if(mUseColourCheckBox->isChecked()) {
        mEditedDatablock->setColour(mColourEditField->getColour());
    }

    emit unlitDatablockModified();
}
