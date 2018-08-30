#include "screencaptureprocessor.h"

#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QFile>
#include <QTime>
#include <QApplication>

#include <iostream>
#include <fstream>
using namespace std;


QString numberListToString(const QList<int> *array, const QString &separator = ","){
    QStringList list;
    for(int i=0; i<array->size(); i++){
        list.append(QString::number(array->at(i)));
    }
    return list.join(separator);
}


uint ImageBlock::m_pixelSize = 3;
uint ImageBlock::m_imageWidth = 0;
uint ImageBlock::m_imageHeight = 0;
#ifdef Q_OS_WIN
bool ImageBlock::m_bitmapFormat = true;
#else
bool ImageBlock::m_bitmapFormat = false;
#endif
uint ImageBlock::m_scanLinesInterval = 1;


ImageBlock::ImageBlock(const QRect &blockRect, int index, QObject *parent)
    :QObject(parent), m_blockRect(blockRect), m_blockIndex(index)
{
    m_blockDataSize = m_blockRect.width() * m_blockRect.height() * m_pixelSize;

    m_blockData = 0;

    m_blockData = new uchar[m_blockDataSize];
    memset(m_blockData, 0, m_blockDataSize);

    m_siblingRight = 0;
    m_siblingBelow = 0;


    m_scanLineIndex = index % m_blockRect.height();


}

ImageBlock::~ImageBlock()
{
    QMutexLocker locker(&m_mutex);

    delete [] m_blockData;
}

void ImageBlock::setImageInfo(uint pixelSize, uint width, uint height)
{
    m_pixelSize = pixelSize;
    m_imageWidth = width;
    m_imageHeight = height;
}

void ImageBlock::setBitmapFormat(bool bitmapFormat)
{
    m_bitmapFormat = bitmapFormat;
}

bool ImageBlock::isBitmapFormat()
{
    return m_bitmapFormat;
}

void ImageBlock::setScanLinesInterval(uint lines)
{
    m_scanLinesInterval = lines;
    if(m_imageHeight <= lines){
        m_scanLinesInterval = m_imageHeight / 2;
    }
}


