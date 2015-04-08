/*
 ****************************************************************************
 * imageresourcebase.cpp
 *
 * Created on: 2009-11-6
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
 * Last Modified on: 2010-05-14
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#include <QFile>
#include <QPixmap>

#include "imageresourcebase.h"

#ifdef Q_OS_WIN32
#include <Windows.h>
#endif




namespace HEHUI {

ImageResourceBase::ImageResourceBase(QObject *parent)
    :QObject(parent)
{
    // TODO Auto-generated constructor stub

}

ImageResourceBase::~ImageResourceBase() {
    // TODO Auto-generated destructor stub
}

QIcon ImageResourceBase::createIcon(const QString &iconFileName, const QString &defaultIconName,  QIcon::Mode mode){
    QString file;
    if(QFile::exists(iconFileName)){
        file = iconFileName;
    }else if(QFile::exists(defaultIconName)){
        file = defaultIconName;
    }else{
        file = QString(":/resources/images/emptyicon.png");
    }


    if(mode == QIcon::Disabled){
        QIcon icon;
        QSize size = QImage(file).size();
        QPixmap pixmap = QIcon(file).pixmap(size, mode);
        icon.addPixmap(pixmap);
        return icon;
    }

    return QIcon(file);

}


QIcon ImageResourceBase::emptyIcon()
{
    static const QIcon empty_icon(QLatin1String(":/resources/images/emptyicon.png"));
    return  empty_icon;
}

///////////////////////////////////////////////////
#ifdef Q_OS_WIN32

////From activeqt/shared/qaxutils.cpp
QImage ImageResourceBase::WinHBITMAPToImage(HBITMAP bitmap, bool noAlpha)
{
    // Verify size
    BITMAP bitmap_info;
    memset(&bitmap_info, 0, sizeof(BITMAP));

    const int res = GetObject(bitmap, sizeof(BITMAP), &bitmap_info);
    if (!res) {
        qErrnoWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap info");
        return QImage();
    }
    const int w = bitmap_info.bmWidth;
    const int h = bitmap_info.bmHeight;

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = -h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = w * h * 4;

    // Get bitmap bits
    QScopedArrayPointer<uchar> data(new uchar[bmi.bmiHeader.biSizeImage]);
    HDC display_dc = GetDC(0);
    if (!GetDIBits(display_dc, bitmap, 0, h, data.data(), &bmi, DIB_RGB_COLORS)) {
        ReleaseDC(0, display_dc);
        qWarning("%s, failed to get bitmap bits", __FUNCTION__);
        return QImage();
    }

    QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;
    uint mask = 0;
    if (noAlpha) {
        imageFormat = QImage::Format_RGB32;
        mask = 0xff000000;
    }

    // Create image and copy data into image.
    QImage image(w, h, imageFormat);
    if (image.isNull()) { // failed to alloc?
        ReleaseDC(0, display_dc);
        qWarning("%s, failed create image of %dx%d", __FUNCTION__, w, h);
        return QImage();
    }
    const int bytes_per_line = w * sizeof(QRgb);
    for (int y = 0; y < h; ++y) {
        QRgb *dest = (QRgb *) image.scanLine(y);
        const QRgb *src = (const QRgb *) (data.data() + y * bytes_per_line);
        for (int x = 0; x < w; ++x) {
            const uint pixel = src[x];
            if ((pixel & 0xff000000) == 0 && (pixel & 0x00ffffff) != 0)
                dest[x] = pixel | 0xff000000;
            else
                dest[x] = pixel | mask;
        }
    }
    ReleaseDC(0, display_dc);
    return image;
}

//HBITMAP ImageResourceBase::PixmapToWinHBITMAP(const QImage &image, bool noAlpha)
//{
//    if (image.isNull())
//        return 0;

//    HBITMAP bitmap = 0;
//    const int w = image.width();
//    const int h = image.height();

//    HDC display_dc = GetDC(0);

//    // Define the header
//    BITMAPINFO bmi;
//    memset(&bmi, 0, sizeof(bmi));
//    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
//    bmi.bmiHeader.biWidth       = w;
//    bmi.bmiHeader.biHeight      = -h;
//    bmi.bmiHeader.biPlanes      = 1;
//    bmi.bmiHeader.biBitCount    = 32;
//    bmi.bmiHeader.biCompression = BI_RGB;
//    bmi.bmiHeader.biSizeImage   = w * h * 4;

//    // Create the pixmap
//    uchar *pixels = 0;
//    bitmap = CreateDIBSection(display_dc, &bmi, DIB_RGB_COLORS, (void **) &pixels, 0, 0);
//    ReleaseDC(0, display_dc);
//    if (!bitmap) {
//        qErrnoWarning("%s, failed to create dibsection", __FUNCTION__);
//        return 0;
//    }
//    if (!pixels) {
//        qErrnoWarning("%s, did not allocate pixel data", __FUNCTION__);
//        return 0;
//    }

//    // Copy over the data
//    QImage::Format imageFormat = QImage::Format_ARGB32;
//    if (noAlpha)
//        imageFormat = QImage::Format_RGB32;
//    else
//        imageFormat = QImage::Format_ARGB32_Premultiplied;

//    const QImage image2 = image.convertToFormat(imageFormat);
//    const int bytes_per_line = w * 4;
//    for (int y=0; y < h; ++y)
//        memcpy(pixels + y * bytes_per_line, image2.scanLine(y), bytes_per_line);

//    return bitmap;
//}

HBITMAP ImageResourceBase::GetScreenshotBmp(){

    HDC     hDC;
    HDC     MemDC;
    BYTE*   Data;
    HBITMAP   hBmp;
    BITMAPINFO   bi;

    memset(&bi,   0,   sizeof(bi));
    bi.bmiHeader.biSize   =   sizeof(BITMAPINFO);
    bi.bmiHeader.biWidth   =  GetSystemMetrics(SM_CXSCREEN);
    bi.bmiHeader.biHeight   = GetSystemMetrics(SM_CYSCREEN);
    bi.bmiHeader.biPlanes   =   1;
    bi.bmiHeader.biBitCount   =   24;

    hDC   =   GetDC(NULL);
    MemDC   =   CreateCompatibleDC(hDC);
    hBmp   =   CreateDIBSection(MemDC, &bi, DIB_RGB_COLORS, (void**)&Data, NULL, 0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC, 0, 0, bi.bmiHeader.biWidth, bi.bmiHeader.biHeight, hDC, 0, 0, SRCCOPY);
    ReleaseDC(NULL,   hDC);
    DeleteDC(MemDC);
    DeleteObject(Data);

    return   hBmp;
}

QImage ImageResourceBase::screenshot(){
    HBITMAP bitmap = GetScreenshotBmp();
    QImage image = WinHBITMAPToImage(bitmap);
    DeleteObject(bitmap);

    return image;
}


#endif
///////////////////////////////////////////////////


}
