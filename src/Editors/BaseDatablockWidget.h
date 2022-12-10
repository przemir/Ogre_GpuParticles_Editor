/*
 * File: BaseDatablockWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#ifndef BASEDATABLOCKWIDGET_H
#define BASEDATABLOCKWIDGET_H

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
class BaseDatablockWidget : public QObject
{
    Q_OBJECT
public:
    BaseDatablockWidget();

    /// Fills layout and modifies current grid layout row.
    void createGui(QGridLayout* gridLayout, int& row);

    void setEditedObject(Ogre::HlmsUnlitDatablock* editedDatablock);
    void baseDatablockToGui();

    static const QStringList CompareFunctionNames;

private slots:
    void onBaseDatablockModified();

signals:
    void baseDatablockModified();

private:
    Ogre::HlmsDatablock* mEditedDatablock = nullptr;
    QComboBox* mCompareFunctionComboBox;
    QCheckBox* mAlphaTestShadowCasterOnlyCheckBox;
    QDoubleSpinBox* mAlphaTestTresholdSpinBox;
    QDoubleSpinBox* mShadowConstantBiasSpinBox;
};

#endif