bool ImageBlock::fastCompare(const uchar *previousFullImageData, const uchar *newFullImageData, int size, QList<int> *changedBlockIndexList, QList<ImageBlock*> *workList, int startX, int startY)
{

    Q_ASSERT(workList);
    workList->removeAll(this);

    Q_ASSERT(changedBlockIndexList);
    if(changedBlockIndexList->contains(m_blockIndex)){
        return true;
    }


    {
        QMutexLocker locker(&m_mutex);
        assert(size >= m_blockDataSize);
        if(size < m_blockDataSize){
            return false;
            delete [] m_blockData;
            m_blockData = new uchar[m_blockDataSize];
        }
        //memset(m_blockData, 0, blockDataSize);
    }


    bool changed = false;


    if(!previousFullImageData || (!newFullImageData)){
        qCritical()<<"Invalid parameter!";
        return false;
    }

    //    if(newData->size() != m_pixelSize * m_imageWidth * m_imageHeight){
    //        qCritical()<<"new data size does not match the parameters!";
    //        return false;
    //    }



    int newDataOffset = 0;
    int blockDataOffset = 0;


    if(startX >= 0 && startX < m_blockRect.width()){
        newDataOffset = (m_blockRect.topLeft().y() * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize + startX;

        for(int i=0; i<3; i++){
            newDataOffset += i;
            if(*(previousFullImageData+newDataOffset) != *(newFullImageData+newDataOffset)){
                changed = true;
                changedBlockIndexList->append(m_blockIndex);

                if(m_siblingBelow && (!changedBlockIndexList->contains(m_siblingBelow->blockIndex()))){
                    m_siblingBelow->fastCompare(previousFullImageData, newFullImageData, size, changedBlockIndexList, workList, startX, -1);
                }

                break;
            }
        }

    }


    if(!changed){
        if(startY >= 0 && startY < m_blockRect.height()){
            newDataOffset = ((m_blockRect.topLeft().y() + startY) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

            for(int i=0; i<3; i++){
                newDataOffset += i;
                if(*(previousFullImageData+newDataOffset) != *(newFullImageData+newDataOffset)){
                    changed = true;
                    changedBlockIndexList->append(m_blockIndex);

                    if(m_siblingRight && (!changedBlockIndexList->contains(m_siblingRight->blockIndex()))){
                        m_siblingRight->fastCompare(previousFullImageData, newFullImageData, size, changedBlockIndexList, workList, -1, startY);
                    }
                    break;
                }
            }
        }

    }


    int x = 0;
    int y = 0;
    if(!changed){

        newDataOffset = 0;
        blockDataOffset = 0;
        QList<int> sequenceList;
        for(int i=m_scanLineIndex; i<m_blockRect.height(); i+=m_scanLinesInterval+1){
            sequenceList.append(i);
        }
        for(int i=m_scanLineIndex-m_scanLinesInterval-1; i>=0; i-=(m_scanLinesInterval+1)){
            sequenceList.append(i);
        }
        //qWarning()<<"-----------m_scanLineIndex:"<<m_scanLineIndex<<"  "<<m_scanLinesInterval;
        //qWarning()<<sequenceList;

        int lineNumber=0;
        for(int i=0; i<sequenceList.size(); i++){
            lineNumber = sequenceList.at(i);

            newDataOffset = ((m_blockRect.topLeft().y() + lineNumber) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

            for(int j=0; j<m_blockRect.width() * m_pixelSize; j++){

                if(*(previousFullImageData+newDataOffset) != *(newFullImageData+newDataOffset)){
                    changed = true;
                    changedBlockIndexList->append(m_blockIndex);

                    x = j/m_pixelSize;
                    y = lineNumber;

                    if(m_siblingBelow && (!changedBlockIndexList->contains(m_siblingBelow->blockIndex()))){
                        m_siblingBelow->fastCompare(previousFullImageData, newFullImageData, size, changedBlockIndexList, workList, j/m_pixelSize, -1);
                    }
                    if(m_siblingRight && (!changedBlockIndexList->contains(m_siblingRight->blockIndex()))){
                        m_siblingRight->fastCompare(previousFullImageData, newFullImageData, size, changedBlockIndexList, workList, -1, lineNumber);
                    }

                    break;
                }


                newDataOffset++;
                blockDataOffset++;
            }

            if(changed){break;}

        }

    }


    {
        QMutexLocker locker(&m_mutex);

        if(changed){
            newDataOffset = 0;
            blockDataOffset = 0;
            int lineSize = m_blockRect.width() * m_pixelSize;
            for(int i=0; i<m_blockRect.height(); i++){
                newDataOffset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

                memcpy(m_blockData+blockDataOffset, newFullImageData + newDataOffset, lineSize);
                blockDataOffset += lineSize;

            }

        }
    }

    m_scanLineIndex++;
    if(m_scanLineIndex >= m_blockRect.height()){
        m_scanLineIndex = 0;
    }


    return changed;
}

bool ImageBlock::compare1(const uchar *previousData, const uchar *newData, int size)
{
    QMutexLocker locker(&m_mutex);

    assert(size >= m_blockDataSize);
    if(size < m_blockDataSize){
        return false;
        delete [] m_blockData;
        m_blockData = new uchar[m_blockDataSize];
    }
    //memset(m_blockData, 0, blockDataSize);

    bool changed = false;


    if(!previousData || (!newData)){
        qCritical()<<"Invalid parameter!";
        return false;
    }

    //    if(newData->size() != m_pixelSize * m_imageWidth * m_imageHeight){
    //        qCritical()<<"new data size does not match the parameters!";
    //        return false;
    //    }


    int offset = 0;
    int blockDataIndex = 0;
    for(int i=0; i<m_blockRect.height(); i++){
        offset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

        for(int j=0; j<m_blockRect.width() * m_pixelSize; j++){
            m_blockData[blockDataIndex] = newData[offset];

            if(previousData[offset] != newData[offset]){
                //if(qAbs((uchar)previousData->at(offset) - (uchar)newData->at(offset)) >1){

//                m_blockData[blockDataIndex] = newData[offset];
//                if((char)0 == newData[offset]){
//                    m_blockData[blockDataIndex] = (char)1;
//                }

                changed = true;

                //qWarning()<<""<<i<<":"<<j<<" "<< (short)previousData->at(offset) <<" "<< (short)newData->at(offset);

            }
            //else{
            //    m_blockData[blockDataIndex] = (char)0;
            //}
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

bool ImageBlock::compare2(const uchar *previousData, const uchar *newData, int size)
{
    QMutexLocker locker(&m_mutex);

    assert(size >= m_blockDataSize);
    if(size < m_blockDataSize){
        return false;
        delete [] m_blockData;
        m_blockData = new uchar[m_blockDataSize];
    }
    //memset(m_blockData, 0, blockDataSize);

    bool changed = false;


    if(!previousData || (!newData)){
        qCritical()<<"Invalid parameter!";
        return false;
    }

    //    if(newData->size() != m_pixelSize * m_imageWidth * m_imageHeight){
    //        qCritical()<<"new data size does not match the parameters!";
    //        return false;
    //    }


    int offset = 0;
    int blockDataIndex = 0;
    for(int i=0; i<m_blockRect.height(); i++){
        offset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

        for(int j=0; j<m_blockRect.width() * m_pixelSize; j++){
            m_blockData[blockDataIndex] = newData[offset];

            if(previousData[offset] != newData[offset]){
                //if(qAbs((uchar)previousData->at(offset) - (uchar)newData->at(offset)) >1){

//                m_blockData[blockDataIndex] = newData[offset];
//                if((char)0 == newData[offset]){
//                    m_blockData[blockDataIndex] = (char)1;
//                }

                changed = true;
                break;

                //qWarning()<<""<<i<<":"<<j<<" "<< (short)previousData->at(offset) <<" "<< (short)newData->at(offset);

            }

            offset++;
            blockDataIndex++;
        }
    }


    if(changed){

        int newDataOffset = 0;
        int blockDataOffset = 0;
        int lineSize = m_blockRect.width() * m_pixelSize;
        for(int i=0; i<m_blockRect.height(); i++){
            newDataOffset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

            memcpy(m_blockData+blockDataOffset, newData + newDataOffset, lineSize);
            blockDataOffset += lineSize;

        }

    }



    //    if(changed){
    //        QImage imgx((uchar*)m_blockData.data(), m_blockRect.width(), m_blockRect.height(), QImage::Format_RGB888);
    //        imgx.save(QString("%1x%2.bmp").arg(m_blockRect.topLeft().x()).arg(m_blockRect.topLeft().y()));
    //    }

    return changed;
}

bool ImageBlock::compare(const uchar *previousFullImageData, const uchar *newFullImageData, int size)
{
    QMutexLocker locker(&m_mutex);

    assert(size >= m_blockDataSize);
    if(size < m_blockDataSize){
        return false;
        delete [] m_blockData;
        m_blockData = new uchar[m_blockDataSize];
    }
    //memset(m_blockData, 0, blockDataSize);

    bool changed = false;


    if(!previousFullImageData || (!newFullImageData)){
        qCritical()<<"Invalid parameter!";
        return false;
    }

    //    if(newData->size() != m_pixelSize * m_imageWidth * m_imageHeight){
    //        qCritical()<<"new data size does not match the parameters!";
    //        return false;
    //    }



    int newDataOffset = 0;
    int blockDataOffset = 0;
    for(int i=0; i<m_blockRect.height(); i++){
        newDataOffset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

        for(int j=0; j<m_blockRect.width() * m_pixelSize; j++){

            if(*(previousFullImageData+newDataOffset) != *(newFullImageData+newDataOffset)){
                changed = true;
                break;
            }

            newDataOffset++;
            blockDataOffset++;
        }
    }


    if(changed){

        newDataOffset = 0;
        blockDataOffset = 0;
        int lineSize = m_blockRect.width() * m_pixelSize;
        for(int i=0; i<m_blockRect.height(); i++){
            newDataOffset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

            memcpy(m_blockData+blockDataOffset, newFullImageData + newDataOffset, lineSize);
            blockDataOffset += lineSize;

        }

    }



    //    if(changed){
    //        QImage imgx((uchar*)m_blockData.data(), m_blockRect.width(), m_blockRect.height(), QImage::Format_RGB888);
    //        imgx.save(QString("%1x%2.bmp").arg(m_blockRect.topLeft().x()).arg(m_blockRect.topLeft().y()));
    //    }

    return changed;
}

bool ImageBlock::assembleBlockData(uchar *previousData, int size)
{
    if(!previousData){
        qCritical()<<"Invalid parameter!";
        return false;
    }

    if(size < m_blockDataSize){
        qCritical()<<"Invalid block data size!";
        return false;
    }

    QMutexLocker locker(&m_mutex);

    uchar* ptr= m_blockData;
//    if(m_bitmapFormat){
//        ////BGR to RGB
//        uchar temp = 0;
//        int offset = 0;
//        for(int i=0; i<m_blockRect.width()*m_blockRect.height(); i++){
//            temp = ptr[offset+2];

//            ptr[offset+2] = ptr[offset];
//            ptr[offset] = temp;

//            offset += 3;
//        }

//    }

//    int offset = 0;
//    int blockDataIndex = 0;
//    for(int i=0; i<m_blockRect.height(); i++){
//        offset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

//        for(int j=0; j<m_blockRect.width() * m_pixelSize; j++){
//            //if(m_blockData[blockDataIndex] != (char)0){
//                previousData[offset] = m_blockData[blockDataIndex];
//            //}
//            offset++;
//            blockDataIndex++;
//        }
//    }


    {

        int newDataOffset = 0;
        int blockDataOffset = 0;
        int lineSize = m_blockRect.width() * m_pixelSize;
        for(int i=0; i<m_blockRect.height(); i++){
            newDataOffset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

            memcpy(previousData + newDataOffset, m_blockData+blockDataOffset, lineSize);
            blockDataOffset += lineSize;

        }

    }


    return true;
}

bool ImageBlock::assembleBlockData1(uchar *previousData, int size)
{
    if(!previousData){
        qCritical()<<"Invalid parameter!";
        return false;
    }

    if(size < m_blockDataSize){
        qCritical()<<"Invalid block data size!";
        return false;
    }

    QMutexLocker locker(&m_mutex);

    uchar* ptr= m_blockData;
//    if(m_bitmapFormat){
//        ////BGR to RGB
//        uchar temp = 0;
//        int offset = 0;
//        for(int i=0; i<m_blockRect.width()*m_blockRect.height(); i++){
//            temp = ptr[offset+2];

//            ptr[offset+2] = ptr[offset];
//            ptr[offset] = temp;

//            offset += 3;
//        }

//    }

    int offset = 0;
    int blockDataIndex = 0;
    for(int i=0; i<m_blockRect.height(); i++){
        offset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

        for(int j=0; j<m_blockRect.width() * m_pixelSize; j++){
            //if(m_blockData[blockDataIndex] != (char)0){
                previousData[offset] = m_blockData[blockDataIndex];
            //}
            offset++;
            blockDataIndex++;
        }
    }




    return true;
}

QRect ImageBlock::blockRect()
{
    return m_blockRect;
}

QByteArray ImageBlock::blockData()
{
    QMutexLocker locker(&m_mutex);

    QByteArray ba;
    ba.resize(m_blockDataSize);
    memcpy(ba.data(), (const char*)m_blockData, m_blockDataSize);
    return ba;

    //return QByteArray((const char*)m_blockData, m_blockDataSize);
}

bool ImageBlock::asignBlockData(const uchar *newFullImageData, int size)
{

    QMutexLocker locker(&m_mutex);

    assert(size >= m_blockDataSize);
    if(size < m_blockDataSize){
        return false;
        //delete [] m_blockData;
        //m_blockData = new uchar[m_blockDataSize];
    }
    //memset(m_blockData, 0, blockDataSize);

    int newDataOffset = 0;
    int blockDataOffset = 0;

    int lineSize = m_blockRect.width() * m_pixelSize;
    for(int i=0; i<m_blockRect.height(); i++){
        newDataOffset = ((m_blockRect.topLeft().y() + i) * m_imageWidth + m_blockRect.topLeft().x()) * m_pixelSize;

        memcpy(m_blockData+blockDataOffset, newFullImageData + newDataOffset, lineSize);
        blockDataOffset += lineSize;
    }

    return true;
}

const int ImageBlock::blockIndex()
{
    return m_blockIndex;
}

bool ImageBlock::setBlockData(const QByteArray &blockData)
{
    if(blockData.size() != m_blockDataSize){
        return false;
    }

    QMutexLocker locker(&m_mutex);
    memcpy(m_blockData, (uchar*)blockData.data(), m_blockDataSize);

    return true;
}

void ImageBlock::resetBlockData()
{
    memset(m_blockData, 0, m_blockDataSize);
}

void ImageBlock::setSiblings(ImageBlock *right, ImageBlock *below)
{
    m_siblingRight = right;
    m_siblingBelow = below;

    //qWarning()<<"------------"<<m_blockIndex<<" :"<<(m_siblingRight?m_siblingRight->blockIndex():0)<<" "<<(m_siblingBelow?m_siblingBelow->blockIndex():0);

}

///////////////////////////
ScreenCaptureProcessor::ScreenCaptureProcessor(bool captureMode, QObject *parent)
    : QThread(parent), m_captureMode(captureMode)
{

    bi = {0};
    bf = {0};

    m_previousBitmapData = 0;
    m_newBitmapData = 0;
    m_dataSize = 0;

    m_pixelSize = 0;
    m_imageWidth = 0;
    m_imageHeight = 0;

    m_capture = 0;
    m_initialized = false;
    m_blockRows = 0;
    m_blockColumns = 0;

    if(captureMode){
        m_capture = new ScreenCapture(this);
        m_capture->init();
        m_capture->getScreenRectInfo(&m_pixelSize, &m_imageWidth, &m_imageHeight);
        ImageBlock::setImageInfo(m_pixelSize, m_imageWidth, m_imageHeight);

        m_dataSize = m_imageWidth * m_imageHeight * m_pixelSize;
        m_previousBitmapData = new uchar[m_dataSize];
        m_newBitmapData = new uchar[m_dataSize];
        memset(m_previousBitmapData, 1, m_dataSize);
        memset(m_newBitmapData, 1, m_dataSize);


//        bi.biSize = sizeof(BITMAPINFOHEADER);
//        bi.biWidth = m_imageWidth;
//        bi.biHeight = m_imageHeight;
//        bi.biPlanes = 1;
//        bi.biBitCount = m_pixelSize * 3;
//        bi.biCompression = BI_RGB;

//        bf.bfType = 0x4d42;
//        bf.bfSize = m_dataSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
//        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


        //m_previousBitmapData.resize(m_imageWidth * m_imageHeight * m_pixelSize);
        //m_previousBitmapData.fill(1);



        setBlockInfo(10, 10);

    }

    m_timeInterval = 250;
    //m_linesInterval = 10;
    m_lineStartOffset = 0;


    m_aboutToQuit = false;
    m_threadFinished = true;
    m_bitmapFormat = ImageBlock::isBitmapFormat();

    m_updateFixedBitmapData = false;

    m_keyframeID = 1;
    m_keyFrameInterval = m_orignalKeyFrameInterval = 5000;

    m_lastKeyframeTimestamp = 0;

    m_keyFrameSent = false;
    m_moreThanHalfChangesCount = 0;


}

ScreenCaptureProcessor::~ScreenCaptureProcessor()
{
    setAboutToQuit(true);
    quit();

    while (!threadFinished()) {
        qApp->processEvents();
    }

    if(m_captureMode){
        m_capture->deInitilize();
        delete m_capture;
        m_capture = 0;
    }

    foreach (ImageBlock *blk, m_imageBlocks) {
        delete blk;
        blk = 0;
    }
    m_imageBlocks.clear();

    m_changedBlockIndexList.clear();
    delete [] m_previousBitmapData;
    delete [] m_newBitmapData;

}

void ScreenCaptureProcessor::getScreenRectInfo(int *pixelSize, int *width, int *height, uint *blockRows, uint *blockColumns, bool *bitmapFormat)
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

    if(bitmapFormat){
        *bitmapFormat = m_bitmapFormat;
    }

}

void ScreenCaptureProcessor::setScreenRectInfo(int pixelSize, int width, int height, uint blockRows, uint blockColumns, bool bitmapFormat)
{
    m_pixelSize = pixelSize;
    m_imageWidth = width;
    m_imageHeight = height;
    m_blockRows = blockRows;
    m_blockColumns = blockColumns;
    ImageBlock::setImageInfo(m_pixelSize, m_imageWidth, m_imageHeight);
    setBitmapFormat(bitmapFormat);

    setBlockInfo(blockRows, blockColumns);

    delete [] m_previousBitmapData;
    delete [] m_newBitmapData;
    m_dataSize = m_imageWidth * m_imageHeight * m_pixelSize;
    m_previousBitmapData = new uchar[m_dataSize];
    m_newBitmapData = new uchar[m_dataSize];
    memset(m_previousBitmapData, 1, m_dataSize);
    memset(m_newBitmapData, 1, m_dataSize);


    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = m_imageWidth;
    bi.biHeight = m_imageHeight;
    bi.biPlanes = 1;
    bi.biBitCount = m_pixelSize * 8;
    bi.biCompression = BI_RGB;

    bf.bfType = 0x4d42;
    bf.bfSize = m_dataSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

}

void ScreenCaptureProcessor::updateBlockData(QList<int> *blockIndexList, QList<QByteArray> *blockDataList)
{

    if(!m_initialized || m_captureMode){return;}
    if(m_imageWidth < 1 || m_imageHeight < 1 || m_pixelSize < 1){return;}

    if(blockIndexList->size() != blockDataList->size() || (blockIndexList->size() > m_imageBlocks.size())){return;}

    QMutexLocker locker(&m_mutex);

    m_changedBlockIndexList.clear();

    for(int i=0; i<blockIndexList->size(); i++){
        int idx = blockIndexList->at(i);
        if(idx >= m_imageBlocks.size() ){
            qCritical()<<"Error! Invalid index!";
            return;
        }
        ImageBlock *blk = m_imageBlocks.at(idx);
        if(!blk){return;}

        blk->setBlockData(blockDataList->at(i));
    }

    m_changedBlockIndexList = *blockIndexList;

    cond.wakeAll();
}

bool ScreenCaptureProcessor::getScreenCapture(QList<int> *blockIndex, QList<QByteArray> *blockData, bool onlyChanged)
{
    if(!m_captureMode){return false;}

    if(!blockIndex || (!blockData)){return false;}
    blockIndex->clear();

    QMutexLocker locker(&m_mutex);

    if(!onlyChanged){
        m_changedBlockIndexList.clear();
        for(int i=0; i<m_imageBlocks.size(); i++){
            m_changedBlockIndexList.append(i);
        }
    }
    *blockIndex = m_changedBlockIndexList;
    m_changedBlockIndexList.clear();


    for(int i=0; i<blockIndex->size(); i++){
        int idx = blockIndex->at(i);
        if(idx >= m_imageBlocks.size() ){
            qCritical()<<"Error! Invalid index!";
            return false;
        }
        ImageBlock *blk = m_imageBlocks.at(idx);
        if(!blk){return false;}

        //if(blk->compare(&m_previousBitmapData, &m_newBitmapData)){
        blockData->append(blk->blockData());
        //}
    }

    //    foreach (int idx, changedBlockIndexList) {
    //        ImageBlock *blk = m_imageBlocks.at(idx);
    //        //if(blk->compare(&m_previousBitmapData, &m_newBitmapData)){
    //            blockData->append(blk->blockData());
    //        //}
    //    }

    if(!onlyChanged){
        m_updateFixedBitmapData = true;
    }


    return true;
}

void ScreenCaptureProcessor::setAboutToQuit(bool quit)
{
    QMutexLocker locker(&m_mutex);
    m_aboutToQuit = quit;

    cond.wakeAll();
}

bool ScreenCaptureProcessor::isAboutToQuit()
{
    QMutexLocker locker(&m_mutex);
    return m_aboutToQuit;
}

bool ScreenCaptureProcessor::threadFinished()
{
    QMutexLocker locker(&m_mutex);
    return m_threadFinished;
}

void ScreenCaptureProcessor::setBitmapFormat(bool bitmap)
{
    QMutexLocker locker(&m_mutex);
    m_bitmapFormat = bitmap;
    ImageBlock::setBitmapFormat(bitmap);
}

void ScreenCaptureProcessor::setTimerInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    m_timeInterval = interval;
}

void ScreenCaptureProcessor::setBlockInfo(uint blockRows, uint blockColumns)
{
    QMutexLocker locker(&m_mutex);

    foreach (ImageBlock *blk, m_imageBlocks) {
        delete blk;
        blk = 0;
    }
    m_imageBlocks.clear();
    m_changedBlockIndexList.clear();

    int maxValue = 100;

    m_blockRows = qMax((uint)1, blockRows);
    m_blockColumns = qMax((uint)1, blockColumns);
    while (m_imageWidth % m_blockColumns) {
        m_blockColumns++;
        if(m_blockColumns > maxValue){
            m_blockColumns = 1;
            break;
        }
    }
    while (m_imageHeight % m_blockRows) {
        m_blockRows++;
        if(m_blockRows > maxValue){
            m_blockRows = 1;
            break;
        }
    }
    qDebug()<<"Screen Height:"<<m_imageHeight<<" Width:"<<m_imageWidth;
    qDebug()<<"Block Rows:"<<m_blockRows<<" Columns:"<<m_blockColumns;

    int blockWidth = m_imageWidth / m_blockColumns;
    int blockHeight = m_imageHeight / m_blockRows;




    QRect rect;
    rect.setTopLeft(QPoint(0, 0));
    rect.setWidth(blockWidth);
    rect.setHeight(blockHeight);

    for(uint i=0; i<m_blockRows; i++){
        for(uint j=0; j<m_blockColumns; j++){
            rect.moveTopLeft(QPoint(j*blockWidth, i*blockHeight));
            ImageBlock *blk = new ImageBlock(rect, m_imageBlocks.size(), this);
            m_imageBlocks.append(blk);
        }
    }

    ImageBlock *siblingRight = 0;
    ImageBlock *siblingBelow = 0;
    for(uint i=0; i<m_blockRows; i++){
        for(uint j=0; j<m_blockColumns; j++){
            ImageBlock *blk = m_imageBlocks.at(i*m_blockColumns + j);
            if(j<m_blockColumns-1){
                siblingRight = m_imageBlocks.at(i*m_blockColumns + j + 1);
            }else{
                siblingRight = 0;
            }
            if(i<m_blockRows-1){
                siblingBelow = m_imageBlocks.at((i+1)*m_blockColumns + j);
            }else{
               siblingBelow = 0;
            }

            blk->setSiblings(siblingRight, siblingBelow);

            qDebug()<<"-----"<<i*m_blockColumns + j<<":"<<(i*m_blockColumns + j + 1)<<" "<<((i+1)*m_blockColumns + j);
        }
    }

    m_initialized = true;


    qWarning()<<m_imageBlocks.first()->blockRect().topLeft()<<" "<<m_imageBlocks.last()->blockRect().bottomRight();

    emit signalScreenRectInfoChanged(m_pixelSize, m_imageWidth, m_imageHeight, m_blockRows, m_blockColumns);
}

const QByteArray ScreenCaptureProcessor::fullFixedBitmapData()
{
    QMutexLocker locker(&m_mutex);
    //return QByteArray((const char*)m_previousBitmapData, m_dataSize);

    QByteArray ba;
    ba.resize(m_dataSize);
    memcpy(ba.data(), (const char*)m_previousBitmapData, m_dataSize);
    return ba;

}

//const uchar* ScreenCaptureProcessor::fullFixedBitmapData()
//{
//    QMutexLocker locker(&m_mutex);
//    return m_newBitmapData;
//}

bool ScreenCaptureProcessor::setFullFixedBitmapData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    if(!m_initialized || (data.size() != m_dataSize)){
        return false;
    }

    memcpy(m_newBitmapData, (uchar*)data.data(), m_dataSize);

    m_changedBlockIndexList.clear();
    for(int i=0; i<m_imageBlocks.size(); i++){
        ImageBlock *blk = m_imageBlocks.at(i);
        //blk->compare(m_previousBitmapData, m_newBitmapData, m_dataSize);
        blk->asignBlockData(m_newBitmapData, m_dataSize);
        //changedBlockIndexList.append(i);
    }



    {

//        QList<ImageBlock*> workList = m_imageBlocks;
//        while (!workList.isEmpty()) {
//            ImageBlock *blk = workList.first();
//            if(!blk){return false;}

//            if(m_changedBlockIndexList.contains(blk->blockIndex())){
//                qDebug()<<""<<blk->blockIndex()<<" ignored";
//                continue;
//            }
//            qDebug()<<""<<blk->blockIndex()<<" called";
//            blk->fastCompare(m_previousBitmapData, m_newBitmapData, m_dataSize, &m_changedBlockIndexList, &workList, 0, 0);
//            //changedBlockIndexList.append(i);
//        }


    }




    memcpy(m_previousBitmapData, m_newBitmapData, m_dataSize);

    generateImage();


//    foreach (ImageBlock *blk, m_imageBlocks) {
//        if(!blk){return false;}
//        blk->resetBlockData();
//        blk->compare(m_previousBitmapData, m_previousBitmapData, m_dataSize);
//    }


//    if(m_bitmapFormat){
//        uchar* ptr= m_previousBitmapData;

//        ////BGR to RGB
//        uchar temp = 0;
//        int offset = 0;
//        for(int i=0; i<m_imageWidth*m_imageHeight; i++){
//            temp = ptr[offset+2];

//            ptr[offset+2] = ptr[offset];
//            ptr[offset] = temp;

//            offset += 3;
//        }
//    }

    if(!m_captureMode){
//        changedBlockIndexList.clear();
//        for(int i=0; i<m_imageBlocks.size(); i++){
//            changedBlockIndexList.append(i);
//        }

        cond.wakeAll();
    }

    return true;
}

quint16 ScreenCaptureProcessor::keyframeID()
{
    QMutexLocker locker(&m_mutex);
    return m_keyframeID;
}

void ScreenCaptureProcessor::setKeyFrameSent(bool sent)
{
    QMutexLocker locker(&m_keyFrameMutex);
    m_keyFrameSent = sent;

    qDebug()<<"----m_keyFrameSent:"<<m_keyFrameSent;
}

bool ScreenCaptureProcessor::isKeyFrameSent()
{
    QMutexLocker locker(&m_keyFrameMutex);
    return m_keyFrameSent;
}


void ScreenCaptureProcessor::enqueue(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_queue.enqueue(data);
}

QByteArray ScreenCaptureProcessor::dequeue()
{
    QMutexLocker locker(&m_mutex);

    if(m_queue.isEmpty()){
        return QByteArray();
    }

    return m_queue.dequeue();
}


void ScreenCaptureProcessor::updateKeyFrame()
{
    if(!m_captureMode){
        return;
    }

    setKeyFrameSent(false);

    //QMutexLocker locker(&m_mutex);

    m_lastKeyframeTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_keyframeID++;
    m_keyframeID %= 0xFFFF;
    if(0 == m_keyframeID){
        m_keyframeID++;
    }


//    QImage screenImage = QImage("./test/0.png");
//    memcpy(m_previousBitmapData, screenImage.bits(), m_dataSize);


    memcpy(m_previousBitmapData, m_newBitmapData, m_dataSize);


    m_changedBlockIndexList.clear();
    for(int i=0; i<m_imageBlocks.size(); i++){
        ImageBlock *blk = m_imageBlocks.at(i);
        //blk->compare(m_previousBitmapData, m_newBitmapData, m_dataSize);
        blk->asignBlockData(m_newBitmapData, m_dataSize);
        m_changedBlockIndexList.append(i);
    }


//    changedBlockIndexList.clear();
//    for(int i=0; i<m_imageBlocks.size(); i++){
//        changedBlockIndexList.append(i);
//    }

    m_moreThanHalfChangesCount = 0;
    m_keyFrameInterval = m_orignalKeyFrameInterval;


    cond.wakeAll();
}

void ScreenCaptureProcessor::generateImage()
{
    if(m_bitmapFormat){
        generateImageFromBitmapData();
    }else{
//        QImage image(m_previousBitmapData, m_imageWidth, m_imageHeight, QImage::Format_RGB32);
//        image = image.copy();
//        static quint64 idx = 0;
//        idx++;
//        image.save(QString("%1-orig.png").arg(idx));

        QImage newImage(m_newBitmapData, m_imageWidth, m_imageHeight, QImage::Format_RGB32);
        newImage = newImage.copy();


//        {
//            qDebug()<<"----------:"<<m_changedBlockIndexList.size();//<<"  "<<m_changedBlockIndexList;
//            QPainter painter(&newImage);
//            painter.setPen(Qt::red);
//            for(int i=0; i<m_changedBlockIndexList.size(); i++){
//                ImageBlock *blk = m_imageBlocks.at(m_changedBlockIndexList.at(i));
//                QRect rect = blk->blockRect();
//                painter.drawRect(rect);
//                painter.drawText(rect, QString::number(i));
//            }
//        }
//        newImage.save(QString("%1.png").arg(idx));




        if(!newImage.isNull()){
            emit signalScreenDataChanged(newImage);
        }
    }
}

void ScreenCaptureProcessor::generateImageFromBitmapData()
{

    {

        //            if(m_bitmapFormat){
        //                ////BGR to RGB
        //                uchar temp = 0;
        //                int offset = 0;
        //                for(int i=0; i<m_imageWidth*m_imageHeight; i++){
        //                    temp = ptr[offset+2];

        //                    ptr[offset+2] = ptr[offset];
        //                    ptr[offset] = temp;

        //                    offset += 3;
        //                }
        //            }

        //QImage image(ptr, m_imageWidth, m_imageHeight, QImage::Format_RGB888);


    }



    {

        QByteArray ba;
        ba.append((const char*)&bf, sizeof(BITMAPFILEHEADER));
        ba.append((const char*)&bi, sizeof(BITMAPINFOHEADER));
        ba.append((const char*)m_newBitmapData, m_dataSize);

        QImage image;
        image.loadFromData(ba, "bmp");
        //image.save(QString("test1-%1.png").arg(QTime::currentTime().toString("mm.ss.zzz")));

//                if(image.isNull()){
//                    ofstream fs;
//                    fs.open("test.bmp", ios::out|ios::binary);
//                    fs.write((const char*)&bf, sizeof(BITMAPFILEHEADER));
//                    fs.write((const char*)m_previousBitmapData, m_dataSize);
//                    fs.close();
//                }


        {
            qDebug()<<"----------:"<<m_changedBlockIndexList.size();//<<"  "<<m_changedBlockIndexList;
            QPainter painter(&image);
            painter.setPen(Qt::red);
            for(int i=0; i<m_changedBlockIndexList.size(); i++){
                ImageBlock *blk = m_imageBlocks.at(m_changedBlockIndexList.at(i));
                QRect rect = blk->blockRect();
                rect.moveTop(m_imageHeight-rect.height()-1-rect.top());
                painter.drawRect(rect);
                //painter.drawText(rect, QString::number(i));
            }
        }


        if(!image.isNull()){
            emit signalScreenDataChanged(image.copy());
        }

    }

}

void ScreenCaptureProcessor::run()
{

    {
        QMutexLocker locker(&m_mutex);
        m_threadFinished = false;

        if(!m_previousBitmapData){
            m_previousBitmapData = new uchar[m_dataSize];
            memset(m_previousBitmapData, 1, m_dataSize);
        }
        if(!m_newBitmapData){
            m_newBitmapData = new uchar[m_dataSize];
            memset(m_newBitmapData, 1, m_dataSize);
        }


        qWarning()<<"Thread started!";
    }



    if(m_captureMode){
        //        QTimer keyFrameTimer;
        //        keyFrameTimer.setInterval(m_keyFrameInterval);
        //        connect(&keyFrameTimer, SIGNAL(timeout()), this, SLOT(updateKeyFrame()));
        //        keyFrameTimer.start();


        m_lastKeyframeTimestamp = QDateTime::currentMSecsSinceEpoch();

        QElapsedTimer timer;
        qint64 elapsed = 0;
        timer.start();

        while (!isAboutToQuit()) {
            if(QDateTime::currentMSecsSinceEpoch() - m_lastKeyframeTimestamp > m_keyFrameInterval ){
                {
                    QMutexLocker locker(&m_mutex);
                    updateKeyFrame();
                }
                continue;
            }

//            if(m_moreThanHalfChangesCount >  (m_keyFrameInterval/m_timeInterval) ){
//                qDebug()<<"----------------------------------"<<m_moreThanHalfChangesCount;
//                updateKeyFrame();
//                continue;
//            }

            elapsed = timer.elapsed();
            if(elapsed < m_timeInterval){
                msleep(m_timeInterval - elapsed);
            }
            timer.restart();

            {
                bool changed = false;

                {
                    QMutexLocker locker(&m_mutex);


                    m_changedBlockIndexList.clear();
                    //m_newBitmapData.clear();
                    m_capture->capture();
                    const uchar *dataArray = m_capture->dataArray();
                    memcpy(m_newBitmapData, dataArray, m_dataSize);


                    if(0){
                        for(int i=0; i<m_imageBlocks.size(); i++){
                            ImageBlock *blk = m_imageBlocks.at(i);
                            if(blk->compare(m_previousBitmapData, m_newBitmapData, m_dataSize)){
                                m_changedBlockIndexList.append(i);
                                //qWarning()<<" "<<i<<" changed";
                                changed = true;
                            }
                        }

                        if(m_changedBlockIndexList.size() == m_imageBlocks.size()){
                            updateKeyFrame();
                            continue;
                        }
                    }



                    if(1){

                         //qWarning()<<"  ";
                        QList<int> changedBlockIndexList;

                        QList<ImageBlock*> workList = m_imageBlocks;
                        while (!workList.isEmpty()) {
                            ImageBlock *blk = workList.first();

                            if(changedBlockIndexList.contains(blk->blockIndex())){
                                continue;
                            }
                            blk->fastCompare(m_previousBitmapData, m_newBitmapData, m_dataSize, &changedBlockIndexList, &workList, 0, 0);
                        }
                        if(changedBlockIndexList.size() == m_imageBlocks.size()){
                            updateKeyFrame();
                            continue;
                        }


                        if( changedBlockIndexList.size() < (int)(m_imageBlocks.size() * 0.2) ){
                            m_keyFrameInterval += m_timeInterval;
                        }else{
                            m_keyFrameInterval = m_orignalKeyFrameInterval;
                        }

//                        if( changedBlockIndexList.size() > (int)(m_imageBlocks.size() * 0.5) ){
//                            m_moreThanHalfChangesCount++;
//                        }else{
//                            m_moreThanHalfChangesCount = 0;
//                        }

//                        if(isKeyFrameSent()){
                            m_changedBlockIndexList = changedBlockIndexList;
                            //qDebug()<<"------------changedBlockIndexList:"<<changedBlockIndexList.size();//<<" "<<changedBlockIndexList;

//                        }else{
//                            for(int i=0; i<m_imageBlocks.size(); i++){
//                                m_changedBlockIndexList.append(i);
//                            }
//                        }

                        if(!m_changedBlockIndexList.isEmpty()){
                            changed = true;
                        }

                    }


                }


                if(changed){
//                    generateImage();
                    //qWarning()<<"---3---"<<timer.elapsed();

                    emit signalScreenDataChanged();
                    //qWarning()<<"---4---"<<timer.elapsed();

                }
                //qWarning()<<"---5---"<<timer.elapsed();



            }



        }

    }else{

        while (!isAboutToQuit()) {
            {
                QMutexLocker locker(&m_mutex);
                if(m_changedBlockIndexList.isEmpty()){
                    cond.wait(&m_mutex);
                }

            }


            QMutexLocker locker(&m_mutex);
            memcpy(m_newBitmapData, m_previousBitmapData, m_dataSize);


            QElapsedTimer t2;
            t2.start();

            foreach (int idx, m_changedBlockIndexList) {
                ImageBlock *blk = m_imageBlocks.at(idx);
                if(!blk){return;}

                blk->assembleBlockData(m_newBitmapData, m_dataSize);
            }

            QList<int> list = m_changedBlockIndexList;
            emit signalScreenDataChanged(list);




            //            QByteArray data;
            //            data.resize(m_previousBitmapData.size());
            //            memcpy(data.data(), m_previousBitmapData.data(), m_previousBitmapData.size());
            //            qWarning()<<"--1--"<<t2.elapsed();
            //            t2.restart();
            //            uchar* ptr= (uchar*)data.data();

            uchar* ptr= m_previousBitmapData;


            //            if(m_bitmapFormat){
            //                ////BGR to RGB
            //                uchar temp = 0;
            //                int offset = 0;
            //                for(int i=0; i<m_imageWidth*m_imageHeight; i++){
            //                    temp = ptr[offset+2];

            //                    ptr[offset+2] = ptr[offset];
            //                    ptr[offset] = temp;

            //                    offset += 3;
            //                }
            //            }
            //qWarning()<<"--2--"<<t2.elapsed();
            //t2.restart();



            generateImage();


            m_changedBlockIndexList.clear();


            qWarning()<<"--4--"<<t2.elapsed();
            t2.restart();

        }

    }






    {
        QMutexLocker locker(&m_mutex);
        m_threadFinished = true;

        m_keyframeID = 1;

        m_changedBlockIndexList.clear();

        delete [] m_previousBitmapData;
        m_previousBitmapData = 0;
        delete [] m_newBitmapData;
        m_newBitmapData = 0;

        qWarning()<<"Thread finished!";
    }

}
