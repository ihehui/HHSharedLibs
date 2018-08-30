/*
 ****************************************************************************
 * imageviewercontroler.h
 *
 * Created on: 2010-6-8
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
 * Last Modified on: 2012-8-21
 * Last Modified by: 贺辉
 ***************************************************************************
 */


#ifndef IMAGEVIEWERCONTROLER_H
#define IMAGEVIEWERCONTROLER_H

#include <QWidget>
#include <QToolButton>
#include <QMouseEvent>

namespace Ui
{
class ImageViewerControlerUI;
}

namespace HEHUI
{
class ImageViewerControler : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewerControler(QWidget *parent = 0, Qt::WindowFlags fl = /*Qt::Popup |*/ Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    ~ImageViewerControler();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);



signals:
    void signalRotate(int angle);
    void signalFlip(bool horizontally, bool vertically);

    void signalFlipHorizontal();
    void signalFlipVertical();

    void signalZoomIn();
    void signalZoomOut();
    void signalZoomFitBest();
    void signalZoomOrignal();

    void signalSave();
    void signalSaveAs();

    void signalDragged(const QPoint &globalPoint);

    void signalPin(bool pin);

public slots:
    void reset();
    void setScaleButtonsVisible(bool visible = true);
    void setRotateButtonsVisible(bool visible = true);
    void setFlipButtonsVisible(bool visible = true);
    void setSaveImageButtonsVisible(bool visible = true);

    void addToolButton(QToolButton *button);

    void insertWidget(int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

    void setIconSize(int width, int height);
    void setIconSize(const QSize &size);
    QSize iconSize() const;

    void pinControler(bool pin);


private slots:
    void on_toolButtonRotateLeft_clicked();
    void on_dial_valueChanged(int value);
    void on_toolButtonRotateRight_clicked();

    void on_toolButtonFlipVertical_clicked()
    {
        emit signalFlip(false, true);
    }
    void on_toolButtonFlipHorizontal_clicked()
    {
        emit signalFlip(true, false);
    }

    void on_toolButtonZoomIn_clicked()
    {
        emit signalZoomIn();
    }
    void on_toolButtonZoomOut_clicked()
    {
        emit signalZoomOut();
    }
    void on_toolButtonZoomFitBest_clicked()
    {
        emit signalZoomFitBest();
    }
    void on_toolButtonZoomOriginal_clicked()
    {
        emit signalZoomOrignal();
    }

    void on_toolButtonSave_clicked()
    {
        emit signalSave();
    }
    void on_toolButtonSaveAs_clicked()
    {
        emit signalSaveAs();
    }



    void on_toolButtonPin_clicked(bool checked);


private:
    Ui::ImageViewerControlerUI *ui;

    QSize m_iconSize;

    QPoint m_dragPoint;



};

} //namespace HEHUI

#endif // IMAGEVIEWERCONTROLER_H
