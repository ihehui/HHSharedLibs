/*
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

#include "../core_lib.h"


using namespace std;

namespace HEHUI {

class CORE_LIB_API Cryptography {

public:

    Cryptography();

    static QByteArray MD5(const QByteArray &data);
    static QByteArray SHA1(const QByteArray &data);
    static QByteArray getFileMD5(const QString &fileName);
    static QString getFileMD5HexString(const QString &fileName);

//    int stringToByte(unsigned char* destination, string &source);
    void charToByte(unsigned char* destination, const char *source, int sourceLength);
    void byteToChar(char *destination, const unsigned char* source, int sourceLength);

    int teaCrypto(QByteArray *destination, const QByteArray &source, const QByteArray &key, bool encrypt);

    void setMaxBufferSize(int size);
    int getMaxBufferSize() const;

private:
    int maxBufferSize;

};

} //namespace HEHUI

#endif // CRYPTOGRAPHY_H
