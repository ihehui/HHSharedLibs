#ifndef HARDWARINFO_WIN_H
#define HARDWARINFO_WIN_H

#include <QObject>
#include "../systemutilitieslib.h"

#include "Windows.h"



namespace HEHUI
{
class WMIQuery;


class SYSUTIL_LIB_API HardwareInfoWin : public QObject
{
    Q_OBJECT
public:
    explicit HardwareInfoWin(QObject *parent = 0);
    ~HardwareInfoWin();

    QString getCPUTemperature();
    int getCPUTemperature2(int processorIndex = 0);
    bool getCPUInfo(int *numberOfLogicalProcessors, int *numberOfProcessorCores = 0, int *numberOfPhysicalProcessorPackages = 0, QString *modelName = 0);

    //All harddisk temperatures are separted by ','
    QString getHardDiskTemperature();

    float getMotherBoardTemperature();

    static QString WinOSProductKey();

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

#endif // HARDWARINFO_WIN_H
