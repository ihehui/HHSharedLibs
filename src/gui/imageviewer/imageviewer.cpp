/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include <math.h>

#include "imageviewer.h"
#include "imageviewercontroler.h"
#include "animationcontroler.h"


#ifndef QT_NO_PRINTER
    #include <QPrinter>
    #include <QPrintDialog>
#endif




namespace HEHUI {



ImageViewer::ImageViewer(QWidget *parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{

    //setWindowOpacity(0.85);

    currentImageDirectory = QDir::homePath();
    curImageIndex = 0;

    imageLabel = new QLabel(tr("No Image loaded"), this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setBackgroundRole(QPalette::Mid);
    imageLabel->setAutoFillBackground(true);
    imageLabel->setScaledContents(true);

    curPixmap = QPixmap();
    orignalPixmap = QPixmap();

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Mid);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setWidget(imageLabel);
    scrollArea->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(scrollArea, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));

    scaleFactor = 1;
    m_fitToWindow = true;
    rotateAngle = 0;


    createActions();

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    mainLayout->setMargin(0);
    setLayout(mainLayout);


    imageControler = new ImageViewerControler(this);
    connect(imageControler, SIGNAL(signalRotate(int)), this, SLOT(rotate(int)));
    connect(imageControler, SIGNAL(signalFlip(bool,bool)), this, SLOT(flip(bool,bool)));

    //    connect(imageControler, SIGNAL(signalFlipHorizontal()), this, SLOT(flipHor()));
    //    connect(imageControler, SIGNAL(signalFlipVertical()), this, SLOT(flipVer()));
    connect(imageControler, SIGNAL(signalZoomIn()), this, SLOT(zoomIn()));
    connect(imageControler, SIGNAL(signalZoomOut()), this, SLOT(zoomOut()));
    connect(imageControler, SIGNAL(signalZoomFitBest()), this, SLOT(zoomFitBest()));
    connect(imageControler, SIGNAL(signalZoomOrignal()), this, SLOT(zoomOrignal()));

    connect(imageControler, SIGNAL(signalSave()), this, SLOT(save()));
    connect(imageControler, SIGNAL(signalSaveAs()), this, SLOT(saveAs()));


    animationControler = new AnimationControler(this);
    connect(animationControler, SIGNAL(signalFrameChanged(const QPixmap &)), this, SLOT(updateAnimationFrame(const QPixmap &)));


    toolButtonClose = new QToolButton(this);
    toolButtonClose->resize(35, 35);
    toolButtonClose->setIconSize(QSize(32, 32));
    toolButtonClose->setIcon(QIcon(":/resources/images/close.png"));
    toolButtonClose->setStyleSheet("QToolButton {\n    background-color: transparent;\n    border-color: transparent;\n    border-style: solid;\n}\n\nQToolButton:hover {\n   background-color: transparent;\n}\n\nQToolButton:pressed {\n    background-color: transparent;\n    padding-left: 5px;\n    padding-top: 5px;\n}");
    connect(toolButtonClose, SIGNAL(clicked()), this, SLOT(close()));


    int minimumWidth = qMax(imageControler->frameSize().width(), animationControler->frameGeometry().width());
    minimumWidth = qMax(minimumWidth, 600);
    int minimumHeight = imageControler->frameSize().height() + animationControler->frameGeometry().height();
    minimumHeight = qMax(minimumHeight, 400);
    m_minimumScrollAreaSize = QSize(minimumWidth, minimumHeight) + QSize(40, 40);
    scrollArea->setMinimumSize(m_minimumScrollAreaSize);


    runningValidMovie = false;

    tipLabel = new QLabel(this);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    tipLabel->setMinimumSize(minimumWidth, 40);
    tipLabel->hide();

    tipTimer = new QTimer(this);
    tipTimer->setSingleShot(true);
    tipTimer->setInterval(3000);
    connect(tipTimer, SIGNAL(timeout()), tipLabel, SLOT(hide()));

    tipScaleFactor = true;

    flipHorizontally = false;
    flipVertically = false;

    dragPosition = QPoint(0, 0);
    m_dragable = true;
    m_defaultSavePath = "./";

    imageLabel->setMouseTracking(true);
    scrollArea->setMouseTracking(true);
    //setMouseTracking(true);

    imageLabel->installEventFilter(this);
    scrollArea->installEventFilter(this);
    imageControler->installEventFilter(this);
    animationControler->installEventFilter(this);
    installEventFilter(this);


    setWindowTitle(tr("Image Viewer"));
    resize(800, 600);


    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry(this);
    int desktopWidth = rect.width();
    int desktopHeight = rect.height();
    int windowWidth = frameGeometry().width();
    int windowHeight = frameGeometry().height();
    move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);
    raise();

}

