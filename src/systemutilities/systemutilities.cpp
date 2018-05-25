#include "systemutilities.h"

#include <QProcess>
#include <QDebug>



#ifdef Q_OS_WIN32
#include "winutilities.h"
#else
#include "unixutilities.h"
#endif

namespace HEHUI
{


SystemUtilities::SystemUtilities(QObject *parent) : QObject(parent)
{

}

SystemUtilities::~SystemUtilities()
{

}

int SystemUtilities::getCPULoad()
{

#ifdef Q_OS_WIN32
    return WinUtilities::getCPULoad();
#else
    QProcess process;
    //QString cmdString = QString("top -n 1 |grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
    QString cmdString = QString("top -n 1 |grep Cpu | cut -d \",\" -f 4");
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return 0;
    }
    if(!process.waitForReadyRead()) {
        return 0;
    }
    QString idle = QString::fromLocal8Bit(process.readAllStandardOutput());
    if(!idle.endsWith("%id")) {
        return 0;
    }
    idle = idle.replace("%id", "");
    return idle.toUInt();

#endif

}

QString SystemUtilities::getCPUName()
{

#ifdef Q_OS_WIN32
    return WinUtilities::getCPUName();
#else

    QProcess process;
    QString cmdString = QString("cat /proc/cpuinfo |grep \"model name\" | cut -d \":\" -f 2");
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return QSysInfo::prettyProductName();
    }
    if(!process.waitForReadyRead()) {
        return QSysInfo::prettyProductName();
    }
    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();

#endif

}

QString SystemUtilities::getCPUSerialNumber()
{

#ifdef Q_OS_WIN32
    return WinUtilities::getCPUSerialNumber();
#else
    QProcess process;
    QString cmdString = QString("dmidecode -t 4 | grep ID"); //root only

    process.start(cmdString);
    if(!process.waitForFinished()) {
        return QSysInfo::prettyProductName();
    }
    if(!process.waitForReadyRead()) {
        return QSysInfo::prettyProductName();
    }
    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();

#endif

}

QString SystemUtilities::getHardDriveSerialNumber(unsigned int driveIndex)
{

#ifdef Q_OS_WIN32
    return WinUtilities::getHardDriveSerialNumber(driveIndex);
#else

    QProcess process;
    QString cmdString = QString("lsscsi | grep disk | awk '{print $7}'");
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return QSysInfo::prettyProductName();
    }
    if(!process.waitForReadyRead()) {
        return QSysInfo::prettyProductName();
    }
    QStringList disks =  QString::fromLocal8Bit(process.readAllStandardOutput()).split("\n");
    if(driveIndex >= (unsigned int)disks.size()){
        return "";
    }
    process.close();

    cmdString = QString("hdparm -i %1 | grep SerialNo | awk '{print $4}' | cut -d \"=\" -f 2").arg(disks.at(driveIndex)); //root only
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return QSysInfo::prettyProductName();
    }
    if(!process.waitForReadyRead()) {
        return QSysInfo::prettyProductName();
    }

    return QString::fromLocal8Bit(process.readAllStandardOutput());

#endif

}


bool SystemUtilities::getMemoryStatus(quint64 *totalBytes, int *loadPercentage)
{

#ifdef Q_OS_WIN32

    return WinUtilities::getMemoryStatus(totalBytes, loadPercentage);

#else
    QProcess process;
    //QString cmdString = QString("top -n 1 | grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
    //QString cmdString = QString("cat /proc/meminfo | grep Mem");
    QString cmdString = QString("top -n 1 | grep Mem");

    process.start(cmdString);
    if(!process.waitForFinished()) {
        return false;
    }
    if(!process.waitForReadyRead()) {
        return false;
    }
    QString memString = QString::fromLocal8Bit(process.readAllStandardOutput());
    if(!memString.startsWith("Mem:")) {
        return false;
    }
    memString = memString.replace("Mem:", "").simplified();
    QStringList list = memString.split(",");
    if(list.size() != 4) {
        return false;
    }
    QString str = list.at(0);
    quint64 totalMem = str.replace("total", "").simplified().toULongLong();
    str = list.at(0);
    quint64 usedMem = str.replace("used", "").simplified().toULongLong();
    if(!totalMem || !usedMem) {
        return false;
    }

    if(totalBytes) {
        *totalBytes = totalMem * 1024;
    }

    if(loadPercentage) {
        *loadPercentage = (int)(((float)usedMem / (*totalBytes)) * 100);
    }


    return true;
#endif

    return true;

}

