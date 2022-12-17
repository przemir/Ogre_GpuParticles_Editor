/*
 * File: ResizableStackedWidget.h
 * Author: Przemysław Bągard
 * Created: 2021-2-9
 *
 */

#ifndef RESIZABLESTACKEDWIDGET_H
#define RESIZABLESTACKEDWIDGET_H

#include <QStackedWidget>

class QVBoxLayout;

/// VBoxLayout widget with only one widget visible.
class ResizableStackedWidget : public QWidget
{
    Q_OBJECT
public:
    ResizableStackedWidget(QWidget* parent = nullptr);

    void addWidget(QWidget* pWidget);
    void setCurrentIndex(int index);
    int indexOf(QWidget* pWidget);
    int count() const;

private:
    int mCurrentIndex;
    QVBoxLayout* mLayout;
    QVector<QWidget*> mWidgets;

signals:
    void currentIndexChanged(int);
};

#endif
