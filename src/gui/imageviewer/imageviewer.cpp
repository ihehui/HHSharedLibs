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


#include <QtWidgets>

#include <math.h>

#include "imageviewer.h"
#include "imageviewercontroler.h"
#include "animationcontroler.h"


#ifndef QT_NO_PRINTER
    #include <QPrinter>
    #include <QPrintDialog>
#endif



namespace HEHUI
{



ImageViewer::ImageViewer(QWidget *parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{

    //setWindowOpacity(0.85);
    setContextMenuPolicy(Qt::CustomContextMenu);

    currentImageDirectory = QDir::homePath();
    m_curImageIndex = 0;

    m_imageLabel = new RenderWidget(this);
    //m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setAlignmentCenter(true);
    m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_imageLabel->setBackgroundRole(QPalette::Mid);
    m_imageLabel->setAutoFillBackground(true);
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    m_curPixmap = QPixmap();
    m_orignalPixmap = QPixmap();

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setBackgroundRole(QPalette::Mid);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_imageLabel);
    m_scrollArea->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_scrollArea, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));

    m_scaleFactor = 1.0;
    m_fitToWindow = true;
    m_rotateAngle = 0;

    createActions();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_scrollArea);
    m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    m_mainLayout->setMargin(0);
    setLayout(m_mainLayout);

    m_imageControler = new ImageViewerControler(this);
    connect(m_imageControler, SIGNAL(signalRotate(int)), this, SLOT(rotate(int)));
    connect(m_imageControler, SIGNAL(signalFlip(bool, bool)), this, SLOT(flip(bool, bool)));

    //    connect(imageControler, SIGNAL(signalFlipHorizontal()), this, SLOT(flipHor()));
    //    connect(imageControler, SIGNAL(signalFlipVertical()), this, SLOT(flipVer()));
    connect(m_imageControler, SIGNAL(signalZoomIn()), this, SLOT(zoomIn()));
    connect(m_imageControler, SIGNAL(signalZoomOut()), this, SLOT(zoomOut()));
    connect(m_imageControler, SIGNAL(signalZoomFitBest()), this, SLOT(zoomFitBest()));
    connect(m_imageControler, SIGNAL(signalZoomOrignal()), this, SLOT(zoomOrignal()));

    connect(m_imageControler, SIGNAL(signalSave()), this, SLOT(save()));
    connect(m_imageControler, SIGNAL(signalSaveAs()), this, SLOT(saveAs()));

    m_animationControler = new AnimationControler(this);
    connect(m_animationControler, SIGNAL(signalFrameChanged(const QPixmap &)), this, SLOT(updateAnimationFrame(const QPixmap &)));

    m_toolButtonClose = new QToolButton(this);
    m_toolButtonClose->resize(35, 35);
    m_toolButtonClose->setIconSize(QSize(32, 32));
    m_toolButtonClose->setIcon(QIcon(":/resources/images/close.png"));
    m_toolButtonClose->setStyleSheet("QToolButton {\n    background-color: transparent;\n    border-color: transparent;\n    border-style: solid;\n}\n\nQToolButton:hover {\n   background-color: transparent;\n}\n\nQToolButton:pressed {\n    background-color: transparent;\n    padding-left: 5px;\n    padding-top: 5px;\n}");
    connect(m_toolButtonClose, SIGNAL(clicked()), this, SLOT(close()));


    int minimumWidth = qMax(m_imageControler->frameSize().width(), m_animationControler->frameGeometry().width());
    minimumWidth = qMax(minimumWidth, 600);
    int minimumHeight = m_imageControler->frameSize().height() + m_animationControler->frameGeometry().height();
    minimumHeight = qMax(minimumHeight, 400);
    m_minimumScrollAreaSize = QSize(minimumWidth, minimumHeight) + QSize(40, 40);
    m_scrollArea->setMinimumSize(m_minimumScrollAreaSize);


    m_runningValidMovie = false;

    m_tipLabel = new QLabel(this);
    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_tipLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_tipLabel->setMinimumSize(minimumWidth, 40);
    m_tipLabel->hide();

    m_tipTimer = new QTimer(this);
    m_tipTimer->setSingleShot(true);
    m_tipTimer->setInterval(3000);
    connect(m_tipTimer, SIGNAL(timeout()), m_tipLabel, SLOT(hide()));

    m_tipScaleFactor = true;

    m_flipHorizontally = false;
    m_flipVertically = false;

    m_dragPosition = QPoint(0, 0);
    m_dragable = true;
    m_defaultSavePath = "./";

    m_imageLabel->setMouseTracking(true);
    m_scrollArea->setMouseTracking(true);
    //setMouseTracking(true);

    m_imageLabel->installEventFilter(this);
    m_scrollArea->installEventFilter(this);
    m_imageControler->installEventFilter(this);
    m_animationControler->installEventFilter(this);
    installEventFilter(this);


    setWindowTitle(tr("Image Viewer"));
    //    resize(800, 600);

    //    QDesktopWidget* desktop = QApplication::desktop();
    //    QRect rect = desktop->availableGeometry(this);
    //    int desktopWidth = rect.width();
    //    int desktopHeight = rect.height();
    //    int windowWidth = frameGeometry().width();
    //    int windowHeight = frameGeometry().height();
    //    move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);
    //    raise();

}

