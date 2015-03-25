#ifndef HARDWAREMONITOR_H
#define HARDWAREMONITOR_H

#include <QObject>
#include "../wmlib.h"

#include "Windows.h"



namespace HEHUI {
class WMIQuery;


class WM_LIB_API HardwareMonitor : public QObject
{
    Q_OBJECT
public:
    explicit HardwareMonitor(QObject *parent = 0);
    ~HardwareMonitor();

    QString getCPUTemperature();
    int getCPUTemperature2(int processorIndex = 0);
    bool getCPUInfo(int *numberOfLogicalProcessors, int *numberOfProcessorCores = 0, int *numberOfPhysicalProcessorPackages = 0);

    //All harddisk temperatures are separted by ','
    QString getHardDiskTemperature();

    float getMotherBoardTemperature();

    static QString WinOSProductKey();

    QString EDIDBinToChr(const QString &bin);
    QString MonitorID(const QString &pnpDeviceID);

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
    bool initWinRing0();
    void initWMIQuery();

    DWORD CountSetBits(ULONG_PTR bitMask);

private:
    bool m_winRing0Initialized;
    HMODULE hModule;
    int numberOfLogicalProcessors;

    WMIQuery *m_wmiQuery;

};

} //namespace HEHUI

#endif // HARDWAREMONITOR_H
