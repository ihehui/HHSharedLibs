/****************************************************************************
* winutilities.h
*
* Created on: 2015-1-9
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
  * Last Modified on: 2015-04-7
  * Last Modified by: 贺辉
  ***************************************************************************
*/





#ifndef WINUTILITIES_H_
#define WINUTILITIES_H_



#include <QObject>
#include <QJsonArray>
//#include <QImage>

#include <Windows.h>


#ifdef __GNUC__
#define __out
#endif

#include "systemutilitieslib.h"
#include "utilities_def.h"

namespace HEHUI
{


class SYSUTIL_LIB_API WinUtilities
{

public:
    WinUtilities();
    virtual ~WinUtilities();

    static QString WinSysErrorMsg(DWORD winErrorCode, DWORD dwLanguageId = 0);
    static void freeMemory();

    static __int64 CompareFileTime ( FILETIME time1, FILETIME time2 );
    //Period: 1S
    static int getCPULoad();
    static bool getMemoryStatus(quint64 *totalBytes, int *loadPercentage);
    static bool getDiskPartionStatus(const QString &partionRootPath, float *totalBytes, float *freeBytes);
    static QString getFileSystemName(const QString &filePath);
    static QStringList getLogicalDrives();


    static QString getCPUName();
    static QString getCPUSerialNumber();
    static QString getHardDriveSerialNumber(unsigned int driveIndex = 0);



    ////The following are predefined version information Unicode strings.
    ////   Comments InternalName ProductName
    ////   CompanyName LegalCopyright ProductVersion
    ////   FileDescription LegalTrademarks PrivateBuild
    ////   FileVersion OriginalFilename SpecialBuild
    static bool getFileVersion(const QString &fileName, QStringList *predefinedVersionInfo, DWORD *errorCode = 0);


    //Computer
    static bool setComputerName(const QString &newComputerName, DWORD *errorCode = 0);
    static QString getComputerName(DWORD *errorCode = 0);
    static bool getComputerNameInfo(QString *dnsDomain, QString *dnsHostname, QString *netBIOSName, DWORD *errorCode = 0);
    static QString getJoinInformation(bool *isJoinedToDomain = 0, const QString &serverName = "", DWORD *errorCode = 0);
    static bool renameMachineInDomain(const QString &newMachineName, const QString &accountName, const QString &password, const QString &serverName = "", DWORD *errorCode = 0);
    static bool joinWorkgroup(const QString &workgroup, DWORD *errorCode = 0);
    static bool joinDomain(const QString &domainName, const QString &accountName, const QString &password, const QString &serverName = "", DWORD *errorCode = 0);
    static bool unjoinDomain(const QString &accountName, const QString &password, const QString &serverName = "", DWORD *errorCode = 0);

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

    static bool windowsVersionName(QString *versionName);
    static bool windowsVersionName(wchar_t *str, int bufferSize);
    static bool is64BitApplication();
    static void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo);
    static bool is64BitOS();
    static bool isWow64();

    // Returns true if run under a windows NT family
    static bool isWinNTFamily();
    // Returns true if run under windows XP
    static bool isWin2000();
    // Returns true if run under windows XP
    static bool isWinXP();
    // Returns true if run windows 2003 server.
    static bool isWin2003Server();
    // Returns true if run under Windows Vista or later
    static bool isVistaOrLater();

    static QString getEnvironmentVariable(const QString &environmentVariable);


    //Users
    static bool createLocalUser(const QString &userName, const QString &userPassword, const QString &comment, DWORD *errorCode = 0);
    static bool createLocalUser(LPWSTR userName, LPWSTR userPassword, LPWSTR comment, DWORD *errorCode = 0);
    static bool deleteLocalUser(const QString &userName, DWORD *errorCode = 0);
    static bool deleteLocalUser(LPWSTR userName, DWORD *errorCode = 0);
    static bool updateUserPassword(const QString &userName = "", const QString &password = "", DWORD *errorCode = 0, bool activeIfAccountDisabled = false);

    enum UserAccountState {UAS_Unknown, UAS_Disabled, UAS_Enabled};
    static bool setupUserAccountState(const QString &userName,  bool enableAccount, DWORD *errorCode = 0);
    static UserAccountState getUserAccountState(const QString &userName, DWORD *errorCode = 0);

    static QString getUserNameOfCurrentThread(DWORD *errorCode = 0);
    static bool getLogonInfoOfCurrentUser(QString *userName, QString *domain, QString *logonServer = 0, DWORD *apiStatus = 0);
    static void getAllUsersLoggedOn(QStringList *users, const QString &serverName = "", DWORD *apiStatus = 0);
    static QStringList localUsers(DWORD *apiStatus = 0) ;
    static QStringList localCreatedUsers() ;
    static QStringList getMembersOfLocalGroup(const QString &groupName, const QString &serverName = "");
    static bool getLocalGroupsTheUserBelongs(QStringList *groups, const QString &userName = "", DWORD *errorCode = 0);
    static bool getGlobalGroupsTheUserBelongs(QStringList *groups, const QString &userName = "", const QString &serverName = "", DWORD *errorCode = 0);
    static bool addUserToLocalGroup(const QString &userName, const QString &groupName, DWORD *errorCode = 0);
    static bool addUserToLocalGroup(LPWSTR userName, LPCWSTR groupName, DWORD *errorCode = 0);
    static bool deleteUserFromLocalGroup(const QString &userName, const QString &groupName, DWORD *errorCode = 0);
    static bool deleteUserFromLocalGroup(LPWSTR userName,  LPCWSTR groupName, DWORD *errorCode = 0);

