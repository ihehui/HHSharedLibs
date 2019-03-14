#ifndef HARDWAREINFO_UNIX_H
#define HARDWAREINFO_UNIX_H

#include <QObject>
#include "../systemutilitieslib.h"




namespace HEHUI
{


class SYSUTIL_LIB_API HardwareInfoUnix : public QObject
{
    Q_OBJECT
public:
    explicit HardwareInfoUnix(QObject *parent = 0);
    ~HardwareInfoUnix();

    QString getCPUTemperature();
    bool getCPUInfo(int *numberOfLogicalProcessors, int *numberOfProcessorCores = 0, int *numberOfPhysicalProcessorPackages = 0, QString *modelName = 0);

    //All harddisk temperatures are separted by ','
    QString getHardDiskTemperature();

    float getMotherBoardTemperature();


    QString EDIDBinToChr(const QString &bin);
    QString monitorID(const QString &pnpDeviceID);

    bool getOSInfo(QJsonObject *object);
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

    bool getProcessOutput(const QString &cmd, QByteArray *output);


private:


};


} //namespace HEHUI

#endif // HARDWAREINFO_UNIX_H