ImageViewer::~ImageViewer()
{
    m_tipTimer->stop();
}


//#ifndef QT_NO_WHEELEVENT
//void MoviePlayer::wheelEvent(QWheelEvent *e)
//{

//    if(QApplication::keyboardModifiers() == Qt::ControlModifier){
//        scaleImage(pow((double)2, -e->delta() / 240.0));
//        e->accept();
//    }else{
//        QWidget::wheelEvent(e);
//    }

//}
//#endif

//void MoviePlayer::keyPressEvent(QKeyEvent *event)
//{
//    switch (event->key()) {
//    case Qt::Key_Up:
//        break;
//    case Qt::Key_Down:
//        break;
//    case Qt::Key_Left:
//        break;
//    case Qt::Key_Right:
//        break;
//    case Qt::Key_Plus:
//    case Qt::Key_Equal:
//        zoomIn();
//        break;
//    case Qt::Key_Minus:
//    case Qt::Key_Underscore:
//        zoomOut();
//        break;
//    case Qt::Key_Space:
//    case Qt::Key_Enter:
//        break;
//    default:
//        QWidget::keyPressEvent(event);
//    }
//}

//void MoviePlayer::resizeEvent(QResizeEvent * event){

//    if(scaleFactor == 1){
//        zoomFitBest();
//        event->accept();
//    }else{
//        QWidget::resizeEvent(event);
//    }

//}

//void MoviePlayer::mouseMoveEvent(QMouseEvent * event){

//    QPoint globalPos = event->globalPos();
//    QPoint pos = scrollArea->mapFromGlobal(globalPos);

//    if(pos.y() <= controler->height()){
//        QPoint tl = scrollArea->mapToGlobal(QPoint( (scrollArea->viewport()->width() - controler->width())/2, 1) );
//        controler->move(tl);
//        controler->show();
//    }else{
//        controler->hide();
//    }

//    event->accept();

//    qDebug()<<"--------------pos:"<<pos;


//}

RenderWidget *ImageViewer::renderWidget()
{
    return m_imageLabel;
}

QScrollArea *ImageViewer::scrollArea()
{
    return m_scrollArea;
}

ImageViewerControler *ImageViewer::imageControler()
{
    return m_imageControler;
}

void ImageViewer::processBrightnessAndContrast(QImage &image, int brightness, int contrast)
{
    if(image.isNull()) {
        return;
    }
    ////----------------------------
    ////Algorithm: newColor = contrast * color(x,y) +  brightness )
    ////----------------------------

    //QDateTime time = QDateTime::currentDateTime();

    quint8 values[256];
    for(int i = 0; i < 256; i++) {
        int value = (contrast * 0.01) * i + brightness;
        values[i] = qBound(0, value, 255);
    }

    for(int y = 0; y < image.height(); y++) {
        QRgb *rgb = (QRgb *)image.scanLine(y);
        for(int x = 0; x < image.width(); x++) {
            rgb[x] = qRgb(values[qRed(rgb[x])], values[qGreen(rgb[x])], values[qBlue(rgb[x])]);
            //image.setPixel(x, y , qRgb(r,g,b));
        }
    }

    //qDebug()<<"----Time:"<<QDateTime::currentDateTime().msecsTo(time);

}

