/*
 ****************************************************************************
 * windowsmanagement.h
 *
 * Created on: 2009-5-1
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
 * Last Modified on: 2010-05-15
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#ifndef WINDOWSMANAGEMENT_H
#define WINDOWSMANAGEMENT_H

#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMutex>
#include <QDateTime>
#include <QPair>

#ifdef Q_OS_WIN32
//#define _WIN32_WINNT 0x0500
#include <windows.h>

#endif


#include "wmlib.h"



namespace HEHUI {


class WM_LIB_API WindowsManagement : public QObject
{
    Q_OBJECT
public:
    //enum Location{LEATHER_PRODUCTS_FACTORY_DG = 1, HANDBAG_FACTORY_DG, No3_Branch_Factory, LEATHER_PRODUCTS_FACTORY_YD};
    enum Location{No1_Branch_Factory = 0, No2_Branch_Factory, No3_Branch_Factory, No4_Branch_Factory, LEATHER_PRODUCTS_FACTORY_YD};
    enum UserAccountState{UAS_Unknown, UAS_Disabled, UAS_Enabled};

    explicit WindowsManagement(QObject *parent = 0);


    bool addNewSitoyUserToLocalSystem(const QString &userName, const QString &userPassword, const QString &userComment, const QString &emails, const QString &dept);
    QString lastError() const;
    QStringList outputMessages() const;

#ifdef Q_OS_WIN32
public:

    bool enablePrivilege(LPCTSTR privilegeName);
    bool isNT6OS();

    void freeMemory();

    QString getEnvironmentVariable(const QString &environmentVariable);

    QString getExeFileVersion(const QString &exeFileName);

//    QStringList localUsers() ;
//    QStringList localCreatedUsers() ;
//    QString getUserNameOfCurrentThread();




    bool isUserAutoLogin();
    bool setUserAutoLogin(const QString &userName, const QString &password, bool autoLogin);

    bool initNewSitoyUser();
    bool userNeedInit(const QString &userName = "");

    float getDiskFreeSpace(const QString &directoryName);

    bool isAdmin(const QString &userName = "");

    bool updateUserPassword(const QString &userName = "", const QString &password = "", bool activeIfAccountDisabled = false);
    bool setupUserAccountState(const QString &userName,  bool enableAccount);
    UserAccountState getUserAccountState(const QString &userName);
    bool getUserLastLogonAndLogoffTime(const QString &userName, QDateTime *lastLogonTime, QDateTime *lastLogoffTime);
    QDateTime currentDateTimeOnServer(const QString &server, const QString &userName, const QString &password);
    bool setLocalTime(const QDateTime &datetime);



    void getLocalGroupsTheUserBelongs(QStringList *groups, const QString &userName = "");
    void getGlobalGroupsTheUserBelongs(QStringList *groups, const QString &userName = "", const QString &serverName = "");

    void cleanTemporaryFiles();
    //void deleteFiles(const QString &path);
    void deleteFiles(const QString &path, const QStringList & nameFilters = QStringList(), const QStringList & ignoredFiles = QStringList(), const QStringList & ignoredDirs = QStringList());

    void modifySystemSettings();

    QString getFileSystemName(const QString &rootPath);

    bool getTokenByProcessName(HANDLE &hToken, const QString &processName, bool justQuery = true);
    QList<HANDLE> getTokenListByProcessName(const QString &processName, bool justQuery = true);
    QString getAccountNameOfProcess(HANDLE &hToken);
    QString getAccountNameOfProcess(const QString &processName);

    void showAdministratorAccountInLogonUI(bool show);
    bool createHiddenAdmiAccount();
    bool deleteHiddenAdmiAccount();
    bool hiddenAdmiAccountExists();

    bool setupUSBStorageDevice(bool enableRead, bool enableWrite);
    bool readUSBStorageDeviceSettings(bool *readable, bool *writeable);

    bool setupProgrames(bool enable);

    bool setDeskWallpaper(const QString &wallpaperPath);

    void test();


    //private:
    QString userInfoFilePath();

    bool addUserToLocalSystem(const QString &userName, const QString &userPassword, const QString &comment);
    bool addUserToLocalSystem(LPWSTR userName, LPWSTR userPassword, LPWSTR comment);
    bool deleteUserFromLocalSystem(const QString &userName);
    bool deleteUserFromLocalSystem(LPWSTR userName);

    bool addUserToLocalGroup(const QString &userName, const QString &groupName);
    bool addUserToLocalGroup(LPWSTR userName, LPCWSTR groupName);
    bool deleteUserFromLocalGroup(const QString &userName, const QString &groupName);
    bool deleteUserFromLocalGroup(LPWSTR userName,  LPCWSTR groupName);
    QStringList getMembersOfLocalGroup(const QString &groupName, const QString &serverName = "");
    QStringList localGroups();

    bool setComputerName(const QString &newComputerName);
//    QString getComputerName();
//    bool getComputerNameInfo(QString *dnsDomain, QString *dnsHostname, QString *netBIOSName);
    bool joinWorkgroup(const QString &workgroup);
    bool joinDomain(const QString &domainName, const QString &accountName, const QString &password, const QString &serverName = "");
    bool unjoinDomain(const QString &accountName, const QString &password, const QString &serverName = "");
//    QString getJoinInformation(bool *isJoinedToDomain = 0, const QString &serverName = "");
    bool renameMachineInDomain(const QString &newMachineName, const QString &accountName, const QString &password, const QString &serverName = "");
//    void getAllUsersLoggedOn(QStringList *users, const QString &serverName = "");
//    bool getLogonInfoOfCurrentUser(QString *userName, QString *domain, QString *logonServer);

    bool isStartupWithWin(const QString &applicationFilePath, const QString &parameters, const QString &valueNameString);
    bool setStartupWithWin(const QString &applicationFilePath, const QString &parameters, const QString &valueNameString, bool startupWithWin);

    bool addOEMailAccount(const QString &userName, const QString &department, bool intEmail, const QString &storeRoot, const QString &accountName);
    bool addLiveMailAccount(const QString &userName, const QString &department, bool intEmail, const QString &storeRoot, const QString &accountName);
    bool addOutlookMailAccount(const QString &userName, const QString &department, bool intEmail, const QString &storeRoot, const QString &accountName);
    QString stringToOutlookHexString(const QString &string);
    QString outlookInstalledPath();

    bool addConnectionToNetDrive();
    bool addPrinterConnections(const QString &department);

    bool setupIME();

    void setLocation(Location location);
    //    void setNewComputerNameToBeUsed(const QString &m_newComputerNameToBeUsed);


    //////////////////////////////////////////////////////
    bool runAs(const QString &userName, const QString &dowmainName, const QString &password,
               const QString &exeFilePath, const QString &parameters = "", bool show = true,
               const QString &workingDir = "", bool wait = false, DWORD milliseconds = 6000);

    bool runAsForInteractiveService(const QString &userName, const QString &domainName, const QString &password,
                                    const QString &exeFilePath, const QString &parameters = "", bool show = true,
                                    const QString &workingDir = "");
    bool runAsForDesktopApplication(const QString &userName, const QString &domainName, const QString &password,
                                    const QString &exeFilePath, const QString &parameters = "", bool show = true,
                                    const QString &workingDir = "", bool wait = false, DWORD milliseconds = 6000);
    //////////////////////////////////////////////////////

    QString getCPUSerialNumber();
    QString getHardDriveSerialNumber(unsigned int driveIndex = 0);


#endif


signals:
    void signalAddingUserJobDone(bool result);
    void signalInitializingUserJobDone(bool result);
    void signalProgressUpdate(const QString &message, int percent);
    void signalProcessOutputUpdated(const QString &message);



private:
    //    void setLastErrorString(const QString &errorString);


private:
    QMutex mutex;
    QString m_lastErrorString;
    QStringList m_outputMsgs;

    bool runningNT6OS;
    bool m_isAdmin;

    Location location;

    QString m_currentUserName;


};

} //namespace HEHUI

#endif // WINDOWSMANAGEMENT_H