ImageViewer::~ImageViewer(){

    tipTimer->stop();

}

void ImageViewer::setImage(const QImage &image){

    //Resize
    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry(this);
    int desktopWidth = rect.width();
    int desktopHeight = rect.height();
    //    int newWindowWidth = qMin(image.width(), desktopWidth-40);
    //    int newWindowHeight = qMin(image.height(), desktopHeight-40);
    QSize minSize = QSize(qMin(image.width(), desktopWidth-40), qMin(image.height(), desktopHeight-40));
    QSize newSize = image.size().scaled(minSize, Qt::KeepAspectRatio);
    resize(newSize);
    move((desktopWidth - frameGeometry().width()) / 2, (desktopHeight - frameGeometry().height()) / 2);



    updatePixmap(QPixmap());
    imageLabel->resize(1,1);

    scaleFactor = 1;
    m_fitToWindow = true;
    rotateAngle = 0;
    flipHorizontally = false;
    flipVertically = false;

    imageControler->reset();

    qApp->processEvents();

    updatePixmap(QPixmap::fromImage(image));
    orignalPixmap = curPixmap;


    QSize viewportSize = scrollArea->viewport()->size();
    if(image.width() > viewportSize.width() ||  image.height() > viewportSize.height()){
        //zoomFitBest();
        QTimer::singleShot(50, this, SLOT(zoomFitBest()));
    }else{
        //zoomOrignal();
        QTimer::singleShot(50, this, SLOT(zoomOrignal()));
    }




    updateActions();

}