void ImageViewer::setImage(const QImage &image, bool moveViewerToCenter, bool adjustViewerSize)
{
    setImage(QPixmap::fromImage(image), moveViewerToCenter, adjustViewerSize);
}

void ImageViewer::setImage(const QPixmap &pixmap, bool moveViewerToCenter, bool adjustViewerSize)
{
    if(pixmap.isNull()) {
        qCritical() << "Error! Invalid pixmap!";
        updatePixmap(pixmap);
        return;
    }

    if(moveViewerToCenter) {
        moveToCenter(adjustViewerSize);
    }

    updatePixmap(QPixmap());
    m_imageLabel->resize(1, 1);

    m_scaleFactor = 1;
    m_fitToWindow = true;
    m_rotateAngle = 0;
    m_flipHorizontally = false;
    m_flipVertically = false;

    m_imageControler->reset();

    qApp->processEvents();

    updatePixmap(pixmap);
    m_orignalPixmap = m_curPixmap;

    QSize viewportSize = m_scrollArea->viewport()->size();
    if(pixmap.width() > viewportSize.width() ||  pixmap.height() > viewportSize.height()) {
        QTimer::singleShot(10, this, SLOT(zoomFitBest()));
    } else {
        QTimer::singleShot(10, this, SLOT(zoomOrignal()));
    }

    updateActions();
}

void ImageViewer::setImages(const QStringList &images, unsigned int initIndex)
{
    this->m_images = images;

    int size = images.size();
    m_curImageIndex = initIndex;
    if(m_curImageIndex >= size) {
        m_curImageIndex = size - 1;
    }

    openFile(images.at(m_curImageIndex));

}

bool ImageViewer::eventFilter(QObject *obj, QEvent *event)
{

    switch (event->type()) {
    case QEvent::KeyRelease: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *> (event);
        return processKeyEvent(obj, keyEvent);
    }
    break;

    //    case QEvent::Enter:
    //    {
    //        //qDebug()<<"----QEvent::Enter"<<" "<<this;
    //        return true;
    //    }
    //        break;

    case QEvent::Leave: {
        //qDebug()<<"----QEvent::Leave"<<" "<<this;

        if(obj == m_imageControler) {
            m_imageControler->hide();
            raise();
            setFocus();
            return true;
        }
    }
    break;

    case QEvent::MouseButtonPress: {
        if(!m_dragable) {
            return QObject::eventFilter(obj, event);
        }

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);
        if(!mouseEvent) {
            return false;
        }
        if (mouseEvent->button() == Qt::LeftButton) {
            m_dragPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
            return true;
        }
    }
    break;

    case QEvent::MouseMove: {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);
        if(!mouseEvent) {
            return false;
        }

        if (mouseEvent->buttons() & Qt::LeftButton) {
            if(!m_dragable) {
                return QObject::eventFilter(obj, event);
            }
            move(mouseEvent->globalPos() - m_dragPosition);
            //return true;
        }


        if(m_curPixmap.isNull()) {
            return false;
        }

        QPoint globalPos = mouseEvent->globalPos();
        QPoint pos = m_scrollArea->mapFromGlobal(globalPos);

        int scrollAreaWidth = m_scrollArea->viewport()->width();
        int scrollAreaHeight = m_scrollArea->viewport()->height();

        if(pos.x() < (scrollAreaWidth - m_toolButtonClose->width() * 2) && pos.y() <= m_imageControler->height() && pos.y() > 0) {
            QPoint tl = m_scrollArea->mapToGlobal(QPoint( (scrollAreaWidth - m_imageControler->width()) / 2, 1) );
            m_imageControler->move(tl);
            m_imageControler->show();
        } else {
            m_imageControler->hide();
            raise();
            setFocus();
        }

        if(m_runningValidMovie) {
            int animationControlerHeight = m_animationControler->height();
            if(pos.y() <= scrollAreaHeight && pos.y() > (scrollAreaHeight - animationControlerHeight) ) {
                QPoint tl = m_scrollArea->mapToGlobal(QPoint( (scrollAreaWidth - m_animationControler->width()) / 2, (scrollAreaHeight - animationControlerHeight) ) );
                m_animationControler->move(tl);
                m_animationControler->show();
            } else {
                m_animationControler->hide();
                raise();
                setFocus();
            }
        }

        return true;
    }
    break;

    case QEvent::MouseButtonDblClick: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);
        if(!mouseEvent) {
            return false;
        }
        return processMouseButtonDblClick(obj, mouseEvent);
    }
    break;

    case QEvent::Wheel: {
        //qDebug()<<"----QEvent::Wheel"<<" "<<this;

        QWheelEvent *e = static_cast<QWheelEvent *> (event);
        if(!e) {
            return false;
        }

        if(m_curPixmap.isNull()) {
            return false;
        }

        if(QApplication::keyboardModifiers() == Qt::ControlModifier) {
            scaleImage(pow((double)2, -e->delta() / 240.0));
            return true;
        }

    }
    break;

    case QEvent::Resize: {
        //qDebug()<<"----QEvent::Resize"<<" obj:"<<obj;

        moveControler();

        if(obj == this && m_fitToWindow && m_imageLabel->pixmap()) {
            zoomFitBest();
            return true;
        }

    }
    break;

    default:
        break;
    }

    return QObject::eventFilter(obj, event);

}

