#include <QDebug>

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


namespace HEHUI {


HardwareMonitor::HardwareMonitor(QObject *parent) : QObject(parent)
{

    m_initialized = false;

    //InitializeOls();

    if(!InitOpenLibSys(&hModule)){
        qCritical()<<"ERROR! Initialization failed.";
        return;
    }

    DWORD status = GetDllStatus();
    switch (status) {
    case OLS_DLL_NO_ERROR:
        qDebug()<<"No error";
        m_initialized = true;
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

}

HardwareMonitor::~HardwareMonitor()
{
    //DeinitializeOls();
    DeinitOpenLibSys(&hModule);
}

QString HardwareMonitor::getCPUTemperature(){
    if(!m_initialized){
        return "";
    }

    if(!IsMsr()){
        return "";
    }

    int numberOfLogicalProcessors = 0;
    getCPUInfo(&numberOfLogicalProcessors);

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

    for(int i=0;i<numberOfLogicalProcessors;i++){
        mask = 1<<i;
        result=SetThreadAffinityMask(GetCurrentThread(), mask);
        qDebug()<<"----------result:"<<result;

        Rdmsr(0x19c,&eax,&edx);//read Temperature
        SetThreadAffinityMask(GetCurrentThread(),result);
        results.append(QString::number(100-((eax&0x007f0000)>>16)));
    }

    return results.join(",");

}

int HardwareMonitor::getCPUTemperature2(int processorIndex){
    if(!m_initialized){
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

    qDebug()<<"\nGetLogicalProcessorInformation results:\n";
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







} //namespace HEHUI
