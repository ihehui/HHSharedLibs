/*
 ****************************************************************************
 * selecttargetimagewidget.cpp
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





#include <QDebug>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QClipboard>
#include <QMessageBox>

#include <QToolTip>

#include "selecttargetimagewidget.h"
#include "ui_selecttargetimagewidget.h"



namespace HEHUI {


SelectTargetImageWidget::SelectTargetImageWidget(const QImage &originalImage, const QRect &selectedRect, QWidget *parent, Qt::WindowFlags fl)
    : QWidget(parent, fl), m_originalImage(originalImage),
      ui(new Ui::SelectTargetImageWidgetUI)
{

    ui->setupUi(this);

    Q_ASSERT_X(!originalImage.isNull(), "TargetPixmapWidget::TargetPixmapWidget(...)", "Invalid Pixmap!");

    move(selectedRect.topLeft());
    resize(selectedRect.width(), selectedRect.height());


    //setBackgroundRole(QPalette::Base);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //    QPalette palette;
    //    palette.setColor(backgroundRole(), Qt::blue);
    //    setPalette(palette);

    //setWindowModality(Qt::ApplicationModal);




}

SelectTargetImageWidget::~SelectTargetImageWidget()
{
    delete ui;
}

void SelectTargetImageWidget::paintEvent(QPaintEvent *)
{

    QRect r = geometry();
    QPoint topLeftPos = r.topLeft();
    m_resultImage = m_originalImage.copy(r);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

    //painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.drawImage(0, 0, m_resultImage);


    //    QPen pen;
    //    pen.setColor(Qt::blue);
    //    pen.setWidth(5);
    //    painter.setPen(pen);
    //    painter.drawRect(rect());


    //    qDebug();
    //    qDebug()<<"rect(): x:"<<rect().x()<<" y:"<<rect().y()<<" w:"<<rect().width()<<" h:"<<rect().height();
    //    qDebug()<<"mapToGlobal(rect().topLeft()): x:"<<mapToGlobal(rect().topLeft()).x()<<" y:"<<mapToGlobal(rect().topLeft()).y()<<" w:"<<rect().width()<<" h:"<<rect().height();

    //    qDebug()<<"frameGeometry(): x:"<<frameGeometry().x()<<" y:"<<frameGeometry().y()<<" w:"<<frameGeometry().width()<<" h:"<<frameGeometry().height();
    //    qDebug()<<"mapTo(this,frameGeometry().topLeft()): x:"<<mapTo(this,frameGeometry().topLeft()).x()<<" y:"<<mapTo(this,frameGeometry().topLeft()).y()<<" w:"<<frameGeometry().width()<<" h:"<<frameGeometry().height();


    painter.end();

    QToolTip::showText(topLeftPos, QString("Size:%1*%2").arg(r.width()).arg(r.height()));


}

void SelectTargetImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        //dragPosition = event->globalPos() - frameGeometry().topLeft();
        dragPosition = event->globalPos() - geometry().topLeft();
        //setCursor(Qt::OpenHandCursor);
        update();
        event->accept();
    }
}

void SelectTargetImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {

        QPoint newTopLeft = event->globalPos() - dragPosition;
        QPoint newBottomRight = newTopLeft + QPoint(frameGeometry().width(), frameGeometry().height());
        if(newTopLeft.x() <= 0){
            newTopLeft.setX(0);
        }
        if(newTopLeft.y() <= 0){
            newTopLeft.setY(0);
        }
        if(newBottomRight.x() >= m_originalImage.width()){
            newTopLeft.setX(m_originalImage.width() - frameGeometry().width());
        }
        if(newBottomRight.y() >= m_originalImage.height()){
            newTopLeft.setY(m_originalImage.height() - frameGeometry().height());
        }

        //move(event->globalPos() - dragPosition);
        move(newTopLeft);
        update();

        event->accept();

    }

}





void SelectTargetImageWidget::keyReleaseEvent(QKeyEvent *  event){

    if(event->key() == Qt::Key_Escape){
        close();
        event->accept();
    }

    if(QApplication::keyboardModifiers() == Qt::ControlModifier && event->key() == Qt::Key_S){
        saveScreenshot();
        event->accept();
    }

}

void SelectTargetImageWidget::mouseReleaseEvent(QMouseEvent *event){
    setCursor(Qt::ArrowCursor);
}

void SelectTargetImageWidget::mouseDoubleClickEvent(QMouseEvent *event){
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(m_resultImage);

    emit imageSelected(m_resultImage);


}

void SelectTargetImageWidget::resizeEvent(QResizeEvent * /* event */)
{

    //    int side = qMin(width(), height());
    //    QRegion maskedRegion(-2, -2, frameGeometry().width(), frameGeometry().height(), QRegion::Rectangle);

    //    setMask(maskedRegion);

    //update();

}

//void TargetPixmapWidget::enterEvent(QEvent *event){
//    //QMouseEvent *e = static_cast<QMouseEvent *> (event);
//    QPoint mousePos = mapToGlobal(QCursor::pos());
//    QPoint windowPos = mapToGlobal(geometry().topLeft());

//    QPoint div = mousePos - windowPos;

//    if( (div.x() == 0 && div.y() == 0) || (div.x() == geometry().width() && div.y() == geometry().height()) ){
//        setCursor(Qt::SizeFDiagCursor);
//        event->accept();
//        return;
//    }

//    if( (div.x() == 0 && div.y() == geometry().height()) || (div.x() == geometry().width() && div.y() == 0) ){
//        setCursor(Qt::SizeBDiagCursor);
//        event->accept();
//        return;
//    }

//    QToolTip::showText(mousePos, QString("mousePos:%1 %2<br>windowTopLeft:%3 %4<br>windowBottomRight:%5 %6")
//                       .arg(mousePos.x()).arg(mousePos.y())
//                       .arg(windowPos.x()).arg(windowPos.y())
//                       .arg(mapToGlobal(geometry().bottomRight()).x()).arg(mapToGlobal(geometry().bottomRight()).y())

//                       );

//    if(mousePos.x() == windowPos.x() || mousePos.x() == geometry().bottomRight().x() ){
//        setCursor(Qt::SizeHorCursor);
//        event->accept();
//    }else{
//        setCursor(Qt::ArrowCursor);
//    }

//    if(mousePos.y() == windowPos.y() || mousePos.y() == geometry().bottomRight().y() ){
//        setCursor(Qt::SizeVerCursor);
//        event->accept();
//    }else{
//        setCursor(Qt::ArrowCursor);
//    }

//    setCursor(Qt::ArrowCursor);

//}

void SelectTargetImageWidget::saveScreenshot()
{
    QString format = "png";
    QString initialPath = QDir::currentPath() + tr("/Screenshot%1.%2").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss")).arg(format) ;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), initialPath,
                                                    tr("%1 Files (*.%2);;All Files (*)").arg(format.toUpper()).arg(format));

    if (!fileName.isEmpty()){
        if(!m_resultImage.save(fileName, format.toLatin1())){
            QMessageBox::critical(this, tr("Error"), tr("Can not save image!"));
        }
    }

}





} //namespace HEHUI
