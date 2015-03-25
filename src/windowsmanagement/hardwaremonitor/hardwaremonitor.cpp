#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDate>

#include "hardwaremonitor.h"

//Load-Time Dynamic Linking
//#include "OlsApi.h"
//#define OLS_DLL_NO_ERROR						0
//#define OLS_DLL_UNSUPPORTED_PLATFORM			1
//#define OLS_DLL_DRIVER_NOT_LOADED				2
//#define OLS_DLL_DRIVER_NOT_FOUND				3
//#define OLS_DLL_DRIVER_UNLOADED					4
//#define OLS_DLL_DRIVER_NOT_LOADED_ON_NETWORK	5
//#define OLS_DLL_UNKNOWN_ERROR					9


//Run-Time Dynamic Linking
#include "OlsApiInit.h"

#include "activex/wmiquery.h"

#include "../winutilities.h"


namespace HEHUI {


HardwareMonitor::HardwareMonitor(QObject *parent) : QObject(parent)
{

    m_winRing0Initialized = false;
    numberOfLogicalProcessors = 0;
    m_wmiQuery = 0;

}

HardwareMonitor::~HardwareMonitor()
{

    if(m_winRing0Initialized){
        //DeinitializeOls();
        DeinitOpenLibSys(&hModule);
    }

    if(m_wmiQuery){
        delete m_wmiQuery;
    }

}

bool HardwareMonitor::initWinRing0(){

    if(m_winRing0Initialized){
        return true;
    }

    //InitializeOls();

    if(!InitOpenLibSys(&hModule)){
        qCritical()<<"ERROR! Initialization failed.";
        return false;
    }

    DWORD status = GetDllStatus();
    switch (status) {
    case OLS_DLL_NO_ERROR:
        m_winRing0Initialized = true;
        break;
    case OLS_DLL_UNSUPPORTED_PLATFORM:
        qCritical()<<"Unsupported Platform";
        break;
    case OLS_DLL_DRIVER_NOT_LOADED:
        qCritical()<<"Driver not loaded";
        break;
    case OLS_DLL_DRIVER_NOT_FOUND:
        qCritical()<<"Driver not found";
        break;
    case OLS_DLL_DRIVER_UNLOADED:
        qCritical()<<"Driver unloaded by other process";
        break;
    case OLS_DLL_DRIVER_NOT_LOADED_ON_NETWORK:
        qCritical()<<"Driver not loaded because of executing on Network Drive (1.0.8 or later) ";
        break;
    case OLS_DLL_UNKNOWN_ERROR:
        qCritical()<<"Unknown error ";
        break;
    default:
        break;
    }

    return true;

}

void HardwareMonitor::initWMIQuery(){
    if(!m_wmiQuery){
        m_wmiQuery = new WMIQuery(this);
    }
}

QString HardwareMonitor::getCPUTemperature(){

    initWinRing0();
    if(!m_winRing0Initialized){
        return "";
    }

    if(!IsMsr()){
        return "";
    }


    DWORD eax=0,edx=0;
    ULONG result;

    DWORD mask = 0;
    //SetThreadAffinityMask(GetCurrentThread(),dwProcessAffinityMask);
    //dwProcessAffinityMask为下面的DWORD值，在msdn上面查的
    //0x0001 1
    //0x0002 2
    //0x0003 1 or 2
    //0x0004 3
    //0x0005 1 or 3
    //0x0007 1, 2, or 3
    //0x000F 1, 2, 3, or 4

    //0x0001 00000000 00000001 1
    //0x0003 00000000 00000011 1 and 2
    //0x0007 00000000 00000111 1, 2 and 3
    //0x0009 00000000 00001001 1 and 4
    //0x007F 00000000 01111111 1, 2, 3, 4, 5, 6 and 7

    QStringList results;
    getCPUInfo(&numberOfLogicalProcessors);

    for(int i=0;i<numberOfLogicalProcessors;i++){
        mask = 1<<i;
        result=SetThreadAffinityMask(GetCurrentThread(), mask);

        Rdmsr(0x19c,&eax,&edx);//read Temperature
        SetThreadAffinityMask(GetCurrentThread(),result);
        results.append(QString::number(100-((eax&0x007f0000)>>16)));
    }

    return results.join(",");

}

int HardwareMonitor::getCPUTemperature2(int processorIndex){

    initWinRing0();
    if(!m_winRing0Initialized){
        return 0;
    }

    if(!IsMsr()){
        return 0;
    }

    DWORD mask = 1<<processorIndex;
    //SetThreadAffinityMask(GetCurrentThread(),dwProcessAffinityMask);
    //dwProcessAffinityMask为下面的DWORD值，在msdn上面查的
    //0x0001 1
    //0x0002 2
    //0x0003 1 or 2
    //0x0004 3
    //0x0005 1 or 3
    //0x0007 1, 2, or 3
    //0x000F 1, 2, 3, or 4

    //0x0001 00000000 00000001 1
    //0x0003 00000000 00000011 1 and 2
    //0x0007 00000000 00000111 1, 2 and 3
    //0x0009 00000000 00001001 1 and 4
    //0x007F 00000000 01111111 1, 2, 3, 4, 5, 6 and 7

    DWORD eax=0,edx=0;
    ULONG result;
    result=SetThreadAffinityMask(GetCurrentThread(), mask);
    Rdmsr(0x19c,&eax,&edx);//read Temperature
    SetThreadAffinityMask(GetCurrentThread(),result);

    return 100-((eax&0x007f0000)>>16);
}


// Helper function to count set bits in the processor mask.
DWORD HardwareMonitor::CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);
bool HardwareMonitor::getCPUInfo(int *numberOfLogicalProcessors, int *numberOfProcessorCores, int *numberOfPhysicalProcessorPackages)
{
    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;

    glpi = (LPFN_GLPI) GetProcAddress(
                GetModuleHandle(TEXT("kernel32")),
                "GetLogicalProcessorInformation");
    if (NULL == glpi)
    {
        qCritical()<<"\nGetLogicalProcessorInformation is not supported.";
        return false;
    }

    while (!done)
    {
        DWORD rc = glpi(buffer, &returnLength);

        if (FALSE == rc)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (buffer)
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                            returnLength);

                if (NULL == buffer)
                {
                    qCritical()<<"\nError: Allocation failure.";
                    return false;
                }
            }
            else
            {
                qCritical()<<"Error:" << GetLastError();
                return false;
            }
        }
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
    {
        switch (ptr->Relationship)
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            numaNodeCount++;
            break;

        case RelationProcessorCore:
            processorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.
            Cache = &ptr->Cache;
            if (Cache->Level == 1)
            {
                processorL1CacheCount++;
            }
            else if (Cache->Level == 2)
            {
                processorL2CacheCount++;
            }
            else if (Cache->Level == 3)
            {
                processorL3CacheCount++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            qCritical()<<"Error: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.";
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    qDebug()<<"\nGetLogicalProcessorInformation results:";
    qDebug()<<"Number of NUMA nodes: " << numaNodeCount;
    qDebug()<<"Number of physical processor packages: " << processorPackageCount;
    qDebug()<<"Number of processor cores: " << processorCoreCount;
    qDebug()<<"Number of logical processors: " << logicalProcessorCount;
    qDebug()<<"Number of processor L1/L2/L3 caches: "
           <<processorL1CacheCount<<"/"
          <<processorL2CacheCount<<"/"
         <<processorL3CacheCount;

    free(buffer);

    if(numberOfLogicalProcessors){
        *numberOfLogicalProcessors = logicalProcessorCount;
    }

    if(numberOfProcessorCores){
        *numberOfProcessorCores = processorCoreCount;
    }

    if(numberOfPhysicalProcessorPackages){
        *numberOfPhysicalProcessorPackages = processorPackageCount;
    }

    return true;
}

QString HardwareMonitor::getHardDiskTemperature(){
    initWMIQuery();

    QString queryString = QString("SELECT VendorSpecific FROM MSStorageDriver_ATAPISmartData ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "VendorSpecific", "ROOT/WMI");
    if(list.isEmpty()){return "";}

    QStringList results;
    foreach (QVariantList variantList, list) {
        if(variantList.size() != 1){return "";}
        QByteArray ba = variantList.at(0).toByteArray();

        unsigned char temp;
        for(int i=0;i<ba.size();i++){
            temp = ba[i];
            if(temp == 0xc2){
                temp = ba[i+5];
                results.append(QString::number(temp));
                break;
            }

        }

    }

    return results.join(",");

}

float HardwareMonitor::getMotherBoardTemperature(){
    initWMIQuery();

    QString queryString = QString("SELECT CurrentTemperature FROM MSAcpi_ThermalZoneTemperature ");
    qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "CurrentTemperature", "ROOT/WMI");
    if(list.isEmpty()){return 0;}

    foreach (QVariantList variantList, list) {
        if(variantList.size() != 1){return 0;}

        float nTemperature = variantList.at(0).toFloat();
        nTemperature = (nTemperature - 2732)/10;

        return nTemperature;
    }

    return 0;

}

QString HardwareMonitor::WinOSProductKey(){

    ////See:http://www.codeproject.com/Articles/15261/WebControls/

    QString value;
    bool ok = WinUtilities::regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "DigitalProductId", &value, true);
    if(!ok){
        qCritical()<<"ERROR! Can not read registry.";
        return "";
    }
    QByteArray byteBuffer = QByteArray::fromHex(value.toLatin1());
    if(byteBuffer.isEmpty()){return "";}

    BYTE   *DigitalProductID;
    BYTE ProductKeyExtract [15]; //Extract Key
    char sCDKey  [256];   //Temp, adding a Window Product Key
    long ByteCounter;    //Counter
    long ByteConvert;    //Convert
    int  nCur;      //XOR calculate

    char *KeyChars[] = {
        "B","C","D","F","G","H","J","K","M",
        "P","Q","R","T","V","W","X","Y",
        "2","3","4","6","7","8","9",NULL
    };

    DWORD DataLength = 164;

    //Allocate Memory
    DigitalProductID = (BYTE *)malloc(DataLength);

    //Memory Initializationd
    memset(DigitalProductID, 0, DataLength);

    DigitalProductID = (BYTE *)byteBuffer.data();

    //reading a value start position 52, by 66
    for(ByteCounter=52; ByteCounter<=66; ByteCounter++)
    {
        ProductKeyExtract[ByteCounter - 52] =
                DigitalProductID[ByteCounter];
    }
    //Last Indexer
    ProductKeyExtract[sizeof(ProductKeyExtract)] = NULL;

    memset(sCDKey, 0, sizeof(sCDKey));
    for(ByteCounter=24; ByteCounter>=0; ByteCounter--)
    {
        nCur = 0;

        for(ByteConvert=14; ByteConvert>=0; ByteConvert--)
        {
            nCur = (nCur * 256) ^ ProductKeyExtract[ByteConvert];  //XOR&#44228;&#49328;
            ProductKeyExtract[ByteConvert] = nCur / 24;
            nCur = nCur % 24;
        }

        _strrev(sCDKey);
        strcat_s(sCDKey, KeyChars[nCur]);
        _strrev(sCDKey);

        //Insert "-"
        if(!(ByteCounter % 5) && (ByteCounter))
        {
            _strrev(sCDKey);
            strcat_s(sCDKey, "-");
            _strrev(sCDKey);
        }
    }

    return QString::fromLatin1(sCDKey);
}