void ImageViewer::setImages(const QStringList &images, unsigned int initIndex){
    this->images = images;

    int size = images.size();
    curImageIndex = initIndex;
    if(curImageIndex >= size){
        curImageIndex = size - 1;
    }

    openFile(images.at(curImageIndex));

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

bool ImageViewer::eventFilter(QObject *obj, QEvent *event)
{

    switch (event->type()) {
    case QEvent::KeyRelease:
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *> (event);
        //        if(keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down){
        //        }

        if(keyEvent->key() == Qt::Key_Escape){
            qApp->quit();
            return true;
        }

        if(keyEvent->key() == Qt::Key_Right){
            openFile(curImageIndex + 1);
            return true;
        }
        if(keyEvent->key() == Qt::Key_Left){
            openFile(curImageIndex - 1);
            return true;
        }
        //        if(QApplication::keyboardModifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_P){

        //        }
        //        if(QApplication::keyboardModifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_E){

        //        }


        return true;
    }
        break;

        //    case QEvent::Enter:
        //    {
        //        //qDebug()<<"----QEvent::Enter"<<" "<<this;
        //        return true;
        //    }
        //        break;

    case QEvent::Leave:
    {
        //qDebug()<<"----QEvent::Leave"<<" "<<this;

        if(obj == imageControler){
            imageControler->hide();
            raise();
            setFocus();
            return true;
        }
    }
        break;

    case QEvent::MouseButtonPress:
    {
        if(!m_dragable){
            return QObject::eventFilter(obj, event);
        }

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);
        if(!mouseEvent){return false;}
        if (mouseEvent->button() == Qt::LeftButton) {
            dragPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
            return true;
        }
    }
        break;

    case QEvent::MouseMove:
    {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);
        if(!mouseEvent){return false;}

        if (mouseEvent->buttons() & Qt::LeftButton) {
            if(!m_dragable){
                return QObject::eventFilter(obj, event);
            }
            move(mouseEvent->globalPos() - dragPosition);
            //return true;
        }


        if(curPixmap.isNull()){return false;}

        QPoint globalPos = mouseEvent->globalPos();
        QPoint pos = scrollArea->mapFromGlobal(globalPos);

        int scrollAreaWidth = scrollArea->viewport()->width();
        int scrollAreaHeight = scrollArea->viewport()->height();

        if(pos.x() < (scrollAreaWidth - toolButtonClose->width()*2) && pos.y() <= imageControler->height() && pos.y() > 0){
            QPoint tl = scrollArea->mapToGlobal(QPoint( (scrollAreaWidth - imageControler->width())/2, 1) );
            imageControler->move(tl);
            imageControler->show();
        }else{
            imageControler->hide();
            raise();
            setFocus();
        }

        if(runningValidMovie){
            int animationControlerHeight = animationControler->height();
            if(pos.y() <= scrollAreaHeight && pos.y() > (scrollAreaHeight - animationControlerHeight) ){
                QPoint tl = scrollArea->mapToGlobal(QPoint( (scrollAreaWidth - animationControler->width())/2, (scrollAreaHeight - animationControlerHeight) ) );
                animationControler->move(tl);
                animationControler->show();
            }else{
                animationControler->hide();
                raise();
                setFocus();
            }
        }

        return true;
    }
        break;

    case QEvent::MouseButtonDblClick:
    {
        if(obj == scrollArea || obj == imageLabel){
            //imageControler->reset();
            reset();
            return true;
        }
    }
        break;

    case QEvent::Wheel:
    {
        //qDebug()<<"----QEvent::Wheel"<<" "<<this;

        QWheelEvent *e = static_cast<QWheelEvent *> (event);
        if(!e){return false;}

        if(curPixmap.isNull()){return false;}

        if(QApplication::keyboardModifiers() == Qt::ControlModifier){
            scaleImage(pow((double)2, -e->delta() / 240.0));
            return true;
        }

    }
        break;

    case QEvent::Resize:
    {
        //qDebug()<<"----QEvent::Resize"<<" obj:"<<obj;

        moveCloseButton();

        if(obj == this && m_fitToWindow && imageLabel->pixmap()){
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

void ImageViewer::setScaleButtonsVisible(bool visible){
    imageControler->setScaleButtonsVisible(visible);
}

void ImageViewer::setRotateButtonsVisible(bool visible){
    imageControler->setRotateButtonsVisible(visible);
}

void ImageViewer::setFlipButtonsVisible(bool visible){
    imageControler->setFlipButtonsVisible(visible);
}

void ImageViewer::setCloseButtonVisible(bool visible){
    toolButtonClose->setVisible(visible);
}

void ImageViewer::setDragable(bool dragable){
    m_dragable = dragable;
}

void ImageViewer::open()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Image Files"),
                                                    currentImageDirectory,
                                                    tr("Images (*.png *.xpm *.jpg *.mng *.svg *.gif *.bmp);;All files (*.*)")
                                                          );
    if (!fileNames.isEmpty()){
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

    runningValidMovie = animationControler->setFileName(fileName);
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

void ImageViewer::openFile(int imageIndex){

    int size = images.size();
    curImageIndex = imageIndex;
    if(curImageIndex >= size){
        curImageIndex = 0;
    }else if(curImageIndex < 0){
        curImageIndex = size - 1;
    }

    openFile(images.at(curImageIndex));

}

inline void ImageViewer::updatePixmap(const QPixmap &pixmap){
    curPixmap = pixmap;
    imageLabel->setPixmap(curPixmap);
}


void ImageViewer::updateAnimationFrame(const QPixmap &pixmap){
    //qDebug()<<"--ImageViewer::updateAnimationFrame(...)";

    updatePixmap(pixmap);
    orignalPixmap = curPixmap;

    tipScaleFactor = false;

    QMatrix matrix;
    matrix.rotate(rotateAngle);

    updatePixmap( QPixmap::fromImage(curPixmap.toImage().mirrored(flipHorizontally, flipVertically)).transformed(matrix,Qt::SmoothTransformation) );

    if(m_fitToWindow){
        zoomFitBest();
    }else{
        scaleImage(1);
    }

    tipScaleFactor = true;

}

void ImageViewer::updateAnimationFrame(const QImage &image){

    updatePixmap(QPixmap::fromImage(image));
    orignalPixmap = curPixmap;

    tipScaleFactor = false;

    QMatrix matrix;
    matrix.rotate(rotateAngle);

    updatePixmap( QPixmap::fromImage(image.mirrored(flipHorizontally, flipVertically)).transformed(matrix,Qt::SmoothTransformation) );

    if(m_fitToWindow){
        zoomFitBest();
    }else{
        scaleImage(1);
    }

    tipScaleFactor = true;

}

void ImageViewer::setText(const QString &text){
    imageLabel->setText(text);
}

bool ImageViewer::setDefaultSavePath(const QString &path){

    QDir dir;
    if(!dir.mkpath(path)){
        QMessageBox::critical(this, tr("Error"), tr("Can not create path:<p>%1</p>").arg(path));
        return false;
    }

    m_defaultSavePath = path;

    return false;

}

QString ImageViewer::defaultSavePath() {

    if(m_defaultSavePath.trimmed().isEmpty()){
        m_defaultSavePath = "./";
    }

    return m_defaultSavePath;
}

void ImageViewer::rotate(int angle){

    if(angle == 0){return;}

    //    const QPixmap *pixmap = imageLabel->pixmap();
    //    Q_ASSERT(pixmap);
    //    if(!pixmap){return;}

    if(curPixmap.isNull()){return;}

    QMatrix matrix;
    matrix.rotate(angle);

    //    QPixmap pixmap = curPixmap.transformed(matrix,Qt::SmoothTransformation);
    //    imageLabel->setPixmap(pixmap);
//    updatePixmap(curPixmap.transformed(matrix,Qt::SmoothTransformation));
    updatePixmap( QPixmap::fromImage(orignalPixmap.toImage().mirrored(flipHorizontally, flipVertically)).transformed(matrix,Qt::SmoothTransformation) );


    if(m_fitToWindow){
        zoomFitBest();
    }else{
        scaleImage(1);
    }

    rotateAngle = angle;

}

void ImageViewer::flip(bool horizontally, bool vertically){

    //    const QPixmap *pixmap = imageLabel->pixmap();
    //    Q_ASSERT(pixmap);
    //    if(!pixmap){return;}

    if(curPixmap.isNull()){return;}

    //QImage image = pixmap->toImage().mirrored(true, false);

    updatePixmap(QPixmap::fromImage(curPixmap.toImage().mirrored(horizontally, vertically)));
    //    curPixmap = QPixmap::fromImage(curPixmap.toImage().mirrored(horizontally, vertically));
    //    imageLabel->setPixmap(curPixmap);

    if(m_fitToWindow){
        zoomFitBest();
    }


    if(horizontally){
        flipHorizontally = !flipHorizontally;
    }
    if(vertically){
        flipVertically = !flipVertically;
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

    const QPixmap *pixmap = imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap){return;}

    QSize imageSize = pixmap->size();
    QSize viewportSize = scrollArea->viewport()->size();

    QSize newImageSize = imageSize.scaled(viewportSize, Qt::KeepAspectRatio);
    imageLabel->resize(newImageSize);

    scaleFactor = (double)newImageSize.width() / imageSize.width();

    m_fitToWindow = true;


    updateActions();

    showScaleFactor();
}

void ImageViewer::zoomOrignal()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;

    m_fitToWindow = false;

    showScaleFactor();

}

void ImageViewer::save(){

    QString path = m_defaultSavePath + QString("/%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));

    const QPixmap *pixmap = imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap){return;}

    if(!pixmap->save(path)){
        QMessageBox::critical(this, tr("Error"), tr("Can not save image as:<p>%1</p>").arg(path));
    }


}

void ImageViewer::saveAs(){

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
    if(path.isEmpty()){return;}
    QFileInfo info(path);
    QString sufffix = info.suffix().trimmed();
    if(sufffix.isEmpty()){
        sufffix = filtersHash.key(selectedFilter);
        path += sufffix;
    }


    const QPixmap *pixmap = imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap){return;}

    if(!pixmap->save(path)){
        QMessageBox::critical(this, tr("Error"), tr("Can not save image as:<p>%1</p>").arg(path));
    }


}

