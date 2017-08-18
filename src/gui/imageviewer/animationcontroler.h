/*
 ****************************************************************************
 * animationcontroler.h
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


#ifndef ANIMATIONCONTROLER_H
#define ANIMATIONCONTROLER_H

#include <QWidget>
#include <QMovie>


namespace Ui
{
class AnimationControler;
}

namespace HEHUI
{

class AnimationControler : public QWidget
{
    Q_OBJECT

public:
    explicit AnimationControler(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Popup | Qt::FramelessWindowHint);
    ~AnimationControler();


    QPixmap currentPixmap() const;
    bool isValidMovie();

signals:
    void signalFrameChanged(const QImage &pixmap);

public slots:
    bool setFileName(const QString &fileName);

private slots:
    void updateFrame();
    void updateAnimationControls();
    void goToFrame(int frame);


private:
    Ui::AnimationControler *ui;

    QMovie *movie;



};

} //namespace HEHUI

#endif // ANIMATIONCONTROLER_H
