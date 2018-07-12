#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QObject>
#include <QByteArray>
#include <QMutex>

#ifdef Q_OS_WIN
    #include <Windows.h>
#else

#endif



class ScreenCapture : public QObject
{
    Q_OBJECT
public:
    explicit ScreenCapture(QObject *parent = 0);
    ~ScreenCapture();

    void getScreenRectInfo(int *pixelSize, int *width, int *height);

signals:

public slots:
    bool init();
    void capture(QByteArray *dataArray);



private:

    int m_nWidth;
    int m_nHeight;
    int m_nBitCount;

#ifdef Q_OS_WIN

    HWND m_hWnd;
    HWND m_hDesktop;

    HDC m_hSrcDC;
    HDC m_hMemDC;
    HBITMAP m_hBitmap;
    HBITMAP m_hOldBitmap;
    BITMAPINFOHEADER m_bi;
    BITMAPFILEHEADER m_bf;
    DWORD m_dwSize;
    HANDLE m_hDib;



#else
    QScreen *screen;


#endif

    QMutex m_mutex;

    bool m_initialized;

};

#endif // SCREENCAPTURE_H
