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

void ImageResourceBase::setBrightnessAndContrast(QImage &image, int brightness, int contrast){
    ////
    /// contrast:0~300
    /// brightness:-250~250
    ////


    if(image.isNull()){return;}

    quint8 values[256];
    for(int i=0; i<256; i++){
        int value = (contrast*0.01) * i + brightness;
        values[i] = qBound(0, value, 255);
    }

    for(int y=0; y<image.height(); y++){
        QRgb *rgb = (QRgb *)image.scanLine(y);
        for(int x=0; x<image.width(); x++){
            rgb[x] = qRgb(values[qRed(rgb[x])], values[qGreen(rgb[x])], values[qBlue(rgb[x])]);
            //image.setPixel(x, y , qRgb(r,g,b));
        }
    }

}

void ImageResourceBase::averageBlur(QImage &origin, QImage *dstImage, int kernelWidth, int kernelHeight){
    if(origin.isNull()){return;}

    if(!(kernelWidth%2) || kernelWidth < 3){return;}
    if(!(kernelHeight%2) || kernelHeight < 3){return;}

    if(!dstImage){
        dstImage = new QImage(origin);
    }

    int kernelSize = kernelWidth * kernelHeight;
    if(kernelSize >= origin.width() * origin.height()){return;}

    int kernelWidthMedian = kernelWidth/2;
    int kernelMedian = kernelHeight/2;

    QColor color;
    int r,g,b;

    for(int x=kernelWidthMedian; x<origin.width()-kernelWidthMedian; x++){

        for(int y=kernelMedian; y<origin.height()-kernelMedian; y++){
            //QRgb *rgb = (QRgb *)image.scanLine(y);
            //    rgb[x] = qRgb(values[qRed(rgb[x])], values[qGreen(rgb[x])], values[qBlue(rgb[x])]);
            //image.setPixel(x, y , qRgb(r,g,b));

            r = 0;
            g = 0;
            b = 0;

            for(int i = -kernelWidthMedian; i<= kernelWidthMedian; i++){
                for(int j = -kernelMedian; j<= kernelMedian; j++){
                    color = QColor(origin.pixel(x+i, y+j));
                    r += color.red();
                    g += color.green();
                    b += color.blue();
                }
            }

            r = qBound(0, r/kernelSize, 255);
            g = qBound(0, g/kernelSize, 255);
            b = qBound(0, b/kernelSize, 255);

            dstImage->setPixel(x,y, qRgb(r,g,b));

        }
    }

}

int ImageResourceBase::median(int array[], int arraySize){
    int i, j, temp;
    for(int i=0; i<arraySize-1; i++){
        for(int j=0; j<arraySize-1-i; j++){
            if(array[j] > array[j+1]){
                temp = array[j];
                array[j] = array[j+1];
                array[j+1] = temp;
            }
        }
    }

    if((arraySize & 1) > 0){
        temp = array[(arraySize+1)/2];
    }else{
        temp = (array[arraySize/2] + array[arraySize/2+1])/2;
    }

    return temp;
}

void ImageResourceBase::medianBlur(QImage &origin, QImage *newImage, int kernelWidth, int kernelHeight){
    if(origin.isNull()){return;}

    if(!(kernelWidth%2) || kernelWidth < 3){return;}
    if(!(kernelHeight%2) || kernelHeight < 3){return;}

    if(!newImage){
        newImage = new QImage(origin);
    }

    int kernelSize = kernelWidth * kernelHeight;
    if(kernelSize >= origin.width() * origin.height()){return;}
    int pointsMedian = kernelSize/2 + 1;

    int kernelWidthMedian = kernelWidth/2;
    int kernelHeightMedian = kernelHeight/2;

    QColor color;
    int r,g,b;

    for(int x=kernelWidthMedian; x<origin.width()-kernelWidthMedian; x++){

        for(int y=kernelHeightMedian; y<origin.height()-kernelHeightMedian; y++){
            //QRgb *rgb = (QRgb *)image.scanLine(y);
            //    rgb[x] = qRgb(values[qRed(rgb[x])], values[qGreen(rgb[x])], values[qBlue(rgb[x])]);
            //image.setPixel(x, y , qRgb(r,g,b));

            r = 0;
            g = 0;
            b = 0;

            int arrayRed[kernelSize], arrayGreen[kernelSize], arrayBlue[kernelSize];
            int index = 0;
            for(int i = -kernelWidthMedian; i<= kernelWidthMedian; i++){
                for(int j = -kernelHeightMedian; j<= kernelHeightMedian; j++){
                    color = QColor(origin.pixel(x+i, y+j));
                    arrayRed[index] = color.red();
                    arrayGreen[index] = color.green();
                    arrayBlue[index] = color.blue();
                    index++;
                }
            }

            r = median(arrayRed, kernelSize);
            g = median(arrayGreen, kernelSize);
            b = median(arrayBlue, kernelSize);

            newImage->setPixel(x,y, qRgb(r,g,b));

        }
    }

}

