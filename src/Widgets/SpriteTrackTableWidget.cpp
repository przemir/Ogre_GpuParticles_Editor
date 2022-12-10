/*
 * File: SpriteTrackTableWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-7-22
 *
 */

#include "SpriteTrackTableWidget.h"

#include <QApplication>
#include <QComboBox>

SpriteTrackTableWidget::Delegate::Delegate(TrackTableModel<GpuParticleEmitter::SpriteCoord>& trackTableModel,
                                           QObject *parent)
    : QStyledItemDelegate(parent)
    , mTrackTableModel(trackTableModel)
{
}

QWidget* SpriteTrackTableWidget::Delegate::createEditor(QWidget *parent,
                                                        const QStyleOptionViewItem &option,
                                                        const QModelIndex &index) const
{
    QComboBox* field = new QComboBox(parent);
    field->setIconSize(QSize(32, 32));

    for (int i = 0; i < mIcons.size(); ++i) {
        GpuParticleEmitter::SpriteCoord coord = indexToSpriteCoord(i);
        QString coordStr = spriteCoordToString(coord);
        if(!mSpriteNameList[i].isEmpty()) {
            coordStr += " " + mSpriteNameList[i];
        }
        field->addItem(mIcons[i], coordStr);
    }

    if(index.row() >= 0 && index.row() < mTrackTableModel.getValues().size()) {
        GpuParticleEmitter::SpriteCoord value = mTrackTableModel.getValues()[index.row()].second;
        int comboIndex = spriteCoordToIndex(value);
        field->setCurrentIndex(comboIndex);
    }

    return field;
}

void SpriteTrackTableWidget::Delegate::setEditorData(QWidget *editor,
                                                     const QModelIndex &index) const
{
    QComboBox* field = dynamic_cast<QComboBox*>(editor);

    if(index.row() >= 0 && index.row() < mTrackTableModel.getValues().size()) {
        GpuParticleEmitter::SpriteCoord value = mTrackTableModel.getValues()[index.row()].second;
        int comboIndex = spriteCoordToIndex(value);
        field->setCurrentIndex(comboIndex);
    }
}

void SpriteTrackTableWidget::Delegate::setModelData(QWidget *editor,
                                                    QAbstractItemModel *model,
                                                    const QModelIndex &index) const
{
    QComboBox* field = dynamic_cast<QComboBox*>(editor);
    if(field->currentIndex() >= 0) {
        GpuParticleEmitter::SpriteCoord value = indexToSpriteCoord(field->currentIndex());
        mTrackTableModel.setRowSecondValue(index.row(), value);
    }
}

void SpriteTrackTableWidget::Delegate::updateEditorGeometry(QWidget *editor,
                                                            const QStyleOptionViewItem &option,
                                                            const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool SpriteTrackTableWidget::Delegate::hasValue(int row) const
{
    return row >= 0 && row < mTrackTableModel.getValues().size();
}

GpuParticleEmitter::SpriteCoord SpriteTrackTableWidget::Delegate::displayValue(int row) const
{
    return mTrackTableModel.getValues()[row].second;
}

void SpriteTrackTableWidget::Delegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int row = index.row();

    GpuParticleEmitter::SpriteCoord value = displayValue(row);

    QPixmap icon;
    if(!mIcons.isEmpty() && value.col >= 0 && value.row >= 0 && value.col < mSize.col && value.row < mSize.row) {
        int spriteIndex = spriteCoordToIndex(value);
        icon = mIcons[spriteIndex];
    }

    // Get item style settings
    QStyleOptionViewItem myOption = option;
    myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    myOption.decorationAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    myOption.icon = icon;
    myOption.text = spriteCoordToString(value);
    myOption.features = QStyleOptionViewItem::ViewItemFeatures(QStyleOptionViewItem::HasDisplay | QStyleOptionViewItem::HasDecoration);
    myOption.viewItemPosition = QStyleOptionViewItem::Beginning;
    myOption.decorationPosition = QStyleOptionViewItem::Left;
    myOption.decorationSize = QSize(32, 32);

    return QStyledItemDelegate::paint(painter, myOption, index);
}

void SpriteTrackTableWidget::Delegate::setSpriteList(const HlmsParticleDatablock::SpriteCoord size, const QVector<QPixmap>& icons, const QStringList& spriteNameList)
{
    mSize = size;
    mIcons = icons;
    mSpriteNameList = spriteNameList;
}

GpuParticleEmitter::SpriteCoord SpriteTrackTableWidget::Delegate::indexToSpriteCoord(int index) const
{
    GpuParticleEmitter::SpriteCoord coord;
    coord.col = index % mSize.col;
    coord.row = index / mSize.row;
    return coord;
}

int SpriteTrackTableWidget::Delegate::spriteCoordToIndex(const GpuParticleEmitter::SpriteCoord& value) const
{
    return value.col + value.row * mSize.col;
}

QString SpriteTrackTableWidget::Delegate::spriteCoordToString(const GpuParticleEmitter::SpriteCoord& value) const
{
    return tr("(col %1, row %2)").arg(value.col).arg(value.row);
}

SpriteTrackTableWidget::SpriteTrackTableWidget()
{
    mDelegate = new Delegate(mTrackTableModel, this);
    setItemDelegateForColumn(1, mDelegate);
}

void SpriteTrackTableWidget::setSpriteList(const HlmsParticleDatablock::SpriteCoord size, const QVector<QPixmap>& icons, const QStringList& spriteNameList)
{
    mDelegate->setSpriteList(size, icons, spriteNameList);
}

void SpriteTrackTableWidget::refreshSpriteColumn()
{
    if(model()->rowCount() > 0) {
        dataChanged(model()->index(0, 1), model()->index(model()->rowCount()-1, 1));
    }
}

GpuParticleEmitter::SpriteCoord SpriteTrackTableWidget::lerp(const GpuParticleEmitter::SpriteCoord& a, const GpuParticleEmitter::SpriteCoord& b, float percent) const
{
    return a;
}