void ImageViewer::print()
{

    const QPixmap *pixmap = imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap){return;}

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


void ImageViewer::reset(){

    if(curPixmap.isNull()){return;}

    imageControler->reset();
    zoomFitBest();

}


void ImageViewer::showContextMenu(const QPoint &pos){


    QMenu menu;
    //    menu.addAction(zoomInAct);
    //    menu.addAction(zoomOutAct);
    //    menu.addAction(normalSizeAct);
    //    menu.addAction(fitToWindowAct);
    menu.addAction(resetAct);

    menu.addSeparator();
    menu.addAction(saveAsAct);
#ifndef QT_NO_PRINTER
    menu.addAction(printAct);
#endif

    menu.addSeparator();
    menu.addAction(openAct);
    menu.addAction(exitAct);

    menu.exec(mapToGlobal(pos));

}

void ImageViewer::showTip(const QString &tip){


    tipLabel->setText(tip);

    QPoint tl =  QPoint(scrollArea->geometry().center()) - QPoint( tipLabel->geometry().width()/2, tipLabel->geometry().height()/2) ;
    tipLabel->move(tl);
    tipLabel->show();

    tipTimer->start();

    //QToolTip::showText(pos, tip);
}

void ImageViewer::showScaleFactor(){

    if(!tipScaleFactor){return;}

    QString tip = QString("<div align=\"center\" style=\"font-size:20pt;font-weight:bold;color:#ff0000;\"> %1% </div>").arg( (int)(scaleFactor*100));
    showTip(tip);
}

