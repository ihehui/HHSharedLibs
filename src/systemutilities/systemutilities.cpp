#include "systemutilities.h"

#include <QProcess>
#include <QDebug>



#ifdef Q_OS_WIN32
#include "winutilities.h"
#endif

namespace HEHUI {


SystemUtilities::SystemUtilities(QObject *parent) : QObject(parent)
{

}

SystemUtilities::~SystemUtilities()
{

}

int SystemUtilities::getCPULoad(){

#ifdef Q_OS_WIN32
    return WinUtilities::getCPULoad();
#else
    QProcess process;
    //QString cmdString = QString("top -n 1 |grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
    QString cmdString = QString("top -n 1 |grep Cpu | cut -d \",\" -f 4");
    process.start(cmdString);
    if(!process.waitForFinished()){
        return 0;
    }
    if(!process.waitForReadyRead()){
            return 0;
    }
    QString idle = QString::fromLocal8Bit(process->readAllStandardOutput());
    if(!idle.endsWith("%id")){
        return 0;
    }
    idle = idle.replace("%id", "");
    return idle.toUInt();

#endif

}

QString SystemUtilities::getCPUName(){

#ifdef Q_OS_WIN32
    return WinUtilities::getCPUName();
#else

#endif

}

QString SystemUtilities::getCPUSerialNumber(){

#ifdef Q_OS_WIN32
    return WinUtilities::getCPUSerialNumber();
#else

#endif

}

QString SystemUtilities::getHardDriveSerialNumber(unsigned int driveIndex){

#ifdef Q_OS_WIN32
    return WinUtilities::getHardDriveSerialNumber(driveIndex);
#else

#endif

}


bool SystemUtilities::getMemoryStatus(quint64 *totalBytes, int *loadPercentage){

#ifdef Q_OS_WIN32

    return WinUtilities::getMemoryStatus(totalBytes, loadPercentage);

#else
    QProcess process;
    //QString cmdString = QString("top -n 1 | grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
    //QString cmdString = QString("cat /proc/meminfo | grep Mem");
    QString cmdString = QString("top -n 1 | grep Mem");

    process.start(cmdString);
    if(!process.waitForFinished()){
        return false;
    }
    if(!process.waitForReadyRead()){
            return false;
    }
    QString memString = QString::fromLocal8Bit(process->readAllStandardOutput());
    if(!memString.startsWith("Mem:")){
        return false;
    }
    memString = memString.replace("Mem:", "").simplified();
    QStringList list = memString.split(",");
    if(list.size() != 4){return false;}
    unsigned int totalMem = list.at(0).replace("total", "").simplified().toUInt();
    unsigned int usedMem = list.at(1).replace("used", "").simplified().toUInt();
    if(!totalMem || !usedMem){
        return false;
    }

    if(totalBytes){
        *totalBytes = totalMem*1024;
    }

    if(loadPercentage){
        *loadPercentage = usedMem/totalBytes;
    }


    return true;
#endif

    return true;

}

QString SystemUtilities::getOSVersionInfo(){

    QString osInfo;
#ifdef Q_OS_WIN
    if(!WinUtilities::windowsVersionName(&osInfo)){
        osInfo =  QSysInfo::prettyProductName();
        QString bit = WinUtilities::windowsVersionName(&osInfo)?tr("64-bit"):tr("32-bit");
        osInfo += " " + bit;
    }
#else

    QProcess process;
    QString cmdString = QString("lsb_release  -a | grep Description | cut -d ":" -f 2");

    process.start(cmdString);
    if(!process.waitForFinished()){
        return QSysInfo::prettyProductName();
    }
    if(!process.waitForReadyRead()){
        return QSysInfo::prettyProductName();
    }
    osInfo = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();


#endif

    return osInfo;
}






} //namespace HEHUI