bool ImageViewer::processKeyEvent(QObject *obj, QKeyEvent *keyEvent)
{

    qDebug() << "----ImageViewer::processKeyEvent()";


    if(keyEvent->key() == Qt::Key_Escape) {
        qApp->quit();
        return true;
    }

    if(keyEvent->key() == Qt::Key_Right) {
        openFile(m_curImageIndex + 1);
        return true;
    }
    if(keyEvent->key() == Qt::Key_Left) {
        openFile(m_curImageIndex - 1);
        return true;
    }
    //        if(QApplication::keyboardModifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_P){
    //        }
    //        if(QApplication::keyboardModifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_E){
    //        }

    return false;
}

bool ImageViewer::processMouseButtonDblClick(QObject *obj, QMouseEvent *event)
{
    if(obj == m_scrollArea || obj == m_imageLabel) {
        if(m_fitToWindow) {
            zoomOrignal();
        } else {
            zoomFitBest();
        }
        //reset();
        return true;
    }
    return false;
}

void ImageViewer::setScaleButtonsVisible(bool visible)
{
    m_imageControler->setScaleButtonsVisible(visible);
}

void ImageViewer::setRotateButtonsVisible(bool visible)
{
    m_imageControler->setRotateButtonsVisible(visible);
}

void ImageViewer::setFlipButtonsVisible(bool visible)
{
    m_imageControler->setFlipButtonsVisible(visible);
}

void ImageViewer::setCloseButtonVisible(bool visible)
{
    m_toolButtonClose->setVisible(visible);
}

void ImageViewer::setDragable(bool dragable)
{
    m_dragable = dragable;
}

void ImageViewer::open()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Image Files"),
                            currentImageDirectory,
                            tr("Images (*.png *.xpm *.jpg *.mng *.svg *.gif *.bmp);;All files (*.*)")
                                                         );
    if (!fileNames.isEmpty()) {
        //openFile(fileNames);
        setImages(fileNames);
    }

}


