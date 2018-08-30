#include "screencapture.h"

#include <QImage>
#include <QDebug>
#include <QApplication>

//#include <iostream>
//#include <fstream>
//using namespace std;



ScreenCapture::ScreenCapture(QObject *parent) : QObject(parent)
{
    m_dataArray = 0;

    m_nWidth = 0;
    m_nHeight = 0;


#ifdef Q_OS_WIN
    m_nBitCount = 24;

    m_hWnd = NULL;
    m_hDesktop = NULL;
    m_hSrcDC = NULL;

    m_hMemDC = NULL;
    m_hBitmap = NULL;
    m_hOldBitmap = NULL;

    m_bi = {0};
    m_bf = {0};

    m_imageBytes = 0;

    m_hDib = NULL;

#else
    m_nBitCount = 32;

    screen = 0;

#endif

    m_initialized = false;

}

ScreenCapture::~ScreenCapture()
{
    deInitilize();
}

//const QByteArray ScreenCapture::bitmapData()
//{
//    QMutexLocker locker(&m_mutex);

//    return m_bmpData;
//}

void ScreenCapture::getScreenRectInfo(int *pixelSize, int *width, int *height)
{
    if(!m_initialized){
        init();
    }

    if(pixelSize){
        *pixelSize = m_nBitCount / 8;
    }

    if(width){
        *width = m_nWidth;
    }

    if(height){
        *height = m_nHeight;
    }
}

const uchar * ScreenCapture::dataArray() const
{
    return m_dataArray;
}

bool ScreenCapture::init()
{

    delete [] m_dataArray;
    m_imageBytes = 0;

#ifdef Q_OS_WIN

    if(NULL == m_hWnd)
    {
        m_hWnd = ::GetDesktopWindow();
    }
    assert(m_hWnd);

    RECT rect;
    ::GetWindowRect(m_hWnd, &rect);
    m_nWidth = rect.right - rect.left;
    m_nHeight = rect.bottom - rect.top;

    if(NULL == m_hSrcDC){
        m_hSrcDC = ::GetWindowDC(m_hWnd);
        //m_hSrcDC = ::CreateDC(L"display",NULL,NULL,NULL);
    }
    assert(m_hSrcDC);

    if(NULL == m_hMemDC){
        m_hMemDC = ::CreateCompatibleDC(m_hSrcDC);
    }
    assert(m_hMemDC);

    if(NULL == m_hBitmap){
        m_hBitmap = ::CreateCompatibleBitmap(m_hSrcDC, m_nWidth, m_nHeight);
    }
    assert(m_hBitmap);

    m_hOldBitmap = (HBITMAP)::SelectObject(m_hMemDC, m_hBitmap);

    m_bi = {0};
    m_bi.biSize = sizeof(BITMAPINFOHEADER);
    m_bi.biWidth = m_nWidth;
    m_bi.biHeight = m_nHeight;
    m_bi.biPlanes = 1;
    m_bi.biBitCount = m_nBitCount;
    m_bi.biCompression = BI_RGB;
    m_imageBytes = ((m_nWidth * m_nBitCount + 31) / 32) * 4 * m_nHeight;

    m_dataArray = new uchar[m_imageBytes];
    memset(m_dataArray, 0, m_imageBytes);

    m_bf = {0};

    if(NULL == m_hDib){
        m_hDib = GlobalAlloc(GHND, m_imageBytes + sizeof(BITMAPINFOHEADER));
    }

#else
    screen = QApplication::primaryScreen();
    if (!screen) {
        qCritical() << "ERROR! No primary screen.";
        return false;
    }

    QPixmap pixmap = screen->grabWindow(0);
    QImage screenImage = pixmap.toImage();

    m_nWidth = screenImage.width();
    m_nHeight = screenImage.height();
    m_nBitCount = screenImage.depth();

    if(m_imageBytes != (quint32)(screenImage.bytesPerLine()*screenImage.height())){
        delete [] m_dataArray;
        m_imageBytes = (quint32)(screenImage.bytesPerLine()*screenImage.height());
        m_dataArray = new uchar[m_imageBytes];
        memset(m_dataArray, 0, m_imageBytes);
    }


#endif

    m_initialized = true;
    return true;
}

void ScreenCapture::deInitilize()
{

    delete [] m_dataArray;
    m_dataArray = 0;
    m_imageBytes = 0;

    m_nWidth = 0;
    m_nHeight = 0;
    m_nBitCount = 0;

#ifdef Q_OS_WIN

    GlobalUnlock(m_hDib);
    GlobalFree(m_hDib);

    ::SelectObject(m_hMemDC, m_hOldBitmap);
    ::DeleteObject(m_hBitmap);
    ::DeleteDC(m_hMemDC);
    ::ReleaseDC(m_hWnd, m_hSrcDC);

#else


#endif

    m_initialized = false;

}

