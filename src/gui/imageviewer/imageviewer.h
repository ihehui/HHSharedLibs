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
 *=============================  Usage  =============================
 *   setScaleButtonsVisible(false);
 *   setRotateButtonsVisible(false);
 *   setFlipButtonsVisible(false);
 *   setSaveImageButtonsVisible(false);
 *   setCloseButtonVisible(false);
 *   setContextMenuPolicy(Qt::CustomContextMenu);
 *   setDragable(false);
 *   setTipScaleFactor(false);
 *   imageControler()->updateGeometry();
 *
 *   QImage image(size(), QImage::Format_RGB32);
 *   image.fill(Qt::gray);
 *   setImage(image, false, true);
 *   setText("Test");
 *
 *===================================================================
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ****************************************************************************
 */

/*
 ***************************************************************************
 * Last Modified on: 2017-2-27
 * Last Modified by: 贺辉
 ***************************************************************************
 */


#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QMutex>

#include "renderwidget.h"


#if defined(IMAGEVIEWER_LIBRARY_EXPORT)
    #define IMAGEVIEWER_LIB_API Q_DECL_EXPORT
#else
    #define IMAGEVIEWER_LIB_API Q_DECL_IMPORT
#endif


namespace HEHUI
{
class ImageViewerControler;
class AnimationControler;


class IMAGEVIEWER_LIB_API ImageViewer : public QWidget
{
    Q_OBJECT

public:
    enum ControlMode{
        CONTROL_IMAGEVIEW,
        CONTROL_REMOTEDESKTOPCONTROL
    };

    ImageViewer(ControlMode mode = CONTROL_IMAGEVIEW, QWidget *parent = 0, Qt::WindowFlags fl = Qt::FramelessWindowHint);
    virtual ~ImageViewer();

    QSize sizeHint() const;

    RenderWidget *renderWidget();
    QScrollArea *scrollArea();
    ImageViewerControler *imageControler();
    QPoint mapToRenderWidget(const QPoint &point);


    enum ScaleMode{
        Scale_Auto, //Scale the image only if it's size is larger than the view.
        Scale_FitBestInView, //The image will be fitted to fill the view keeping aspect ratio.
        Scale_Original //Display the Original image, no scale.
    };
    void setScaleMode(ScaleMode mode);


    static void processBrightnessAndContrast(QImage &image, int brightness, int contrast);

protected:
    //#ifndef QT_NO_WHEELEVENT
    //    void wheelEvent(QWheelEvent *);
    //#endif
    //    void keyPressEvent(QKeyEvent *event);
    //    void resizeEvent(QResizeEvent * event);
    //    void mouseMoveEvent(QMouseEvent * mouseEvent);
    //    void resizeEvent(QResizeEvent *event);

    bool eventFilter(QObject *obj, QEvent *event);
    virtual bool processKeyEvent(QObject *obj, QKeyEvent *keyEvent);
    virtual bool processMouseButtonDblClick(QObject *obj, QMouseEvent *event);
    virtual bool processMouseMoveEvent(QMouseEvent * mouseEvent);




public slots:
    void setImage(const QPixmap &pixmap, bool moveViewerToCenter = true, bool adjustViewerSize = true, bool fitToWindow = true);
    void setImage(const QImage &image, bool moveViewerToCenter = true, bool adjustViewerSize = true, bool fitToWindow = true);
    void replaceImage(const QImage &image);
    void replaceImage(const QPixmap &pixmap);

    void setImages(const QStringList &m_images, unsigned int initIndex = 0);

    void moveToCenter(bool adjustViewerSize = true);

    void setScaleButtonsVisible(bool visible = true);
    void setRotateButtonsVisible(bool visible = true);
    void setFlipButtonsVisible(bool visible = true);
    void setSaveImageButtonsVisible(bool visible = true);
    void setCloseButtonVisible(bool visible = true);
    void setDragable(bool dragable);

    void openFile(const QString &fileName);
    void openFile(int imageIndex);
    void updateAnimationFrame(const QImage &image);
    void setText(const QString &text);

    bool setDefaultSavePath(const QString &path);
    QString defaultSavePath();

    void setTipScaleFactor(bool showTip = true);

    void setRenderWidgetAspectRatioMode(Qt::AspectRatioMode mode);


protected slots:
    void open();


    virtual void save();
    void saveAs();
    void print();

    void rotate(int angle);
    void flip(bool horizontally, bool vertically);

    void zoomIn();
    void zoomOut();
    void zoomFitBest();
    void zoomOrignal();
    void zoomByExpanding();
    void updateRenderSize();

    void reset();

    virtual void showContextMenu(const QPoint &globalPos);

    void showScaleFactor();
    void showImageInfo();
    void moveControler();
    void controlerDragged(const QPoint &globalPoint);
    void showControler();

    void pinControler(bool pin);

private:
    void createActions();
    void updateActions();

    void updateImage(const QImage &image);


    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void showTip(const QString &tip);


private:
    QString m_currentImageDirectory;
    QStringList m_images;
    int m_curImageIndex;

    RenderWidget *m_imageRender;
    QImage m_curPixmap;
    QImage m_orignalPixmap;

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

    Qt::AspectRatioMode m_renderWidgetAspectRatioMode;
    ScaleMode m_scaleMode;

    QMutex m_mutex;

    ControlMode m_controlMode;
    bool m_pinControler;

};

} //namespace HEHUI

#endif