    static bool isAdmin(const QString &userName = "");
    static void showAdministratorAccountInLogonUI(bool show);
    static bool getUserLastLogonAndLogoffTime(const QString &userName, QDateTime *lastLogonTime, QDateTime *lastLogoffTime);

    static bool createHiddenAdmiAccount();
    static bool deleteHiddenAdmiAccount();
    static bool hiddenAdmiAccountExists();

    //    class WinUserInfo
    //    {
    //    public:
    //        WinUserInfo(){
    //            accountDisabled = false;
    //            cannotChangePassword = false;
    //            accountLocked = false;
    //            passwordNeverExpires = false;

    //            lastLogonTime_t = 0;
    //            lastLogoffTime_t = 0;

    //            mustChangePassword = false;
    //        }

    //        QString userName;
    //        QString homeDir;
    //        QString comment;

    //        bool accountDisabled;
    //        bool cannotChangePassword;
    //        bool accountLocked;
    //        bool passwordNeverExpires;

    //        QString fullName;

    //        unsigned int lastLogonTime_t;
    //        unsigned int lastLogoffTime_t;

    //        QString logonServer;
    //        QString sid;
    //        QString profile;
    //        bool mustChangePassword;

    //        QString groups;

    //    };
    static bool getAllUsersInfo(QJsonArray *jsonArray, DWORD *errorCode = 0);
    static bool createOrModifyUser(QJsonObject *userObject, DWORD *errorCode = 0);


    //    //Service
    //    typedef struct SERVICE_INFO {
    //        SERVICE_INFO()
    //        {
    //            processID = 0;
    //            serviceType = 0xFFFFFFFF;
    //            startType = 0xFFFFFFFF;
    //        }
    //        QString serviceName;
    //        QString displayName;
    //        DWORD processID;
    //        QString description;
    //        DWORD startType;
    //        QString account;
    //        QString dependencies;
    //        QString binaryPath;
    //        DWORD serviceType;

    //    } ServiceInfo;
    static bool serviceOpenSCManager(SC_HANDLE *schSCManager, DWORD *errorCode = 0, DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS);
    static bool serviceOpenService(const QString &serviceName, SC_HANDLE *schSCManager, SC_HANDLE *schService, DWORD *errorCode = 0, DWORD dwDesiredAccess = SERVICE_ALL_ACCESS);
    static bool serviceQueryInfo(const QString &serviceName, ServiceInfo *serviceInfo,  DWORD *errorCode = 0);
    static bool serviceQueryInfo(SC_HANDLE *schSCManager, const QString &serviceName, ServiceInfo *serviceInfo,  DWORD *errorCode = 0);
    static bool serviceChangeStartType(const QString &serviceName, DWORD startType = SERVICE_DEMAND_START, DWORD *errorCode = 0);
    static bool serviceChangeDescription(const QString &serviceName, const QString &description, DWORD *errorCode = 0);
    static bool serviceDelete(const QString &serviceName, DWORD *errorCode = 0);
    static bool serviceGetAllServicesInfo(QJsonArray *jsonArray, DWORD *errorCode = 0, DWORD serviceType = SERVICE_WIN32);
    static bool serviceStart(const QString &serviceName, DWORD *errorCode = 0);
    static bool serviceStop(const QString &serviceName, DWORD *errorCode = 0);
    static bool serviceStopDependentServices(SC_HANDLE *schSCManager, SC_HANDLE *schService);
    static void serviceCloseHandle(SC_HANDLE *schSCManager, SC_HANDLE *schService);

    //System
    static BOOL EnableShutdownPrivilege();
    static BOOL Shutdown(BOOL bForce);
    static BOOL Shutdown(const QString &machineName, const QString &message, DWORD timeout, bool forceAppsClosed, bool rebootAfterShutdown);
    static BOOL Logoff(BOOL bForce);
    static BOOL Reboot(BOOL bForce);
    static BOOL LockWindows();


    //////////////////////////////////////////////////////
    static bool runAs(const QString &userName, const QString &dowmainName, const QString &password,
                      const QString &exeFilePath, const QString &parameters = "", bool show = true,
                      const QString &workingDir = "", bool wait = false, DWORD milliseconds = 6000);

    static bool runAsForInteractiveService(const QString &userName, const QString &domainName, const QString &password,
                                           const QString &exeFilePath, const QString &parameters = "", bool show = true,
                                           const QString &workingDir = "");
    static bool runAsForDesktopApplication(const QString &userName, const QString &domainName, const QString &password,
                                           const QString &exeFilePath, const QString &parameters = "", bool show = true,
                                           const QString &workingDir = "", bool wait = false, DWORD milliseconds = 6000);

