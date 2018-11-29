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
    void signalWindowAdjusted(const QImage &image);

public slots:

    QImage loadRawGrayscaleImage(const QString &fileName, unsigned int imageWidth, unsigned int imageHeight, unsigned int headerSize, unsigned char pixelSize = 2);
    QImage loadRawGrayscaleImage(unsigned short *data, unsigned int imageWidth, unsigned int imageHeight, unsigned short defaultWindowWidth = 0, unsigned short defaultWindowLevel = 0, unsigned char lowBitsWidth = 16);

    QImage adjustWindow(unsigned short windowWidth, unsigned short windowLevel);
//    QImage adjustWindow2(unsigned short windowWidth, unsigned short windowLevel);
//    QImage adjustWindow3(unsigned short windowWidth, unsigned short windowLevel);

    bool averageBlur(const QImage &origin, QImage *dstImage, int kernelWidth = 3, int kernelHeight = 3);


    unsigned short * rawImageDataArray(){return m_rawImageDataArray;}

    void getWindowParameters(unsigned short *windowWidth, unsigned short *windowLevel, unsigned short *pixelMinValue = 0, unsigned short *pixelMaxValue = 0);

private:

    unsigned int m_imageWidth;
    unsigned int m_imageHeight;
    unsigned char m_pixelSize; //in bytes

    QString m_errorString;

//    QByteArray m_rawImageData;

    unsigned short *m_rawImageDataArray;

    unsigned short m_pixelMinValue;
    unsigned short m_pixelMaxValue;

    unsigned short m_windowWidth;
    unsigned short m_windowLevel;


    bool m_convertToHU;
    float m_rescaleSlope;
    float m_rescaleIntercept;





};

#endif // RAWIMAGEREADER_H