void ImageResourceBase::guassianBlur(QImage &origin, QImage *newImage, float sigma){
    ////////
    //// g(x,y) = ( 1 / (2*pi*σ^2) ) * exp( -(x^2+y^2)/(2*σ^2) )
    //// g(x,y):weight of pixel(x,y)
    //// x,y:Pixel Point;
    //// σ:radius,0.0~250
    ////
    //// kernel: diamet = 6σ+1 = 2*radius+1; σ=radius/3
    ////////

    if(origin.isNull()){return;}

    int diamet = (int)(6 * sigma + 1);
    if(!(diamet%2) || diamet < 3){return;}


    float SQRT_2PI = 2.506628274631f;
    float sigmaMul2PI = 1.0f / (SQRT_2PI * sigma * sigma);
    float divSigmaPow2 = 1.0f / (2.0f * sigma * sigma);
    float sum = 0.0;

    int kernelSize = diamet*diamet;
    float kernel[kernelSize];

    for(int i=0; i<diamet; i++){
        for(int j=0; j<diamet; j++){
            float weight = sigmaMul2PI * exp(-(i*i + j*j) * divSigmaPow2);
            kernel[i*diamet+j] = weight;
            sum += weight;
        }
    }
    for(int i=0; i<kernelSize; i++){
            kernel[i] /= sum;
    }


    if(!newImage){
        newImage = new QImage(origin);
    }

    int kernelMedian = diamet/2;
    QColor color;
    int r,g,b;

    for(int x=kernelMedian; x<origin.width()-kernelMedian; x++){

        for(int y=kernelMedian; y<origin.height()-kernelMedian; y++){
            r = 0;
            g = 0;
            b = 0;

            int index = 0;
            for(int i = -kernelMedian; i<= kernelMedian; i++){
                for(int j = -kernelMedian; j<= kernelMedian; j++){
                    color = QColor(origin.pixel(x+i, y+j));
                    float weight =  kernel[index++];
                    if(weight<0.000001){continue;}
                    r += (int)(color.red() * weight);
                    g += (int)(color.green() * weight);
                    b += (int)(color.blue() * weight);
                }
            }

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            newImage->setPixel(x,y, qRgb(r,g,b));

        }
    }

}

void ImageResourceBase::weightedFilter(const QImage &origin, QImage *newImage, const int kernel[], int kernelWidth, int divisor){
/////  **Blur** :
///    int kernel2D [5][5]= {{0,0,1,0,0},
///                         {0,1,3,1,0},
///                         {1,3,7,3,1},
///                         {0,1,3,1,0},
///                         {0,0,1,0,0}};
///    int kernel[25];
///    for(int i=0; i<5; i++){
///        for(int j=0; j<5; j++){
///            kernel[i*5+j] = (kernel2D[i][j]);
///        }
///    }
///    weightedFilter(orignalImage, &theImage, kernel, 5, 27);
///
///     **Sharpen** :
///     int sharpen[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
///     int sharpen[9] = {-1, -1, -1, -1, 9, -1, -1, -1, -1};
///     int sharpen[9] = {1, -2, 1, -2, 5, -2, 1, -2, 1};
///     int sharpen[9] = {-1, -2, -1, -2, 19, -2, -1, -2, -1}; divisor=7;
///
/////


    if(origin.isNull()){return;}

    int kernelSize = kernelWidth*kernelWidth;
    if(!(kernelSize%2) || kernelSize < 3){return;}
    if(!divisor){return;}

    if(!newImage){
        newImage = new QImage(origin);
    }

    int r,g,b;
    QColor color;
    int kernelMedian = kernelWidth/2;

    for(int x=kernelMedian; x<newImage->width()-(kernelMedian); x++){
        for(int y=kernelMedian; y<newImage->height()-(kernelMedian); y++){

            r = 0;
            g = 0;
            b = 0;


            int index = 0;
            for(int i = -kernelMedian; i<= kernelMedian; i++){
                for(int j = -kernelMedian; j<= kernelMedian; j++){
                    color = QColor(origin.pixel(x+i, y+j));
                    //int weight = kernel[(kernelMedian+i)*kernelWidth + (kernelMedian+j)];
                    int weight = kernel[index++];

                    //qDebug()<<"--"<<(kernelMedian+i)*kernelWidth + (kernelMedian+j)<<":"<<weight;
                    if(!weight){continue;}
                    r += color.red()*weight;
                    g += color.green()*weight;
                    b += color.blue()*weight;
                }
            }

            if(divisor == 1){
                r = qBound(0, r, 255);
                g = qBound(0, g, 255);
                b = qBound(0, b, 255);
            }else{
                r = qBound(0, r/divisor, 255);
                g = qBound(0, g/divisor, 255);
                b = qBound(0, b/divisor, 255);
            }


            newImage->setPixel(x,y, qRgb(r,g,b));

        }
    }

}






}
