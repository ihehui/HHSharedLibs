/*
 ****************************************************************************
 * userbase.h
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

#ifndef USERBASE_H_
#define USERBASE_H_

#include <QObject>
#include <QString>
#include <QMetaObject>
#include <QCoreApplication>
#include <QDateTime>

#include "core_lib.h"

namespace HEHUI {

class CORE_LIB_API UserBase: public QObject {
    Q_OBJECT

public:
    UserBase(const QString &userID = "", const QString &userName = "", const QString &password = "", QObject *parent = 0);

    virtual ~UserBase();

    void setUserID(const QString &id);
    QString getUserID() const;

    void setUserName(const QString &userName);
    QString getUserName() const;

    void setPassword(const QString &pwd, bool hashThePassword = false);
    QString getPassword() const;

    QString getAuthenticode() const;
    void setAuthenticode(const QString &authenticode);

    void setVerified(bool v) ;
    bool isVerified();

    QByteArray encryptedPassword() const;


private:

    QString userID;
    QString userName;
    QString password;
    QString authenticode;

    bool m_isVerified;


};

} //namespace HEHUI

#endif /* USERBASE_H_ */
