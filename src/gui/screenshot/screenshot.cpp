/*
 ****************************************************************************
 * screenshot.cpp
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



#include <QDesktopWidget>
#include <QPainter>
#include <QDir>
#include <QFileDialog>
#include <QKeyEvent>
#include <QToolTip>
#include <QScreen>

#include "screenshot.h"

#include "ui_screenshot.h"


namespace HEHUI
{


Screenshot::Screenshot(QWidget *parent, Qt::WindowFlags fl)
    : QWidget(parent, fl),
      ui(new Ui::ScreenshotUI)
{
    ui->setupUi(this);

    //    originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        originalPixmap = screen->grabWindow(0);
    } else {
        originalPixmap = QPixmap();
    }

    backgroundImage = QImage(originalPixmap.size(), QImage::Format_ARGB32_Premultiplied);

    drawBackground();

    showFullScreen();

    m_targetPixmapWidget = 0;

}

Screenshot::~Screenshot()
{
    delete ui;
}

bool Screenshot::event(QEvent *e)
{

    switch(e->type()) {
    case QEvent::KeyRelease: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *> (e);
        if(keyEvent->key() == Qt::Key_Escape) {
            if(m_targetPixmapWidget) {
                m_targetPixmapWidget->close();
            }
            close();
            return true;
        }
    }
    break;
    case QEvent::MouseButtonPress: {
        QMouseEvent *event = static_cast<QMouseEvent *> (e);
        if(event->button() == Qt::LeftButton) {
            topLeft = event->globalPos();
            bottomRight = QPoint(topLeft.x() + 5, topLeft.y() + 5);
            if(!m_targetPixmapWidget) {
                showSelectedRect();
                QToolTip::showText(topLeft, QString("Press X:%1 Y:%2").arg(topLeft.x()).arg(topLeft.y()));
                return true;
            } else {
                //m_targetPixmapWidget->resize(0,0);
                updateSelectedRect();
                m_targetPixmapWidget->move(topLeft);

            }
        }
    }
    break;
    case QEvent::MouseMove: {
        QMouseEvent *event = static_cast<QMouseEvent *> (e);
        if(event->buttons() & Qt::LeftButton) {
            bottomRight = event->globalPos();
            //QToolTip::showText(bottomRight, QString("Move X:%1 Y:%2").arg(bottomRight.x()).arg(bottomRight.y()));

            updateSelectedRect();
            return true;
        }
    }
    break;
    default:
        break;
    }

    //return false;
    return QWidget::event(e);
}

void Screenshot::drawBackground()
{

    QPainter painter(&backgroundImage);

    painter.setCompositionMode(QPainter::CompositionMode_HardLight);

    painter.drawPixmap(originalPixmap.rect(), originalPixmap);
    painter.fillRect(originalPixmap.rect(), Qt::lightGray);

    //    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    //    painter.drawImage(0, 0, destinationImage);

    //    painter.setCompositionMode(mode);
    //    painter.drawImage(0, 0, sourceImage);

    //    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    //    painter.fillRect(resultImage.rect(), Qt::white);

    painter.end();

    ui->label->setPixmap(QPixmap::fromImage(backgroundImage));

    raise();

}

//void Screenshot::drawForeground(){

//    QRect rect = QRect(topLeft, bottomRight);

//    resultPixmap = originalPixmap.copy(rect);

//    QPainter painter(&backgroundImage);

//    //painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
//    painter.drawPixmap(topLeft, resultPixmap);
//    //painter.fillRect(originalPixmap.rect(), Qt::lightGray);


//    painter.end();

//    ui.label->setPixmap(QPixmap::fromImage(backgroundImage));
//    //ui.label->setPixmap(resultPixmap);


//}

void Screenshot::showSelectedRect()
{

    if(!m_targetPixmapWidget) {
        m_targetPixmapWidget = new SelectTargetImageWidget(originalPixmap.toImage(), QRect(topLeft, bottomRight), this);
        connect(m_targetPixmapWidget, SIGNAL(imageSelected(const QImage &)), this, SIGNAL(imageSelected(const QImage &)));
    }

    m_targetPixmapWidget->show();
    //m_targetPixmapWidget->raise();

}

void Screenshot::updateSelectedRect()
{
    m_targetPixmapWidget->resize(bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y());
}








} //namespace HEHUI

