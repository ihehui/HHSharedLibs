/*
 ****************************************************************************
 * selecttargetimagewidget.h
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




#ifndef SELECTTARGETIMAGEWIDGET_H
#define SELECTTARGETIMAGEWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>





namespace Ui {
class SelectTargetImageWidgetUI;
}

namespace HEHUI {


class SelectTargetImageWidget : public QWidget
{
    Q_OBJECT


public:
    //SelectTargetImageWidget(const QImage &m_originalImage, const QRect &selectedRect, QWidget *parent = 0, Qt::WindowFlags fl = Qt::Popup | Qt::FramelessWindowHint | Qt::SplashScreen);
    SelectTargetImageWidget(const QImage &m_originalImage, const QRect &selectedRect, QWidget *parent = 0, Qt::WindowFlags fl = Qt::Window | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);

    ~SelectTargetImageWidget();


protected:
    void paintEvent(QPaintEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void resizeEvent(QResizeEvent *event);


    //    void enterEvent(QEvent *event);


signals:
    void imageSelected(const QImage &image);


private slots:
    void saveScreenshot();


private:
    Ui::SelectTargetImageWidgetUI *ui;

    QImage m_originalImage;
    QImage m_resultImage;

    QPoint dragPosition;

};

} //namespace HEHUI

#endif // SELECTTARGETIMAGEWIDGET_H
