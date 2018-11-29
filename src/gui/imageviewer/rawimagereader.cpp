#include "rawimagereader.h"

#include <QFile>
#include <QDebug>
#include <QElapsedTimer>
#include <QDateTime>
#include <QColor>


#ifndef MAX_IMAGE_SIZE
#define MAX_IMAGE_SIZE 10240*1024
#endif

#ifndef MAX_KERNEL_SIZE
#define MAX_KERNEL_SIZE 256
#endif



RawImageReader::RawImageReader(QObject *parent) : QObject(parent)
{

    m_imageWidth = 1024;
    m_imageHeight = 768;
    m_pixelSize = 2;

    m_errorString = "";

    m_rawImageDataArray = 0;

    m_pixelMinValue = 65535;
    m_pixelMaxValue = 0;

    m_windowWidth = 65535;
    m_windowLevel = 32767;

    m_convertToHU = false;
    m_rescaleSlope = 0.25;
    m_rescaleIntercept = -1024;

}

RawImageReader::~RawImageReader()
{

    if(m_rawImageDataArray){
        delete m_rawImageDataArray;
    }

}

QImage RawImageReader::loadRawGrayscaleImage(const QString &fileName, unsigned int imageWidth, unsigned int imageHeight, unsigned int headerSize, unsigned char pixelSize)
{
    m_errorString = "";

    if(m_rawImageDataArray){
        delete m_rawImageDataArray;
        m_rawImageDataArray = 0;
    }

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        m_errorString = file.errorString();
        return QImage();
    }

    m_pixelMinValue = 65535;
    m_pixelMaxValue = 0;

    m_imageWidth = imageWidth;
    m_imageHeight = imageHeight;
    m_pixelSize = pixelSize;
    Q_ASSERT(m_pixelSize == 2);
    unsigned int dimension = m_imageWidth * m_imageHeight;


    if(dimension * pixelSize + headerSize != file.size()){
        m_errorString = tr("The dimension is larger than the file size!");
        return QImage();
    }



    m_rawImageDataArray = new unsigned short[dimension];
    memset(m_rawImageDataArray, 0, sizeof(unsigned short) * dimension);
    QByteArray tempData;
    file.seek(headerSize);
//    for(int i=0; i<dimension; i++){
//        tempData.clear();
//        tempData.resize(0);
//        tempData = file.read(m_pixelSize);
//        if(tempData.isEmpty()){
//            m_errorString = file.errorString();
//            delete m_rawImageDataArray;
//            m_rawImageDataArray = 0;
//            return QImage();
//        }

//        //memcpy(&m_rawImageDataArray[i], tempData.data(), sizeof(unsigned int));
//        memcpy(&m_rawImageDataArray[i], tempData.data(), tempData.size());

//        if(m_pixelMinValue > m_rawImageDataArray[i]){
//            m_pixelMinValue = m_rawImageDataArray[i];
//        }
//        if(m_pixelMaxValue < m_rawImageDataArray[i]){
//            m_pixelMaxValue = m_rawImageDataArray[i];
//        }

//    }

    unsigned short temp = 0;
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    for(unsigned int i=0; i<dimension; i++){
        temp = 0;
        in >> temp;

        if(m_convertToHU){
            // Pixel to HU:  Hu=pixel_val*rescale_slope+rescale_intercept;
            float hu = temp * m_rescaleSlope + m_rescaleIntercept;
            temp = (unsigned short)(qBound((float)0, hu, (float)65535));
        }

        m_rawImageDataArray[i] = temp;
        if(m_pixelMinValue > temp){
            m_pixelMinValue = temp;
        }
        if(m_pixelMaxValue < temp){
            m_pixelMaxValue = temp;
        }
    }




//    QFile f("/home/hehui/01.raw");
//    f.open(QIODevice::WriteOnly);

//    QDataStream out(&f);
//    out.setByteOrder(QDataStream::LittleEndian);
//    for(int i=0; i<dimension; i++){
//        out << m_rawImageDataArray[i];
//    }
//    f.flush();
//    f.close();


//    {

//        QFile f("/home/hehui/test-2.raw");
//        f.open(QIODevice::WriteOnly);

//        QDataStream out(&f);
//        out.setByteOrder(QDataStream::LittleEndian);

//        int i =0,j=0;
//        for(i=0; i<50*imageWidth; i++){
//            out << 0;
//        }

//        i = 50*imageWidth;
//        for(j=i; j<dimension; j++){
//            out << m_rawImageDataArray[j-i];
//        }
//        f.flush();
//        f.close();

