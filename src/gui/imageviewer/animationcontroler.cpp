/*
 ****************************************************************************
 * animationcontroler.cpp
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


#include "animationcontroler.h"
#include "ui_animationcontroler.h"


namespace HEHUI {


AnimationControler::AnimationControler(QWidget *parent, Qt::WindowFlags fl) :
    QWidget(parent, fl),
    ui(new Ui::AnimationControler)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    movie = new QMovie(this);
    movie->setCacheMode(QMovie::CacheAll);

    connect(movie, SIGNAL(frameChanged(int)), this, SLOT(updateFrame()));
    connect(movie, SIGNAL(stateChanged(QMovie::MovieState)), this, SLOT(updateAnimationControls()));

    connect(ui->toolButtonPlay, SIGNAL(clicked()), movie, SLOT(start()));
    connect(ui->toolButtonPause, SIGNAL(clicked(bool)), movie, SLOT(setPaused(bool)));
    connect(ui->toolButtonStop, SIGNAL(clicked()), movie, SLOT(stop()));

    connect(ui->horizontalSliderCurrentFrame, SIGNAL(valueChanged(int)), this, SLOT(goToFrame(int)));
    connect(ui->spinBoxSpeed, SIGNAL(valueChanged(int)), movie, SLOT(setSpeed(int)));

}

AnimationControler::~AnimationControler()
{
    delete ui;
}

QPixmap AnimationControler::currentPixmap() const{
    return movie->currentPixmap();
}

bool AnimationControler::isValidMovie(){
    return (movie->isValid() && movie->frameCount() > 1);
}

bool AnimationControler::setFileName(const QString &fileName){

    movie->stop();
    movie->setFileName(fileName);
    if(movie->isValid() && movie->frameCount() > 1){
        movie->start();
        return true;
    }else{
        movie->stop();
    }

    return false;

}


void AnimationControler::updateFrame()
{

    bool hasFrames = (movie->currentFrameNumber() >= 0);
    if (hasFrames) {
        if (movie->frameCount() > 0) {
            ui->horizontalSliderCurrentFrame->setMaximum(movie->frameCount() - 1);
        } else {
            if (movie->currentFrameNumber() > ui->horizontalSliderCurrentFrame->maximum())
                ui->horizontalSliderCurrentFrame->setMaximum(movie->currentFrameNumber());
        }
        ui->horizontalSliderCurrentFrame->setValue(movie->currentFrameNumber());
    } else {
        ui->horizontalSliderCurrentFrame->setMaximum(0);
    }
    ui->horizontalSliderCurrentFrame->setEnabled(hasFrames);

    emit signalFrameChanged(movie->currentPixmap());

}

void AnimationControler::updateAnimationControls()
{

    ui->horizontalSliderCurrentFrame->setEnabled(movie->isValid() && movie->frameCount() != 1);

    ui->toolButtonPlay->setEnabled(movie->isValid() && movie->frameCount() != 1 && movie->state() == QMovie::NotRunning);
    ui->toolButtonPause->setEnabled(movie->state() != QMovie::NotRunning);
    ui->toolButtonPause->setChecked(movie->state() == QMovie::Paused);
    ui->toolButtonStop->setEnabled(movie->state() != QMovie::NotRunning);


}

void AnimationControler::goToFrame(int frame)
{
    movie->jumpToFrame(frame);
}




} //namespace HEHUI
