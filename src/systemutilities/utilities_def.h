#ifndef UTILITIES_DEF_H
#define UTILITIES_DEF_H



namespace HEHUI {

#ifndef Q_OS_WIN32

enum StartType{
    SERVICE_AUTO_START,
    SERVICE_DEMAND_START,
    SERVICE_DISABLED
};

#endif

//Service
typedef struct SERVICE_INFO {
    SERVICE_INFO()
    {
        processID = 0;
        serviceType = 0xFFFFFFFF;
        startType = 0xFFFFFFFF;
    }
    QString serviceName;
    QString displayName;
    unsigned long processID;
    QString description;
    unsigned long startType;
    QString account;
    QString dependencies;
    QString binaryPath;
    unsigned long serviceType;

} ServiceInfo;




}

#endif // UTILITIES_DEF_H
