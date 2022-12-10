/*
 * File: ImageChooserDialog.h
 * Author: Przemysław Bągard
 * Created: 2022-12-5
 *
 */

#ifndef IMAGECHOOSERDIALOG_H
#define IMAGECHOOSERDIALOG_H

#include <OgrePrerequisites.h>
#include <QDialog>

class QListView;
class QStandardItemModel;
class QSortFilterProxyModel;
class QLineEdit;
class QCheckBox;

class ParticleEditorData;

class ImageChooserDialog : public QDialog
{
    Q_OBJECT
public:
    ImageChooserDialog(ParticleEditorData& _data,
                       const QSize& size,
                       QWidget *parent = nullptr);

private:

    void createGui();
    void createListItems();

private:
    ParticleEditorData& data;
    QLineEdit* mSearchEdit;
    QCheckBox* mSearchUseWildcardsCheckBox;
    QListView* mListView;
    QStandardItemModel* mListModel;
    QSortFilterProxyModel* mListSortFilterProxyModel;

private slots:
    void searchEditChanged();
    void slotDoubleClicked();
};

#endif