//    }


    qWarning()<<QString("Pixel MIN:%1, MAX:%2").arg(m_pixelMinValue).arg(m_pixelMaxValue);


    unsigned short windowWidth = m_pixelMaxValue - m_pixelMinValue;
    unsigned short windowLevel = (m_pixelMaxValue + m_pixelMinValue) / 2;
    return adjustWindow(windowWidth, windowLevel);

}

QImage RawImageReader::loadRawGrayscaleImage(unsigned short *data, unsigned int imageWidth, unsigned int imageHeight, unsigned short defaultWindowWidth, unsigned short defaultWindowLevel, unsigned char lowBitsWidth)
{
    unsigned char lbWidth = lowBitsWidth;
    if(0 == lbWidth || lbWidth > 16){
        lbWidth = 16;
    }

    if(m_rawImageDataArray){
        delete m_rawImageDataArray;
        m_rawImageDataArray = 0;
    }

    if(!data){return QImage();}

    m_pixelMinValue = 65535;
    m_pixelMaxValue = 0;

    m_imageWidth = imageWidth;
    m_imageHeight = imageHeight;
    unsigned int dimension = imageWidth * imageHeight;
    m_rawImageDataArray = new unsigned short[dimension];
    memset(m_rawImageDataArray, 0, sizeof(unsigned short) * dimension);


    unsigned short temp = 0;
    for(unsigned int i=0; i<dimension; i++){
        if(16 == lbWidth){
            temp = data[i];
        }else{
            temp = data[i] & ((1 << lbWidth) -1);
        }
        m_rawImageDataArray[i] = temp;
        if(m_pixelMinValue > temp){
            m_pixelMinValue = temp;
        }
        if(m_pixelMaxValue < temp){
            m_pixelMaxValue = temp;
        }
    }


//    {
//        QFile f("/home/hehui/test-2.raw");
//        f.open(QIODevice::WriteOnly);

//        QDataStream out(&f);
//        out.setByteOrder(QDataStream::LittleEndian);

//        int i =0,j=0;
//        for(i=0; i<50*imageWidth; i++){
//            out << 255;
//        }

//        for(j=i; j<dimension; j++){
//            out << m_rawImageDataArray[j-i];
//        }
//        f.flush();
//        f.close();
//    }


    qWarning()<<QString("Pixel MIN:%1, MAX:%2").arg(m_pixelMinValue).arg(m_pixelMaxValue);


    unsigned short windowWidth = m_pixelMaxValue - m_pixelMinValue;
    unsigned short windowLevel = (m_pixelMaxValue + m_pixelMinValue) / 2;
    if(defaultWindowWidth && defaultWindowLevel){
        windowWidth = defaultWindowWidth;
        windowLevel = defaultWindowLevel;
    }
    return adjustWindow(windowWidth, windowLevel);

}

