#include "imageviewercontroler.h"
#include "ui_imageviewercontroler.h"


namespace HEHUI {



ImageViewerControler::ImageViewerControler(QWidget *parent, Qt::WindowFlags fl) :
    QWidget(parent, fl),
    ui(new Ui::ImageViewerControlerUI)
{

    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground, true);

    setMouseTracking(true);

}

ImageViewerControler::~ImageViewerControler()
{
    delete ui;
}

void ImageViewerControler::reset(){
    ui->dial->setValue(180);
//    setScaleButtonsVisible(true);
//    setRotateButtonsVisible(true);
//    setFlipButtonsVisible(true);
}

void ImageViewerControler::setScaleButtonsVisible(bool visible){
    ui->toolButtonZoomIn->setVisible(visible);
    ui->toolButtonZoomOut->setVisible(visible);
    ui->toolButtonZoomFitBest->setVisible(visible);
    ui->toolButtonZoomOriginal->setVisible(visible);
}

void ImageViewerControler::setRotateButtonsVisible(bool visible){
    ui->toolButtonRotateLeft->setVisible(visible);
    ui->dial->setVisible(visible);
    ui->toolButtonRotateRight->setVisible(visible);
}

void ImageViewerControler::setFlipButtonsVisible(bool visible){
    ui->toolButtonFlipVertical->setVisible(visible);
    ui->toolButtonFlipHorizontal->setVisible(visible);

}

void ImageViewerControler::on_toolButtonRotateLeft_clicked(){


    int value = ui->dial->value();

    int a = value / 90 - 1;
    if(a <= 0){ a = 4;}
    ui->dial->setValue(a*90);

}

void ImageViewerControler::on_dial_valueChanged(int value){

    if(value <= 180){
        emit signalRotate(180 + value);
    }else{
        emit signalRotate(value - 180);
    }


}

void ImageViewerControler::on_toolButtonRotateRight_clicked(){

    int value = ui->dial->value();

    int a = value / 90 + 1;
    if(a >= 4){a = 0;}
    ui->dial->setValue(a*90);

}







} //namespace HEHUI
