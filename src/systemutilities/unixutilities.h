#ifndef UNIXUTILITIES_H
#define UNIXUTILITIES_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

#include "utilities_def.h"



typedef struct CPU_PACKED         //定义一个cpu occupy的结构体
{
    char name[20];             //定义一个char类型的数组名name有20个元素
    unsigned int user;        //定义一个无符号的int类型的user
    unsigned int nice;        //定义一个无符号的int类型的nice
    unsigned int system;    //定义一个无符号的int类型的system
    unsigned int idle;         //定义一个无符号的int类型的idle
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
}CPU_OCCUPY;

double cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n);
void get_cpuoccupy (CPU_OCCUPY *cpust);
double getCpuRate();
void getSysDrivesStatus(int *count, char info[][512]);
bool getHardDriveSerialNumber(char *drive, char *sn, int snMaxLength);     // drive: /dev/sda, Root NOLY!!!


/////////////////////////////////////////////////////////////////////////

class UnixUtilities : public QObject
{
    Q_OBJECT
public:
    explicit UnixUtilities(QObject *parent = 0);

    static int getCPULoad();
    static bool getMemoryStatus(quint64 *totalBytes, int *loadPercentage);

    static QStringList getDrivesStatus();
    static QString getDriveSN(const QString &drive); // drive: /dev/sda, Root NOLY!!!

    static bool getAllUsersLoggedOn(QStringList *users);
    static QStringList localCreatedUsers();
    static bool getLocalGroupsTheUserBelongs(QStringList *groups, const QString &userName = "", unsigned long *errorCode = 0);
    static bool getAllUsersInfo(QJsonArray *jsonArray, unsigned long *errorCode = 0);

    static bool serviceGetAllServicesInfo(QJsonArray *jsonArray, unsigned long *errorCode = 0, unsigned long serviceType = 0);

    static QString getOSVersionInfo();
    static bool getOSInfo(QJsonObject *object);


    static bool shutdown(const QString &machineName, const QString &message, unsigned long timeout, bool forceAppsClosed, bool rebootAfterShutdown, QString *errorMessage = 0);
    static bool setComputerName(const QString &newComputerName, const QString &rootPassword, unsigned long *errorCode = 0, QString *errorMessage = 0);


signals:

public slots:


};

#endif // UNIXUTILITIES_H
