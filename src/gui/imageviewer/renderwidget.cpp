/*
 ****************************************************************************
 * renderwidget.cpp
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



#include "renderwidget.h"

#include <QPainter>
#include <QDebug>
#include <QStyle>
#include <QStyleOption>


namespace HEHUI
{


RenderWidget::RenderWidget(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{

    m_image = QPixmap();
    m_preWidgetSize = size();
    m_preImageSize = QSize(0, 0);
    m_scaleFactor = 1.0;
    m_center = QPoint(0, 0);

    m_aspectRatioMode = Qt::KeepAspectRatio;
    m_scaledContents = false;
    m_alignmentCenter = true;

}

const QPixmap *RenderWidget::pixmap() const
{
    return &m_image;
}

Qt::AspectRatioMode RenderWidget::aspectRatioMode() const
{
    return m_aspectRatioMode;
}

bool RenderWidget::scaledContents() const
{
    return m_scaledContents;
}

void RenderWidget::paintEvent(QPaintEvent *event)
{

    if(m_image.isNull()) {
        return;
    }
    if(m_image.size() != m_preImageSize || size() != m_preWidgetSize) {
        QSize newImageSize = m_image.size();
        if(m_scaledContents) {
            newImageSize = m_image.size().scaled(size(), m_aspectRatioMode);
        }
        m_scaleFactor = (double)newImageSize.width() / m_image.width();
        m_center = QPoint(width() / 2, height() / 2) - QPoint(newImageSize.width() / 2, newImageSize.height() / 2);
        m_preImageSize = m_image.size();
        m_preWidgetSize = size();
    }

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if(m_alignmentCenter) {
        painter.translate(m_center);
    }

    if(m_scaleFactor != 1.0) {
        painter.scale(m_scaleFactor, m_scaleFactor);
    }

    painter.drawPixmap(0, 0, m_image);
    painter.end();

    if(!styleSheet().trimmed().isEmpty()) {
        QStyleOption opt;
        opt.init(this);
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
        painter.end();
    }

}

void RenderWidget::setText(const QString &text)
{

}

void RenderWidget::setPixmap(const QPixmap &pixmap)
{
    m_image = pixmap;
    update();
}

void RenderWidget::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    m_aspectRatioMode = mode;
    update();
}

void RenderWidget::setScaledContents(bool scale)
{
    m_scaledContents = scale;
}

void RenderWidget::setAlignmentCenter(bool alignmentCenter)
{
    m_alignmentCenter = alignmentCenter;
}

void RenderWidget::adjustSize()
{
    if(!m_image.isNull()) {
        resize(m_image.size());
        update();
    }
}






} //namespace HEHUI
