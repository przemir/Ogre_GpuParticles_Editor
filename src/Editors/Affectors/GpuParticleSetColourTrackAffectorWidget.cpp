/*
 * File: GpuParticleSetColourTrackAffectorWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2022-12-15
 *
 */

#include "GpuParticleSetColourTrackAffectorWidget.h"
#include <GpuParticles/Affectors/GpuParticleSetColourTrackAffector.h>
#include <ColourTrackTableWidget.h>
#include <QGridLayout>
#include <QGroupBox>

GpuParticleSetColourTrackAffectorWidget::GpuParticleSetColourTrackAffectorWidget()
{
    createGui();
}

void GpuParticleSetColourTrackAffectorWidget::createGui()
{
    QGridLayout* grid = new QGridLayout();
    int row = 0;

    {
        mColourTrackGroup = new QGroupBox();
        mColourTrackGroup->setCheckable(true);
        mColourTrackGroup->setTitle("Colour track");
        connect(mColourTrackGroup, SIGNAL(clicked(bool)), this, SLOT(onAffectorModified()));

        {
            QVBoxLayout* layout = new QVBoxLayout();

            {
                mColourTrackTableWidget = new ColourTrackTableWidget();
                mColourTrackTableWidget->setMaxCount(GpuParticleAffectorCommon::MaxTrackValues);
                mColourTrackTableWidget->setMinimumHeight(MinimumTrackHeight);

                connect(mColourTrackTableWidget, SIGNAL(dataModified()), this, SLOT(onAffectorModified()));
                layout->addWidget(mColourTrackTableWidget);
            }

            mColourTrackGroup->setLayout(layout);
        }

        grid->addWidget(mColourTrackGroup, row++, 0, 1, 2);
    }

    setLayout(grid);
}

void GpuParticleSetColourTrackAffectorWidget::affectorToGui()
{
    {
        QSignalBlocker bl(mColourTrackGroup);
        mColourTrackGroup->setChecked(mEditedObject->mEnabled);
    }

    {
        QVector<QPair<float, Ogre::ColourValue> > values;

        for(GpuParticleAffectorCommon::Vector3Track::iterator it = mEditedObject->mColourTrack.begin();
            it != mEditedObject->mColourTrack.end(); ++it) {

            float time = it->first;
            Ogre::ColourValue ogreColour(it->second.x, it->second.y, it->second.z, 1.0f);
            values.push_back(qMakePair(time, ogreColour));
        }

        QSignalBlocker bl(mColourTrackTableWidget);
        mColourTrackTableWidget->mTrackTableModel.setValues(values);
    }
}

void GpuParticleSetColourTrackAffectorWidget::onAffectorModified()
{
    if(!mEditedObject) {
        return;
    }

    mEditedObject->mEnabled = mColourTrackGroup->isChecked();

    {
        QVector<QPair<float, Ogre::ColourValue> > values = mColourTrackTableWidget->mTrackTableModel.getValues();
        mEditedObject->mColourTrack.clear();
        for (int i = 0; i < values.size(); ++i) {
            float time = values[i].first;
            Ogre::ColourValue ogreColour = values[i].second;
            mEditedObject->mColourTrack[time] = Ogre::Vector3(ogreColour.r, ogreColour.g, ogreColour.b);
        }
    }

    emit affectorModified();
}
