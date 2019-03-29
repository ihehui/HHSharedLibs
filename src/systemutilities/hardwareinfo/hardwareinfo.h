#ifndef HARDWAREINFO_H_
#define HARDWAREINFO_H_

#include <QObject>
#include "../systemutilitieslib.h"


namespace HEHUI
{


class SYSUTIL_LIB_API HardwareInfo : public QObject
{
    Q_OBJECT
public:
    explicit HardwareInfo(QObject *parent = 0);
    ~HardwareInfo();

    QString getCPUTemperature();
    bool getCPUInfo(int *numberOfLogicalProcessors, int *numberOfProcessorCores = 0, int *numberOfPhysicalProcessorPackages = 0, QString *modelName = 0);

    //All harddisk temperatures are separted by ','
    QString getHardDiskTemperature();

    float getMotherBoardTemperature();


    QString monitorID(const QString &pnpDeviceID);

//    bool getOSInfo(QJsonObject *object);
    bool getBaseBoardInfo(QJsonObject *object);
    bool getProcessorInfo(QJsonObject *object);
    bool getPhysicalMemoryInfo(QJsonObject *object);
    bool getDiskDriveInfo(QJsonObject *object);
    bool getVideoControllerInfo(QJsonObject *object);
    bool getSoundDeviceInfo(QJsonObject *object);
    bool getMonitorInfo(QJsonObject *object);
    bool getNetworkAdapterInfo(QJsonObject *object);



public slots:

private:



private:


};

} //namespace HEHUI

#endif // _HARDWARE_INFO_H_