QImage RawImageReader::adjustWindow(unsigned short windowWidth, unsigned short windowLevel)
{
    if(!m_rawImageDataArray){return QImage();}

    m_windowWidth = windowWidth;
    m_windowLevel = windowLevel;
    //m_windowWidth = qBound((unsigned short)1, m_windowLevel, (unsigned short)(m_pixelMaxValue - m_pixelMinValue) );
    m_windowLevel = qBound(m_pixelMinValue, m_windowLevel, m_pixelMaxValue);

//    QElapsedTimer timer;
//    timer.start();

    double wWidth = windowWidth;
    double wLevel = windowLevel;

    if(0 == wWidth){
        wWidth = m_pixelMaxValue - m_pixelMinValue;
    }
    if(0 == wLevel){
        wLevel = m_pixelMaxValue + m_pixelMinValue / 2;
    }


    ///////////
    /////    X:Orignal pixel value(0-65535). y:New pixel value(0-255).
    //// M1. y = (x - (wLevel - wWidth/2.0)) * 255.0/wWidth; //
    //// M2. y = ( (x - (wLevel-0.5)) / (wWidth-1) + 0.5 ) * 255.0;
    ///////////


    int min = (int)((2.0*wLevel - wWidth) / 2.0 + 0.5);
    int max = (int)((2.0*wLevel + wWidth) / 2.0 + 0.5);

    QImage img(m_imageWidth, m_imageHeight, QImage::Format_RGB32);
    int pixel = 0;
    int idx = 0;
    for(unsigned int i=0; i<m_imageHeight; i++){
        //uchar *line = img.scanLine(i);
        QRgb *rgb = (QRgb *)img.scanLine(i);
        for(unsigned int j=0; j<m_imageWidth; j++){

            if(m_rawImageDataArray[idx] >= max){
                pixel = 255;
            }else if(m_rawImageDataArray[idx] <= min){
                pixel = 0;
            }else{
                pixel = (int)( ( (float)(m_rawImageDataArray[idx] - (wLevel-0.5)) / (wWidth-1) + 0.5 ) * 255.0 );
                if(pixel > 255){
                    pixel = 255;
                }else if(pixel < 0){
                    pixel = 0;
                }
            }

            //line[j] = (unsigned char)pixel;
            rgb[j] = qRgb((unsigned char)pixel, (unsigned char)pixel , (unsigned char)pixel);

            idx++;
        }
    }


//    unsigned int dimension = m_imageWidth * m_imageHeight;
//    QFile f("/home/hehui/3.raw");
//    f.open(QIODevice::WriteOnly);
//    QDataStream out(&f);
//    out.setByteOrder(QDataStream::LittleEndian);
//    const uchar* bits = img.constBits();
//    for(int i=0; i<dimension; i++){
//        out << bits[i];
//    }
//    f.flush();
//    f.close();


//    qCritical()<<"elapsed:"<<timer.elapsed();




//    {
//        QFile f("c:/bg.raw");
//        f.open(QIODevice::WriteOnly);
//        QDataStream out(&f);
//        out.setByteOrder(QDataStream::LittleEndian);
//        for(unsigned int i=0; i<m_imageHeight; i++){
//            QRgb *rgb = (QRgb *)img.scanLine(i);
//            for(unsigned int j=0; j<m_imageWidth; j++){
//                out << (unsigned short)qGreen(rgb[j]);
//            }
//        }

//        f.flush();
//        f.close();
//    }


//    img.save(QString("/home/hehui/tmp/00_%1.jpg").arg(QDateTime::currentDateTime().toString("hhmmsszzz")));


    emit signalWindowAdjusted(img);
    return img;
}

bool RawImageReader::averageBlur(const QImage &origin, QImage *dstImage, int kernelWidth, int kernelHeight)
{
    if(origin.isNull()) {
        return false;
    }

    if(origin.width() * origin.height() > MAX_IMAGE_SIZE){
        qCritical()<<QString("Image size must not exceed %1 Bytes! ").arg(MAX_IMAGE_SIZE);
        return false;
    }

    if(!(kernelWidth % 2) || kernelWidth < 3) {
        return false;
    }
    if(!(kernelHeight % 2) || kernelHeight < 3) {
        return false;
    }

    if(!dstImage) {
        dstImage = new QImage(origin);
    }

    int kernelSize = kernelWidth * kernelHeight;
    if(kernelSize >= origin.width() * origin.height()) {
        qCritical()<<QString("Kernel size must not exceed image size! ");
        return false;
    }
    if(kernelSize > MAX_KERNEL_SIZE) {
        qCritical()<<QString("Kernel size must not exceed %1! ").arg(MAX_KERNEL_SIZE);
        return false;
    }

    int kernelWidthMedian = kernelWidth / 2;
    int kernelMedian = kernelHeight / 2;

    QColor color;
    int r, g, b;

    for(int x = kernelWidthMedian; x < origin.width() - kernelWidthMedian; x++) {

        for(int y = kernelMedian; y < origin.height() - kernelMedian; y++) {
            //QRgb *rgb = (QRgb *)image.scanLine(y);
            //    rgb[x] = qRgb(values[qRed(rgb[x])], values[qGreen(rgb[x])], values[qBlue(rgb[x])]);
            //image.setPixel(x, y , qRgb(r,g,b));

            r = 0;
            g = 0;
            b = 0;

            for(int i = -kernelWidthMedian; i <= kernelWidthMedian; i++) {
                for(int j = -kernelMedian; j <= kernelMedian; j++) {
                    color = QColor(origin.pixel(x + i, y + j));
                    r += color.red();
                    g += color.green();
                    b += color.blue();
                }
            }

            r = qBound(0, r / kernelSize, 255);
            g = qBound(0, g / kernelSize, 255);
            b = qBound(0, b / kernelSize, 255);

            dstImage->setPixel(x, y, qRgb(r, g, b));

        }
    }

    return true;
}

void RawImageReader::getWindowParameters(unsigned short *windowWidth, unsigned short *windowLevel, unsigned short *pixelMinValue, unsigned short *pixelMaxValue)
{
    if(windowWidth){
        *windowWidth = m_windowWidth;
    }

    if(windowLevel){
        *windowLevel = m_windowLevel;
    }

    if(pixelMinValue){
        *pixelMinValue = m_pixelMinValue;
    }

    if(pixelMaxValue){
        *pixelMaxValue = m_pixelMaxValue;
    }
}


