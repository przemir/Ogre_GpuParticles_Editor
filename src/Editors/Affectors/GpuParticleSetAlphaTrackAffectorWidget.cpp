/*
 * File: GpuParticleSetAlphaTrackAffectorWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include "GpuParticleSetAlphaTrackAffectorWidget.h"
#include <GpuParticles/Affectors/GpuParticleSetAlphaTrackAffector.h>
#include <FloatTrackTableWidget.h>
#include <QGridLayout>
#include <QGroupBox>

GpuParticleSetAlphaTrackAffectorWidget::GpuParticleSetAlphaTrackAffectorWidget()
{
    createGui();
}

void GpuParticleSetAlphaTrackAffectorWidget::createGui()
{
    QGridLayout* grid = new QGridLayout();
    int row = 0;

    {
        mAlphaTrackGroup = new QGroupBox();
        mAlphaTrackGroup->setCheckable(true);
        mAlphaTrackGroup->setTitle("Alpha track");
        connect(mAlphaTrackGroup, SIGNAL(clicked(bool)), this, SLOT(onAffectorModified()));

        {
            QVBoxLayout* layout = new QVBoxLayout();

            {
                mAlphaTrackTableWidget = new FloatTrackTableWidget(0.0f, 1.0f, 0.01f);
                mAlphaTrackTableWidget->setMaxCount(GpuParticleAffectorCommon::MaxTrackValues);
                mAlphaTrackTableWidget->setMinimumHeight(MinimumTrackHeight);

                connect(mAlphaTrackTableWidget, SIGNAL(dataModified()), this, SLOT(onAffectorModified()));
                layout->addWidget(mAlphaTrackTableWidget);
            }

            mAlphaTrackGroup->setLayout(layout);
        }

        grid->addWidget(mAlphaTrackGroup, row++, 0, 1, 2);
    }

    setLayout(grid);
}

void GpuParticleSetAlphaTrackAffectorWidget::setEditedObject(GpuParticleAffector* affector)
{
    mEditedObject = dynamic_cast<GpuParticleSetAlphaTrackAffector*>(affector);

    setEnabled(mEditedObject);

    affectorToGui();
}

void GpuParticleSetAlphaTrackAffectorWidget::affectorToGui()
{
    {
        QSignalBlocker bl(mAlphaTrackGroup);
        mAlphaTrackGroup->setChecked(mEditedObject->mEnabled);
    }

    {
        QVector<QPair<float, float> > values;

        for(GpuParticleAffectorCommon::FloatTrack::iterator it = mEditedObject->mAlphaTrack.begin();
            it != mEditedObject->mAlphaTrack.end(); ++it) {

            float time = it->first;
            float ogreAlpha = it->second;
            values.push_back(qMakePair(time, ogreAlpha));
        }

        QSignalBlocker bl(mAlphaTrackTableWidget);
        mAlphaTrackTableWidget->mTrackTableModel.setValues(values);
    }
}

void GpuParticleSetAlphaTrackAffectorWidget::onAffectorModified()
{
    if(!mEditedObject) {
        return;
    }

    mEditedObject->mEnabled = mAlphaTrackGroup->isChecked();

    {
        QVector<QPair<float, float> > values = mAlphaTrackTableWidget->mTrackTableModel.getValues();
        mEditedObject->mAlphaTrack.clear();
        for (int i = 0; i < values.size(); ++i) {
            float time = values[i].first;
            float ogreAlpha = values[i].second;
            mEditedObject->mAlphaTrack[time] = ogreAlpha;
        }
    }

    emit affectorModified();
}
