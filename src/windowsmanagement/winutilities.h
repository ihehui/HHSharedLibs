/****************************************************************************
* winutilities.h
*
* Created on: 2009-11-9
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
  * Last Modified on: 2010-08-19
  * Last Modified by: 贺辉
  ***************************************************************************
*/





#ifndef WINUTILITIES_H_
#define WINUTILITIES_H_



#include <QObject>
#include <QJsonArray>
//#include <QImage>

#include <Windows.h>
#include "wmlib.h"



namespace HEHUI {

//class WM_LIB_API ServiceInfo : public QObject{
//public:
//    ServiceInfo() : QObject(){
//        processID = 0;
//        serviceType = 0x00000020;
//        startType = 0x00000002;
//    }
//    QString serviceName;
//    QString displayName;
//    DWORD processID;
//    QString description;
//    DWORD startType;
//    QString account;
//    QString dependencies;
//    QString binaryPath;
//    DWORD serviceType;


//} ;

class WM_LIB_API WinUtilities {

public:
    WinUtilities();
    virtual ~WinUtilities();

    static QString WinSysErrorMsg(DWORD winErrorCode, DWORD dwLanguageId = 0);
    static void freeMemory();

    //Computer
    static QString getComputerName(DWORD *errorCode = 0);
    static bool getComputerNameInfo(QString *dnsDomain, QString *dnsHostname, QString *netBIOSName, DWORD *errorCode = 0);
    static QString getJoinInformation(bool *isJoinedToDomain = 0, const QString &serverName = "", DWORD *errorCode = 0);
    static bool setupUSBStorageDevice(bool enableRead, bool enableWrite);
    static bool readUSBStorageDeviceSettings(bool *readable, bool *writeable);


    //Registry
    static bool parseRegKeyString(const QString &keyString, HKEY *rootKey, QString *subKeyString);
    static bool regOpen(const QString &key, HKEY *hKey, REGSAM samDesired = KEY_READ);
    static bool regRead(HKEY hKey, const QString &valueName, QString *value);
    static bool regRead(const QString &key, const QString &valueName, QString *value, bool on64BitView = false);
    static bool regEnumVal(HKEY hKey, QStringList *valueNameList);
    static bool regEnumVal(const QString &key, QStringList *valueNameList, bool on64BitView = false);
    static bool regEnumKey(HKEY hKey, QStringList *keyNameList);
    static bool regEnumKey(const QString &key, QStringList *keyNameList, bool on64BitView = false);
    static bool regCreateKey(HKEY hKey, const QString &subKeyName, HKEY *hSubKey = 0);
    static bool regCreateKey(const QString &key, const QString &subKeyName, HKEY *hSubKey = 0, bool on64BitView = false);
    static bool regSetValue(HKEY hKey, const QString &valueName, const QString &value, DWORD valueType);
    static bool regSetValue(const QString &key, const QString &valueName, const QString &value, DWORD valueType, bool on64BitView = false);
    static bool regDeleteKey(HKEY hKey, const QString &subKeyName, bool on64BitView = false);
    static bool regDeleteKey(const QString &key, bool on64BitView = false);
    static bool regDeleteValue(HKEY hKey, const QString &valueName);
    static bool regDeleteValue(const QString &key, const QString &valueName, bool on64BitView = false);
    static void regCloseKey(HKEY hKey);

    static bool is64BitApplication();
    static bool isWow64();

    //Users
    static QString getUserNameOfCurrentThread(DWORD *errorCode = 0);
    static bool getLogonInfoOfCurrentUser(QString *userName, QString *domain, QString *logonServer = 0, DWORD *apiStatus = 0);
    static void getAllUsersLoggedOn(QStringList *users, const QString &serverName = "", DWORD *apiStatus = 0);
    static QStringList localUsers(DWORD *apiStatus = 0) ;
    static QStringList localCreatedUsers() ;
    static QStringList getMembersOfLocalGroup(const QString &groupName, const QString &serverName = "");


    //Service
    typedef struct SERVICE_INFO{
        SERVICE_INFO(){
            processID = 0;
            serviceType = 0xFFFFFFFF;
            startType = 0xFFFFFFFF;
        }
        QString serviceName;
        QString displayName;
        DWORD processID;
        QString description;
        DWORD startType;
        QString account;
        QString dependencies;
        QString binaryPath;
        DWORD serviceType;

    } ServiceInfo;
    static bool serviceOpenSCManager(SC_HANDLE *schSCManager, DWORD *errorCode = 0, DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS);
    static bool serviceOpenService(const QString &serviceName, SC_HANDLE *schSCManager, SC_HANDLE *schService, DWORD *errorCode = 0, DWORD dwDesiredAccess = SERVICE_ALL_ACCESS);
    static bool serviceQueryInfo(const QString &serviceName, ServiceInfo *serviceInfo,  DWORD *errorCode = 0);
    static bool serviceQueryInfo(SC_HANDLE *schSCManager, const QString &serviceName, ServiceInfo *serviceInfo,  DWORD *errorCode = 0);
    static bool serviceChangeStartType(const QString &serviceName, DWORD startType = SERVICE_DEMAND_START, DWORD *errorCode = 0);
    static bool serviceChangeDescription(const QString &serviceName, const QString &description, DWORD *errorCode = 0);
    static bool serviceDelete(const QString &serviceName, DWORD *errorCode = 0);
    static bool serviceGetAllServicesInfo(QJsonArray *jsonArray, DWORD serviceType = SERVICE_WIN32, DWORD *errorCode = 0);
    static bool serviceStart(const QString &serviceName, DWORD *errorCode = 0);
    static bool serviceStop(const QString &serviceName, DWORD *errorCode = 0);
    static bool serviceStopDependentServices(SC_HANDLE *schSCManager, SC_HANDLE *schService);
    static void serviceCloseHandle(SC_HANDLE *schSCManager, SC_HANDLE *schService);

    //System
    static BOOL EnableShutdownPrivilege();
    static BOOL Shutdown(BOOL bForce);
    static BOOL Logoff(BOOL bForce);
    static BOOL Reboot(BOOL bForce);

    //GDI+
    //Get image format Clsid
    static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
    static QByteArray ConvertHBITMAPToJpeg(HBITMAP hbitmap);
    //static QPixmap WinHBITMAPToPixmap(HBITMAP bitmap, bool noAlpha = true);
    static HBITMAP GetScreenshotBmp();
    static HBITMAP GetScreenshotBmp1();

    static HBITMAP GetScreenshotBmpForNT5InteractiveService();
    static HBITMAP setDesktop1();
    static bool setDesktop();










private:


};

} //namespace HEHUI

#endif /* WINUTILITIES_H_ */
