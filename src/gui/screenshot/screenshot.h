/*
 ****************************************************************************
 * screenshot.h
 *
 * Created on: 2011-7-27
 *     Author: 贺辉
 *    License: LGPL
 *    Comment:
 *
 *
 *    =============================  Usage  =============================
 *|
 *|
 *    ===================================================================
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ****************************************************************************
 */

/*
 ***************************************************************************
 * Last Modified on: 2011-7-27
 * Last Modified by: 贺辉
 ***************************************************************************
 */



#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QWidget>
#include <QEvent>



#include "selecttargetimagewidget.h"

#include "../guilib.h"



namespace Ui {
class ScreenshotUI;
}

namespace HEHUI {


class GUI_LIB_API Screenshot : public QWidget
{
    Q_OBJECT

public:
    //Screenshot(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Popup | Qt::FramelessWindowHint | Qt::SplashScreen);
    Screenshot(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    ~Screenshot();

protected:
    bool event(QEvent *e);

signals:
    void imageSelected(const QImage &image);

public slots:


private:
    void drawBackground();

    void showSelectedRect();
    void updateSelectedRect();

private:
    Ui::ScreenshotUI *ui;

    SelectTargetImageWidget * m_targetPixmapWidget;

    QPixmap originalPixmap;

    QPixmap resultPixmap;

    //    QImage sourceImage;
    //    QImage destinationImage;
    QImage backgroundImage;

    QPoint topLeft;
    QPoint bottomRight;

};

} //namespace HEHUI

#endif // SCREENSHOT_H
