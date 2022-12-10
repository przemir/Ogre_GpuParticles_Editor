/*
 * File: SpriteTrackTableWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-22
 *
 */

#ifndef SPRITETRACKTABLEWIDGET_H
#define SPRITETRACKTABLEWIDGET_H

#include "TrackTableModel.h"
#include "TrackTableWidget.h"

#include <QStyledItemDelegate>

#include <GpuParticles/GpuParticleEmitter.h>

class SpriteTrackTableWidget : public TrackTableWidget<GpuParticleEmitter::SpriteCoord>
{
public:

    class Delegate : public QStyledItemDelegate // public QItemDelegate
    {
    public:

        Delegate(TrackTableModel<GpuParticleEmitter::SpriteCoord>& trackTableModel, QObject *parent = 0);

        QWidget *createEditor(QWidget *parent,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

        void setEditorData(QWidget *editor,
                           const QModelIndex &index) const;

        void setModelData(QWidget *editor,
                          QAbstractItemModel *model,
                          const QModelIndex &index) const;

        void updateEditorGeometry(QWidget *editor,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;

        bool hasValue(int row) const;
        GpuParticleEmitter::SpriteCoord displayValue(int row) const;

        void paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        void setSpriteList(const HlmsParticleDatablock::SpriteCoord size, const QVector<QPixmap>& icons, const QStringList& spriteNameList);

        GpuParticleEmitter::SpriteCoord indexToSpriteCoord(int index) const;
        int spriteCoordToIndex(const GpuParticleEmitter::SpriteCoord& value) const;
        QString spriteCoordToString(const GpuParticleEmitter::SpriteCoord& value) const;

    private:
        TrackTableModel<GpuParticleEmitter::SpriteCoord>& mTrackTableModel;
        HlmsParticleDatablock::SpriteCoord mSize;
        QVector<QPixmap> mIcons;
        QStringList mSpriteNameList;
    };

public:
    SpriteTrackTableWidget();

    void setSpriteList(const HlmsParticleDatablock::SpriteCoord size, const QVector<QPixmap>& icons, const QStringList& spriteNameList);
    void refreshSpriteColumn();

protected:

    virtual GpuParticleEmitter::SpriteCoord lerp(const GpuParticleEmitter::SpriteCoord& a, const GpuParticleEmitter::SpriteCoord& b, float percent) const override;

    Delegate* mDelegate;
};

#endif
