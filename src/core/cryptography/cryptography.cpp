/*
 ****************************************************************************
 * cryptography.cpp
 *
 * Created on: 2010-6-1
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
 * Last Modified on: 2010-6-1
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#include "cryptography.h"
#include "tea/teacrypt.h"

#include <QFile>
#include <QDebug>


namespace HEHUI
{

Cryptography::Cryptography()
{
    maxBufferSize = 1024000; //1 MB;

    memset(keyUCharArray, 0, sizeof(keyUCharArray));

    destUCharArray = new unsigned char[maxBufferSize];
    memset(destUCharArray, 0, sizeof(unsigned char)*maxBufferSize);


}

Cryptography::~Cryptography()
{
    delete [] destUCharArray;
}

QByteArray Cryptography::MD5(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

QByteArray Cryptography::SHA1(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha1);
}

QByteArray Cryptography::getFileMD5(const QString &fileName, QString *errorString)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCritical("ERROR! Failed to open file!");
        if(errorString) {
            *errorString = file.errorString();
        }
        return QByteArray();
    }

    QCryptographicHash md5Hash(QCryptographicHash::Md5);
    while (!file.atEnd()) {
        QByteArray block = file.read(1024);
        if(block.isEmpty()) {
            qCritical("ERROR! Failed to read file!");
            if(errorString) {
                *errorString = file.errorString();
            }
            return QByteArray();
        }
        md5Hash.addData(block);
    }

    return md5Hash.result();
}

QString Cryptography::getFileMD5HexString(const QString &fileName, QString *errorString)
{
    return QString(getFileMD5(fileName, errorString).toHex());
}

/*
int Cryptography::stringToByte(unsigned char* destination, string &source) {
        int length = source.length() + 1;
        char temp[length];
        strcpy(temp, source.c_str());

        //	std::cout<<endl;
        //	std::cout<<"temp:"<<temp<<endl;

        for (int i = 0; i < length; i++) {
                if (temp[i] < 0) {
                	destination[i] = temp[i] + 256;
                } else {
                	destination[i] = temp[i];
                }
        }
        //	std::cout<<"out:"<<out<<endl;

        return length;

}
*/

void Cryptography::charToByte(unsigned char *destination, const char *source, int sourceLength)
{

    for (int i = 0; i < sourceLength; i++) {
        if (source[i] < 0) {
            destination[i] = source[i] + 256;
        } else {
            destination[i] = source[i];
        }
    }

}

void  Cryptography::byteToChar(char *destination, const unsigned char *source, int sourceLength)
{

    for (int i = 0; i < sourceLength; i++) {
        if (source[i] > 127) {
            destination[i] = source[i] - 256;
        } else {
            destination[i] = source[i];
        }
    }

}

int Cryptography::teaCrypto(QByteArray *destination, const QByteArray &source, const QByteArray &key, bool encrypt)
{

    QMutexLocker locker(&m_mutex);

    if(source.size() >= (maxBufferSize - 16)) {
        qCritical("Critical Error! Source data too long! Source data size:%d Bytes! Accepted size:%d Bytes!", source.size(), (maxBufferSize - 16));
        return 0;
    }



//	QByteArray md5Key = QCryptographicHash::hash(key, QCryptographicHash::Md5);
//        unsigned char * keyChar = new unsigned char[md5Key.size()+1];
//        charToByte(keyChar, md5Key.data(), md5Key.size());

    //Key for TEA is 16 Bytes
    QByteArray tempKeyArray = key;
    if(key.size() < CRYPTO_KEY_SIZE) {
        tempKeyArray.append("0123456789ABCDEF");
    }
    tempKeyArray.resize(CRYPTO_KEY_SIZE);

    memset(keyUCharArray, 0, sizeof(keyUCharArray));
//    unsigned char *keyUCharArray = new unsigned char[16];
    charToByte(keyUCharArray, tempKeyArray.data(), CRYPTO_KEY_SIZE);

//        qDebug(" ");
//        for(int i = 0; i<16; i++){
//            qDebug("keyChar[%d]:%d", i, keyUCharArray[i]);
//        }

    int sourceLength = source.size();
    unsigned char *sourceUCharArray = new unsigned char[sourceLength + 1];
    charToByte(sourceUCharArray, source.data(), sourceLength);

//        qDebug("----source.size():%d", sourceLength);
//        for(int i = 0; i<sourceLength; i++){
//            qDebug("sourceChar[%d]:%d", i, sourceUCharArray[i]);
//        }

    int destLength = maxBufferSize;
//    unsigned char *destUCharArray = new unsigned char[maxBufferSize];

    memset(destUCharArray, 0, sizeof(unsigned char)*maxBufferSize);


    if(encrypt) {
        TEACrypt::encrypt(sourceUCharArray, sourceLength, keyUCharArray, destUCharArray, &destLength);
    } else {
        bool ok = TEACrypt::decrypt(sourceUCharArray, sourceLength, keyUCharArray, destUCharArray, &destLength);
        if(!ok) {
            qCritical("Critical Error! Data Decryption Failed!");
            //delete [] destUCharArray;
            delete [] sourceUCharArray;
            //delete [] keyUCharArray;
            return 0;
        }
    }


//        qDebug("----destLength:%d", destLength);
//        for(int i = 0; i<destLength; i++){
//            qDebug("destChar[%d]:%d", i, destUCharArray[i]);
//        }

    //char *tempDestCharArray = new char[destLength];

    destination->clear();
    destination->resize(destLength);
    destination->fill(0);
    byteToChar(destination->data(), destUCharArray, destLength);


//    //char *tempDestCharArray = (char *)destUCharArray;
//    //byteToChar(tempDestCharArray, destUCharArray, destLength);

//    destination->clear();
//    destination->resize(0);
//    //destination->append(tempDestCharArray, destLength);

//    {
//        for (int i = 0; i < destLength; i++) {
//            if (destUCharArray[i] > 127) {
//                destination->append(destUCharArray[i]  - 256);
//            } else {
//                destination->append(destUCharArray[i]);
//            }
//        }

//    }




//        for(int i = 0; i<destLength; i++){
//            qDebug("temp[%d]:%d", i, tempDestCharArray[i]);
//        }
//        qDebug("destination->size():%d", destination->size());

    //delete [] destUCharArray;
    delete [] sourceUCharArray;
    //delete [] keyUCharArray;
    //delete [] tempDestCharArray;

    //qDebug()<<" src:"<<source.size()<<"   dst:"<<destLength;

    return destLength;

}


void Cryptography::setMaxBufferSize(int size)
{
    QMutexLocker locker(&m_mutex);

    this->maxBufferSize = size;

    delete [] destUCharArray;

    destUCharArray = new unsigned char[maxBufferSize];
    memset(destUCharArray, 0, sizeof(unsigned char)*maxBufferSize);

}

int Cryptography::getMaxBufferSize()
{
    QMutexLocker locker(&m_mutex);

    return this->maxBufferSize;

}





} //namespace HEHUI