void ImageViewer::openFile(const QString &fileName)
{

    currentImageDirectory = QFileInfo(fileName).path();

    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::information(this, tr("Error"), tr("Cannot load %1.").arg(fileName));
        return;
    }

    m_runningValidMovie = m_animationControler->setFileName(fileName);
    setImage(image);




    //    //Resize
    //    QDesktopWidget* desktop = QApplication::desktop();
    //    QRect rect = desktop->availableGeometry(this);
    //    int desktopWidth = rect.width();
    //    int desktopHeight = rect.height();
    //    //    int newWindowWidth = qMin(image.width(), desktopWidth-40);
    //    //    int newWindowHeight = qMin(image.height(), desktopHeight-40);
    //    QSize minSize = QSize(qMin(image.width(), desktopWidth-40), qMin(image.height(), desktopHeight-40));
    //    QSize newSize = image.size().scaled(minSize, Qt::KeepAspectRatio);
    //    resize(newSize);
    //    move((desktopWidth - frameGeometry().width()) / 2, (desktopHeight - frameGeometry().height()) / 2);

    //    updatePixmap(QPixmap());
    //    imageLabel->resize(1,1);

    //    scaleFactor = 1;
    //    m_fitToWindow = true;
    //    rotateAngle = 0;
    //    flipHorizontally = false;
    //    flipVertically = false;

    //    imageControler->reset();

    //    qApp->processEvents();

    //    updatePixmap(QPixmap::fromImage(image));
    //    orignalPixmap = curPixmap;

    //    QSize viewportSize = scrollArea->viewport()->size();
    //    if(image.width() > viewportSize.width() ||  image.height() > viewportSize.height()){
    //        //zoomFitBest();
    //        QTimer::singleShot(50, this, SLOT(zoomFitBest()));
    //    }else{
    //        //zoomOrignal();
    //        QTimer::singleShot(50, this, SLOT(zoomOrignal()));
    //    }

    //    runningValidMovie = animationControler->setFileName(fileName);

    //    updateActions();


}

void ImageViewer::openFile(int imageIndex)
{

    int size = m_images.size();
    m_curImageIndex = imageIndex;
    if(m_curImageIndex >= size) {
        m_curImageIndex = 0;
    } else if(m_curImageIndex < 0) {
        m_curImageIndex = size - 1;
    }

    openFile(m_images.at(m_curImageIndex));

}

inline void ImageViewer::updatePixmap(const QPixmap &pixmap)
{
    m_curPixmap = pixmap;
    m_imageLabel->setPixmap(m_curPixmap);
}


void ImageViewer::updateAnimationFrame(const QPixmap &pixmap)
{
    //qDebug()<<"--ImageViewer::updateAnimationFrame(...)";

    updatePixmap(pixmap);
    m_orignalPixmap = m_curPixmap;

    m_tipScaleFactor = false;

    QMatrix matrix;
    matrix.rotate(m_rotateAngle);

    updatePixmap( QPixmap::fromImage(m_curPixmap.toImage().mirrored(m_flipHorizontally, m_flipVertically)).transformed(matrix, Qt::SmoothTransformation) );

    if(m_fitToWindow) {
        zoomFitBest();
    } else {
        scaleImage(1);
    }

    m_tipScaleFactor = true;

}

void ImageViewer::updateAnimationFrame(const QImage &image)
{

    updatePixmap(QPixmap::fromImage(image));
    m_orignalPixmap = m_curPixmap;

    m_tipScaleFactor = false;

    QMatrix matrix;
    matrix.rotate(m_rotateAngle);

    updatePixmap( QPixmap::fromImage(image.mirrored(m_flipHorizontally, m_flipVertically)).transformed(matrix, Qt::SmoothTransformation) );

    if(m_fitToWindow) {
        zoomFitBest();
    } else {
        scaleImage(1);
    }

    m_tipScaleFactor = true;

}

void ImageViewer::setText(const QString &text)
{
    m_imageLabel->setText(text);
}

bool ImageViewer::setDefaultSavePath(const QString &path)
{

    QDir dir;
    if(!dir.mkpath(path)) {
        QMessageBox::critical(this, tr("Error"), tr("Can not create path:<p>%1</p>").arg(path));
        return false;
    }

    m_defaultSavePath = path;

    return false;

}

QString ImageViewer::defaultSavePath()
{

    if(m_defaultSavePath.trimmed().isEmpty()) {
        m_defaultSavePath = "./";
    }

    return m_defaultSavePath;
}