bool ScreenCapture::capture()
{
    assert(m_initialized);
    assert(m_dataArray);


    if(!m_initialized){
        qCritical()<<"Not initialized!";
        return false;
    }


#ifdef Q_OS_WIN


    ::BitBlt(m_hMemDC, 0, 0, m_nWidth, m_nHeight, m_hSrcDC, 0, 0, SRCCOPY|CAPTUREBLT);


    LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(m_hDib);
    *lpbi = m_bi;

    ::GetDIBits(m_hMemDC, m_hBitmap, 0, m_nHeight, (BYTE*)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO*)lpbi, DIB_RGB_COLORS);


    m_bf.bfType = 0x4d42;
    m_bf.bfSize = m_imageBytes + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    m_bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    memcpy(m_dataArray, (const uchar*)lpbi + sizeof(BITMAPINFOHEADER), m_imageBytes);


    GlobalUnlock(m_hDib);
//    GlobalFree(hDib);

#else
    QPixmap pixmap = screen->grabWindow(0);
    QImage screenImage = pixmap.toImage();
//    screenImage = QImage("./test/1.png");

    if(m_imageBytes != (quint32)(screenImage.bytesPerLine()*screenImage.height())){
        delete [] m_dataArray;
        m_imageBytes = (quint32)(screenImage.bytesPerLine()*screenImage.height());
        m_dataArray = new uchar[m_imageBytes];
        memset(m_dataArray, 0, m_imageBytes);
    }

    memcpy(m_dataArray, screenImage.bits(), m_imageBytes);

#endif

    return true;
}

void ScreenCapture::capture(QByteArray *dataArray)
{
    assert(m_initialized);
    assert(dataArray);


    if(!m_initialized){
        qCritical()<<"Not initialized!";
        return;
    }


#ifdef Q_OS_WIN


    ::BitBlt(m_hMemDC, 0, 0, m_nWidth, m_nHeight, m_hSrcDC, 0, 0, SRCCOPY);


    LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(m_hDib);
    *lpbi = m_bi;

    ::GetDIBits(m_hMemDC, m_hBitmap, 0, m_nHeight, (BYTE*)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO*)lpbi, DIB_RGB_COLORS);


    m_bf.bfType = 0x4d42;
    m_bf.bfSize = m_imageBytes + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    m_bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


    if(dataArray->size() < m_imageBytes){
        dataArray->resize(m_imageBytes);
    }
    memcpy(dataArray->data(), (const char*)lpbi + sizeof(BITMAPINFOHEADER), m_imageBytes);
//    if(dataArray){
//        dataArray->append((const char*)lpbi + sizeof(BITMAPINFOHEADER), m_imageBytes);
//    }



//    QMutexLocker locker(&m_mutex);
//    m_bmpData.clear();
//    m_bmpData.append((const char*)lpbi + sizeof(BITMAPINFOHEADER), dwSize);

//    QImage img5((uchar*)m_bmpData.data(), nWidth, nHeight, QImage::Format_RGB888);
//    img5.save("test5.bmp");


//    ofstream fs;
//    fs.open("test.bmp", ios::out|ios::binary);
//    fs.write((const char*)&bf, sizeof(BITMAPFILEHEADER));
//    fs.write((const char*)lpbi, bf.bfSize);
//    fs.close();

//    QByteArray ba;
//    ba.append((const char*)&bf, sizeof(BITMAPFILEHEADER));
//    ba.append((const char*)lpbi, bf.bfSize);

//    QImage img2;
//    img2.loadFromData(ba, "bmp");
//    img2.save("test2.bmp");


//    ////BGR to RGB
//    uchar* ptr= (uchar*)lpbi + sizeof(BITMAPINFOHEADER);
//    uchar temp = 0;
//    int offset = 0;
//    for(int i=0; i<nWidth*nHeight; i++){
//        temp = ptr[offset+2]; //G

//        ptr[offset+2] = ptr[offset];
//        ptr[offset] = temp;

//        offset += 3;
//    }

//    QImage img4(ptr, nWidth, nHeight, QImage::Format_RGB888);
//    img4.save("test4.bmp");


    GlobalUnlock(m_hDib);
//    GlobalFree(hDib);

#else
    QPixmap pixmap = screen->grabWindow(0);
    QImage screenImage = pixmap.toImage();

    if(m_imageBytes != (quint32)(screenImage.bytesPerLine()*screenImage.height())){
        delete [] m_dataArray;
        m_imageBytes = (quint32)(screenImage.bytesPerLine()*screenImage.height());
        m_dataArray = new uchar[m_imageBytes];
        memset(m_dataArray, 0, m_imageBytes);
    }

    memcpy(dataArray->data(), screenImage.bits(), m_imageBytes);

#endif

}