//QImage RawImageReader::adjustWindow2(unsigned short windowWidth, unsigned short windowLevel)
//{

//    QElapsedTimer timer;
//    timer.start();


//    unsigned int dimension = m_imageWidth * m_imageHeight;

//    double wWidth = windowWidth;
//    double wLevel = windowLevel;

//    if(0 == wWidth || (0 == wLevel)){
//        wWidth = m_pixelMaxValue - m_pixelMinValue;
//        wLevel = m_pixelMaxValue + m_pixelMinValue / 2;
//    }


//    ///////////
//    /////    X:Orignal pixel value(0-65535). y:New pixel value(0-255).
//    //// M1. y = (x - (wLevel - wWidth/2.0)) * 255.0/wWidth; //
//    //// M2. y = ( (x - (wLevel-0.5)) / (wWidth-1) + 0.5 ) * 255.0;
//    ///////////


//    unsigned char *tempArray = new unsigned char[dimension];
//    memset(tempArray, 0, sizeof(unsigned char)*dimension);

//    int min = (int)((2.0*wLevel - wWidth) / 2.0 + 0.5);
//    int max = (int)((2.0*wLevel + wWidth) / 2.0 + 0.5);

//    int pixel = 0;
//    for(int i=0; i<dimension; i++){
//        if(m_rawImageDataArray[i] >= max){
//            pixel = 255;
//        }else if(m_rawImageDataArray[i] <= min){
//            pixel = 0;
//        }else{
//            pixel = (int)( ( (float)(m_rawImageDataArray[i] - (wLevel-0.5)) / (wWidth-1) + 0.5 ) * 255.0 );
//            if(pixel > 255){
//                pixel = 255;
//            }else if(pixel < 0){
//                pixel = 0;
//            }
//        }

//        tempArray[i] = (unsigned char)pixel;

//    }

////    QFile f("/home/hehui/1.raw");
////    f.open(QIODevice::WriteOnly);
////    QDataStream out(&f);
////    out.setByteOrder(QDataStream::LittleEndian);
////    for(int i=0; i<dimension; i++){
////        out << tempArray[i];
////    }
////    f.flush();
////    f.close();

//    QImage img =  QImage(tempArray, m_imageWidth, m_imageHeight, QImage::Format_Grayscale8);
//    qCritical()<<"elapsed:"<<timer.elapsed();

//    delete tempArray;

//    return img;
//}

//QImage RawImageReader::adjustWindow3(unsigned short windowWidth, unsigned short windowLevel)
//{
//    unsigned int dimension = m_imageWidth * m_imageHeight;

//    double wWidth = windowWidth;
//    double wLevel = windowLevel;

//    if(0 == wWidth || (0 == wLevel)){
//        unsigned short pixelMax = 0;
//        for(int i=0; i<dimension; i++){
//            if(m_rawImageDataArray[i] > pixelMax){
//                pixelMax = m_rawImageDataArray[i];
//            }
//        }

//        wWidth = pixelMax;
//        wLevel = pixelMax/2;
//    }

////    wWidth = 2622;
////    wLevel = 820;

//    int min = (int)((2.0*wLevel - wWidth) / 2.0 + 0.5);
//    int max = (int)((2.0*wLevel + wWidth) / 2.0 + 0.5);

//    unsigned char tempArray[dimension];
//    memset(tempArray, 0, sizeof(unsigned char) * dimension);

//    double factor = 255.0 / (double)(max - min);
//    int pixel = 0;
//    for(int i=0; i<dimension; i++){
//        if(m_rawImageDataArray[i] > max){
//            pixel = 255;
//        }else if(m_rawImageDataArray[i] < min){
//            pixel = 0;
//        }else{
//            pixel = (int)(((double)(m_rawImageDataArray[i] - min)) * factor);
//            if(pixel > 255){
//                pixel = 255;
//            }else if(pixel < 0){
//                pixel = 0;
//            }
//        }

//        tempArray[i] = (unsigned char)pixel;

//    }

//    QFile f("/home/hehui/1.raw");
//    f.open(QIODevice::WriteOnly);
//    QDataStream out(&f);
//    for(int i=0; i<dimension; i++){
//        out << tempArray[i];
//    }
//    f.flush();
//    f.close();


//    return QImage(tempArray, m_imageWidth, m_imageHeight, QImage::Format_Grayscale8);
//}
