/*
 ****************************************************************************
 * adsi.h
 *
 * Created on: 2012-10-18
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
 * Last Modified on: 2012-10-18
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#ifndef ADSI_H
#define ADSI_H

#include <QObject>
#include <QLibrary>

#include <windows.h>
#include <string.h>

#include "wmlib.h"

using namespace std;


class WM_LIB_API ADSI : public QObject
{
    Q_OBJECT
public:
    explicit ADSI(QObject *parent = 0);
    ~ADSI();

    bool loadLibrary(const QString &fileName);
    bool unloadLibrary();
    bool isLibraryLoaded();
    QString lastErrorString() const;

    bool AD_Open(const QString &userID, const QString &password, const QString &serverHost, ulong securityFlag = 1);
    void AD_Close();
    long AD_GetLastErrorCode();
    QString AD_GetLastErrorString();
    QString AD_DefaultNamingContext();
    bool AD_ObjectExists(const QString &object, const QString &property);
    bool AD_RenameObject(const QString &object, const QString &cn);
    bool AD_MoveObject(const QString &ou, const QString &object, const QString &cn = "");
    bool AD_DeleteObject(const QString &object, const QString &objectClass);
    bool AD_UnlockObject(const QString &object);
    bool AD_EnableObject(const QString &object, bool enable);
    bool AD_IsObjectDisabled(const QString &object);
    bool AD_SetAccountExpire(const QString &object, const QString &expireDateTime);
    bool AD_SetPasswordExpire(const QString &object, bool enableExpire);
    bool AD_SetUserPasswordChange(const QString &object, bool enableChange);
    bool AD_GetUserPasswordChange(const QString &object, long *userCanChangePassword);
    QString AD_GetObjectAttribute(const QString &object, const QString &attribute);
    bool AD_ModifyAttribute(const QString &object, const QString &attribute, const QString &value, long option = 0);
    bool AD_CreateOU(const QString &parentOU, const QString &ouName);
    QString AD_GetAllOUs(const QString &root, const QString &separator, const QString &subOUSeparator);
    QString AD_GetObjectsInOU(const QString &ou, const QString &filter, const QString &dataToRetrieve, const QString &itemSeparator, const QString &attributeSeparator);
    bool AD_CreateUser(const QString &ou, const QString &userName, const QString &userCN);
    bool AD_SetPassword(const QString &userName, const QString &password, bool expire = false);

    bool accountDisabled(const QString &samAccountName);
    bool userMustChangePassword(const QString &samAccountName);
    bool userCanChangePassword(const QString &samAccountName);
    bool passwordNeverExpires(const QString &samAccountName);


    QString ComputerName();
    QString UserNameOfCurrentThread();


signals:
    
public slots:

private:
   QLibrary * adsiLibrary;
   QString m_lastErrorString;


   typedef long (WINAPI * AD_OpenFunction)(LPCWSTR, LPCWSTR, LPCWSTR, DWORD);
   AD_OpenFunction m_AD_Open;

   typedef long (WINAPI * AD_CloseFunction)();
   AD_CloseFunction m_AD_Close;

   typedef long (WINAPI * AD_GetLastErrorCodeFunction)();
   AD_GetLastErrorCodeFunction m_AD_GetLastErrorCode;

   typedef LPCWSTR (WINAPI * AD_GetLastErrorStringFunction)();
   AD_GetLastErrorStringFunction m_AD_GetLastErrorString;

   typedef LPCWSTR (WINAPI * AD_DefaultNamingContextFunction)();
   AD_DefaultNamingContextFunction m_AD_DefaultNamingContext;

   typedef long (WINAPI * AD_ObjectExistsFunction)(LPCWSTR, LPCWSTR);
   AD_ObjectExistsFunction m_AD_ObjectExists;

   typedef long (WINAPI * AD_RenameObjectFunction)(LPCWSTR, LPCWSTR);
   AD_RenameObjectFunction m_AD_RenameObject;

   typedef long (WINAPI * AD_MoveObjectFunction)(LPCWSTR, LPCWSTR, LPCWSTR);
   AD_MoveObjectFunction m_AD_MoveObject;

   typedef long (WINAPI * AD_DeleteObjectFunction)(LPCWSTR, LPCWSTR);
   AD_DeleteObjectFunction m_AD_DeleteObject;

   typedef long (WINAPI * AD_UnlockObjectFunction)(LPCWSTR);
   AD_UnlockObjectFunction m_AD_UnlockObject;

   typedef long (WINAPI * AD_EnableObjectFunction)(LPCWSTR, long);
   AD_EnableObjectFunction m_AD_EnableObject;

   typedef long (WINAPI * AD_IsObjectDisabledFunction)(LPCWSTR);
   AD_IsObjectDisabledFunction m_AD_IsObjectDisabled;

   typedef long (WINAPI * AD_SetAccountExpireFunction)(LPCWSTR, LPCWSTR);
   AD_SetAccountExpireFunction m_AD_SetAccountExpire;

   typedef long (WINAPI * AD_SetPasswordExpireFunction)(LPCWSTR, long);
   AD_SetPasswordExpireFunction m_AD_SetPasswordExpire;

   typedef long (WINAPI * AD_SetUserPasswordChangeFunction)(LPCWSTR, long);
   AD_SetUserPasswordChangeFunction m_AD_SetUserPasswordChange;

   typedef long (WINAPI * AD_GetUserPasswordChangeFunction)(LPCWSTR, long *);
   AD_GetUserPasswordChangeFunction m_AD_GetUserPasswordChange;

   typedef LPCWSTR (WINAPI * AD_GetObjectAttributeFunction)(LPCWSTR, LPCWSTR);
   AD_GetObjectAttributeFunction m_AD_GetObjectAttribute;

   typedef long (WINAPI * AD_ModifyAttributeFunction)(LPCWSTR, LPCWSTR, LPCWSTR, long);
   AD_ModifyAttributeFunction m_AD_ModifyAttribute;

   typedef long (WINAPI * AD_CreateOUFunction)(LPCWSTR, LPCWSTR);
   AD_CreateOUFunction m_AD_CreateOU;

   typedef LPCWSTR (WINAPI * AD_GetAllOUsFunction)(LPCWSTR, LPCWSTR, LPCWSTR);
   AD_GetAllOUsFunction m_AD_GetAllOUs;

   typedef LPCWSTR (WINAPI * AD_GetObjectsInOUFunction)(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR);
   AD_GetObjectsInOUFunction m_AD_GetObjectsInOU;

   typedef long (WINAPI * AD_CreateUserFunction)(LPCWSTR, LPCWSTR, LPCWSTR);
   AD_CreateUserFunction m_AD_CreateUser;

   typedef long (WINAPI * AD_SetPasswordFunction)(LPCWSTR, LPCWSTR, long);
   AD_SetPasswordFunction m_AD_SetPassword;


   typedef LPCWSTR (WINAPI * ComputerNameFunction)();
   ComputerNameFunction m_ComputerName;

   typedef LPCWSTR (WINAPI * UserNameOfCurrentThreadFunction)();
   UserNameOfCurrentThreadFunction m_UserNameOfCurrentThread;





    
};

#endif // ADSI_H
