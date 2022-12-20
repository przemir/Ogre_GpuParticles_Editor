/*
 * File: GpuParticleSetSizeTrackAffectorWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-19
 *
 */

#include "GpuParticleSetSizeTrackAffectorWidget.h"
#include <GpuParticles/Affectors/GpuParticleSetSizeTrackAffector.h>
#include <Vector2TrackTableWidget.h>
#include <QGridLayout>
#include <QGroupBox>

GpuParticleSetSizeTrackAffectorWidget::GpuParticleSetSizeTrackAffectorWidget()
{
    createGui();
}

void GpuParticleSetSizeTrackAffectorWidget::createGui()
{
    QGridLayout* grid = new QGridLayout();
    int row = 0;

    {
        mSizeTrackGroup = new QGroupBox();
        mSizeTrackGroup->setCheckable(true);
        mSizeTrackGroup->setTitle("Size track");
        connect(mSizeTrackGroup, SIGNAL(clicked(bool)), this, SLOT(onAffectorModified()));

        {
            QVBoxLayout* layout = new QVBoxLayout();

            {
                mSizeTrackTableWidget = new Vector2TrackTableWidget();
                mSizeTrackTableWidget->setMaxCount(GpuParticleAffectorCommon::MaxTrackValues);
                mSizeTrackTableWidget->setMinimumHeight(MinimumTrackHeight);

                connect(mSizeTrackTableWidget, SIGNAL(dataModified()), this, SLOT(onAffectorModified()));
                layout->addWidget(mSizeTrackTableWidget);
            }

            mSizeTrackGroup->setLayout(layout);
        }

        grid->addWidget(mSizeTrackGroup, row++, 0, 1, 2);
    }

    setLayout(grid);
}

void GpuParticleSetSizeTrackAffectorWidget::affectorToGui()
{
    {
        QSignalBlocker bl(mSizeTrackGroup);
        mSizeTrackGroup->setChecked(mEditedObject->mEnabled);
    }

    {
        QVector<QPair<float, Ogre::Vector2> > values;

        for(GpuParticleAffectorCommon::Vector2Track::iterator it = mEditedObject->mSizeTrack.begin();
            it != mEditedObject->mSizeTrack.end(); ++it) {

            float time = it->first;
            Ogre::Vector2 ogreSize(it->second.x, it->second.y);
            values.push_back(qMakePair(time, ogreSize));
        }

        QSignalBlocker bl(mSizeTrackTableWidget);
        mSizeTrackTableWidget->mTrackTableModel.setValues(values);
    }
}

void GpuParticleSetSizeTrackAffectorWidget::onAffectorModified()
{
    if(!mEditedObject) {
        return;
    }

    mEditedObject->mEnabled = mSizeTrackGroup->isChecked();

    {
        QVector<QPair<float, Ogre::Vector2> > values = mSizeTrackTableWidget->mTrackTableModel.getValues();
        mEditedObject->mSizeTrack.clear();
        for (int i = 0; i < values.size(); ++i) {
            float time = values[i].first;
            Ogre::Vector2 ogreSize = values[i].second;
            mEditedObject->mSizeTrack[time] = Ogre::Vector2(ogreSize.x, ogreSize.y);
        }
    }

    emit affectorModified();
}