void ImageViewer::moveToCenter(bool adjustViewerSize)
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry(this);
    int desktopWidth = rect.width();
    int desktopHeight = rect.height();
    //    int newWindowWidth = qMin(image.width(), desktopWidth-40);
    //    int newWindowHeight = qMin(image.height(), desktopHeight-40);
    //Resize
    const QPixmap *pixmap = m_imageLabel->pixmap();
    if(adjustViewerSize && pixmap) {
        QSize minSize = QSize(qMin(pixmap->width(), desktopWidth - 40), qMin(pixmap->height(), desktopHeight - 40));
        QSize newSize = pixmap->size().scaled(minSize, Qt::KeepAspectRatio);
        resize(newSize);
    }
    move((desktopWidth - frameGeometry().width()) / 2, (desktopHeight - frameGeometry().height()) / 2);
}

void ImageViewer::rotate(int angle)
{

    if(angle == 0) {
        return;
    }

    //    const QPixmap *pixmap = imageLabel->pixmap();
    //    Q_ASSERT(pixmap);
    //    if(!pixmap){return;}

    if(m_curPixmap.isNull()) {
        return;
    }

    QMatrix matrix;
    matrix.rotate(angle);

    //    QPixmap pixmap = curPixmap.transformed(matrix,Qt::SmoothTransformation);
    //    imageLabel->setPixmap(pixmap);
    //    updatePixmap(curPixmap.transformed(matrix,Qt::SmoothTransformation));
    updatePixmap( QPixmap::fromImage(m_orignalPixmap.toImage().mirrored(m_flipHorizontally, m_flipVertically)).transformed(matrix, Qt::SmoothTransformation) );


    if(m_fitToWindow) {
        zoomFitBest();
    } else {
        scaleImage(1);
    }

    m_rotateAngle = angle;

}

void ImageViewer::flip(bool horizontally, bool vertically)
{

    //    const QPixmap *pixmap = imageLabel->pixmap();
    //    Q_ASSERT(pixmap);
    //    if(!pixmap){return;}

    if(m_curPixmap.isNull()) {
        return;
    }

    //QImage image = pixmap->toImage().mirrored(true, false);

    updatePixmap(QPixmap::fromImage(m_curPixmap.toImage().mirrored(horizontally, vertically)));
    //    curPixmap = QPixmap::fromImage(curPixmap.toImage().mirrored(horizontally, vertically));
    //    imageLabel->setPixmap(curPixmap);

    if(m_fitToWindow) {
        zoomFitBest();
    }


    if(horizontally) {
        m_flipHorizontally = !m_flipHorizontally;
    }
    if(vertically) {
        m_flipVertically = !m_flipVertically;
    }



}

//void ImageViewer::flipHor(){
//    //    const QPixmap *pixmap = imageLabel->pixmap();
//    //    Q_ASSERT(pixmap);
//    //    if(!pixmap){return;}

//    if(curPixmap.isNull()){return;}

//    //QImage image = pixmap->toImage().mirrored(true, false);

//    updatePixmap(QPixmap::fromImage(curPixmap.toImage().mirrored(false, true)));

////    curPixmap = QPixmap::fromImage(curPixmap.toImage().mirrored(true, false));
////    imageLabel->setPixmap(curPixmap);

//    if(m_fitToWindow){
//        zoomFitBest();
//    }

//}

//void ImageViewer::flipVer(){
//    //    const QPixmap *pixmap = imageLabel->pixmap();
//    //    Q_ASSERT(pixmap);
//    //    if(!pixmap){return;}

//    if(curPixmap.isNull()){return;}

//    //QImage image = pixmap->toImage().mirrored(true, false);
//    updatePixmap(QPixmap::fromImage(curPixmap.toImage().mirrored(true, false)));

////    curPixmap = QPixmap::fromImage(curPixmap.toImage().mirrored(false, true));
////    imageLabel->setPixmap(curPixmap);

//    if(m_fitToWindow){
//        zoomFitBest();
//    }
//}

void ImageViewer::zoomIn()
{
    scaleImage(1.1);

    m_fitToWindow = false;
}

void ImageViewer::zoomOut()
{
    scaleImage(0.9);

    m_fitToWindow = false;
}

