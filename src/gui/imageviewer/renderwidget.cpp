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

    m_image = QImage();
    m_text = "";
    m_preWidgetSize = QSize(0, 0);
    m_preImageSize = QSize(0, 0);
    m_scaleFactor = 1.0;
    m_center = QPoint(0, 0);

    m_aspectRatioMode = Qt::KeepAspectRatio;
    m_scaledContents = false;
    m_alignmentCenter = true;

}

const QImage *RenderWidget::image() const
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
    Q_UNUSED(event);

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if(!m_image.isNull()) {
        painter.save();

        if(m_image.size() != m_preImageSize || size() != m_preWidgetSize) {
            QSize newImageSize = m_image.size();
            if(m_scaledContents) {
                newImageSize.scale(size(), m_aspectRatioMode);
            }
            m_scaleFactor = (double)newImageSize.width() / m_image.width();
            m_center = QPoint(width() / 2, height() / 2) - QPoint(newImageSize.width() / 2, newImageSize.height() / 2);
            m_preImageSize = m_image.size();
            m_preWidgetSize = size();
        }

        if(m_alignmentCenter) {
            painter.translate(m_center);
        }

        if(m_scaleFactor != 1.0) {
            painter.scale(m_scaleFactor, m_scaleFactor);
        }

        painter.drawImage(0, 0, m_image);

        painter.restore();
    }

    if(!m_text.trimmed().isEmpty()){
        painter.drawText(this->rect(), Qt::AlignCenter, m_text);
    }

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
    m_text = text;
    update();
}

void RenderWidget::setPixmap(const QImage &image)
{
    m_image = image;
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
        //repaint();
    }

    update();
}



} //namespace HEHUI
