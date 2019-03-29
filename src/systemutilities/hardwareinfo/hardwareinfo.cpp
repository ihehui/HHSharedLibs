#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDate>
#include <QProcess>


#include "hardwareinfo.h"

#ifdef Q_OS_WIN32
    #include "hardwareinfo_win.h"
#else
    #include "hardwareinfo_unix.h"
#endif


namespace HEHUI {


HardwareInfo::HardwareInfo(QObject *parent) : QObject(parent)
{

}

HardwareInfo::~HardwareInfo()
{

}

QString HardwareInfo::getCPUTemperature(){

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getCPUTemperature();
}


bool HardwareInfo::getCPUInfo(int *numberOfLogicalProcessors, int *numberOfProcessorCores, int *numberOfPhysicalProcessorPackages, QString *modelName)
{

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getCPUInfo(numberOfLogicalProcessors, numberOfProcessorCores, numberOfPhysicalProcessorPackages, modelName);
}

QString HardwareInfo::getHardDiskTemperature(){

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getHardDiskTemperature();
}

float HardwareInfo::getMotherBoardTemperature(){

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getMotherBoardTemperature();
}

QString HardwareInfo::monitorID(const QString &pnpDeviceID){
    if(pnpDeviceID.trimmed().isEmpty()){return "";}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.monitorID(pnpDeviceID);
}

//bool HardwareInfo::getOSInfo(QJsonObject *object){
//    if(!object){return false;}

//#ifdef Q_OS_WIN32
//    HardwareInfoWin hw;
//#else
//    HardwareInfoUnix hw;
//#endif

//    return hw.getOSInfo(object);
//}

bool HardwareInfo::getBaseBoardInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getBaseBoardInfo(object);
}

bool HardwareInfo::getProcessorInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getProcessorInfo(object);
}

bool HardwareInfo::getPhysicalMemoryInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getPhysicalMemoryInfo(object);
}

bool HardwareInfo::getDiskDriveInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getDiskDriveInfo(object);
}

bool HardwareInfo::getVideoControllerInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getVideoControllerInfo(object);
}

bool HardwareInfo::getSoundDeviceInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getSoundDeviceInfo(object);
}

bool HardwareInfo::getMonitorInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getMonitorInfo(object);
}

bool HardwareInfo::getNetworkAdapterInfo(QJsonObject *object){
    if(!object){return false;}

#ifdef Q_OS_WIN32
    HardwareInfoWin hw;
#else
    HardwareInfoUnix hw;
#endif

    return hw.getNetworkAdapterInfo(object);
}



} //namespace HEHUI