void ImageViewer::zoomFitBest()
{

    const QPixmap *pixmap = m_imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap) {
        return;
    }

    QSize imageSize = pixmap->size();
    QSize viewportSize = m_scrollArea->viewport()->size();

    QSize newImageSize = imageSize.scaled(viewportSize, Qt::KeepAspectRatio);
    m_imageLabel->resize(newImageSize);

    m_scaleFactor = (double)newImageSize.width() / imageSize.width();

    m_fitToWindow = true;


    updateActions();

    showScaleFactor();
}

void ImageViewer::zoomOrignal()
{
    m_imageLabel->adjustSize();
    m_scaleFactor = 1.0;

    m_fitToWindow = false;

    showScaleFactor();

}

void ImageViewer::save()
{

    QString path = m_defaultSavePath + QString("/%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));

    const QPixmap *pixmap = m_imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap) {
        return;
    }

    if(!pixmap->save(path)) {
        QMessageBox::critical(this, tr("Error"), tr("Can not save image as:<p>%1</p>").arg(path));
    }


}

void ImageViewer::saveAs()
{

    QStringList filters;
    filters << "PNG (*.png)" << "JPEG (*.jpg)" << "XPM (*.xpm)" << tr("All Files (*)");

    QMultiHash <QString, QString>filtersHash;
    filtersHash.insert(".png", filters.at(0));
    filtersHash.insert(".jpg", filters.at(1));
    filtersHash.insert(".xpm", filters.at(2));
    filtersHash.insert(".png", filters.at(3) );


    QFileDialog dlg;
    QString selectedFilter;
    QString path = dlg.getSaveFileName(this, tr("Save Image As:"), m_defaultSavePath, filters.join(";;"), &selectedFilter);
    if(path.isEmpty()) {
        return;
    }
    QFileInfo info(path);
    QString sufffix = info.suffix().trimmed();
    if(sufffix.isEmpty()) {
        sufffix = filtersHash.key(selectedFilter);
        path += sufffix;
    }


    const QPixmap *pixmap = m_imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap) {
        return;
    }

    if(!pixmap->save(path)) {
        QMessageBox::critical(this, tr("Error"), tr("Can not save image as:<p>%1</p>").arg(path));
    }


}

void ImageViewer::print()
{

    const QPixmap *pixmap = m_imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap) {
        return;
    }

#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = pixmap->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(pixmap->rect());
        painter.drawPixmap(0, 0, *pixmap);
    }
#endif

}


void ImageViewer::reset()
{

    if(m_curPixmap.isNull()) {
        return;
    }

    m_imageControler->reset();
    zoomFitBest();

}


void ImageViewer::showContextMenu(const QPoint &pos)
{


    QMenu menu;
    //    menu.addAction(zoomInAct);
    //    menu.addAction(zoomOutAct);
    //    menu.addAction(normalSizeAct);
    //    menu.addAction(fitToWindowAct);
    menu.addAction(m_resetAct);

    menu.addSeparator();
    menu.addAction(m_saveAsAct);
#ifndef QT_NO_PRINTER
    menu.addAction(m_printAct);
#endif

    menu.addSeparator();
    menu.addAction(m_openAct);
    menu.addAction(m_exitAct);

    menu.exec(mapToGlobal(pos));

}

void ImageViewer::showTip(const QString &tip)
{


    m_tipLabel->setText(tip);

    QPoint tl =  QPoint(m_scrollArea->geometry().center()) - QPoint( m_tipLabel->geometry().width() / 2, m_tipLabel->geometry().height() / 2) ;
    m_tipLabel->move(tl);
    m_tipLabel->show();

    m_tipTimer->start();

    //QToolTip::showText(pos, tip);
}



void ImageViewer::showScaleFactor()
{

    if(!m_tipScaleFactor) {
        return;
    }

    QString tip = QString("<div align=\"center\" style=\"font-size:20pt;font-weight:bold;color:#ff0000;\"> %1% </div>").arg( (int)(m_scaleFactor * 100));
    showTip(tip);
}

