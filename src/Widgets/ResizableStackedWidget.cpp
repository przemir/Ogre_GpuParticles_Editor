/*
 * File: ResizableStackedWidget.cpp
 * Author: Przemysław Bągard
 * Created: 2021-2-9
 *
 */

#include "ResizableStackedWidget.h"

#include <QBoxLayout>

ResizableStackedWidget::ResizableStackedWidget(QWidget* parent)
    : QWidget(parent)
{
    mLayout = new QVBoxLayout();
    mLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mLayout);

    mCurrentIndex = -1;
}

void ResizableStackedWidget::addWidget(QWidget* pWidget)
{
    mLayout->addWidget(pWidget);
    mWidgets.push_back(pWidget);

    if(mWidgets.size() != 1) {
        pWidget->setVisible(false);
        mCurrentIndex = 0;
    }
}

void ResizableStackedWidget::setCurrentIndex(int index)
{
    if(mCurrentIndex < mWidgets.size() && mWidgets[mCurrentIndex]) {
        mWidgets[mCurrentIndex]->setVisible(false);
    }

    mCurrentIndex = index;

    if(mCurrentIndex < mWidgets.size() && mWidgets[mCurrentIndex]) {
        mWidgets[mCurrentIndex]->setVisible(true);
    }

    emit currentIndexChanged(mCurrentIndex);
}

int ResizableStackedWidget::indexOf(QWidget* pWidget)
{
    for (int i = 0; i < mWidgets.size(); ++i) {
        if(mWidgets[i] == pWidget) {
            return i;
        }
    }
    return -1;
}

int ResizableStackedWidget::count() const
{
    return mWidgets.size();
}
