#ifndef SCREENCAPTUREPROCESSOR_H
#define SCREENCAPTUREPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QElapsedTimer>
#include <QByteArray>
#include <QRect>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>


#include "screencapture.h"


#ifndef Q_OS_WIN
typedef quint16             WORD;
typedef quint32             DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef char                CHAR;
typedef short               SHORT;
typedef quint32             LONG;


#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)


#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L

#endif


class ImageBlock : public QObject
{
    Q_OBJECT
public:
    ImageBlock(const QRect &blockRect, int index, QObject *parent);
    virtual ~ImageBlock();

    static void setImageInfo(uint pixelSize, uint width, uint height);
    static void setBitmapFormat(bool bitmapFormat);
    static bool isBitmapFormat();
    static void setScanLinesInterval(uint lines);

    bool fastCompare(const uchar *previousFullImageData, const uchar *newFullImageData, int size, QList<int> *changedBlockIndexList, QList<ImageBlock *> *workList, int startX, int startY);
    bool compare1(const uchar *previousData, const uchar *newData, int size);
    bool compare2(const uchar *previousData, const uchar *newData, int size);
    bool compare(const uchar *previousFullImageData, const uchar *newFullImageData, int size);

    bool assembleBlockData(uchar *previousData, int size);
    bool assembleBlockData1(uchar *previousData, int size);

    QRect blockRect();
    QByteArray blockData();
    bool asignBlockData(const uchar *newFullImageData, int size);
    const int blockIndex();
    bool setBlockData(const QByteArray &blockData);
    void resetBlockData();

    void setSiblings(ImageBlock *right, ImageBlock *below);


private:
    QMutex m_mutex;
    QRect m_blockRect;
    int m_blockIndex;
    int m_blockDataSize;
    uchar *m_blockData;

    ImageBlock *m_siblingRight;
    ImageBlock *m_siblingBelow;
    int m_scanLineIndex;


    static uint m_pixelSize;
    static uint m_imageWidth;
    static uint m_imageHeight;
    static bool m_bitmapFormat;
    static uint m_scanLinesInterval;


};


class ScreenCaptureProcessor : public QThread
{
    Q_OBJECT
public:
    explicit ScreenCaptureProcessor(bool captureMode, QObject *parent = 0);
    ~ScreenCaptureProcessor();

signals:
    void signalScreenRectInfoChanged(int pixelSize, int width, int height, uint blockRows, uint blockColumns);
    void signalScreenDataChanged(const QImage &image);
    void signalScreenDataChanged(const QList<int> &blockIndexList);
    void signalScreenDataChanged();

    void signalSeenGeometryChanged();


public slots:
    void getScreenRectInfo(int *pixelSize, int *width, int *height, uint *blockRows, uint *blockColumns, bool *bitmapFormat);
    void setScreenRectInfo(int pixelSize, int width, int height, uint blockRows, uint blockColumns, bool bitmapFormat); //Receiver

    bool getScreenCapture(QList<int> *blockIndex, QList<QByteArray> *blockData, bool onlyChanged = true);
    void updateBlockData(QList<int> *blockIndexList, QList<QByteArray> *blockDataList); //Receiver

    void setAboutToQuit();
    bool isAboutToQuit();
    bool threadFinished();

    void setBitmapFormat(bool bitmap);

    void setTimerInterval(int interval);

    void clearBlockInfo();
    void setBlockInfo(uint blockRows, uint blockColumns);

    const QByteArray fullFixedBitmapData();
//    const uchar* fullFixedBitmapData();

    bool setFullFixedBitmapData(const QByteArray &data);

    quint16 keyframeID();
    void setKeyFrameSent(bool sent);
    bool isKeyFrameSent();


    void enqueue(const QByteArray &data);
    QByteArray dequeue();


    bool init();
    void deInitilize();

private slots:


    //void timerTimeout();
    void updateKeyFrame();

    void generateImage();
    void generateImageFromBitmapData();

    bool setDesktop();

    void checkSeenGeometry();


protected:
    void run();

private:
    bool m_captureMode;

    BITMAPINFOHEADER bi;
    BITMAPFILEHEADER bf;

    uchar *m_previousBitmapData;
    uchar * m_newBitmapData;
    int m_dataSize;

    int m_pixelSize;
    int m_imageWidth;
    int m_imageHeight;

    ScreenCapture *m_capture;
    bool m_initialized;

    uint m_blockRows;
    uint m_blockColumns;

    int m_timeInterval; //ms
    //int m_linesInterval;
    //int m_lineStartOffset;




//    QTimer m_timer;

    QList<ImageBlock*> m_imageBlocks;

    QList<int> m_changedBlockIndexList;


    QWaitCondition cond;
    QMutex m_mutex;


    bool m_aboutToQuit;
    bool m_threadFinished;

    bool m_bitmapFormat;

    bool m_updateFixedBitmapData;

    quint16 m_keyframeID;
    int m_keyFrameInterval; //ms
    int m_orignalKeyFrameInterval; //ms
    qint64 m_lastKeyframeTimestamp;

    bool m_keyFrameSent;
    QMutex m_keyFrameMutex;

    int m_moreThanHalfChangesCount;

    QQueue<QByteArray> m_queue;

    QTimer m_seenGeometryChangedEmitterTimer;


};

#endif // SCREENCAPTUREPROCESSOR_H