void ImageViewer::showImageInfo()
{

    QString tip = QString("<div align=\"center\" style=\"font-size:20pt;font-weight:bold;color:#ff0000;\"> %1% </div>").arg( (int)(m_scaleFactor * 100));
    showTip(tip);
}

void ImageViewer::moveControler()
{
    int scrollAreaWidth = m_scrollArea->viewport()->width();
    //int scrollAreaHeight = m_scrollArea->viewport()->height();

    QPoint tr = m_scrollArea->mapTo(this, QPoint( (scrollAreaWidth - m_toolButtonClose->width() ), 1 ));
    m_toolButtonClose->move(tr);

    QPoint tl = m_scrollArea->mapToGlobal(QPoint( (scrollAreaWidth - m_imageControler->width()) / 2, 1) );
    m_imageControler->move(tl);
}


void ImageViewer::createActions()
{

    m_zoomInAct = new QAction(tr("Zoom &In"), this);
    m_zoomInAct->setShortcut(tr("Ctrl++"));
    m_zoomInAct->setEnabled(false);
    connect(m_zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    m_zoomOutAct = new QAction(tr("Zoom &Out"), this);
    m_zoomOutAct->setShortcut(tr("Ctrl+-"));
    m_zoomOutAct->setEnabled(false);
    connect(m_zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    m_normalSizeAct = new QAction(tr("&Normal Size"), this);
    m_normalSizeAct->setShortcut(tr("Ctrl+N"));
    m_normalSizeAct->setEnabled(false);
    connect(m_normalSizeAct, SIGNAL(triggered()), this, SLOT(zoomOrignal()));

    m_resetAct = new QAction(tr("&Reset..."), this);
    m_resetAct->setShortcut(tr("Ctrl+R"));
    m_resetAct->setEnabled(false);
    connect(m_resetAct, SIGNAL(triggered()), this, SLOT(reset()));



    m_fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    m_fitToWindowAct->setEnabled(false);
    //fitToWindowAct->setCheckable(true);
    m_fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(m_fitToWindowAct, SIGNAL(triggered()), this, SLOT(zoomFitBest()));

    m_saveAsAct = new QAction(tr("&Save As..."), this);
    m_saveAsAct->setShortcut(tr("Ctrl+S"));
    m_saveAsAct->setEnabled(false);
    connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    m_printAct = new QAction(tr("&Print..."), this);
    m_printAct->setShortcut(tr("Ctrl+P"));
    m_printAct->setEnabled(false);
    connect(m_printAct, SIGNAL(triggered()), this, SLOT(print()));


    m_openAct = new QAction(tr("&Open..."), this);
    m_openAct->setShortcut(tr("Ctrl+O"));
    connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

    m_exitAct = new QAction(tr("E&xit"), this);
    m_exitAct->setShortcut(tr("Ctrl+Q"));
    connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));






}

void ImageViewer::updateActions()
{

    bool imageValid = !m_curPixmap.isNull();

    m_zoomInAct->setEnabled(imageValid);
    m_zoomOutAct->setEnabled(imageValid);
    m_normalSizeAct->setEnabled(imageValid);
    m_fitToWindowAct->setEnabled(imageValid);

    m_resetAct->setEnabled(imageValid);
    m_saveAsAct->setEnabled(imageValid);

    m_printAct->setEnabled(imageValid);

}



void ImageViewer::scaleImage(double factor)
{

    const QPixmap *pixmap = m_imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap) {
        return;
    }

    //    Q_ASSERT(imageLabel->pixmap());
    //Q_ASSERT(!movie->currentPixmap().isNull());


    m_scaleFactor *= factor;

    if(m_scaleFactor > 10) {
        m_scaleFactor = 10;
    }
    if(m_scaleFactor < 0.1) {
        m_scaleFactor = 0.1;
    }

    m_imageLabel->resize(m_scaleFactor * pixmap->size());

    adjustScrollBar(m_scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(m_scrollArea->verticalScrollBar(), factor);

    m_zoomInAct->setEnabled(m_scaleFactor < 3.0);
    m_zoomOutAct->setEnabled(m_scaleFactor > 0.333);

    showScaleFactor();

}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep() / 2)));
}










} //namespace HEHUI
