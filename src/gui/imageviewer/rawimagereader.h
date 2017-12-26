#ifndef RAWIMAGEREADER_H
#define RAWIMAGEREADER_H

#include <QObject>
#include <QImage>

class RawImageReader : public QObject
{
    Q_OBJECT
public:
    explicit RawImageReader(QObject *parent = nullptr);
    virtual ~RawImageReader();

signals:

public slots:

    QImage loadRawGrayscaleImage(const QString &fileName, unsigned int imageWidth, unsigned int imageHeight, unsigned int headerSize, unsigned char pixelSize = 2);

    QImage adjustWindow(unsigned short windowWidth, unsigned short windowLevel);
    QImage adjustWindow2(unsigned short windowWidth, unsigned short windowLevel);
    QImage adjustWindow3(unsigned short windowWidth, unsigned short windowLevel);


    unsigned short * rawImageDataArray(){return m_rawImageDataArray;}


private:

    unsigned int m_imageWidth;
    unsigned int m_imageHeight;
    unsigned char m_pixelSize; //in bytes

    QString m_errorString;

//    QByteArray m_rawImageData;

    unsigned short *m_rawImageDataArray;

    unsigned short m_pixelMinValue;
    unsigned short m_pixelMaxValue;




};

#endif // RAWIMAGEREADER_H