void ImageViewer::showImageInfo(){

    QString tip = QString("<div align=\"center\" style=\"font-size:20pt;font-weight:bold;color:#ff0000;\"> %1% </div>").arg( (int)(scaleFactor*100));
    showTip(tip);
}

void ImageViewer::moveCloseButton(){
    QPoint tr = scrollArea->mapTo(this, QPoint( (scrollArea->viewport()->width() - toolButtonClose->width() ), 1 ));
    toolButtonClose->move(tr);
}


void ImageViewer::createActions()
{

    zoomInAct = new QAction(tr("Zoom &In"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+N"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(zoomOrignal()));

    resetAct = new QAction(tr("&Reset..."), this);
    resetAct->setShortcut(tr("Ctrl+R"));
    resetAct->setEnabled(false);
    connect(resetAct, SIGNAL(triggered()), this, SLOT(reset()));



    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setEnabled(false);
    //fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(zoomFitBest()));

    saveAsAct = new QAction(tr("&Save As..."), this);
    saveAsAct->setShortcut(tr("Ctrl+S"));
    saveAsAct->setEnabled(false);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setEnabled(false);
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));


    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));






}

void ImageViewer::updateActions()
{

    bool imageValid = !curPixmap.isNull();

    zoomInAct->setEnabled(imageValid);
    zoomOutAct->setEnabled(imageValid);
    normalSizeAct->setEnabled(imageValid);
    fitToWindowAct->setEnabled(imageValid);

    resetAct->setEnabled(imageValid);
    saveAsAct->setEnabled(imageValid);

    printAct->setEnabled(imageValid);

}



void ImageViewer::scaleImage(double factor)
{

    const QPixmap *pixmap = imageLabel->pixmap();
    Q_ASSERT(pixmap);
    if(!pixmap){return;}

    //    Q_ASSERT(imageLabel->pixmap());
    //Q_ASSERT(!movie->currentPixmap().isNull());


    scaleFactor *= factor;

    if(scaleFactor > 10){
        scaleFactor = 10;
    }
    if(scaleFactor < 0.1){
        scaleFactor = 0.1;
    }

    imageLabel->resize(scaleFactor * pixmap->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);

    showScaleFactor();

}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}










} //namespace HEHUI
