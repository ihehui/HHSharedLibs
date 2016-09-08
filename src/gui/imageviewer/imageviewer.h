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


#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>

#include "renderwidget.h"


#if defined(IMAGEVIEWER_LIBRARY_EXPORT)
#  define IMAGEVIEWER_LIB_API Q_DECL_EXPORT
#else
#  define IMAGEVIEWER_LIB_API Q_DECL_IMPORT
#endif


namespace HEHUI {


class ImageViewerControler;
class AnimationControler;


class IMAGEVIEWER_LIB_API ImageViewer : public QWidget
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = 0, Qt::WindowFlags fl = Qt::FramelessWindowHint);
    ~ImageViewer();

    RenderWidget * renderWidget();
    QScrollArea * scrollArea();
    ImageViewerControler *imageControler();

    static void processBrightnessAndContrast(QImage &image, int brightness, int contrast);


public slots:
    void setImage(const QImage &image, bool moveViewerToCenter = true, bool adjustViewerSize = true);
    void setImage(const QPixmap &pixmap, bool moveViewerToCenter = true, bool adjustViewerSize = true);
    void setImages(const QStringList &m_images, unsigned int initIndex = 0);


protected:
    //#ifndef QT_NO_WHEELEVENT
    //    void wheelEvent(QWheelEvent *);
    //#endif
    //    void keyPressEvent(QKeyEvent *event);
    //    void resizeEvent(QResizeEvent * event);
    //    void mouseMoveEvent(QMouseEvent * event);

    bool eventFilter(QObject *obj, QEvent *event);
    virtual bool processKeyEvent(QObject *obj, QKeyEvent *keyEvent);
    virtual bool processMouseButtonDblClick(QObject *obj, QMouseEvent *event);


public slots:
    void setScaleButtonsVisible(bool visible = true);
    void setRotateButtonsVisible(bool visible = true);
    void setFlipButtonsVisible(bool visible = true);
    void setCloseButtonVisible(bool visible = true);
    void setDragable(bool dragable);

    void openFile(const QString &fileName);
    void openFile(int imageIndex);
    void updateAnimationFrame(const QImage &image);
    void setText(const QString &text);

    bool setDefaultSavePath(const QString &path);
    QString defaultSavePath();

    void moveToCenter(bool adjustViewerSize = true);


private slots:
    void open();

    void updatePixmap(const QPixmap &pixmap);

    void updateAnimationFrame(const QPixmap &pixmap);

    virtual void save();
    void saveAs();
    void print();

    void rotate(int angle);
    void flip(bool horizontally, bool vertically);

    void zoomIn();
    void zoomOut();
    void zoomFitBest();
    void zoomOrignal();

    void reset();

    virtual void showContextMenu(const QPoint &pos);

    void showScaleFactor();
    void showImageInfo();
    void moveControler();

private:
    void createActions();
    void updateActions();

    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void showTip(const QString &tip);


private:
    QString currentImageDirectory;
    QStringList m_images;
    int m_curImageIndex;

    RenderWidget *m_imageLabel;
    QPixmap m_curPixmap;
    QPixmap m_orignalPixmap;

    QScrollArea *m_scrollArea;
    double m_scaleFactor;
    bool m_fitToWindow;
    int m_rotateAngle;

    QAction *m_zoomInAct;
    QAction *m_zoomOutAct;
    QAction *m_normalSizeAct;
    QAction *m_fitToWindowAct;

    QAction *m_resetAct;

    QAction *m_saveAsAct;
    QAction *m_printAct;

    QAction *m_openAct;
    QAction *m_exitAct;

    QVBoxLayout *m_mainLayout;

    ImageViewerControler *m_imageControler;
    AnimationControler *m_animationControler;
    QToolButton *m_toolButtonClose;

    QSize m_minimumScrollAreaSize;
    bool m_runningValidMovie;

    QLabel *m_tipLabel;
    QTimer *m_tipTimer;
    bool m_tipScaleFactor;

    bool  m_flipHorizontally;
    bool m_flipVertically;


    QPoint m_dragPosition;
    bool m_dragable;

    QString m_defaultSavePath;




};

} //namespace HEHUI

#endif
