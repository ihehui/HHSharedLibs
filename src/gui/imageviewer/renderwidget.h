/*
 ****************************************************************************
 * renderwidget.h
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


#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QWidget>

namespace HEHUI
{


class RenderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RenderWidget(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    const QPixmap *pixmap() const;
    Qt::AspectRatioMode aspectRatioMode() const;
    bool scaledContents() const;


protected:
    void paintEvent(QPaintEvent *event);

signals:

public slots:
    void setText(const QString &text);
    void setPixmap(const QPixmap &pixmap);
    void setAspectRatioMode(Qt::AspectRatioMode mode);
    void setScaledContents(bool scale);
    void setAlignmentCenter(bool alignmentCenter);
    void adjustSize();

private:
    QPixmap m_image;
    QSize m_preWidgetSize;
    QSize m_preImageSize;
    float m_scaleFactor;
    QPoint m_center;
    Qt::AspectRatioMode m_aspectRatioMode;
    bool m_scaledContents;
    bool m_alignmentCenter;

};

} //namespace HEHUI

#endif // RENDERWIDGET_H