QString HardwareMonitor::EDIDBinToChr(const QString &bin){
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

QString HardwareMonitor::MonitorID(const QString &pnpDeviceID){

    QString value;
    bool ok = WinUtilities::regRead("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Enum\\" + pnpDeviceID + "\\Device Parameters", "EDID", &value);
    if(!ok){
        qCritical()<<"ERROR! Can not read registry.";
        return "";
    }
    QByteArray byteBuffer = QByteArray::fromHex(value.toLatin1());
    if(byteBuffer.isEmpty()){return "";}

    QByteArray productID;
    productID.append(byteBuffer[11]);
    productID.append(byteBuffer[10]);

    QByteArray manufacturerID;
    manufacturerID.append(byteBuffer[9]);
    manufacturerID.append(byteBuffer[8]);

    quint16 num = 0;
    memcpy(&num, manufacturerID.data(), 2);

    QString monitorID = QString::number(num, 2);
    monitorID = monitorID.rightJustified(15, '0');

    monitorID = EDIDBinToChr(monitorID.left(5)) + EDIDBinToChr(monitorID.mid(5,5)) + EDIDBinToChr(monitorID.right(5));
    monitorID += productID.toHex().toUpper();

    return monitorID;
}


bool HardwareMonitor::getOSInfo(QJsonObject *object){

    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_OperatingSystem ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "Caption,CSDVersion,OSArchitecture,MUILanguages,InstallDate", "ROOT/CIMV2");
    if(list.isEmpty()){return false;}

    QVariantList variantList = list.at(0);
    if(variantList.size() != 5){return false;}


    QString caption = variantList.at(0).toString();
    QString csdVersion = variantList.at(1).toString();
    QString osArchitecture = variantList.at(2).toString();
    QString muiLanguages = variantList.at(3).toString();
    object->insert("OS", caption + " " + csdVersion + " " + osArchitecture + " " + muiLanguages);

    QDate date = QDate::fromString(variantList.at(4).toString().left(8), "yyyyMMdd");
    object->insert("InstallDate", date.toString("yyyy.MM.dd"));

    object->insert("Key", WinOSProductKey());

    return true;
}

