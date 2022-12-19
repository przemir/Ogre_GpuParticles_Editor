/*
 * File: GpuParticleDepthCollisionAffectorWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include "GpuParticleDepthCollisionAffectorWidget.h"
#include <GpuParticles/Affectors/GpuParticleDepthCollisionAffector.h>
#include <Point3dWidget.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

GpuParticleDepthCollisionAffectorWidget::GpuParticleDepthCollisionAffectorWidget()
{
    createGui();
}

void GpuParticleDepthCollisionAffectorWidget::createGui()
{
    QGridLayout* grid = new QGridLayout();
    int row = 0;

    {
//        grid->addWidget(new QLabel(tr("Gravity:")), row, 0, 1, 1, Qt::AlignRight);
        mEnabledCheckBox = new QCheckBox(tr("Enabled"));
        mEnabledCheckBox->setChecked(false);
        connect(mEnabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(onAffectorModified()));
        grid->addWidget(mEnabledCheckBox, row++, 1, 1, 1);
    }

    QSpacerItem* spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid->addItem(spacerItem, row++, 0);

    setLayout(grid);
}

void GpuParticleDepthCollisionAffectorWidget::setEditedObject(GpuParticleAffector* affector)
{
    mEditedObject = dynamic_cast<GpuParticleDepthCollisionAffector*>(affector);

    setEnabled(mEditedObject);

    affectorToGui();
}

void GpuParticleDepthCollisionAffectorWidget::affectorToGui()
{
    { QSignalBlocker bl(mEnabledCheckBox); mEnabledCheckBox->setChecked(mEditedObject->mEnabled); }
}

void GpuParticleDepthCollisionAffectorWidget::onAffectorModified()
{
    if(!mEditedObject) {
        return;
    }

    mEditedObject->mEnabled = mEnabledCheckBox->isChecked();

    emit affectorModified();
}
