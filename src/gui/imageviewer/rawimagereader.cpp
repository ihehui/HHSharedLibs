#include "rawimagereader.h"

#include <QFile>
#include <QDebug>
#include <QElapsedTimer>


RawImageReader::RawImageReader(QObject *parent) : QObject(parent)
{

    m_imageWidth = 1024;
    m_imageHeight = 768;
    m_pixelSize = 2;

    m_errorString = "";

    m_rawImageDataArray = 0;

    m_pixelMinValue = 65535;
    m_pixelMaxValue = 0;

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
    for(int i=0; i<dimension; i++){
        temp = 0;
        in >> temp;

        m_rawImageDataArray[i] = temp;
        if(m_pixelMinValue > temp){
            m_pixelMinValue = temp;
        }
        if(m_pixelMaxValue < temp){
            m_pixelMaxValue = temp;
        }
    }




    QFile f("/home/hehui/01.raw");
    f.open(QIODevice::WriteOnly);

    QDataStream out(&f);
    out.setByteOrder(QDataStream::LittleEndian);
    for(int i=0; i<dimension; i++){
        out << m_rawImageDataArray[i];
    }
    f.flush();
    f.close();


    unsigned short windowWidth = m_pixelMaxValue - m_pixelMinValue;
    unsigned short windowLevel = (m_pixelMaxValue + m_pixelMinValue) / 2;
    return adjustWindow(windowWidth, windowLevel);

}

QImage RawImageReader::adjustWindow(unsigned short windowWidth, unsigned short windowLevel)
{

//    QElapsedTimer timer;
//    timer.start();

    double wWidth = windowWidth;
    double wLevel = windowLevel;

    if(0 == wWidth || (0 == wLevel)){
        wWidth = m_pixelMaxValue - m_pixelMinValue;
        wLevel = m_pixelMaxValue + m_pixelMinValue / 2;
    }


    ///////////
    /////    X:Orignal pixel value(0-65535). y:New pixel value(0-255).
    //// M1. y = (x - (wLevel - wWidth/2.0)) * 255.0/wWidth; //
    //// M2. y = ( (x - (wLevel-0.5)) / (wWidth-1) + 0.5 ) * 255.0;
    ///////////


    int min = (int)((2.0*wLevel - wWidth) / 2.0 + 0.5);
    int max = (int)((2.0*wLevel + wWidth) / 2.0 + 0.5);

    QImage img(m_imageWidth, m_imageHeight, QImage::Format_Grayscale8);
    int pixel = 0;
    int idx = 0;
    for(int i=0; i<m_imageHeight; i++){
        uchar *line = img.scanLine(i);
        for(int j=0; j<m_imageWidth; j++){

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

            line[j] = (unsigned char)pixel;

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
    return img;
}

QImage RawImageReader::adjustWindow2(unsigned short windowWidth, unsigned short windowLevel)
{

    QElapsedTimer timer;
    timer.start();


    unsigned int dimension = m_imageWidth * m_imageHeight;

    double wWidth = windowWidth;
    double wLevel = windowLevel;

    if(0 == wWidth || (0 == wLevel)){
        wWidth = m_pixelMaxValue - m_pixelMinValue;
        wLevel = m_pixelMaxValue + m_pixelMinValue / 2;
    }


    ///////////
    /////    X:Orignal pixel value(0-65535). y:New pixel value(0-255).
    //// M1. y = (x - (wLevel - wWidth/2.0)) * 255.0/wWidth; //
    //// M2. y = ( (x - (wLevel-0.5)) / (wWidth-1) + 0.5 ) * 255.0;
    ///////////


    unsigned char tempArray[dimension];
    memset(tempArray, 0, sizeof(unsigned char)*dimension);

    int min = (int)((2.0*wLevel - wWidth) / 2.0 + 0.5);
    int max = (int)((2.0*wLevel + wWidth) / 2.0 + 0.5);

    int pixel = 0;
    for(int i=0; i<dimension; i++){
        if(m_rawImageDataArray[i] >= max){
            pixel = 255;
        }else if(m_rawImageDataArray[i] <= min){
            pixel = 0;
        }else{
            pixel = (int)( ( (float)(m_rawImageDataArray[i] - (wLevel-0.5)) / (wWidth-1) + 0.5 ) * 255.0 );
            if(pixel > 255){
                pixel = 255;
            }else if(pixel < 0){
                pixel = 0;
            }
        }

        tempArray[i] = (unsigned char)pixel;

    }

//    QFile f("/home/hehui/1.raw");
//    f.open(QIODevice::WriteOnly);
//    QDataStream out(&f);
//    out.setByteOrder(QDataStream::LittleEndian);
//    for(int i=0; i<dimension; i++){
//        out << tempArray[i];
//    }
//    f.flush();
//    f.close();

    QImage img =  QImage(tempArray, m_imageWidth, m_imageHeight, QImage::Format_Grayscale8);
    qCritical()<<"elapsed:"<<timer.elapsed();
    return img;
}

QImage RawImageReader::adjustWindow3(unsigned short windowWidth, unsigned short windowLevel)
{
    unsigned int dimension = m_imageWidth * m_imageHeight;

    double wWidth = windowWidth;
    double wLevel = windowLevel;

    if(0 == wWidth || (0 == wLevel)){
        unsigned short pixelMax = 0;
        for(int i=0; i<dimension; i++){
            if(m_rawImageDataArray[i] > pixelMax){
                pixelMax = m_rawImageDataArray[i];
            }
        }

        wWidth = pixelMax;
        wLevel = pixelMax/2;
    }

//    wWidth = 2622;
//    wLevel = 820;

    int min = (int)((2.0*wLevel - wWidth) / 2.0 + 0.5);
    int max = (int)((2.0*wLevel + wWidth) / 2.0 + 0.5);

    unsigned char tempArray[dimension];
    memset(tempArray, 0, sizeof(unsigned char) * dimension);

    double factor = 255.0 / (double)(max - min);
    int pixel = 0;
    for(int i=0; i<dimension; i++){
        if(m_rawImageDataArray[i] > max){
            pixel = 255;
        }else if(m_rawImageDataArray[i] < min){
            pixel = 0;
        }else{
            pixel = (int)(((double)(m_rawImageDataArray[i] - min)) * factor);
            if(pixel > 255){
                pixel = 255;
            }else if(pixel < 0){
                pixel = 0;
            }
        }

        tempArray[i] = (unsigned char)pixel;

    }

    QFile f("/home/hehui/1.raw");
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    for(int i=0; i<dimension; i++){
        out << tempArray[i];
    }
    f.flush();
    f.close();


    return QImage(tempArray, m_imageWidth, m_imageHeight, QImage::Format_Grayscale8);
}
