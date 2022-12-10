/*
 * File: UnlitDatablockWidget.h
 * Author: Przemysław Bągard
 * Created: 2022-12-8
 *
 */

#ifndef UNLITDATABLOCKWIDGET_H
#define UNLITDATABLOCKWIDGET_H

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
class UnlitDatablockWidget : public QObject
{
    Q_OBJECT
public:
    UnlitDatablockWidget();

    /// Fills layout and modifies current grid layout row.
    void createGui(QGridLayout* gridLayout, int& row);

    void setEditedObject(Ogre::HlmsUnlitDatablock* editedDatablock);
    void unlitDatablockToGui();

private slots:
    void onUnlitDatablockModified();

signals:
    void unlitDatablockModified();

private:
    Ogre::HlmsUnlitDatablock* mEditedDatablock = nullptr;
    QCheckBox* mUseColourCheckBox;
    ColourEditField* mColourEditField;
};

#endif
