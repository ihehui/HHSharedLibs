#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDate>
#include <QProcess>


#include "hardwareinfo_unix.h"
#include "../unixutilities.h"


namespace HEHUI {


HardwareInfoUnix::HardwareInfoUnix(QObject *parent) : QObject(parent)
{


}

HardwareInfoUnix::~HardwareInfoUnix()
{


}

QString HardwareInfoUnix::getCPUTemperature(){

    QStringList results;

    return results.join(",");

}


bool HardwareInfoUnix::getCPUInfo(int *numberOfLogicalProcessors, int *numberOfProcessorCores, int *numberOfPhysicalProcessorPackages, QString *modelName)
{

    QProcess process;
    QString cmdString = QString("sh -c \"cat /proc/cpuinfo | grep 'model name' | cut -f2 -d:\"");
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return false;
    }

    QByteArray output = process.readAllStandardOutput();
    QString line = "";
    QTextStream in(&output);
    QString name = "";
    int cores = 0;
    while (in.readLineInto(&line)) {
        if(line.trimmed().isEmpty()){continue;}
        name = line.trimmed();
        cores++;
    }

    qDebug()<<"Processor Information:";
    qDebug()<<"Model Name: " << name;
    qDebug()<<"Number of processor cores: " << cores;


    if(numberOfLogicalProcessors){
        *numberOfLogicalProcessors = cores;
    }

    if(numberOfProcessorCores){
        *numberOfProcessorCores = cores;
    }

    if(numberOfPhysicalProcessorPackages){
        *numberOfPhysicalProcessorPackages = cores;
    }

    if(modelName){
        *modelName = name;
    }

    return true;
}

QString HardwareInfoUnix::getHardDiskTemperature(){

    QStringList results;


    return results.join(",");
}

float HardwareInfoUnix::getMotherBoardTemperature(){


    return 0;
}

QString HardwareInfoUnix::EDIDBinToChr(const QString &bin){
    QHash<QString, QString> hash;
    hash.insert("00001", "A");
    hash.insert("00010", "B");
    hash.insert("00011", "C");
    hash.insert("00100", "D");
    hash.insert("00101", "E");
    hash.insert("00110", "F");
    hash.insert("00111", "G");
    hash.insert("01000", "H");
    hash.insert("01001", "I");
    hash.insert("01010", "J");
    hash.insert("01011", "K");
    hash.insert("01100", "L");
    hash.insert("01101", "M");
    hash.insert("01110", "N");
    hash.insert("01111", "O");
    hash.insert("10000", "P");
    hash.insert("10001", "Q");
    hash.insert("10010", "R");
    hash.insert("10011", "S");
    hash.insert("10100", "T");
    hash.insert("10101", "U");
    hash.insert("10110", "V");
    hash.insert("10111", "W");
    hash.insert("11000", "X");
    hash.insert("11001", "Y");
    hash.insert("11010", "Z");

    return hash.value(bin);

}

QString HardwareInfoUnix::monitorID(const QString &pnpDeviceID){
    return "";
}

//bool HardwareInfoUnix::getOSInfo(QJsonObject *object){
//    if(!object){return false;}

//    QString osInfo;

////    QProcess process;
////    QString cmdString = QString("lsb_release  -a | grep Description | cut -d \":\" -f 2");
////    process.start("sh", QStringList()<<"-c"<<cmdString);
////    if(!process.waitForStarted()) {
////        osInfo = QSysInfo::prettyProductName();
////    }
////    if(!process.waitForFinished()) {
////        osInfo = QSysInfo::prettyProductName();
////    }
////    osInfo = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
////    if(osInfo.trimmed().isEmpty()){
////        osInfo = QSysInfo::prettyProductName();
////    }

//    QString unameInfo = QSysInfo::prettyProductName();
//    QProcess process;
//    process.start("uname -srm");
//    if(process.waitForStarted() && process.waitForFinished()) {
//        unameInfo = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
//        if(unameInfo.trimmed().isEmpty()){
//            unameInfo = QSysInfo::prettyProductName();
//        }
//    }

