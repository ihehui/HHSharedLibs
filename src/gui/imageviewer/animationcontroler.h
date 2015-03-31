#ifndef ANIMATIONCONTROLER_H
#define ANIMATIONCONTROLER_H

#include <QWidget>
#include <QMovie>


namespace Ui {
class AnimationControler;
}

namespace HEHUI {


class AnimationControler : public QWidget
{
    Q_OBJECT

public:
    explicit AnimationControler(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Popup | Qt::FramelessWindowHint);
    ~AnimationControler();


    QPixmap currentPixmap() const;
    bool isValidMovie();

signals:
    void signalFrameChanged(const QPixmap &pixmap);

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
