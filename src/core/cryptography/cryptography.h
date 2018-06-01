﻿/*
 ****************************************************************************
 * cryptography.h
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





#ifndef CRYPTOGRAPHY_H
#define CRYPTOGRAPHY_H

#include <iostream>
//#include <stdint.h>


#include <QByteArray>
#include <QCryptographicHash>
#include <QMutex>


#include "../core_lib.h"



#define CRYPTO_KEY_SIZE 16


using namespace std;

namespace HEHUI
{

class CORE_LIB_API Cryptography
{

public:

    Cryptography();
    ~Cryptography();

    static QByteArray MD5(const QByteArray &data);
    static QByteArray SHA1(const QByteArray &data);
    static QByteArray getFileMD5(const QString &fileName, QString *errorString = 0);
    static QString getFileMD5HexString(const QString &fileName, QString *errorString = 0);

//    int stringToByte(unsigned char* destination, string &source);
    void charToByte(unsigned char *destination, const char *source, int sourceLength);
    void byteToChar(char *destination, const unsigned char *source, int sourceLength);

    int teaCrypto(QByteArray *destination, const QByteArray &source, const QByteArray &key, bool encrypt);

    void setMaxBufferSize(int size);
    int getMaxBufferSize();

private:
    int maxBufferSize;

    unsigned char keyUCharArray[CRYPTO_KEY_SIZE];
    unsigned char *destUCharArray;

    QMutex m_mutex;

};

} //namespace HEHUI

#endif // CRYPTOGRAPHY_H