//    QString cmd = QString("sh -c \"lsb_release  -a | grep Description | cut -d \":\" -f 2\"");
//    process.start(cmd);
//    if(!process.waitForStarted() || (!process.waitForFinished())) {
//        osInfo = unameInfo;
//    }


//    osInfo = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
//    if(osInfo.trimmed().isEmpty()){
//        osInfo = unameInfo;
//    }

//    if(!unameInfo.contains(osInfo, Qt::CaseInsensitive)){
//        osInfo += QString("(%1)").arg(unameInfo);
//    }

//    object->insert("OS", osInfo);
//    object->insert("InstallDate", "");
//    object->insert("Key", "");

//    return true;
//}

bool HardwareInfoUnix::getBaseBoardInfo(QJsonObject *object){
    if(!object){return false;}

    QString cmdString = QString("dmidecode -t baseboard");
    QByteArray output;
    bool ok = getProcessOutput(cmdString, &output);
    if(!ok || output.trimmed().isEmpty()){
        return false;
    }

    QString line = "";
    QString manufacturer = "";
    QString productName = "";
    bool started = false;
    QStringList dataList;
    QTextStream in(&output);
    while (in.readLineInto(&line)) {
        line = line.trimmed();

//        if(line.startsWith("System Info:", Qt::CaseInsensitive)){
//            started = true;
//            continue;
//        }
//        if(!started){continue;}

        if(line.startsWith("Manufacturer:", Qt::CaseInsensitive)){
            dataList = line.split(":");
            if(2 == dataList.size()){
                manufacturer = dataList.at(1).trimmed().replace("\"", "");
            }

            continue;
        }

        if(line.startsWith("Product Name:", Qt::CaseInsensitive)){
            dataList = line.split(":");
            if(2 == dataList.size()){
                productName = dataList.at(1).trimmed().replace("\"", "");
            }

            continue;
        }

        if(!manufacturer.isEmpty() && (!productName.isEmpty())){}

    }

    object->insert("BaseBoard", manufacturer + " " + productName);

    return true;

}

bool HardwareInfoUnix::getProcessorInfo(QJsonObject *object){
    if(!object){return false;}

    QString modeName = "";
    getCPUInfo(0, 0, 0, &modeName);

    object->insert("Processor", modeName);

    return true;
}

bool HardwareInfoUnix::getPhysicalMemoryInfo(QJsonObject *object){
    if(!object){return false;}

    QString cmdString = QString("dmidecode -t memory");
    QByteArray output;
    bool ok = getProcessOutput(cmdString, &output);
    if(!ok || output.trimmed().isEmpty()){
        return false;
    }

    QStringList memInfoList;

    QString line = "";
    QString manufacturer = "";
    QString size = "";
    bool started = false;
    QStringList dataList;
    QTextStream in(&output);
    while (in.readLineInto(&line)) {
        line = line.trimmed();

        if(line.startsWith("Handle 0x", Qt::CaseInsensitive)){
            started = false;
            continue;
        }

        if(line.startsWith("Memory Device", Qt::CaseInsensitive)){
            started = true;
            continue;
        }
        if(!started){continue;}

        if(line.startsWith("Size:", Qt::CaseInsensitive)){
            dataList = line.split(":");
            if(2 == dataList.size()){
                size = dataList.at(1).trimmed().replace("\"", "");
            }

            if(size.startsWith("No", Qt::CaseInsensitive)){
                size = "";
                started = false;
            }

            continue;
        }

        if(line.startsWith("Manufacturer:", Qt::CaseInsensitive)){
            dataList = line.split(":");
            if(2 == dataList.size()){
                manufacturer = dataList.at(1).trimmed().replace("\"", "");
            }

            continue;
        }


        if(!manufacturer.isEmpty() && (!size.isEmpty())){
            memInfoList.append(manufacturer + " " + size);
        }

    }

    object->insert("PhysicalMemory", memInfoList.join(";"));

    return true;
}