    //// Run application in active console session with current console process token information.
    //// Service application(usually in session 0) can start another process in active console session(usually session 1) which has the same token information, the child process can interact with the input desktop(usually session 1)
    static bool runASCurrentConsoleProcessInActiveConsoleSession(const QString &exeFilePath, const QString &parameters = "", bool show = true,
                                           const QString &workingDir = "", bool wait = false, DWORD milliseconds = 6000);

    static bool getTokenByProcessName(HANDLE &hToken, const QString &processName, bool justQuery = true);
    static QList<HANDLE> getTokenListByProcessName(const QString &processName, bool justQuery = true);
    static QString getAccountNameOfProcess(HANDLE &hToken);
    static QString getAccountNameOfProcess(const QString &processName);
    //////////////////////////////////////////////////////


    //GDI+
    //Get image format Clsid
    static int GetEncoderClsid(const WCHAR *format, CLSID *pClsid);
    static QByteArray ConvertHBITMAPToJpeg(HBITMAP hbitmap);
    //static QPixmap WinHBITMAPToPixmap(HBITMAP bitmap, bool noAlpha = true);
    //HBITMAP PixmapToWinHBITMAP(const QImage &image, bool noAlpha = true);

    static HBITMAP GetScreenshotBmp();
    static HBITMAP GetScreenshotBmp1();

    static HBITMAP GetScreenshotBmpForNT5InteractiveService();
    static HBITMAP setDesktop1();
    static bool setDesktop();

    static bool setDeskWallpaper(const QString &wallpaperPath);
    // Restores the desktop wallpaper.
    static bool restoreWallpaper();
    // Hides the desktop wallpaper.
    static bool disableWallpaper();

    static QDateTime currentDateTimeOnServer(const QString &server, const QString &userName, const QString &password);
    static bool setLocalTime(const QDateTime &datetime);

    // Returns true if the Aero is On.
    static bool isAeroOn();

    // Param: path - points to current application file path, with file name
    static bool getCurrentModuleFileName(QString *path);

/////////////////////////////////////////////////////////////////////////////////////////////////////////

    static DWORD getActiveConsoleSessionId();

    ////Desktop
    // This funtion gets a handle to a desktop that receive user inputs.
    // If success the function returns a handle to the desktop. On fail the function returns zero.
    // After use the returned handle must be called the closeDesktop() function.
    static HDESK getInputDesktop();

    // This funtion gets a handle to a named desktop by the name.
    // Param: name - a QString object that contain a valid desktop name such as "Winlogon".
    // if success the function returns a handle to the desktop that receives user inputs. On fail the function returns zero.
    // After use the returned handle must be called the closeDesktop() function.
    static HDESK getDesktop(const QString &name);

    // This function closes a handle to a desktop.
    // Param hdesk is a handle to a desktop that will be closed.
    // If success the function return true else false.
    static bool closeDesktop(HDESK hdesk);

    // This function sets a desktop to a thread from that it was called.
    // Param: newDesktop - is a handle to a desktop.
    // If success the function return true else false.
    static bool setDesktopToCurrentThread(HDESK newDesktop);

    // This function select a desktop that assigned by name or not to a current thread from that it was called.
    // Param: name - a QString object that contain a valid desktop name such as "Winlogon" that will be assigned to the thread. If name is empty the input desktop will be assigned.
    // If success the function return true else false.
    static bool selectDesktop(const QString &name);

    // Param: desktopName - is a pointer to a QString object that will be used to store the desktop name. If function has failed then the desktopName object will not change. If the function succeeds, the name of a current input desktop stores in the desktopName object.
    // If success the function return true else false.
    static bool getCurrentDesktopName(QString *desktopName);

    // Param: desktopName - is a pointer to a QString object that will be used to store the desktop name. If function has failed then the desktopName object will not change. If the function succeeds, the desktop name of the current thread stores in the desktopName object.
    // If success the function return true else false.
    static bool getThreadDesktopName(QString *desktopName);

    // This function gets desktop name by a handle to a desktop.
    // Param: desktopName - is a pointer to a QString object that will be used to store the desktop name. If function has failed then the desktopName object will not change. If the function succeeds, the desktop name stores in the desktopName object.
    // If success the function return true else false.
    static bool getDesktopName(HDESK desktop, QString *desktopName);

/////////////////////////////////////////////////////////////////////////////////////////////////////////


    ///// Simulates the "ctrl + alt + del" combination by using the "SAS" lib, only under service!
    /////  !!! Modify the registry first!!!
    /////  [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System]
    /////  "SoftwareSASGeneration"=dword:00000003
    /////
    static bool simulateCtrlAltDelUnderVista();


private:

    // Operation system version info.
    //static OSVERSIONINFO m_osVerInfo;

};

} //namespace HEHUI

#endif /* WINUTILITIES_H_ */
