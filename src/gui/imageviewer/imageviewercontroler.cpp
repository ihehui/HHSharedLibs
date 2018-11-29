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


#include "imageviewercontroler.h"
#include "ui_imageviewercontroler.h"


namespace HEHUI
{


ImageViewerControler::ImageViewerControler(QWidget *parent, Qt::WindowFlags fl) :
    QWidget(parent, fl),
    ui(new Ui::ImageViewerControlerUI)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowModality(Qt::WindowModal);

    setMouseTracking(true);

    m_iconSize = QSize(32, 32);
    m_dragPoint = QPoint(0, 0);

}

ImageViewerControler::~ImageViewerControler()
{
    delete ui;
}

void ImageViewerControler::mousePressEvent(QMouseEvent *event)
{
    if(Qt::LeftButton == event->button()){
        m_dragPoint = event->globalPos() - geometry().topLeft();
        event->accept();
    }
}

void ImageViewerControler::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        //move(event->globalPos() - m_dragPoint);
        emit signalDragged(event->globalPos() - m_dragPoint);
        event->accept();
    }
}

void ImageViewerControler::reset()
{
    ui->dial->setValue(180);
//    setScaleButtonsVisible(true);
//    setRotateButtonsVisible(true);
//    setFlipButtonsVisible(true);
}

void ImageViewerControler::setScaleButtonsVisible(bool visible)
{
    ui->toolButtonZoomIn->setVisible(visible);
    ui->toolButtonZoomOut->setVisible(visible);
    ui->toolButtonZoomFitBest->setVisible(visible);
    ui->toolButtonZoomOriginal->setVisible(visible);
}

void ImageViewerControler::setRotateButtonsVisible(bool visible)
{
    ui->toolButtonRotateLeft->setVisible(visible);
    ui->dial->setVisible(visible);
    ui->toolButtonRotateRight->setVisible(visible);
}

void ImageViewerControler::setFlipButtonsVisible(bool visible)
{
    ui->toolButtonFlipVertical->setVisible(visible);
    ui->toolButtonFlipHorizontal->setVisible(visible);
}

void ImageViewerControler::setSaveImageButtonsVisible(bool visible)
{
    ui->toolButtonSave->setVisible(visible);
    ui->toolButtonSaveAs->setVisible(visible);
}

void ImageViewerControler::addToolButton(QToolButton *button)
{
    if(!button) {
        return;
    }
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    ui->horizontalLayout->addWidget(button);
}

void ImageViewerControler::insertWidget(int index, QWidget *widget, int stretch, Qt::Alignment alignment)
{
    ui->horizontalLayout->insertWidget(index, widget, stretch, alignment);
}

void ImageViewerControler::setIconSize(int width, int height)
{
    setIconSize(QSize(width, height));
}

void ImageViewerControler::setIconSize(const QSize &size)
{
    m_iconSize = size;

    ui->toolButtonRotateLeft->setIconSize(size);
    ui->dial->setMaximumSize(size);
    ui->toolButtonRotateRight->setIconSize(size);
    ui->toolButtonFlipVertical->setIconSize(size);
    ui->toolButtonFlipHorizontal->setIconSize(size);
    ui->toolButtonZoomIn->setIconSize(size);
    ui->toolButtonZoomOut->setIconSize(size);
    ui->toolButtonZoomFitBest->setIconSize(size);
    ui->toolButtonZoomOriginal->setIconSize(size);
    ui->toolButtonSave->setIconSize(size);
    ui->toolButtonSaveAs->setIconSize(size);

}

QSize ImageViewerControler::iconSize() const
{
    return m_iconSize;
}

void ImageViewerControler::pinControler(bool pin)
{
    on_toolButtonPin_clicked(pin);
    ui->toolButtonPin->setChecked(pin);
}

void ImageViewerControler::on_toolButtonRotateLeft_clicked()
{
    int value = ui->dial->value();

    int a = value / 90 - 1;
    if(a <= 0) {
        a = 4;
    }
    ui->dial->setValue(a * 90);
}

void ImageViewerControler::on_dial_valueChanged(int value)
{
    if(value <= 180) {
        emit signalRotate(180 + value);
    } else {
        emit signalRotate(value - 180);
    }
}

void ImageViewerControler::on_toolButtonRotateRight_clicked()
{

    int value = ui->dial->value();

    int a = value / 90 + 1;
    if(a >= 4) {
        a = 0;
    }
    ui->dial->setValue(a * 90);

}

void ImageViewerControler::on_toolButtonPin_clicked(bool checked)
{
    if(checked){
        ui->toolButtonPin->setIcon(QIcon(":/resources/images/pin.png"));
    }else{
        ui->toolButtonPin->setIcon(QIcon(":/resources/images/pin2.png"));
    }

    emit signalPin(checked);
}





} //namespace HEHUI