bool HardwareMonitor::getBaseBoardInfo(QJsonObject *object){
    qDebug()<<"--HardwareMonitor::getBaseBoardInfo(...)";

    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT Manufacturer,Product FROM Win32_BaseBoard ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "Manufacturer,Product", "ROOT/CIMV2");
    if(list.isEmpty()){return false;}

    QVariantList variantList = list.at(0);
    if(variantList.size() != 2){return false;}

    object->insert("BaseBoard", variantList.at(0).toString() + " " + variantList.at(1).toString());

    return true;

}

bool HardwareMonitor::getProcessorInfo(QJsonObject *object){
    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_Processor ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "Name,SocketDesignation", "ROOT/CIMV2");
    if(list.isEmpty()){return false;}

    QVariantList variantList = list.at(0);
    if(variantList.size() != 2){return false;}

    object->insert("Processor", variantList.at(0).toString() + " " + variantList.at(1).toString());

    return true;
}

bool HardwareMonitor::getPhysicalMemoryInfo(QJsonObject *object){
    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_PhysicalMemory ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "Manufacturer,Capacity,Speed", "ROOT/CIMV2");
    if(list.isEmpty()){return false;}

    QStringList memoryList;

    foreach (QVariantList variantList, list) {
        if(variantList.size() != 3){return false;}

        QString manufacturer = variantList.at(0).toString();
        int capacity = (variantList.at(1).toLongLong())/(1024*1024);
        QString capacityString = QString::number(capacity) + "MB";
        if(capacity >= 1024){
            capacityString = QString::number(capacity/1024) + "GB";
        }
        QString Speed = variantList.at(2).toString();

        memoryList.append(manufacturer + "(" + capacityString + "," + Speed + ")");
    }

    object->insert("PhysicalMemory", memoryList.join(";"));

    return true;
}

