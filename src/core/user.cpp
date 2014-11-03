/*
 ****************************************************************************
 * user.cpp
 *
 * Created on: 2008-10-16
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
 * Last Modified on: 2010-07-07
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#include "user.h"

namespace HEHUI {

User::User(const QString &userID, const QString & userName, const QString &password, QObject *parent) :
    UserBase(userID, userName, password, parent) {

    setVerified(false);

    rootMode = false;

    trueName = "";
    nickName = "";
    gender = GENDER_UNKNOWN;

    homeAddress = "";
    homePhoneNumber = "";
    homeZipCode = "";
    personalHomepage = "";
    personalEmailAddress = "";

    lastLoginTime = QDateTime();
    lastLogoutTime = QDateTime();
    lastLoginExternalHostAddress = "";
    lastLoginExternalHostPort = 0;

    questionForSecurity = "";
    answerForSecurity = "";
    face = "";

    companyName = "";
    jobTitle = "";
    businessAddress = "";
    businessPhoneNumber = "";
    businessZipCode = "";
    businessFaxNumber = "";
    businessHomepage = "";
    businessEmailAddress = "";

    registrationTime = QDateTime();
    loginTimes = 0;

    description = "";

}

User::~User() {
    // TODO Auto-generated destructor stub
}















} //namespace HEHUI

