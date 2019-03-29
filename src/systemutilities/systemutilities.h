#ifndef SYSTEMUTILITIES_H
#define SYSTEMUTILITIES_H

#include <QObject>

#include "systemutilitieslib.h"



namespace HEHUI
{


class SYSUTIL_LIB_API SystemUtilities : public QObject
{
    Q_OBJECT
public:
    explicit SystemUtilities(QObject *parent = 0);
    ~SystemUtilities();


    static int getCPULoad();
    static QString getCPUName();
    static QString getCPUSerialNumber();

    static QString getHardDriveSerialNumber(unsigned int driveIndex = 0);

    static bool getMemoryStatus(quint64 *totalBytes, int *loadPercentage);
    static bool getDiskPartionStatus(const QString &partionRootPath, float *totalBytes, float *freeBytes);
    static QString getDisksInfo();

    static QString getOSVersionInfo();
    static bool getLogonInfoOfCurrentUser(QString *userName, QString *domain = 0, QString *logonServer = 0, QString *errorMessage = 0);
    static void getAllUsersLoggedOn(QStringList *users, const QString &serverName = "", unsigned long *apiStatus = 0);

    static bool getCurrentModuleFileName(QString *path);

    static QStringList localCreatedUsers();
    static bool getLocalGroupsTheUserBelongs(QStringList *groups, const QString &userName = "", unsigned long *errorCode = 0);
    static bool getAllUsersInfo(QJsonArray *jsonArray, unsigned long *errorCode = 0);

    static bool serviceGetAllServicesInfo(QJsonArray *jsonArray, unsigned long *errorCode = 0, unsigned long serviceType = 0);

    static bool shutdown(const QString &machineName, const QString &message, unsigned long timeout, bool forceAppsClosed, bool rebootAfterShutdown, QString *errorMessage = 0);
    static bool setComputerName(const QString &newComputerName, const QString &adminName, const QString &adminPassword, unsigned long *errorCode = 0, QString *errorMessage = 0);

    static bool getOSInfo(QJsonObject *object);


public slots:


};

} //namespace HEHUI

#endif // SYSTEMUTILITIES_H