bool SystemUtilities::getDiskPartionStatus(const QString &partionRootPath, float *totalBytes, float *freeBytes)
{

#ifdef Q_OS_WIN32

    return WinUtilities::getDiskPartionStatus(partionRootPath, totalBytes, freeBytes);

#else
    QProcess process;
    //QString cmdString = QString("df -l | grep %1 | cut -d " " -f 2-30");
    //QString cmdString = QString("cat /proc/meminfo | grep Mem");
    QString cmdString = QString("df -l | grep %1 | cut -d " " -f 2-30").arg(partionRootPath);

    process.start(cmdString);
    if(!process.waitForFinished()) {
        return false;
    }
    if(!process.waitForReadyRead()) {
        return false;
    }

    QString diskString = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    QStringList list = diskString.split(" ");
    list.removeAll(" ");
    if(list.size() != 5) {
        return false;
    }
    unsigned int total = list.at(0).toUInt();
    unsigned int free = list.at(2).toUInt();
    if(!total) {
        return false;
    }

    if(totalBytes) {
        *totalBytes = total * 1024;
    }

    if(freeBytes) {
        *freeBytes = free * 1024;
    }

#endif

    return true;

}

QString SystemUtilities::getDisksInfo()
{

    QString disksInfo = "";
#ifdef Q_OS_WIN32

    disksInfo = QObject::tr("Partion\tType\tSize\tAvailable\tUsage(%)\n");
    QStringList drives = WinUtilities::getLogicalDrives();
    foreach (QString drive, drives) {
        float totalBytes = 0, freeBytes = 0;
        WinUtilities::getDiskPartionStatus(drive, &totalBytes, &freeBytes);
        disksInfo += drive;
        disksInfo += "\t" + WinUtilities::getFileSystemName(drive);

        float total = ((float)( (int)(100 * (totalBytes / (1024 * 1024 * 1024))) )) / 100;
        float free = ((float)( (int) (100 * (freeBytes / (1024 * 1024 * 1024))) )) / 100;
        float usage = ((float)( (int) (100 * (100 * (totalBytes - freeBytes) / totalBytes)) )) / 100;

        disksInfo += "\t" + QString::number(total) + (total ? "GB" : "");
        disksInfo += "\t" + QString::number(free) + (free ? "GB" : "");
        disksInfo += "\t" + (total ? QString::number(usage) : "0");
        disksInfo += "\n";
    }


#else
    QProcess process;
    //QString cmdString = QString("df -l | grep %1 | cut -d " " -f 2-30");
    QString cmdString = QString("df -lhT");

    process.start(cmdString);
    if(!process.waitForFinished()) {
        return "";
    }
    if(!process.waitForReadyRead()) {
        return "";
    }

    disksInfo = QString::fromLocal8Bit(process.readAllStandardOutput());
    disksInfo = disksInfo.replace("Mounted on", "Mounted");

#endif

    return disksInfo;

}

QString SystemUtilities::getOSVersionInfo()
{

    QString osInfo;
#ifdef Q_OS_WIN
    if(!WinUtilities::windowsVersionName(&osInfo)) {
        osInfo =  QSysInfo::prettyProductName();
        QString bit = WinUtilities::windowsVersionName(&osInfo) ? tr("64-bit") : tr("32-bit");
        osInfo += " " + bit;
    }
#else

    QProcess process;
    QString cmdString = QString("lsb_release  -a | grep Description | cut -d \":\" -f 2");
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return QSysInfo::prettyProductName();
    }
    if(!process.waitForReadyRead()) {
        return QSysInfo::prettyProductName();
    }
    osInfo = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();


#endif

    return osInfo;
}

bool SystemUtilities::getLogonInfoOfCurrentUser(QString *userName, QString *domain, QString *logonServer, QString *errorMessage)
{

#ifdef Q_OS_WIN
    unsigned long apiStatus = 0;
    bool ok = WinUtilities::getLogonInfoOfCurrentUser(userName, domain, logonServer, &apiStatus);
    if( (!ok) && errorMessage){
        *errorMessage = WinUtilities::WinSysErrorMsg(apiStatus);
    }
    return ok;
#else

    QProcess process;
    QString cmdString = QString("whoami");

    process.start(cmdString);
    if(!process.waitForFinished()) {
        if(errorMessage){
            *errorMessage = process.errorString();
        }
        return false;
    }
    if(!process.waitForReadyRead()) {
        if(errorMessage){
            *errorMessage = process.errorString();
        }
        return false;
    }

    *userName = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();

#endif

    return true;

}

void SystemUtilities::getAllUsersLoggedOn(QStringList *users, const QString &serverName, unsigned long *apiStatus)
{
#ifdef Q_OS_WIN
    return WinUtilities::getAllUsersLoggedOn(users, serverName, apiStatus);
#else

    QProcess process;
    QString cmdString = QString("who | cut -d' ' -f1 | sort | uniq");

    process.start(cmdString);
    if(!process.waitForFinished()) {
        return;
    }
    if(!process.waitForReadyRead()) {
        return;
    }

    *users = QString::fromLocal8Bit(process.readAllStandardOutput()).split("\n");

#endif
}







} //namespace HEHUI
