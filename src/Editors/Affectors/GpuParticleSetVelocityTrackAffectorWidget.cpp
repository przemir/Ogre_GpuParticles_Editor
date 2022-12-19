/*
 * File: GpuParticleSetVelocityTrackAffectorWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include "GpuParticleSetVelocityTrackAffectorWidget.h"
#include <GpuParticles/Affectors/GpuParticleSetVelocityTrackAffector.h>
#include <FloatTrackTableWidget.h>
#include <QGridLayout>
#include <QGroupBox>

GpuParticleSetVelocityTrackAffectorWidget::GpuParticleSetVelocityTrackAffectorWidget()
{
    createGui();
}

void GpuParticleSetVelocityTrackAffectorWidget::createGui()
{
    QGridLayout* grid = new QGridLayout();
    int row = 0;

    {
        mVelocityTrackGroup = new QGroupBox();
        mVelocityTrackGroup->setCheckable(true);
        mVelocityTrackGroup->setTitle("Velocity track");
        connect(mVelocityTrackGroup, SIGNAL(clicked(bool)), this, SLOT(onAffectorModified()));

        {
            QVBoxLayout* layout = new QVBoxLayout();

            {
                mVelocityTrackTableWidget = new FloatTrackTableWidget(0.0f, 100.0f, 0.01f);
                mVelocityTrackTableWidget->setMaxCount(GpuParticleAffectorCommon::MaxTrackValues);
                mVelocityTrackTableWidget->setMinimumHeight(MinimumTrackHeight);

                connect(mVelocityTrackTableWidget, SIGNAL(dataModified()), this, SLOT(onAffectorModified()));
                layout->addWidget(mVelocityTrackTableWidget);
            }

            mVelocityTrackGroup->setLayout(layout);
        }

        grid->addWidget(mVelocityTrackGroup, row++, 0, 1, 2);
    }

    setLayout(grid);
}

void GpuParticleSetVelocityTrackAffectorWidget::setEditedObject(GpuParticleAffector* affector)
{
    mEditedObject = dynamic_cast<GpuParticleSetVelocityTrackAffector*>(affector);

    setEnabled(mEditedObject);

    affectorToGui();
}

void GpuParticleSetVelocityTrackAffectorWidget::affectorToGui()
{
    {
        QSignalBlocker bl(mVelocityTrackGroup);
        mVelocityTrackGroup->setChecked(mEditedObject->mEnabled);
    }

    {
        QVector<QPair<float, float> > values;

        for(GpuParticleAffectorCommon::FloatTrack::iterator it = mEditedObject->mVelocityTrack.begin();
            it != mEditedObject->mVelocityTrack.end(); ++it) {

            float time = it->first;
            float ogreVelocity = it->second;
            values.push_back(qMakePair(time, ogreVelocity));
        }

        QSignalBlocker bl(mVelocityTrackTableWidget);
        mVelocityTrackTableWidget->mTrackTableModel.setValues(values);
    }
}

void GpuParticleSetVelocityTrackAffectorWidget::onAffectorModified()
{
    if(!mEditedObject) {
        return;
    }

    mEditedObject->mEnabled = mVelocityTrackGroup->isChecked();

    {
        QVector<QPair<float, float> > values = mVelocityTrackTableWidget->mTrackTableModel.getValues();
        mEditedObject->mVelocityTrack.clear();
        for (int i = 0; i < values.size(); ++i) {
            float time = values[i].first;
            float ogreVelocity = values[i].second;
            mEditedObject->mVelocityTrack[time] = ogreVelocity;
        }
    }

    emit affectorModified();
}
