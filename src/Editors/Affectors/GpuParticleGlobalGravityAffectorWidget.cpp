/*
 * File: GpuParticleGlobalGravityAffectorWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include "GpuParticleGlobalGravityAffectorWidget.h"
#include <GpuParticles/Affectors/GpuParticleGlobalGravityAffector.h>
#include <Point3dWidget.h>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

GpuParticleGlobalGravityAffectorWidget::GpuParticleGlobalGravityAffectorWidget()
{
    createGui();
}

void GpuParticleGlobalGravityAffectorWidget::createGui()
{
    QGridLayout* grid = new QGridLayout();
    int row = 0;

    {
        grid->addWidget(new QLabel(tr("Gravity:")), row, 0, 1, 1, Qt::AlignRight);
        mGravityWidget = new Point3dWidget();
        mGravityWidget->setValue(Ogre::Vector3::ZERO, false);
        connect(mGravityWidget, SIGNAL(valueChanged(float,float,float)), this, SLOT(onAffectorModified()));
        grid->addWidget(mGravityWidget, row++, 1, 1, 1);
    }

    QSpacerItem* spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid->addItem(spacerItem, row++, 0);

    setLayout(grid);
}

void GpuParticleGlobalGravityAffectorWidget::setEditedObject(GpuParticleAffector* affector)
{
    mEditedObject = dynamic_cast<GpuParticleGlobalGravityAffector*>(affector);

    setEnabled(mEditedObject);

    affectorToGui();
}

void GpuParticleGlobalGravityAffectorWidget::affectorToGui()
{
    { QSignalBlocker bl(mGravityWidget); mGravityWidget->setValue(mEditedObject->mGravity, false); }
}

void GpuParticleGlobalGravityAffectorWidget::onAffectorModified()
{
    if(!mEditedObject) {
        return;
    }

    mEditedObject->mGravity = mGravityWidget->getValue();

    emit affectorModified();
}
