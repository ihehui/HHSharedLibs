#ifndef SYSTEMUTILITIES_H
#define SYSTEMUTILITIES_H

#include <QObject>

#include "systemutilitieslib.h"


namespace HEHUI {


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

    static QString getOSVersionInfo();



public slots:


};

} //namespace HEHUI

#endif // SYSTEMUTILITIES_H