bool HardwareMonitor::getDiskDriveInfo(QJsonObject *object){
    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_DiskDrive ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "Caption,Size", "ROOT/CIMV2");
    if(list.isEmpty()){return false;}

    QStringList diskList;

    foreach (QVariantList variantList, list) {
        if(variantList.size() != 2){return false;}

        QString caption = variantList.at(0).toString();
        int size = (variantList.at(1).toLongLong())/(1024*1024*1024);
        QString sizeString = QString::number(size) + "GB";

        diskList.append(caption + "(" + sizeString + ")");
    }

    object->insert("DiskDrive", diskList.join(";"));

    return true;
}

bool HardwareMonitor::getVideoControllerInfo(QJsonObject *object){
    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_VideoController ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "VideoProcessor", "ROOT/CIMV2");
    if(list.isEmpty()){return false;}

    QVariantList variantList = list.at(0);
    if(variantList.size() != 1){return false;}

    object->insert("VideoController", variantList.at(0).toString());

    return true;
}

bool HardwareMonitor::getSoundDeviceInfo(QJsonObject *object){
    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_SoundDevice ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "ProductName", "ROOT/CIMV2");
    if(list.isEmpty()){return false;}

    QVariantList variantList = list.at(0);
    if(variantList.size() != 1){return false;}

    object->insert("SoundDevice", variantList.at(0).toString());

    return true;
}

bool HardwareMonitor::getMonitorInfo(QJsonObject *object){
    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_DesktopMonitor WHERE PNPDeviceID IS NOT NULL ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "PNPDeviceID", "ROOT/CIMV2");
    if(list.isEmpty()){
        qDebug()<<"WMI query result is empty.";
        return false;
    }

    QStringList monitorList;

    foreach (QVariantList variantList, list) {
        if(variantList.size() != 1){return false;}

        QString pnpDeviceID = variantList.at(0).toString();
        QString id = MonitorID(pnpDeviceID);
        if(!id.isEmpty()){
            monitorList.append(id);
        }
    }

    object->insert("Monitor", monitorList.join(";"));

    return true;
}

bool HardwareMonitor::getNetworkAdapterInfo(QJsonObject *object){
    if(!object){return false;}

    initWMIQuery();

    QString queryString = QString("SELECT * FROM Win32_NetworkAdapter WHERE MACAddress IS NOT NULL AND Manufacturer <> 'Microsoft' ");
    //qDebug()<<"queryString:"<<queryString;
    QList<QVariantList> list = m_wmiQuery->queryValues(queryString, "ProductName,MACAddress", "ROOT/CIMV2");
    if(list.isEmpty()){
        qDebug()<<"WMI query result is empty.";
        return false;
    }


    QStringList nicList;

    foreach (QVariantList variantList, list) {
        if(variantList.size() != 2){return false;}

        QString productName = variantList.at(0).toString();
        QString macAddress = variantList.at(1).toString();
        nicList.append(productName + "(" + macAddress + ")");
    }

    object->insert("NetworkAdapter", nicList.join(";"));

    return true;
}





} //namespace HEHUI
