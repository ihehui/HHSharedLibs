#ifndef IMAGEVIEWERCONTROLER_H
#define IMAGEVIEWERCONTROLER_H

#include <QWidget>

namespace Ui {
class ImageViewerControlerUI;
}


namespace HEHUI {



class ImageViewerControler : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewerControler(QWidget *parent = 0, Qt::WindowFlags fl = Qt::Popup | Qt::FramelessWindowHint);
    ~ImageViewerControler();



signals:
    void signalRotate(int angle);
    void signalFlip(bool horizontally, bool vertically);

    void signalFlipHorizontal();
    void signalFlipVertical();

    void signalZoomIn();
    void signalZoomOut();
    void signalZoomFitBest();
    void signalZoomOrignal();

    void signalSaveAs();

public slots:
    void reset();
    void setScaleButtonsVisible(bool visible = true);
    void setRotateButtonsVisible(bool visible = true);
    void setFlipButtonsVisible(bool visible = true);


private slots:
    void on_toolButtonRotateLeft_clicked();
    void on_dial_valueChanged(int value);
    void on_toolButtonRotateRight_clicked();

    void on_toolButtonFlipVertical_clicked(){emit signalFlip(false, true);}
    void on_toolButtonFlipHorizontal_clicked(){emit signalFlip(true, false);}

    void on_toolButtonZoomIn_clicked(){emit signalZoomIn();}
    void on_toolButtonZoomOut_clicked(){emit signalZoomOut();}
    void on_toolButtonZoomFitBest_clicked(){emit signalZoomFitBest();}
    void on_toolButtonZoomOriginal_clicked(){emit signalZoomOrignal();}

    void on_toolButtonSaveAs_clicked(){emit signalSaveAs();}


private:
    Ui::ImageViewerControlerUI *ui;



};

} //namespace HEHUI

#endif // IMAGEVIEWERCONTROLER_H
