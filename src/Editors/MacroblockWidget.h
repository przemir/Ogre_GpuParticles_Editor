/*
 * File: MacroblockWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#ifndef MACROBLOCKWIDGET_H
#define MACROBLOCKWIDGET_H

#include <OgrePrerequisites.h>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGridLayout;

/// It end up as QObject, because all fields should be in the same QGridLayout
/// to align columns from all block widgets.
class MacroblockWidget : public QObject
{
    Q_OBJECT
public:
    MacroblockWidget();

    /// Fills layout and modifies current grid layout row.
    void createGui(QGridLayout* gridLayout, int& row);

    void setEditedObject(Ogre::HlmsDatablock* editedDatablock, bool caster);
    void macroblockToGui();

    /// Note that Ogre::CullingMode enum starts from 1, not 0.
    static const QStringList CullModeNames;
    /// Note that Ogre::PolygonMode enum starts from 1, not 0.
    static const QStringList PolygonModeNames;

private slots:
    void onMacroblockModified();

signals:
    void macroblockModified();

private:
    Ogre::HlmsDatablock* mEditedDatablock = nullptr;
    bool mEditedDatablockCaster = false;
    QCheckBox* mScissorTestCheckbox;
    QCheckBox* mDepthClampCheckbox;
    QCheckBox* mDepthCheckCheckbox;
    QCheckBox* mDepthWriteCheckbox;
    QDoubleSpinBox* mDepthBiasConstantSpinBox;
    QDoubleSpinBox* mDepthBiasSlopeScaleSpinBox;
    QComboBox* mDepthFuncComboBox;
    QComboBox* mCullModeComboBox;
    QComboBox* mPolygonModeComboBox;
};

#endif
