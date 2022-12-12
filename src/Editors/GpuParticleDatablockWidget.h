/*
 * File: GpuParticleDatablockWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-7-13
 *
 */

#ifndef PARTICLEDATABLOCKWIDGET_H
#define PARTICLEDATABLOCKWIDGET_H

#include <QWidget>
#include <TrackTableModel.h>

class QLabel;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class QToolButton;
class QGroupBox;
class QTableView;
class QTableWidget;
class QSpinBox;

class Point3dWidget;
class EulerDegreeHVSWidget;
class ColourEditField;
class RangeWidget;
class SpriteTrackTableWidget;
class QPushButton;

class ParticleEditorData;
class HlmsParticleDatablock;
class SamplerblockWidget;
class BlendblockWidget;
class MacroblockWidget;
class BaseDatablockWidget;
class UnlitDatablockWidget;

class GpuParticleDatablockWidget : public QWidget
{
    Q_OBJECT
public:
    GpuParticleDatablockWidget(ParticleEditorData& _data);

    void setEditedObject(HlmsParticleDatablock* datablock);
    void particleDatablockToGui();

    void refreshLoadFromResourcesActions();

private:

    HlmsParticleDatablock* mEditedDatablock = nullptr;

    QToolButton* mLoadFromResourcesActionButton;
    QAction* mLoadLastPathAction;
    QGroupBox* mSpriteTrackGroup;
    QToolButton* mShowSpriteTrackButton;
    SpriteTrackTableWidget* mSpriteTrackTableWidget;

//    QPushButton* mImageButton;
    QLabel* mImageLabel;
    QLabel* mImageNameLabel;

    QLabel* mFlipbookSizeLabel;
    QSpinBox* mFlipbookSizeXSpinBox;
    QSpinBox* mFlipbookSizeYSpinBox;

    QTableWidget* mSpriteTableWidget;
    SamplerblockWidget* mSamplerblockWidget;
    BlendblockWidget* mBlendblockWidget;
    MacroblockWidget* mMacroblockWidget;
    BaseDatablockWidget* mBaseDatablockWidget;
    UnlitDatablockWidget* mUnlitDatablockWidget;

    static const int IconSize;

    ParticleEditorData& data;

private:
    void updateImage();
    void updateSpriteTable();

private slots:
    void onParticleDatablockModified();
    void flipbookSizeChanged();
    void imageButtonClicked();
    void loadFromResourceClicked();

signals:
    void particleDatablockModified();
};

#endif
