#include "screencaptureprocessor.h"

#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QFile>



int ImageBlock::m_pixelSize = 3;
int ImageBlock::m_imageWidth = 0;
int ImageBlock::m_imageHeight = 0;
ImageBlock::ImageBlock(const QRect &blockRect, QObject *parent)
    :QObject(parent), m_blockRect(blockRect)
{


}

ImageBlock::~ImageBlock()
{

}

void ImageBlock::setImageInfo(int pixelSize, int width, int height)
{
    m_pixelSize = pixelSize;
    m_imageWidth = width;
    m_imageHeight = height;
}


bool ImageBlock::compare(const QByteArray *previousData, const QByteArray *newData)
{
    int blockDataSize = m_blockRect.width() * m_blockRect.height() * m_pixelSize;
    if(m_blockData.size() != blockDataSize){
        m_blockData.resize(blockDataSize);
    }
    m_blockData.fill(0);
    bool changed = false;


    if(!previousData || (!newData)){
        qCritical()<<"Invalid parameter!";
        return false;
    }

    if(previousData->size() != newData->size()){
        qCritical()<<"new data size does not match the previous data size!";
        return false;
    }

    if(newData->size() != m_pixelSize * m_imageWidth * m_imageHeight){
        qCritical()<<"new data size does not match the parameters!";
        return false;
    }


    int offset = 0;
    int blockDataIndex = 0;
    for(int i=0; i<m_blockRect.height(); i++){
        offset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

        for(int j=0; j<m_blockRect.width() * m_pixelSize; j++){
            m_blockData[blockDataIndex] = newData->at(offset);

            //if((uchar)previousData->at(offset) != (uchar)newData->at(offset)){
            if(qAbs((uchar)previousData->at(offset) - (uchar)newData->at(offset)) >1){

                //m_blockData[blockDataIndex] = (uchar)newData->at(offset);
                changed = true;

                //qWarning()<<""<<i<<":"<<j<<" "<< (short)previousData->at(offset) <<" "<< (short)newData->at(offset);

            }
            offset++;
            blockDataIndex++;
        }
    }

//    if(changed){
//        QImage imgx((uchar*)m_blockData.data(), m_blockRect.width(), m_blockRect.height(), QImage::Format_RGB888);
//        imgx.save(QString("%1x%2.bmp").arg(m_blockRect.topLeft().x()).arg(m_blockRect.topLeft().y()));
//    }

    return changed;
}

QRect ImageBlock::blockRect()
{
    return m_blockRect;
}

QByteArray ImageBlock::blockData()
{
    return m_blockData;
}


///////////////////////////
ScreenCaptureProcessor::ScreenCaptureProcessor(QObject *parent) : QObject(parent)
{

    m_capture.init();

    m_pixelSize = 0;
    m_imageWidth = 0;
    m_imageHeight = 0;
    m_capture.getScreenRectInfo(&m_pixelSize, &m_imageWidth, &m_imageHeight);
    ImageBlock::setImageInfo(m_pixelSize, m_imageWidth, m_imageHeight);


    m_timeInterval = 2000;
    m_blockRows = 10;
    m_blockColumns = 10;
    m_linesInterval = 10;
    m_lineStartOffset = 0;

    m_timer.setInterval(m_timeInterval);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));


    int blockWidth = m_imageWidth / m_blockColumns;
    int blockHeight = m_imageHeight / m_blockRows;
    QRect rect;
    rect.setTopLeft(QPoint(0, 0));
    rect.setWidth(blockWidth);
    rect.setHeight(blockHeight);
    for(int i=0; i<m_blockRows; i++){
        for(int j=0; j<m_blockColumns; j++){
            rect.moveTopLeft(QPoint(j*blockWidth, i*blockHeight));
            ImageBlock *blk = new ImageBlock(rect, this);
            m_imageBlocks.append(blk);
            //qWarning()<<""<<i<<"-"<<j<<": "<<rect;
        }
    }

    qWarning()<<m_imageBlocks.first()->blockRect().topLeft()<<" "<<m_imageBlocks.last()->blockRect().bottomRight();


    QTimer::singleShot(3000, &m_timer, SLOT(start()));
//    m_timer.start();
}

void ScreenCaptureProcessor::getScreenRectInfo(int *pixelSize, int *width, int *height, int *blockRows, int *blockColumns)
{

    if(pixelSize){
        *pixelSize = m_pixelSize;
    }

    if(width){
        *width = m_imageWidth;
    }

    if(height){
        *height = m_imageHeight;
    }

    if(blockRows){
        *blockRows = m_blockRows;
    }

    if(blockColumns){
        *blockColumns = m_blockColumns;
    }

}

bool ScreenCaptureProcessor::getScreenCapture(QList<int> *blockIndex, QList<QByteArray> *blockData)
{
    if(!blockIndex || (!blockData)){return false;}

    m_newBitmapData.clear();
    m_capture.capture(&m_newBitmapData);

    if(m_previousBitmapData.isEmpty()){
        m_previousBitmapData = m_newBitmapData;
    }

    for(int i=0; i<m_imageBlocks.size(); i++){
        ImageBlock *blk = m_imageBlocks.at(i);
        if(blk->compare(&m_previousBitmapData, &m_newBitmapData)){
            blockIndex->append(i);
            blockData->append(blk->blockData());
        }
    }

    m_previousBitmapData = m_newBitmapData;

    return true;
}

void ScreenCaptureProcessor::timerTimeout()
{
    m_newBitmapData.clear();
    m_capture.capture(&m_newBitmapData);

    if(m_previousBitmapData.isEmpty()){
        m_previousBitmapData = m_newBitmapData;
        m_newBitmapData.clear();
        return;
    }


    QImage img5((uchar*)m_previousBitmapData.data(), m_imageWidth, m_imageHeight, QImage::Format_RGB888);
    img5.save("000.bmp");
    QImage img6((uchar*)m_newBitmapData.data(), m_imageWidth, m_imageHeight, QImage::Format_RGB888);
    img6.save("001.bmp");



    QImage img7 = img6.copy();
    QPainter painter(&img7);
    painter.setPen(Qt::red);

    bool changed = false;
    for(int i=0; i<m_imageBlocks.size(); i++){
        ImageBlock *blk = m_imageBlocks.at(i);
        if(blk->compare(&m_previousBitmapData, &m_newBitmapData)){
            painter.drawRect(blk->blockRect());

            qWarning()<<" "<<i<<" changed";
            changed = true;
        }
    }

    if(changed){
        img7.save("002.bmp");
        m_timer.stop();
        exit(0);
    }


    m_previousBitmapData = m_newBitmapData;

}