bool HardwareInfoUnix::getDiskDriveInfo(QJsonObject *object){
    if(!object){return false;}

    QString cmdString = QString("lsscsi");
    QByteArray output;
    bool ok = getProcessOutput(cmdString, &output);
    if(!ok || output.trimmed().isEmpty()){
        return false;
    }

    QStringList diskList;

    QString line = "";
    QTextStream in(&output);
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        diskList.append(line);
    }

    object->insert("DiskDrive", diskList.join(";"));

    return true;
}

bool HardwareInfoUnix::getVideoControllerInfo(QJsonObject *object){
    if(!object){return false;}

    QString cmdString = QString("sh -c \"lspci | grep VGA\"");
    QByteArray output;
    bool ok = getProcessOutput(cmdString, &output);
    if(!ok || output.trimmed().isEmpty()){
        return false;
    }

    QStringList vcList;

    QString line = "";
    QStringList dataList;
    QTextStream in(&output);
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        dataList = line.split("VGA compatible controller:", QString::SkipEmptyParts, Qt::CaseInsensitive);
        if(2 == dataList.size()){
            vcList.append(dataList.at(1).trimmed());
        }
    }

    object->insert("VideoController", vcList.join(";"));

    return true;
}

bool HardwareInfoUnix::getSoundDeviceInfo(QJsonObject *object){
    if(!object){return false;}

    QString cmdString = QString("sh -c \"lspci | grep -i Audio\"");
    QByteArray output;
    bool ok = getProcessOutput(cmdString, &output);
    if(!ok || output.trimmed().isEmpty()){
        return false;
    }

    QStringList audioDeviceList;

    QString line = "";
    QStringList dataList;
    QTextStream in(&output);
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        dataList = line.split("Audio device:", QString::SkipEmptyParts, Qt::CaseInsensitive);
        if(2 == dataList.size()){
            audioDeviceList.append(dataList.at(1).trimmed());
        }
    }

    object->insert("SoundDevice", audioDeviceList.join(";"));

    return true;
}

bool HardwareInfoUnix::getMonitorInfo(QJsonObject *object){
    if(!object){return false;}



    object->insert("Monitor", "");

    return true;
}

bool HardwareInfoUnix::getNetworkAdapterInfo(QJsonObject *object){
    if(!object){return false;}

    QString cmdString = QString("sh -c \"lspci | grep -i -E 'ethernet controller|network controller'\"");
    QByteArray output;
    bool ok = getProcessOutput(cmdString, &output);
    if(!ok || output.trimmed().isEmpty()){
        return false;
    }

    QStringList nicList;

    QString line = "";
    QStringList dataList;
    QTextStream in(&output);
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if(line.contains("Ethernet controller:", Qt::CaseInsensitive)){
            dataList = line.split("Ethernet controller:", QString::SkipEmptyParts, Qt::CaseInsensitive);
            if(2 == dataList.size()){
                nicList.append(dataList.at(1).trimmed());
            }
        }

        if(line.contains("Network controller:", Qt::CaseInsensitive)){
            dataList = line.split("Network controller::", QString::SkipEmptyParts, Qt::CaseInsensitive);
            if(2 == dataList.size()){
                nicList.append(dataList.at(1).trimmed());
            }
        }
    }

    object->insert("NetworkAdapter", nicList.join(";"));

    return true;
}


bool HardwareInfoUnix::getProcessOutput(const QString &cmd, QByteArray *output)
{
    if(cmd.trimmed().isEmpty()){
        return false;
    }

    QProcess process;
    process.start(cmd);
    if(!process.waitForFinished()) {
        return false;
    }

    *output =  process.readAllStandardOutput();

    return true;
}



} //namespace HEHUI
