/*
 * File: SamplerblockWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#ifndef SAMPLERBLOCKWIDGET_H
#define SAMPLERBLOCKWIDGET_H

#include <OgreHlmsUnlitDatablock.h>
#include <QDoubleValidator>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class ColourEditField;
class QLineEdit;
class QGridLayout;

/// It end up as QObject, because all fields should be in the same QGridLayout
/// to align columns from all block widgets.
class SamplerblockWidget : public QObject
{
    Q_OBJECT
public:
    SamplerblockWidget();

    /// Fills layout and modifies current grid layout row.
    void createGui(QGridLayout* gridLayout, int& row);

    void setEditedObject(Ogre::HlmsUnlitDatablock* editedDatablock, Ogre::uint8 textureSlotIndex);
    void samplerblockToGui();

    static const QStringList FilterOptionsNames;
    static const QStringList TextureAddressingModeNames;

private slots:
    void onSamplerblockModified();

signals:
    void samplerblockModified();

private:
    Ogre::HlmsUnlitDatablock* mEditedDatablock = nullptr;
    Ogre::uint8 mEditedDatablockTextureSlotIndex = 0;
    QComboBox* mMinFilterComboBox;
    QComboBox* mMagFilterComboBox;
    QComboBox* mMipFilterComboBox;
    QComboBox* mUComboBox;
    QComboBox* mVComboBox;
    QComboBox* mWComboBox;
    QDoubleSpinBox* mMipLodBiasSpinBox;
    QDoubleSpinBox* mMaxAnisotropySpinBox;
    QComboBox* mCompareFunctionComboBox;
    ColourEditField* mBorderColourEditField;
    QLineEdit* mMinLodSpinBox;
    QLineEdit* mMaxLodSpinBox;
};

#endif
