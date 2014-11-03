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





    void setUserID(const QString &id) {
        this->userID = id;
    }

    QString getUserID() const {
        return this->userID;
    }

    void setUserName(const QString &userName){
        this->userName = userName;
    }

    QString getUserName() const {
        return this->userName;
    }

    void setPassword(const QString &pwd) {
        this->password = pwd;
    }

    QString getPassword() const {
        return this->password;
    }

    QString getAuthenticode() const {
        return this->authenticode;
    }

    void setAuthenticode(const QString &authenticode){
        this->authenticode = authenticode;
    }


    void setVerified(bool v) {
        this->isVerified = v;
    }

    bool verified() {
        return this->isVerified;
    }


    //TODO:更名
    //设置是否允许帐户重复登陆
    //	void setSingleUserMode(bool isSingleUserMode) {
    //		this->singleUserMode = isSingleUserMode;
    //	}
    //	;
    //	bool isSingleUserMode() {
    //		return this->singleUserMode;
    //	}

private:

    QString userID;
    QString userName;
    QString password;
    QString authenticode;

    bool isVerified;

    //单用户模式//Single User Mode
    //	bool singleUserMode;

};

} //namespace HEHUI

#endif /* USERBASE_H_ */
