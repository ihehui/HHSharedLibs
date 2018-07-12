#ifndef SCREENCAPTUREPROCESSOR_H
#define SCREENCAPTUREPROCESSOR_H

#include <QObject>
#include <QElapsedTimer>
#include <QByteArray>
#include <QRect>
#include <QTimer>


#include "screencapture.h"


class ImageBlock : public QObject
{
    Q_OBJECT
public:
    ImageBlock(const QRect &blockRect, QObject *parent);
    virtual ~ImageBlock();

    static void setImageInfo(int pixelSize, int width, int height);

    bool compare(const QByteArray *previousData, const QByteArray *newData);
    QRect blockRect();
    QByteArray blockData();



private:
    QRect m_blockRect;
    QByteArray m_blockData;

    static int m_pixelSize;
    static int m_imageWidth;
    static int m_imageHeight;

};


class ScreenCaptureProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ScreenCaptureProcessor(QObject *parent = nullptr);

signals:

public slots:
    void getScreenRectInfo(int *pixelSize, int *width, int *height, int *blockRows, int *blockColumns);
    bool getScreenCapture(QList<int> *blockIndex, QList<QByteArray> *blockData);

private slots:
    void timerTimeout();

private:
    ScreenCapture m_capture;

    int m_pixelSize;
    int m_imageWidth;
    int m_imageHeight;

    int m_timeInterval; //ms
    int m_blockRows;
    int m_blockColumns;
    int m_linesInterval;
    int m_lineStartOffset;

    QByteArray m_previousBitmapData;
    QByteArray m_newBitmapData;


    QTimer m_timer;

    QList<ImageBlock*> m_imageBlocks;



};

#endif // SCREENCAPTUREPROCESSOR_H
