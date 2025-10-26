/*
 * File: BlendblockWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#ifndef BLENDBLOCKWIDGET_H
#define BLENDBLOCKWIDGET_H

#include <OgrePrerequisites.h>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QToolButton;
class QGridLayout;

/// It end up as QObject, because all fields should be in the same QGridLayout
/// to align columns from all block widgets.
class BlendblockWidget : public QObject
{
    Q_OBJECT
public:
    BlendblockWidget();

    /// Fills layout and modifies current grid layout row.
    void createGui(QGridLayout* gridLayout, int& row);

    void setEditedObject(Ogre::HlmsDatablock* editedDatablock, bool caster);
    void blendblockToGui();
    void updateIsTransparentAutoToGui();

    static const QStringList SceneBlendFactorNames;
    static const QStringList SceneBlendOperationNames;
    /// Shortcuts to fill SceneBlendFactor.
    static const QStringList SceneBlendTypeNames;

private slots:
    void sceneBlendTypeClicked();
    void onBlendblockModified();

signals:
    void blendblockModified();

private:
    Ogre::HlmsDatablock* mEditedDatablock = nullptr;
    bool mEditedDatablockCaster = false;
    QComboBox* mAlphaToCoverageComboBox;
    QCheckBox* mBlendChannelMaskCheckBox[4];
    QCheckBox* mIsTransparentMaskCheckBox[2];
    QCheckBox* mSeparateBlendCheckBox;

    QToolButton* mPresetsButton;

    QComboBox* mSourceBlendFactorComboBox;
    QComboBox* mDestBlendFactorComboBox;
    QComboBox* mSourceBlendFactorAlphaComboBox;
    QComboBox* mDestBlendFactorAlphaComboBox;

    QComboBox* mBlendOperationComboBox;
    QComboBox* mBlendOperationAlphaComboBox;
};

#endif
