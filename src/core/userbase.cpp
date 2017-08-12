/*
 ****************************************************************************
 * userbase.cpp
 *
 * Created on: 2009-11-10
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
  * Last Modified on: 2010-05-07
  * Last Modified by: 贺辉
  ***************************************************************************
*/




#include "userbase.h"

#include "cryptography/cryptography.h"


namespace HEHUI
{

UserBase::UserBase(const QString &userID, const QString &userName, const QString &password, QObject *parent)
    : QObject(parent), userID(userID), userName(userName), password(password)
{

    authenticode = "";
    m_isVerified = false;

}

UserBase::~UserBase()
{
    // TODO Auto-generated destructor stub
}


void UserBase::setUserID(const QString &id)
{
    this->userID = id;
}

QString UserBase::getUserID() const
{
    return this->userID;
}

void UserBase::setUserName(const QString &userName)
{
    this->userName = userName;
}

QString UserBase::getUserName() const
{
    return this->userName;
}

void UserBase::setPassword(const QString &pwd, bool hashThePassword)
{
    if(hashThePassword) {
//        QByteArray pswd = QCryptographicHash::hash(pwd.toLatin1(), QCryptographicHash::Md5).toHex();
//        pswd = QCryptographicHash::hash(pswd, QCryptographicHash::Md5).toHex();
        this->password = hashHexString(pwd);
    } else {
        this->password = pwd;
    }
}

QString UserBase::getPassword() const
{
    return this->password;
}

QString UserBase::hashHexString(const QString &str)
{
    QByteArray ba = QCryptographicHash::hash(str.toLatin1(), QCryptographicHash::Md5).toHex();
    ba = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();

    return QString(ba);
}

QString UserBase::getAuthenticode() const
{
    return this->authenticode;
}

void UserBase::setAuthenticode(const QString &authenticode)
{
    this->authenticode = authenticode;
}


void UserBase::setVerified(bool v)
{
    this->m_isVerified = v;
}

bool UserBase::isVerified()
{
    return this->m_isVerified;
}

QByteArray UserBase::encryptedPassword() const
{
    return Cryptography::MD5(Cryptography::SHA1(password.toLatin1()).toHex()).toHex();
}



} //namespace HEHUI
