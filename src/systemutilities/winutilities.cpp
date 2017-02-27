/****************************************************************************
* winutilities.cpp
*
* Created on: 2009-11-9
*     Author: 贺辉
*    License: LGPL
*    Comment:
*
*
*    =============================  Usage  =============================
*|
*|
*    ===================================================================
*
*
* This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
* WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************
*/

/*
  ***************************************************************************
  * Last Modified on: 2010-08-19
  * Last Modified by: 贺辉
  ***************************************************************************
*/




#include "winutilities.h"

#include <QHash>
#include <QImage>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

//#include <windows.h>
#include <Lm.h>
#include <Tlhelp32.h>
#include <Lmjoin.h>
#include <Userenv.h>
#include <Sddl.h>
#include <gdiplus.h>

#pragma comment(lib,"gdi32")
#pragma comment(lib,"gdiplus")
#pragma comment(lib, "netapi32.lib")

const int MaxUserAccountNameLength = 20;
const int MaxUserPasswordLength = LM20_PWLEN;
const int MaxUserCommentLength = 256;
const int MaxGroupNameLength = 256;

#include <string>
#include <sstream>
#pragma comment(lib, "user32.lib")
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#define PRODUCT_PROFESSIONAL	0x00000030
#define VER_SUITE_WH_SERVER	0x00008000


#include "WindowsAPI.h"



namespace HEHUI
{


WinUtilities::WinUtilities()
{
    // TODO Auto-generated constructor stub


}

WinUtilities::~WinUtilities()
{
    // TODO Auto-generated destructor stub
}


QString WinUtilities::WinSysErrorMsg(DWORD winErrorCode, DWORD dwLanguageId)
{
    //    wchar_t buffer[8192];
    //    ZeroMemory(buffer, 8192);

    //    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, winErrorCode, 0, buffer, 8192, 0);
    //    return QString::fromWCharArray(buffer).simplified();



    HMODULE hLib = 0;
    DWORD dwFlags;
    OSVERSIONINFOW os;
    wchar_t buffer[8192];
    DWORD cbBuffer;

    ZeroMemory(buffer, 8192);
    cbBuffer = 0;


    if(winErrorCode >= 2100 && winErrorCode <= 2999) {
        //Undocumented errors %NETWORK_ERROR_FIRST to %NETWORK_ERROR_LAST
        os.dwOSVersionInfoSize = sizeof(os);
        GetVersionExW(&os);
        if(os.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            hLib = LoadLibraryExW(L"NETMSG.DLL", 0, LOAD_LIBRARY_AS_DATAFILE);
        }
    } else if(winErrorCode >= 12000 && winErrorCode <= 12171) {
        //Undocumented errors %INTERNET_ERROR_FIRST to %NTERNET_ERROR_LAST
        hLib = LoadLibraryExW(L"WININET.DLL", 0, LOAD_LIBRARY_AS_DATAFILE);
    }

    dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK;
    if(hLib) {
        dwFlags = dwFlags | FORMAT_MESSAGE_FROM_HMODULE;
    }

    cbBuffer = FormatMessageW(dwFlags, hLib, winErrorCode, dwLanguageId, buffer, 8192, 0);
    if(hLib) {
        FreeLibrary(hLib);
    }

    if(cbBuffer) {
        return QString::fromWCharArray(buffer).simplified();
    } else {
        QString string = QString::number(winErrorCode, 16).toUpper();
        return QString(QObject::tr("Error:0x%1").arg(string.rightJustified(8, '0')));
    }


}

void WinUtilities::freeMemory()
{

#if defined(Q_OS_WIN32)
    //SetProcessWorkingSetSize(GetCurrentProcess(), 0xFFFFFFFF, 0xFFFFFFFF);
    SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
#endif

}

__int64 WinUtilities::CompareFileTime ( FILETIME time1, FILETIME time2 )
{
    __int64 a = time1.dwHighDateTime << 32 | time1.dwLowDateTime ;
    __int64 b = time2.dwHighDateTime << 32 | time2.dwLowDateTime ;
    return   (b - a);
}

int WinUtilities::getCPULoad()
{

    HANDLE hEvent;
    BOOL res ;
    FILETIME preidleTime;
    FILETIME prekernelTime;
    FILETIME preuserTime;

    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    res = GetSystemTimes( &idleTime, &kernelTime, &userTime );
    preidleTime = idleTime;
    prekernelTime = kernelTime;
    preuserTime = userTime ;

    hEvent = CreateEventW (NULL, FALSE, FALSE, NULL);
    WaitForSingleObject( hEvent, 1000);

    res = GetSystemTimes( &idleTime, &kernelTime, &userTime );

    int idle = CompareFileTime( preidleTime, idleTime);
    int kernel = CompareFileTime( prekernelTime, kernelTime);
    int user = CompareFileTime(preuserTime, userTime);

    //qDebug()<<"kernel:"<<kernel<<" user:"<<user<<" idle:"<<idle;

    if((kernel + user) == 0) {
        return 0;
    }

    int cpu = (kernel + user - idle) * 100 / (kernel + user);
    //int cpuidle = ( idle) *100/(kernel+user);
    //cout << "CPU Load:" << cpu << "%" << "      CPU Idle:" <<cpuidle << "%" <<endl;
    //qDebug()<<QString("CPU Load: %1%").arg(cpu);

    return cpu;

}

bool WinUtilities::getMemoryStatus(quint64 *totalBytes, int *loadPercentage)
{

    // Use to convert bytes to KB
    //#define DIV 1024

    // Specify the width of the field in which to print the numbers.
    // The asterisk in the format specifier "%*I64d" takes an integer
    // argument and uses it to pad and right justify the number.
    //#define WIDTH 7


    MEMORYSTATUSEX statex;

    statex.dwLength = sizeof (statex);

    if(!GlobalMemoryStatusEx(&statex)) {
        return false;
    }

    if(totalBytes) {
        *totalBytes = statex.ullTotalPhys;
    }

    if(loadPercentage) {
        *loadPercentage = statex.dwMemoryLoad;
    }

    //    printf ("There is %*ld percent of memory in use.\n",
    //              WIDTH, statex.dwMemoryLoad);
    //    printf ("There are %*I64d total Kbytes of physical memory.\n",
    //              WIDTH, statex.ullTotalPhys/DIV);
    //    printf ("There are %*I64d free Kbytes of physical memory.\n",
    //              WIDTH, statex.ullAvailPhys/DIV);
    //    printf ("There are %*I64d total Kbytes of paging file.\n",
    //              WIDTH, statex.ullTotalPageFile/DIV);
    //    printf ("There are %*I64d free Kbytes of paging file.\n",
    //              WIDTH, statex.ullAvailPageFile/DIV);
    //    printf ("There are %*I64d total Kbytes of virtual memory.\n",
    //              WIDTH, statex.ullTotalVirtual/DIV);
    //    printf ("There are %*I64d free Kbytes of virtual memory.\n",
    //              WIDTH, statex.ullAvailVirtual/DIV);

    //    // Show the amount of extended memory available.
    //    printf ("There are %*I64d free Kbytes of extended memory.\n",
    //              WIDTH, statex.ullAvailExtendedVirtual/DIV);

    return true;
}

bool WinUtilities::getDiskPartionStatus(const QString &partionRootPath, float *totalBytes, float *freeBytes)
{

    ULARGE_INTEGER freeBytesAvailableToUser, ulTotalBytes, totalFreeBytes;
    if(!GetDiskFreeSpaceExW(partionRootPath.toStdWString().c_str(), &freeBytesAvailableToUser, &ulTotalBytes, &totalFreeBytes)) {
        return false;
    }

    if(freeBytes) {
        *freeBytes = (float)freeBytesAvailableToUser.QuadPart;
    }
    if(totalBytes) {
        *totalBytes = (float)ulTotalBytes.QuadPart;
    }

    return true;
}

QString WinUtilities::getFileSystemName(const QString &filePath)
{

    QString path = "";
    QRegExp rxp;
    rxp.setCaseSensitivity(Qt::CaseInsensitive);
    if(filePath.startsWith("\\\\")) {
        rxp.setPattern("^\\\\\\\\([a-zA-Z0-9.]+\\\\{1}){2}");
    } else {
        rxp.setPattern("^[a-zA-Z]:(\\\\|/)");
    }
    if(rxp.indexIn(filePath) != -1) {
        path = rxp.cap(0);
    } else {
        qCritical() << QString("Invalid Root Path '%1' !").arg(filePath) << " " << rxp.errorString();
        return "";
    }


    DWORD size = 256;
    wchar_t *fileSystemNameBuffer = new wchar_t[size];

    bool ok = GetVolumeInformationW(path.toStdWString().c_str(), NULL, 0, NULL, NULL, NULL, fileSystemNameBuffer, size);
    if(!ok) {
        qDebug() << "Failed to get volume information! Error Code:" << GetLastError();
        return "";
    }

    QString fileSystemName = QString::fromWCharArray(fileSystemNameBuffer);
    delete [] fileSystemNameBuffer;

    //qDebug()<<QString("File System Name Of '%1': %2").arg(path).arg(fileSystemName);

    return fileSystemName;

}

QStringList WinUtilities::getLogicalDrives()
{

    DWORD bufferSize = 96;
    QByteArray drives;
    drives.resize(bufferSize);
    drives.fill(0);

    DWORD length = GetLogicalDriveStringsA(bufferSize, drives.data());
    drives.resize(length - 1);

    drives.replace('\0', ',');
    drives.append('\0');
    return QString(drives).split(",");

}

QString WinUtilities::getCPUName()
{
    char psn[64];
    cpu_getName(psn);
    return QString::fromLatin1(psn).trimmed();
}

QString WinUtilities::getCPUSerialNumber()
{
    char psn[64];
    cpu_getPSN(psn);
    return QString::fromLatin1(psn);
}

QString WinUtilities::getHardDriveSerialNumber(unsigned int driveIndex)
{
    char sn[64];
    GetPhysicDriveSerialNumber(0, sn, driveIndex);
    return QString::fromLatin1(sn).trimmed();
}




bool WinUtilities::getFileVersion(const QString &fileName, QStringList *predefinedVersionInfoList, DWORD *errorCode)
{
    qDebug() << "--WinUtilities::getFileVersion(...)";

    if(fileName.trimmed().isEmpty() || (!predefinedVersionInfoList)) {
        if(errorCode) {
            *errorCode = ERROR_INVALID_PARAMETER;
        }
        return false;
    }
    if(predefinedVersionInfoList->isEmpty()) {
        if(errorCode) {
            *errorCode = ERROR_INVALID_PARAMETER;
        }
        return false;
    }


    DWORD dwSize = 0;
    UINT uiSize = GetFileVersionInfoSize(fileName.toStdWString().c_str(), &dwSize);
    if (0 == uiSize) {
        if(errorCode) {
            *errorCode = GetLastError();
        }
        qCritical() << QString("ERROR! GetFileVersionInfoSize failed. Code: %1").arg(GetLastError());
        return false;
    }

    PTSTR pBuffer = new TCHAR[uiSize];
    if (NULL == pBuffer) {
        return false ;
    }

    memset((void *)pBuffer, 0, uiSize);

    if(!GetFileVersionInfoW(fileName.toStdWString().c_str(), 0, uiSize, (PVOID)pBuffer)) {
        if(errorCode) {
            *errorCode = GetLastError();
        }
        qCritical() << QString("ERROR! GetFileVersionInfo failed. Code: %1").arg(GetLastError());

        delete []pBuffer;
        return false;
    }

    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;
    LANGANDCODEPAGE *pLanguage = NULL;
    UINT  uiOtherSize = 0;
    //获取资源相关的 codepage 和language
    if (!VerQueryValueW(pBuffer, L"\\VarFileInfo\\Translation",
                        (PVOID *)&pLanguage, &uiOtherSize)) {
        if(errorCode) {
            *errorCode = GetLastError();
        }
        qCritical() << QString("ERROR! VerQueryValueW failed. Code: %1").arg(GetLastError());

        delete []pBuffer;
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    PVOID pTmp = NULL;   //Must be PVOID or LPVOID
    //OR:
    //wchar_t pTmp[MAX_PATH];
    //memset((void*)pTmp,0,sizeof(pTmp));

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    wchar_t SubBlock[MAX_PATH];
    memset((void *)SubBlock, 0, sizeof(SubBlock));

    for(UINT i = 0; i < (uiOtherSize / sizeof(LANGANDCODEPAGE)); i++ ) {

        ////The following are predefined version information Unicode strings.
        //   Comments InternalName ProductName
        //   CompanyName LegalCopyright ProductVersion
        //   FileDescription LegalTrademarks PrivateBuild
        //   FileVersion OriginalFilename SpecialBuild


        for(int j = 0; j < predefinedVersionInfoList->size(); j++) {
            QString predefinedVersionInfo = predefinedVersionInfoList->at(j);
            wsprintf(SubBlock,
                     L"\\StringFileInfo\\%04x%04x\\%s",
                     pLanguage[i].wLanguage,
                     pLanguage[i].wCodePage,
                     predefinedVersionInfo.toStdWString().c_str()
                    );

            // Retrieve ProductName for language and code page "i".
            VerQueryValueW(pBuffer, SubBlock, (PVOID *)&pTmp, &uiOtherSize);

            QString value = QString::fromWCharArray((wchar_t *)pTmp);
            qDebug() << QString("%1:%2").arg(predefinedVersionInfo).arg(value);

            (*predefinedVersionInfoList)[j] = value;
        }

        //        if(fileDescription){
        //            wsprintf(SubBlock, L"\\StringFileInfo\\%04x%04x\\FileDescription", pLanguage[i].wLanguage, pLanguage[i].wCodePage);
        //            // Retrieve file description for language and code page "i".
        //            VerQueryValueW(pBuffer, SubBlock, (PVOID*)&pTmp, &uiOtherSize);

        //            *fileDescription = QString::fromWCharArray((wchar_t*)pTmp);
        //            qDebug()<<"FileDescription:"<<QString::fromWCharArray((wchar_t*)pTmp);
        //        }


        break;
    }

    delete []pBuffer;
    pBuffer = NULL;


    if(errorCode) {
        *errorCode = ERROR_SUCCESS;
    }
    return true;

}



bool WinUtilities::setComputerName(const QString &newComputerName, DWORD *errorCode)
{

    if(newComputerName.trimmed().isEmpty()) {
        qCritical() << QString("Invalid computer name!");
        return false;
    }

    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\Tcpip\\Parameters", "NV Hostname", newComputerName, REG_SZ, true);

    //if (SetComputerNameExW(ComputerNamePhysicalDnsHostname, computerName)){
    if (SetComputerNameW(newComputerName.toStdWString().c_str())) {
        if(errorCode) {
            *errorCode = GetLastError();
        }
        return true;
    } else {
        qCritical() << QString("Can not set computer name to '%1'.").arg(newComputerName);
        return false;
    }

}

QString WinUtilities::getComputerName(DWORD *errorCode)
{
    qDebug() << "--WinUtilities::getComputerName()";

    QString computerName = "";
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    LPWSTR name = new wchar_t[size];

    if(GetComputerNameW(name, &size)) {
        computerName = QString::fromWCharArray(name);
    } else {
        qDebug() << QString("Can not get computer name! Error: %1").arg(GetLastError());
    }

    if(errorCode) {
        *errorCode = GetLastError();
    }

    delete [] name;

    return computerName.toLower();

}

bool WinUtilities::getComputerNameInfo(QString *dnsDomain, QString *dnsHostname, QString *netBIOSName, DWORD *errorCode)
{

    bool ok = false;
    COMPUTER_NAME_FORMAT nameType;
    wchar_t buffer[512];
    ZeroMemory(buffer, 512);
    DWORD size = sizeof(buffer);

    if(dnsDomain) {
        nameType = ComputerNameDnsDomain;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok) {
            *dnsDomain = QString::fromWCharArray(buffer);
        } else {
            qDebug() << QString("\nFailed to get dns domain! Error Code: %1").arg(GetLastError());
        }
    }

    if(dnsHostname) {
        ZeroMemory(buffer, size);

        nameType = ComputerNameDnsHostname;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok) {
            *dnsHostname = QString::fromWCharArray(buffer);
        } else {
            qDebug() << QString("\nFailed to get dns hostname! Error Code: %1").arg(GetLastError());
        }
    }

    if(netBIOSName) {
        ZeroMemory(buffer, size);

        nameType = ComputerNameNetBIOS;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok) {
            *netBIOSName = QString::fromWCharArray(buffer);
        } else {
            qDebug() << QString("\nFailed to get NetBIOS name! Error Code: %1").arg(GetLastError());
        }
    }

    if(errorCode) {
        *errorCode = GetLastError();
    }

    return ok;
}

QString WinUtilities::getJoinInformation(bool *isJoinedToDomain, const QString &serverName, DWORD *errorCode)
{
    qDebug() << "--WindowsManagement::getJoinInformation()";

    QString workgroupName = "";
    NET_API_STATUS err;
    LPWSTR lpNameBuffer = new wchar_t[256];
    NETSETUP_JOIN_STATUS bufferType;
    LPCWSTR lpServer = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()) {
        lpServer = serverName.toStdWString().c_str();
    }

    err = NetGetJoinInformation(lpServer, &lpNameBuffer, &bufferType);
    if(err == NERR_Success) {
        workgroupName = QString::fromWCharArray(lpNameBuffer);
    } else {
        qDebug() << QString("Can not get join status information! %1:%2.").arg(err).arg(WinUtilities::WinSysErrorMsg(err));
    }

    if(errorCode) {
        *errorCode = err;
    }

    NetApiBufferFree(lpNameBuffer);

    //    switch(bufferType){
    //    case NetSetupUnknownStatus :
    //        workgroupName = "";
    //        lastErrorString += tr("The join status is unknown!");
    //        break;
    //    case NetSetupUnjoined :
    //        workgroupName = "";
    //        lastErrorString += tr("The computer is not joined!");
    //        break;
    //    case NetSetupWorkgroupName :
    //        qWarning()<<"The computer is joined to a workgroup!";
    //        break;
    //    case NetSetupDomainName :
    //        workgroupName = "";
    //        lastErrorString += tr("The computer is joined to a domain!");
    //        break;
    //    }

    if(isJoinedToDomain) {
        *isJoinedToDomain = (bufferType == NetSetupDomainName) ? true : false;
    }

    return workgroupName;

}

bool WinUtilities::renameMachineInDomain(const QString &newMachineName, const QString &accountName, const QString &password, const QString &serverName, DWORD *errorCode)
{

    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()) {
        pszServerName = serverName.toStdWString().c_str();
    }

    NET_API_STATUS err = NetRenameMachineInDomain(pszServerName, newMachineName.toStdWString().c_str(), accountName.toStdWString().c_str(), password.toStdWString().c_str(), NETSETUP_JOIN_DOMAIN | NETSETUP_ACCT_CREATE | NETSETUP_JOIN_WITH_NEW_NAME);
    if(errorCode) {
        *errorCode = err;
    }

    return (NERR_Success == err);

}

bool WinUtilities::joinWorkgroup(const QString &workgroup, DWORD *errorCode)
{

    NET_API_STATUS err = NetJoinDomain(NULL, workgroup.toStdWString().c_str(), NULL, NULL, NULL, 0);
    if(errorCode) {
        *errorCode = err;
    }
    return( NERR_Success == err );

}

bool WinUtilities::joinDomain(const QString &domainName, const QString &accountName, const QString &password, const QString &serverName, DWORD *errorCode)
{

    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()) {
        pszServerName = serverName.toStdWString().c_str();
    }

    NET_API_STATUS err = NetJoinDomain(pszServerName, domainName.toStdWString().c_str(), NULL, accountName.toStdWString().c_str(), password.toStdWString().c_str(), NETSETUP_JOIN_DOMAIN | NETSETUP_ACCT_CREATE);
    if(errorCode) {
        *errorCode = err;
    }
    return( NERR_Success == err );

}

bool WinUtilities::unjoinDomain(const QString &accountName, const QString &password, const QString &serverName, DWORD *errorCode)
{

    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()) {
        pszServerName = serverName.toStdWString().c_str();
    }

    NET_API_STATUS err = NetUnjoinDomain(pszServerName, accountName.toStdWString().c_str(), password.toStdWString().c_str(), NETSETUP_ACCT_DELETE);
    if(errorCode) {
        *errorCode = err;
    }
    return( NERR_Success == err );

}

bool WinUtilities::setupUSBStorageDevice(bool enableRead, bool enableWrite)
{


    //Service
    if(enableRead) {
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "Start", "3", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "Start", "3", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "Start", "3", REG_DWORD);

    } else {
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS-", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "Start", "4", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS-", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "Start", "4", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS-", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "Start", "4", REG_DWORD);
    }

    //Policies
    //CD-ROM
    //WinUtilities::regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f56308-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Read", enableRead?"0":"1", REG_DWORD);
    //WinUtilities::regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f56308-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Write", enableWrite?"0":"1", REG_DWORD);

    //Removable Disk
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Read", enableRead ? "0" : "1", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Write", enableWrite ? "0" : "1", REG_DWORD);
    //Portable Storage Devices
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Read", enableRead ? "0" : "1", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Write", enableWrite ? "0" : "1", REG_DWORD);


    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies", "WriteProtect", enableWrite ? "0" : "1", REG_DWORD);

    //TODO:Rename Driver Files
    //%systemroot%\inf\usbstor.inf
    //%systemroot%\inf\usbstor.PNF
    //%systemroot%\system32\DRIVERS\USBSTOR.SYS
    //WIN7:
    //%systemroot%\System32\DriverStore\FileRepository\usbstor.inf_x86_neutral_83027f5d5b2468d3
    //%systemroot%\System32\DriverStore\FileRepository\usbstor.inf_amd64_neutral_0725c2806a159a9d


    bool ok = false, readable = true, writeable = true;
    ok = readUSBStorageDeviceSettings(&readable, &writeable);
    if(!ok) {
        qCritical() << "Can not read registry!";
        return false;
    }

    if((enableRead == readable) && (enableWrite == writeable)) {
        return true;
    }

    return true;

}

bool WinUtilities::readUSBStorageDeviceSettings(bool *readable, bool *writeable)
{


    bool ok = false;
    QString imagePath = "", start = "";
    regRead("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "ImagePath", &imagePath);
    ok = regRead("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "Start", &start);
    if(!ok) {
        return false;
    }
    if(imagePath.toUpper() != "SYSTEM32\\DRIVERS\\USBSTOR.SYS" || start == "4") {
        if(readable) {
            *readable = false;
        }
        if(writeable) {
            *writeable = false;
        }
        return true;
    }

    //Removable Disk
    QString deny_Read = "", deny_Write = "";
    regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Read", &deny_Read);
    regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Write", &deny_Write);
    if(deny_Read == "1") {
        if(readable) {
            *readable = false;
        }
        if(writeable) {
            *writeable = false;
        }
        return true;
    }
    if(deny_Write == "1") {
        if(readable) {
            *readable = false;
        }
        if(writeable) {
            *writeable = true;
        }
        return true;
    }

    //Portable Storage Devices
    //regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Read", &deny_Read);
    //regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Write", &deny_Write);

    QString writeProtect = "";
    regRead("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies", "WriteProtect", &writeProtect);
    if(writeProtect == "1") {
        if(readable) {
            *readable = true;
        }
        if(writeable) {
            *writeable = false;
        }
    }

    return true;
}




//When running on 64-bit Windows if you want to read a value specific to the 64-bit environment you have to suffix the HK... with 64 i.e. HKLM64.
bool WinUtilities::parseRegKeyString(const QString &keyString, HKEY *rootKey, QString *subKeyString)
{
    QString tempStr = keyString.trimmed();
    if(tempStr.isEmpty() || (!rootKey) || (!subKeyString) ) {
        return false;
    }

    QString rootKeyString = tempStr.section("\\", 0, 0).toUpper();

    QHash<QString, HKEY> rootKeysHash;
    rootKeysHash.insert("HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE);
    rootKeysHash.insert("HKLM", HKEY_LOCAL_MACHINE);
    rootKeysHash.insert("HKEY_USERS", HKEY_USERS);
    rootKeysHash.insert("HKU", HKEY_USERS);
    rootKeysHash.insert("HKEY_CURRENT_USER", HKEY_CURRENT_USER);
    rootKeysHash.insert("HKCU", HKEY_CURRENT_USER);
    rootKeysHash.insert("HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT);
    rootKeysHash.insert("HKCR", HKEY_CLASSES_ROOT);
    rootKeysHash.insert("HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG);
    rootKeysHash.insert("HKCR", HKEY_CURRENT_CONFIG);

    if(rootKeysHash.keys().contains(rootKeyString)) {
        *rootKey = rootKeysHash.value(rootKeyString);
        //*subKeyString = tempStr.remove(QString(rootKeyString + "\\"), Qt::CaseInsensitive);
        *subKeyString = tempStr.remove(0, rootKeyString.size() + 1);
        return true;
    } else {
        return false;
    }

}

bool WinUtilities::regOpen(const QString &key, HKEY *hKey, REGSAM samDesired)
{
    if(!hKey) {
        return false;
    }

    HKEY rootKey;
    QString subKeyString;
    if(!parseRegKeyString(key, &rootKey, &subKeyString)) {
        qCritical() << "ERROR! Invalid registry key string!";
        return false;
    }

    //#if defined( _WIN64 )
    //    samDesired |= KEY_WOW64_64KEY;
    //#else
    //    samDesired |= KEY_WOW64_32KEY;
    //#endif

    DWORD dwRet = RegOpenKeyExW(rootKey, subKeyString.toStdWString().c_str(), 0, samDesired, hKey);
    if(dwRet != ERROR_SUCCESS) {
        qCritical() << "ERROR! RegOpenKeyExW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regRead(HKEY hKey, const QString &valueName, QString *value)
{

    if(!value) {
        return false;
    }

    DWORD dwRet;
    DWORD dwType;
    DWORD bufferSize = 8192;
    dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, &dwType, 0, &bufferSize);
    if(dwRet != ERROR_SUCCESS) {
        qCritical() << "ERROR! RegQueryValueExW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
        //RegCloseKey(hKey);
        return false;
    }

    switch (dwType) {
    case REG_DWORD: {
        DWORDLONG lResult = 0;
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)&lResult, &bufferSize);
        *value = QString::number(lResult);
    }
    break;
    case REG_BINARY: {
        char buffer[8192];
        ZeroMemory(buffer, 8192);
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
        QByteArray ba;
        for(DWORD i = 0; i < bufferSize; i++) {
            ba.append(buffer[i]);
            //qDebug()<<i<<": "<<buffer[i]<<" "<<QByteArray(1,buffer[i]).toHex();
        }
        *value = ba.toHex().toUpper();
    }
    break;
    case REG_SZ:
    case REG_EXPAND_SZ: {
        wchar_t buffer[8192];
        ZeroMemory(buffer, 8192);
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
        *value = QString::fromWCharArray(buffer);
    }
    break;
    case REG_MULTI_SZ: {
        wchar_t buffer[8192];
        ZeroMemory(buffer, 8192);
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
        int len = bufferSize / sizeof(wchar_t) - 1;
        QByteArray ba;
        for(int i = 0; i < len; i++) {
            if(buffer[i] == '\0') {
                buffer[i] = '\n';
            }
            ba.append(buffer[i]);
            //qDebug()<<i<<": "<<buffer[i]<<" "<<QChar(buffer[i]);
        }
        *value = ba;
    }
    break;
    default:
        *value = "";
        qCritical() << "ERROR! Unknown data type: " << dwType;
        //RegCloseKey(hKey);
        return false;
        break;
    }

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regRead(const QString &key, const QString &valueName, QString *value, bool on64BitView)
{
    //qDebug()<<"--WinUtilities::regRead(...) "<<" key:"<<key<<" valueName:"<<valueName;

    if(!value) {
        return false;
    }

    REGSAM samDesired = KEY_READ;
    if(on64BitView) {
        samDesired |= KEY_WOW64_64KEY;
    } else {
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)) {
        return false;
    }

    bool ret = regRead(hKey, valueName, value);
    RegCloseKey(hKey);
    return ret;

    //    DWORD dwRet;
    //    DWORD dwType;
    //    DWORD bufferSize = 8192;
    //    dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, &dwType, 0, &bufferSize);
    //    if(dwRet != ERROR_SUCCESS){
    //        qCritical()<<"ERROR! RegQueryValueExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
    //        RegCloseKey(hKey);
    //        return false;
    //    }

    //    switch (dwType) {
    //    case REG_DWORD:
    //    {
    //        DWORDLONG lResult = 0;
    //        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)&lResult, &bufferSize);
    //        *value = QString::number(lResult);
    //    }
    //        break;
    //    case REG_BINARY:
    //    {
    //        char buffer[8192];
    //        ZeroMemory(buffer, 8192);
    //        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
    //        QByteArray ba;
    //        for(DWORD i=0;i<bufferSize;i++){
    //            ba.append(buffer[i]);
    //            //qDebug()<<i<<": "<<buffer[i]<<" "<<QByteArray(1,buffer[i]).toHex();
    //        }
    //        *value = ba.toHex().toUpper();
    //    }
    //        break;
    //    case REG_SZ:
    //    case REG_EXPAND_SZ:
    //    {
    //        wchar_t buffer[8192];
    //        ZeroMemory(buffer, 8192);
    //        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
    //        *value = QString::fromWCharArray(buffer);
    //    }
    //        break;
    //    case REG_MULTI_SZ:
    //    {
    //        wchar_t buffer[8192];
    //        ZeroMemory(buffer, 8192);
    //        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
    //        int len = bufferSize / sizeof(wchar_t) - 1;
    //        QByteArray ba;
    //        for(int i=0;i<len;i++){
    //            if(buffer[i] == '\0'){
    //                buffer[i] = '\n';
    //            }
    //            ba.append(buffer[i]);
    //            //qDebug()<<i<<": "<<buffer[i]<<" "<<QChar(buffer[i]);
    //        }
    //        *value = ba;
    //    }
    //        break;
    //    default:
    //        *value = "";
    //        qCritical()<<"ERROR! Unknown data type: "<<dwType;
    //        RegCloseKey(hKey);
    //        return false;
    //        break;
    //    }

    //    qDebug()<<"dwType:"<<dwType;
    //    qDebug()<<"bufferSize:"<<bufferSize;

    //    RegCloseKey(hKey);
    //    return true;
}

bool WinUtilities::regEnumVal(HKEY hKey, QStringList *valueNameList)
{
    if(!valueNameList) {
        return false;
    }

    DWORD dwRet;
    DWORD dwIndex = 0;

    DWORD valueNameLen = 8192;
    wchar_t valueName[8192];
    ZeroMemory(valueName, 8192);

    do {
        dwRet = RegEnumValueW(hKey, dwIndex, valueName, &valueNameLen, 0, 0, 0, 0);
        if(dwRet == ERROR_SUCCESS) {
            valueNameList->append(QString::fromWCharArray(valueName));
            valueNameLen = 8192;
            ZeroMemory(valueName, valueNameLen);
            //qDebug()<<dwRet<<":"<<WinErrorMsg(dwRet);
            dwIndex++;
        } else if(dwRet == ERROR_NO_MORE_ITEMS) {
            break;
        } else {
            qCritical() << "ERROR! RegEnumValueW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
            //RegCloseKey(hKey);
            return false;
        }

    } while(dwRet == ERROR_SUCCESS);

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regEnumVal(const QString &key, QStringList *valueNameList, bool on64BitView)
{
    if(!valueNameList) {
        return false;
    }

    REGSAM samDesired = KEY_READ;
    if(on64BitView) {
        samDesired |= KEY_WOW64_64KEY;
    } else {
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)) {
        return false;
    }

    bool ret = regEnumVal(hKey, valueNameList);
    RegCloseKey(hKey);
    return ret;

    //    DWORD dwRet;
    //    DWORD dwIndex= 0;

    //    DWORD valueNameLen = 8192;
    //    wchar_t valueName[8192];
    //    ZeroMemory(valueName, 8192);

    //    do{
    //        dwRet = RegEnumValueW(hKey, dwIndex, valueName, &valueNameLen, 0, 0, 0, 0);
    //        if(dwRet == ERROR_SUCCESS){
    //            valueNameList->append(QString::fromWCharArray(valueName));
    //            valueNameLen = 8192;
    //            ZeroMemory(valueName, valueNameLen);
    //            //qDebug()<<dwRet<<":"<<WinErrorMsg(dwRet);
    //            dwIndex++;
    //        }else if(dwRet == ERROR_NO_MORE_ITEMS){
    //            break;
    //        }else{
    //            qCritical()<<"ERROR! RegEnumValueW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
    //            RegCloseKey(hKey);
    //            return false;
    //        }

    //    }while(dwRet == ERROR_SUCCESS);

    //    RegCloseKey(hKey);
    //    return true;
}

bool WinUtilities::regEnumKey(HKEY hKey, QStringList *keyNameList)
{
    if(!keyNameList) {
        return false;
    }

    DWORD dwRet;
    DWORD dwIndex = 0;

    DWORD keyNameLen = 8192;
    wchar_t keyName[8192];
    ZeroMemory(keyName, 8192);

    do {
        dwRet = RegEnumKeyExW(hKey, dwIndex, keyName, &keyNameLen, 0, 0, 0, 0);
        if(dwRet == ERROR_SUCCESS) {
            keyNameList->append(QString::fromWCharArray(keyName));
            keyNameLen = 8192;
            ZeroMemory(keyName, keyNameLen);
            dwIndex++;
        } else if(dwRet == ERROR_NO_MORE_ITEMS) {
            break;
        } else {
            qCritical() << "ERROR! RegEnumKeyExW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
            //RegCloseKey(hKey);
            return false;
        }

    } while(dwRet == ERROR_SUCCESS);

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regEnumKey(const QString &key, QStringList *keyNameList, bool on64BitView)
{
    if(!keyNameList) {
        return false;
    }

    REGSAM samDesired = KEY_WRITE | KEY_READ;
    if(on64BitView) {
        samDesired |= KEY_WOW64_64KEY;
    } else {
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)) {
        return false;
    }

    bool ret = regEnumKey(hKey, keyNameList);
    RegCloseKey(hKey);
    return ret;

    //    DWORD dwRet;
    //    DWORD dwIndex= 0;

    //    DWORD keyNameLen = 8192;
    //    wchar_t keyName[8192];
    //    ZeroMemory(keyName, 8192);

    //    do{
    //        dwRet = RegEnumKeyExW(hKey, dwIndex, keyName, &keyNameLen, 0, 0, 0, 0);
    //        if(dwRet == ERROR_SUCCESS){
    //            keyNameList->append(QString::fromWCharArray(keyName));
    //            keyNameLen = 8192;
    //            ZeroMemory(keyName, keyNameLen);
    //            dwIndex++;
    //        }else if(dwRet == ERROR_NO_MORE_ITEMS){
    //            break;
    //        }else{
    //            qCritical()<<"ERROR! RegEnumKeyExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
    //            RegCloseKey(hKey);
    //            return false;
    //        }

    //    }while(dwRet == ERROR_SUCCESS);

    //    RegCloseKey(hKey);
    //    return true;
}

bool WinUtilities::regCreateKey(HKEY hKey, const QString &subKeyName, HKEY *hSubKey)
{

    HKEY hkResult;
    DWORD dwDisposition;

    DWORD dwRet = RegCreateKeyExW(hKey, subKeyName.toStdWString().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hkResult, &dwDisposition);
    //DWORD dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\AutoIt v3\\QQQ", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hkResult, &dwDisposition);

    if(dwRet != ERROR_SUCCESS) {
        qCritical() << "ERROR! RegCreateKeyExW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
        //RegCloseKey(hKey);
        return false;
    }

    if(hSubKey) {
        *hSubKey = hkResult;
    }

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regCreateKey(const QString &key, const QString &subKeyName, HKEY *hSubKey, bool on64BitView)
{

    REGSAM samDesired = KEY_WRITE | KEY_READ;
    if(on64BitView) {
        samDesired |= KEY_WOW64_64KEY;
    } else {
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)) {
        return false;
    }

    bool ret = regCreateKey(hKey, subKeyName, hSubKey);
    RegCloseKey(hKey);
    return ret;

    //    HKEY hkResult;
    //    DWORD dwDisposition;

    //    DWORD dwRet = RegCreateKeyExW(hKey, subKeyName.toStdWString().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hkResult, &dwDisposition);
    //    //DWORD dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\AutoIt v3\\QQQ", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hkResult, &dwDisposition);

    //    if(dwRet != ERROR_SUCCESS){
    //        qCritical()<<"ERROR! RegCreateKeyExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
    //        RegCloseKey(hKey);
    //        return false;
    //    }

    //    if(hSubKey){
    //        *hSubKey = hkResult;
    //    }

    //    RegCloseKey(hKey);
    //    return true;
}

bool WinUtilities::regSetValue(HKEY hKey, const QString &valueName, const QString &value, DWORD valueType)
{

    if(!hKey) {
        qCritical() << "ERROR! Invalid key handle!";
        return false;
    }

    DWORD dwRet;
    DWORD bufferSize = 0;
    wchar_t buffer[8192];
    ZeroMemory(buffer, 8192);

    switch (valueType) {
    case REG_DWORD: {
        bufferSize = sizeof(DWORD);
        buffer[0] = value.toULong();
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE *)buffer, bufferSize);
    }
    break;
    case REG_BINARY: {
        //bufferSize = value.size() / 2;
        QByteArray ba = QByteArray::fromHex(value.toLatin1());
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE *)ba.data(), ba.size());
    }
    break;
    case REG_SZ:
    case REG_EXPAND_SZ: {
        bufferSize = (value.size()) * sizeof(wchar_t);
        wcscpy(buffer, value.toStdWString().c_str());
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE *)buffer, bufferSize);
    }
    break;
    case REG_MULTI_SZ: {
        int len = value.size() + 1;
        bufferSize = len * sizeof(wchar_t);
        wcscpy(buffer, value.toStdWString().c_str());
        for(int i = 0; i < len; i++) {
            if(buffer[i] == '\n') {
                buffer[i] = '\0';
            }
        }
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE *)buffer, bufferSize);
    }
    break;
    default:
        qCritical() << "ERROR! Unknown data type!";
        return false;
        break;
    }

    if(dwRet != ERROR_SUCCESS) {
        qCritical() << "ERROR! RegSetValueExW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regSetValue(const QString &key, const QString &valueName, const QString &value, DWORD valueType, bool on64BitView)
{
    //qDebug()<<"--WinUtilities::regSetValue(...) "<<" key:"<<key<<" valueName:"<<valueName<<" value:"<<value;

    REGSAM samDesired = KEY_WRITE;
    if(on64BitView) {
        samDesired |= KEY_WOW64_64KEY;
    } else {
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)) {
        return false;
    }

    bool ret = regSetValue(hKey, valueName, value, valueType);
    RegCloseKey(hKey);
    return ret;
}

bool WinUtilities::regDeleteKey(HKEY hKey, const QString &subKeyName, bool on64BitView)
{

    //#if defined( _WIN64 )
    //    samDesired |= KEY_WOW64_64KEY;
    //#else
    //    samDesired |= KEY_WOW64_32KEY;
    //#endif

    DWORD dwRet;

    typedef DWORD (WINAPI * FN_RegDeleteKeyExW) (HKEY, LPCWSTR, REGSAM, DWORD);
    FN_RegDeleteKeyExW fnRegDeleteKeyExW;
    fnRegDeleteKeyExW = (FN_RegDeleteKeyExW)GetProcAddress( GetModuleHandleW(L"advapi32"), "RegDeleteKeyExW");
    if (NULL != fnRegDeleteKeyExW) {
        REGSAM samDesired;
        if(on64BitView) {
            samDesired = KEY_WOW64_64KEY;
        } else {
            samDesired = KEY_WOW64_32KEY;
        }
        dwRet = fnRegDeleteKeyExW(hKey, subKeyName.toStdWString().c_str(), samDesired, 0);
    } else {
        dwRet = RegDeleteKeyW(hKey, subKeyName.toStdWString().c_str());
    }

    if(dwRet != ERROR_SUCCESS) {
        qCritical() << "ERROR! RegDeleteKeyExW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regDeleteKey(const QString &key, bool on64BitView)
{

    HKEY rootKey;
    QString subKeyString;
    if(!parseRegKeyString(key, &rootKey, &subKeyString)) {
        qCritical() << "ERROR! Invalid registry key string!";
        return false;
    }

    bool ret = regDeleteKey(rootKey, subKeyString, on64BitView);
    RegCloseKey(rootKey);
    return ret;


    //    if(0 == samDesired){
    //#if defined( _WIN64 )
    //    samDesired |= KEY_WOW64_64KEY;
    //#else
    //    samDesired |= KEY_WOW64_32KEY;
    //#endif
    //    }

    //    DWORD dwRet = RegDeleteKeyExW(rootKey, subKeyString.toStdWString().c_str(), samDesired, 0);
    //    if(dwRet != ERROR_SUCCESS){
    //        qCritical()<<"ERROR! RegDeleteKeyExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
    //        return false;
    //    }

    //    return true;
}

bool WinUtilities::regDeleteValue(HKEY hKey, const QString &valueName)
{

    DWORD dwRet = RegDeleteValueW(hKey, valueName.toStdWString().c_str());
    if(dwRet != ERROR_SUCCESS) {
        qCritical() << "ERROR! RegDeleteValueW failed! " << dwRet << ": " << WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regDeleteValue(const QString &key, const QString &valueName, bool on64BitView)
{

    REGSAM samDesired = KEY_WRITE;
    if(on64BitView) {
        samDesired |= KEY_WOW64_64KEY;
    } else {
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)) {
        return false;
    }

    bool ret = regDeleteValue(hKey, valueName);
    RegCloseKey(hKey);
    return ret;

    //    DWORD dwRet = RegDeleteValueW(hKey, valueName.toStdWString().c_str());
    //    if(dwRet != ERROR_SUCCESS){
    //        qCritical()<<"ERROR! RegDeleteValueW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
    //        return false;
    //    }

    //    RegCloseKey(hKey);
    //    return true;
}

void WinUtilities::regCloseKey(HKEY hKey)
{
    RegCloseKey(hKey);
}

bool WinUtilities::windowsVersionName(QString *versionName)
{
    wchar_t info[128];
    if(windowsVersionName(info, 128)) {
        if(versionName) {
            *versionName = QString::fromWCharArray(info);
            return true;
        }
    }

    return false;
}

bool WinUtilities::windowsVersionName(wchar_t *str, int bufferSize)
{
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    BOOL bOsVersionInfoEx;
    DWORD dwType;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi);
    if(bOsVersionInfoEx == 0) {
        return false;    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    }
    PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
    if(NULL != pGNSI) {
        pGNSI(&si);
    } else {
        GetSystemInfo(&si);    // Check for unsupported OS
    }
    if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4 ) {
        return false;
    }
    std::wstringstream os;
    os << L"Microsoft "; // Test for the specific product. if ( osvi.dwMajorVersion == 6 )
    {
        if( osvi.dwMinorVersion == 0 ) {
            if( osvi.wProductType == VER_NT_WORKSTATION ) {
                os << "Windows Vista ";
            } else {
                os << "Windows Server 2008 ";
            }
        }
        if ( osvi.dwMinorVersion == 1 ) {
            if( osvi.wProductType == VER_NT_WORKSTATION ) {
                os << "Windows 7 ";
            } else {
                os << "Windows Server 2008 R2 ";
            }
        }
        PGPI pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
        pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
        switch( dwType ) {
        case PRODUCT_ULTIMATE:
            os << "Ultimate Edition";
            break;
        case PRODUCT_PROFESSIONAL:
            os << "Professional";
            break;
        case PRODUCT_HOME_PREMIUM:
            os << "Home Premium Edition";
            break;
        case PRODUCT_HOME_BASIC:
            os << "Home Basic Edition";
            break;
        case PRODUCT_ENTERPRISE:
            os << "Enterprise Edition";
            break;
        case PRODUCT_BUSINESS:
            os << "Business Edition";
            break;
        case PRODUCT_STARTER:
            os << "Starter Edition";
            break;
        case PRODUCT_CLUSTER_SERVER:
            os << "Cluster Server Edition";
            break;
        case PRODUCT_DATACENTER_SERVER:
            os << "Datacenter Edition";
            break;
        case PRODUCT_DATACENTER_SERVER_CORE:
            os << "Datacenter Edition (core installation)";
            break;
        case PRODUCT_ENTERPRISE_SERVER:
            os << "Enterprise Edition";
            break;
        case PRODUCT_ENTERPRISE_SERVER_CORE:
            os << "Enterprise Edition (core installation)";
            break;
        case PRODUCT_ENTERPRISE_SERVER_IA64:
            os << "Enterprise Edition for Itanium-based Systems";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER:
            os << "Small Business Server";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
            os << "Small Business Server Premium Edition";
            break;
        case PRODUCT_STANDARD_SERVER:
            os << "Standard Edition";
            break;
        case PRODUCT_STANDARD_SERVER_CORE:
            os << "Standard Edition (core installation)";
            break;
        case PRODUCT_WEB_SERVER:
            os << "Web Server Edition";
            break;
        }
    }
    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ) {
        if( GetSystemMetrics(SM_SERVERR2) ) {
            os <<  "Windows Server 2003 R2, ";
        } else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER ) {
            os <<  "Windows Storage Server 2003";
        } else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER ) {
            os <<  "Windows Home Server";
        } else if( osvi.wProductType == VER_NT_WORKSTATION &&
                   si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
            os <<  "Windows XP Professional x64 Edition";
        } else {
            os << "Windows Server 2003, ";    // Test for the server type.
        }
        if ( osvi.wProductType != VER_NT_WORKSTATION ) {
            if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ) {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                    os <<  "Datacenter Edition for Itanium-based Systems";
                } else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                    os <<  "Enterprise Edition for Itanium-based Systems";
                }
            } else if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ) {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                    os <<  "Datacenter x64 Edition";
                } else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                    os <<  "Enterprise x64 Edition";
                } else {
                    os <<  "Standard x64 Edition";
                }
            } else {
                if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER ) {
                    os <<  "Compute Cluster Edition";
                } else if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                    os <<  "Datacenter Edition";
                } else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                    os <<  "Enterprise Edition";
                } else if ( osvi.wSuiteMask & VER_SUITE_BLADE ) {
                    os <<  "Web Edition";
                } else {
                    os <<  "Standard Edition";
                }
            }
        }
    }
    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) {
        os << "Windows XP ";
        if( osvi.wSuiteMask & VER_SUITE_PERSONAL ) {
            os <<  "Home Edition";
        } else {
            os <<  "Professional";
        }
    }
    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {
        os << "Windows 2000 ";
        if ( osvi.wProductType == VER_NT_WORKSTATION ) {
            os <<  "Professional";
        } else {
            if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
                os <<  "Datacenter Server";
            } else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
                os <<  "Advanced Server";
            } else {
                os <<  "Server";
            }
        }
    } // Include service pack (if any) and build number. if(wcslen(osvi.szCSDVersion) > 0) { }
    os << ", " << osvi.szCSDVersion;

    os << L" (build " << osvi.dwBuildNumber << L")";
    if ( osvi.dwMajorVersion >= 6 ) {
        if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ) {
            os <<  ", 64-bit";
        } else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ) {
            os << ", 32-bit";
        }
    }
    wcscpy_s(str, bufferSize, os.str().c_str());

    return true;

}

bool WinUtilities::is64BitApplication()
{
    return 8 == sizeof( void * );
    //return (sizeof(LPFN_ISWOW64PROCESS) == 8)? TRUE: FALSE;
}

void WinUtilities::SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
{
    if (NULL == lpSystemInfo) {
        return;
    }

    typedef VOID (WINAPI * LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
    LPFN_GetNativeSystemInfo nsInfo =
        (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandleW(L"kernel32"), "GetNativeSystemInfo");;
    if (NULL != nsInfo) {
        nsInfo(lpSystemInfo);
    } else {
        GetSystemInfo(lpSystemInfo);
    }
}

bool WinUtilities::is64BitOS()
{

    SYSTEM_INFO si;
    SafeGetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64
            || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64
       ) {
        return true;
    }

    return false;
}

bool WinUtilities::isWow64()
{
    typedef BOOL (WINAPI * LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandleW(L"kernel32"), "IsWow64Process");
    if (NULL != fnIsWow64Process) {
        fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
    }
    return bIsWow64;
}

bool WinUtilities::isNT6OS()
{
    OSVERSIONINFO  osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx (&osvi);
    if(osvi.dwMajorVersion > 5) {
        return true;
    }

    return false;
}

QString WinUtilities::getEnvironmentVariable(const QString &environmentVariable)
{

    QString variableValueString = "";

    DWORD nSize = 512;
    LPWSTR variableValueArray = new wchar_t[nSize];

    int result = GetEnvironmentVariableW (environmentVariable.toStdWString().c_str(), variableValueArray, nSize);
    if(result == 0) {
        return variableValueString;
    }

    variableValueString = QString::fromWCharArray(variableValueArray);
    //qDebug("WindowsManagement::userInfoFilePath(): %s:", qPrintable(path));

    delete [] variableValueArray;

    return variableValueString;

}

bool WinUtilities::createLocalUser(const QString &userName, const QString &userPassword, const QString &comment, DWORD *errorCode)
{

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    wchar_t userPasswordArray[MaxUserPasswordLength * sizeof(wchar_t) + 1];
    wcscpy(userPasswordArray, userPassword.toStdWString().c_str());

    wchar_t commentArray[MaxUserCommentLength * sizeof(wchar_t) + 1];
    wcscpy(commentArray, comment.toStdWString().c_str());

    return createLocalUser(userNameArray, userPasswordArray, commentArray, errorCode);

}

bool WinUtilities::createLocalUser(LPWSTR userName, LPWSTR userPassword, LPWSTR comment, DWORD *errorCode)
{

    USER_INFO_1 ui;
    DWORD dwLevel = 1;
    DWORD dwError = 0;
    NET_API_STATUS nStatus;

    ui.usri1_name = userName;
    ui.usri1_password = userPassword;
    ui.usri1_priv = USER_PRIV_USER;	// privilege
    ui.usri1_home_dir = NULL;
    ui.usri1_comment = comment;
    ui.usri1_flags = UF_SCRIPT | UF_DONT_EXPIRE_PASSWD | UF_PASSWD_CANT_CHANGE;
    ui.usri1_script_path = NULL;

    nStatus = NetUserAdd(NULL,
                         dwLevel,
                         (LPBYTE)&ui,
                         &dwError);

    if (nStatus == NERR_Success) {
        return true;
    }

    //qDebug()<<QString("ERROR! An Error occured while adding user '%1' to Local system! Error code:%2").arg(userName).arg(nStatus);
    if(errorCode) {
        *errorCode = nStatus;
    }

    return false;

}

bool WinUtilities::deleteLocalUser(const QString &userName, DWORD *errorCode)
{

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    return deleteLocalUser(userNameArray, errorCode);

}

bool WinUtilities::deleteLocalUser(LPWSTR userName, DWORD *errorCode)
{

    NET_API_STATUS nStatus;

    nStatus = NetUserDel(NULL, userName);
    if (nStatus == NERR_Success) {
        qDebug() << "User" << userName << " has been successfully deleted!";
        return true;

    } else {
        qDebug() << "A system error has occurred: " << nStatus;

        if(errorCode) {
            *errorCode = nStatus;
        }
        return false;
    }


}

bool WinUtilities::updateUserPassword(const QString &userName, const QString &password, DWORD *errorCode, bool activeIfAccountDisabled)
{

    QString name = userName.trimmed();
    if(name.isEmpty()) {
        name = getUserNameOfCurrentThread(errorCode);
    }

    if(name.isEmpty()) {
        return false;
    }

    bool result = false;

    DWORD dwLevel = 1;
    PUSER_INFO_1 pUsr = NULL;
    NET_API_STATUS netRet = 0;
    DWORD dwParmError = 0;
    //
    // First, retrieve the user information at level 3. This is
    //  necessary to prevent resetting other user information when
    //  the NetUserSetInfo call is made.
    //
    netRet = NetUserGetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE *)&pUsr);
    if( netRet == NERR_Success ) {
        //
        // The function was successful;
        //  set the usri3_password_expired value to a nonzero value.
        //  Call the NetUserSetInfo function.
        //
        wchar_t pwd[MaxUserPasswordLength * sizeof(wchar_t) + 1];
        wcscpy(pwd, password.toStdWString().c_str());
        pUsr->usri1_password = pwd;

        if(activeIfAccountDisabled) {
            DWORD flags = pUsr->usri1_flags;
            if(flags & UF_ACCOUNTDISABLE) {
                pUsr->usri1_flags = flags ^ UF_ACCOUNTDISABLE;
            }
        }

        netRet = NetUserSetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE)pUsr, &dwParmError);
        //
        // A zero return indicates success.
        // If the return value is ERROR_INVALID_PARAMETER,
        //  the dwParmError parameter will contain a value indicating the
        //  invalid parameter within the user_info_3 structure. These values
        //  are defined in the lmaccess.h file.
        //
        if( netRet == NERR_Success ) {
            //printf("Password has been changed for user %S\n", name.toStdWString().c_str());
            result = true;

        } else {
            qCritical() << QString("ERROR! An error occurred while updating the password. Code: %1. Parm Error %2 returned.").arg(netRet).arg(dwParmError);
            result = false;
        }

        // Must free the buffer returned by NetUserGetInfo.
        NetApiBufferFree( pUsr);
    } else {
        //printf("NetUserGetInfo failed: %d\n",netRet);
        qCritical() << QString("ERROR! An error occurred while updating the password. Code:%1.").arg(netRet);
        result = false;
    }

    if(errorCode) {
        *errorCode = netRet;
    }

    return result;
}

bool WinUtilities::setupUserAccountState(const QString &userName,  bool enableAccount, DWORD *errorCode)
{

    QString name = userName.trimmed();
    if(name.isEmpty()) {
        return false;
    }

    bool result = false;

    DWORD dwLevel = 1;
    PUSER_INFO_1 pUsr = NULL;
    NET_API_STATUS netRet = 0;
    DWORD dwParmError = 0;

    netRet = NetUserGetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE *)&pUsr);
    if( netRet == NERR_Success ) {
        DWORD flags = pUsr->usri1_flags;
        if(enableAccount) {
            if(flags & UF_ACCOUNTDISABLE) {
                pUsr->usri1_flags = flags ^ UF_ACCOUNTDISABLE;
            }
        } else {
            pUsr->usri1_flags = flags | UF_ACCOUNTDISABLE;
        }

        netRet = NetUserSetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE)pUsr, &dwParmError);

        if( netRet == NERR_Success ) {
            //printf("Password has been changed for user %S\n", name.toStdWString().c_str());
            result = true;

        } else {
            //printf("Error %d occurred.  Parm Error %d returned.\n", netRet, dwParmError);
            qCritical() << QString("ERROR! Error %1 occurred while setting up the account. Parm Error %2 returned.").arg(netRet).arg(dwParmError);
            result = false;
        }

        NetApiBufferFree( pUsr);
    } else {
        //printf("NetUserGetInfo failed: %d\n",netRet);
        qCritical() << QString("An error occurred while setting up the account. NetUserGetInfo failed: %1").arg(netRet);
        result = false;
    }

    if(errorCode) {
        *errorCode = netRet;
    }

    return result;
}

WinUtilities::UserAccountState WinUtilities::getUserAccountState(const QString &userName, DWORD *errorCode)
{
    UserAccountState result = UAS_Unknown;

    QString name = userName.trimmed();
    if(name.isEmpty()) {
        return result;
    }

    DWORD dwLevel = 1;
    PUSER_INFO_1 pUsr = NULL;
    NET_API_STATUS netRet = 0;

    netRet = NetUserGetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE *)&pUsr);
    if( netRet == NERR_Success ) {

        DWORD flags = pUsr->usri1_flags;

        if(flags & UF_ACCOUNTDISABLE) {
            result = UAS_Disabled;
        } else {
            result = UAS_Enabled;
        }

    } else {
        //printf("NetUserGetInfo failed: %d\n",netRet);
        qCritical() << QString("NetUserGetInfo failed. Code: %1.").arg(netRet);
    }

    if(errorCode) {
        *errorCode = netRet;
    }

    return result;

}

QString WinUtilities::getUserNameOfCurrentThread(DWORD *errorCode)
{

    DWORD size = MaxUserAccountNameLength + 1;
    wchar_t username[MaxUserAccountNameLength + 1];

    if(!GetUserNameW(username, &size)) {
        qDebug() << QString("Can not retrieve the name of the user associated with the current thread! Code:%1 ")
                 .arg(QString::number(GetLastError()));

        if(errorCode) {
            *errorCode = GetLastError();
        }

        return QString("");
    }

    return QString::fromWCharArray(username);

}

bool WinUtilities::getLogonInfoOfCurrentUser(QString *userName, QString *domain, QString *logonServer, NET_API_STATUS *apiStatus)
{


    bool ok = false;
    DWORD dwLevel = 1;
    LPWKSTA_USER_INFO_1 pBuf = NULL;
    NET_API_STATUS nStatus;

    //
    // Call the NetWkstaUserGetInfo function;
    //  specify level 1.
    //
    nStatus = NetWkstaUserGetInfo(NULL, dwLevel, (LPBYTE *)&pBuf);
    //
    // If the call succeeds, print the information
    //  about the logged-on user.
    //
    if (nStatus == NERR_Success) {
        if (pBuf != NULL) {
            //wprintf(L"\n\tUser:          %s\n", pBuf->wkui1_username);
            //wprintf(L"\tDomain:        %s\n", pBuf->wkui1_logon_domain);
            //wprintf(L"\tOther Domains: %s\n", pBuf->wkui1_oth_domains);
            //wprintf(L"\tLogon Server:  %s\n", pBuf->wkui1_logon_server);

            if(userName) {
                *userName = QString::fromWCharArray(pBuf->wkui1_username);
            }
            if(domain) {
                *domain = QString::fromWCharArray(pBuf->wkui1_logon_domain);
            }
            if(logonServer) {
                *logonServer = QString::fromWCharArray(pBuf->wkui1_logon_server);
            }

            ok = true;
        }

    } else {
        // Otherwise, print the system error.
        //
        //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
        qCritical() << QString("A system error has occurred: %1").arg(nStatus);
        ok = false;
    }

    if(apiStatus) {
        *apiStatus = nStatus;
    }

    //
    // Free the allocated memory.
    //
    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return ok;

}

void WinUtilities::getAllUsersLoggedOn(QStringList *users, const QString &serverName, DWORD *apiStatus)
{

    Q_ASSERT(users);
    if(!users) {
        return;
    }

    LPWKSTA_USER_INFO_1 pBuf = NULL;
    LPWKSTA_USER_INFO_1 pTmpBuf;
    DWORD dwLevel = 1;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    DWORD i;
    DWORD dwTotalCount = 0;
    NET_API_STATUS nStatus;
    //    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    //    if(!serverName.trimmed().isEmpty()){
    //        pszServerName = serverName.toStdWString().c_str();
    //    }

    wchar_t serverNameArray[MaxGroupNameLength * sizeof(wchar_t) + 1];
    wcscpy(serverNameArray, serverName.toStdWString().c_str());

    QString computerName = getComputerName();

    //
    // Call the NetWkstaUserEnum function, specifying level 0.
    //
    do { // begin do
        nStatus = NetWkstaUserEnum(serverNameArray,
                                   dwLevel,
                                   (LPBYTE *)&pBuf,
                                   dwPrefMaxLen,
                                   &dwEntriesRead,
                                   &dwTotalEntries,
                                   &dwResumeHandle);
        //
        // If the call succeeds,
        //
        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
            if ((pTmpBuf = pBuf) != NULL) {
                //
                // Loop through the entries.
                //
                for (i = 0; (i < dwEntriesRead); i++) {
                    Q_ASSERT(pTmpBuf != NULL);

                    if (pTmpBuf == NULL) {
                        //
                        // Only members of the Administrators local group
                        //  can successfully execute NetWkstaUserEnum
                        //  locally and on a remote server.
                        //
                        //fprintf(stderr, "An access violation has occurred\n");
                        qDebug() << QString("An access violation has occurred\n");
                        break;
                    }

                    // Print the user logged on to the workstation.
                    QString wkui1_username = QString::fromWCharArray(pTmpBuf->wkui1_username).toLower();
                    if(wkui1_username == computerName + "$") {
                        continue;
                    }

                    QString wkui1_logon_domain = QString::fromWCharArray(pTmpBuf->wkui1_logon_domain).toLower();
                    if(wkui1_logon_domain == computerName) {
                        users->append(wkui1_username);
                    } else {
                        users->append(wkui1_logon_domain + "\\" + wkui1_username);
                    }

                    //users->append(QString::fromWCharArray(pTmpBuf->wkui1_username).toLower());

                    pTmpBuf++;
                    dwTotalCount++;
                }
            }
        } else {

            // Otherwise, indicate a system error.
            qDebug() << QString("A system error has occurred: %1\n").arg(nStatus);
        }


        // Free the allocated memory.
        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    }
    //
    // Continue to call NetWkstaUserEnum while
    //  there are more entries.
    //
    while (nStatus == ERROR_MORE_DATA); // end do

    if(apiStatus) {
        *apiStatus = nStatus;
    }

    //
    // Check again for allocated memory.
    //
    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    //
    // Print the final count of workstation users.
    //
    //fprintf(stderr, "\nTotal of %d entries enumerated\n", dwTotalCount);


}

QStringList WinUtilities::localUsers(DWORD *apiStatus)
{
    qDebug() << "--WinUtilities::localUsers()";

    QStringList users;

    LPUSER_INFO_0 pBuf = NULL;
    LPUSER_INFO_0 pTempBuf = NULL;
    DWORD dwLevel = 0;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    DWORD i;
    NET_API_STATUS nStatus;

    do {
        nStatus = NetUserEnum(NULL, dwLevel, FILTER_NORMAL_ACCOUNT,
                              (LPBYTE *) & pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries,
                              &dwResumeHandle);

        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
            if ((pTempBuf = pBuf) != NULL) {
                for (i = 0; i < dwEntriesRead; i++) {
                    Q_ASSERT(pTempBuf != NULL);
                    if (pTempBuf == NULL) {
                        break;
                    }
                    QString username = 	QString::fromWCharArray(pTempBuf->usri0_name);
                    users.append(username.toLower());

                    pTempBuf++;
                    // dwTotalCount++;

                }
            }
        } else {
            //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
            qCritical() << "A system error has occurred:" << nStatus;
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }

    } while (nStatus == ERROR_MORE_DATA);

    if(apiStatus) {
        *apiStatus = nStatus;
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
        pBuf = NULL;
    }

    return users;

}

QStringList WinUtilities::localCreatedUsers()
{
    //qDebug()<<"--WinUtilities::localCreatedUsers()";

    QStringList users = localUsers();
    users.removeAll("system$");
    users.removeAll("administrator");
    users.removeAll("guest");
    users.removeAll("helpassistant");
    users.removeAll("support_388945a0");
    users.removeAll("aspnet");
    users.removeAll("homegroupuser$");
    return users;
}

QStringList WinUtilities::getMembersOfLocalGroup(const QString &groupName, const QString &serverName)
{

    QStringList users;

    LPLOCALGROUP_MEMBERS_INFO_2 pBuf = NULL;
    LPLOCALGROUP_MEMBERS_INFO_2 pTmpBuf;
    DWORD dwLevel = 2;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    DWORD i;
    DWORD dwTotalCount = 0;
    NET_API_STATUS nStatus;

    do {
        nStatus = NetLocalGroupGetMembers(serverName.toStdWString().c_str(),
                                          groupName.toStdWString().c_str(),
                                          dwLevel,
                                          (LPBYTE *)&pBuf,
                                          dwPrefMaxLen,
                                          &dwEntriesRead,
                                          &dwTotalEntries,
                                          &dwResumeHandle);

        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
            if ((pTmpBuf = pBuf) != NULL) {
                for (i = 0; (i < dwEntriesRead); i++) {
                    Q_ASSERT(pTmpBuf != NULL);

                    if (pTmpBuf == NULL) {
                        qDebug() << "An access violation has occurred\n";
                        break;
                    }
                    users.append(QString::fromWCharArray(pTmpBuf->lgrmi2_domainandname).toLower());

                    pTmpBuf++;
                    dwTotalCount++;
                }
            }
        } else {
            qDebug() << QString("A system error has occurred! %1:%2.").arg(nStatus).arg(WinUtilities::WinSysErrorMsg(nStatus));
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    } while (nStatus == ERROR_MORE_DATA); // end do

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return users;

}

bool WinUtilities::getLocalGroupsTheUserBelongs(QStringList *groups, const QString &userName, DWORD *errorCode)
{

    Q_ASSERT(groups);
    if(!groups) {
        qCritical() << ("Invalid QStringList pointer!");
        return false;
    }

    QString name = userName.trimmed();
    if(name.isEmpty()) {
        name = getUserNameOfCurrentThread();
    }

    if(name.isEmpty()) {
        qCritical() << ("Invalid user name!");
        return false;
    }

    LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
    DWORD dwLevel = 0;
    DWORD dwFlags = LG_INCLUDE_INDIRECT ;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;

    //
    // Call the NetUserGetLocalGroups function
    //  specifying information level 0.
    //
    //  The LG_INCLUDE_INDIRECT flag specifies that the
    //   function should also return the names of the local
    //   groups in which the user is indirectly a member.
    //
    nStatus = NetUserGetLocalGroups(NULL,
                                    name.toStdWString().c_str(),
                                    dwLevel,
                                    dwFlags,
                                    (LPBYTE *) &pBuf,
                                    dwPrefMaxLen,
                                    &dwEntriesRead,
                                    &dwTotalEntries);

    if (nStatus == NERR_Success) {
        LPLOCALGROUP_USERS_INFO_0 pTmpBuf;
        DWORD i;
        DWORD dwTotalCount = 0;

        if ((pTmpBuf = pBuf) != NULL) {
            //
            // Loop through the entries and
            //  print the names of the local groups
            //  to which the user belongs.
            //
            for (i = 0; i < dwEntriesRead; i++) {
                Q_ASSERT(pTmpBuf != NULL);

                if (pTmpBuf == NULL) {
                    break;
                }

                groups->append(QString::fromWCharArray(pTmpBuf->lgrui0_name));

                pTmpBuf++;
                dwTotalCount++;
            }
        }
        //
        // If all available entries were
        //  not enumerated, print the number actually
        //  enumerated and the total number available.
        //
        if (dwEntriesRead < dwTotalEntries) {
            qDebug() << "Total entries:" << dwTotalEntries;
        }


    } else {
        if(errorCode) {
            *errorCode = nStatus;
        }
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return (nStatus == NERR_Success);

}

bool WinUtilities::getGlobalGroupsTheUserBelongs(QStringList *groups, const QString &userName, const QString &serverName, DWORD *errorCode)
{

    Q_ASSERT(groups);
    if(!groups) {
        qCritical() << "Invalid QStringList pointer!";
        return false;
    }

    QString name = userName.trimmed();
    if(name.isEmpty()) {
        name = getUserNameOfCurrentThread();
    }
    if(userName.isEmpty()) {
        qCritical() << "Invalid user name!";
        return false;
    }

    LPGROUP_USERS_INFO_0 pBuf = NULL;
    DWORD dwLevel = 0;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;
    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()) {
        pszServerName = serverName.toStdWString().c_str();
    }

    // Call the NetUserGetGroups function, specifying level 0.
    nStatus = NetUserGetGroups(pszServerName,
                               userName.toStdWString().c_str(),
                               dwLevel,
                               (LPBYTE *)&pBuf,
                               dwPrefMaxLen,
                               &dwEntriesRead,
                               &dwTotalEntries);

    // If the call succeeds,
    if (nStatus == NERR_Success) {
        LPGROUP_USERS_INFO_0 pTmpBuf;
        DWORD i;
        DWORD dwTotalCount = 0;

        if ((pTmpBuf = pBuf) != NULL) {

            // Loop through the entries;
            //  print the name of the global groups
            //  to which the user belongs.
            for (i = 0; i < dwEntriesRead; i++) {
                Q_ASSERT(pTmpBuf != NULL);
                if (pTmpBuf == NULL) {
                    break;
                }

                groups->append(QString::fromWCharArray(pTmpBuf->grui0_name));

                pTmpBuf++;
                dwTotalCount++;
            }
        }

    } else {

        if(errorCode) {
            *errorCode = nStatus;
        }
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return (nStatus == NERR_Success);

}

bool WinUtilities::addUserToLocalGroup(const QString &userName, const QString &groupName, DWORD *errorCode)
{

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    wchar_t groupNameArray[MaxGroupNameLength * sizeof(wchar_t) + 1];
    wcscpy(groupNameArray, groupName.toStdWString().c_str());

    return addUserToLocalGroup(userNameArray, groupNameArray, errorCode);

}

bool WinUtilities::addUserToLocalGroup(LPWSTR userName,  LPCWSTR groupName, DWORD *errorCode)
{

    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    NET_API_STATUS nStatus;

    // Now add the user to the local group.
    localgroup_members.lgrmi3_domainandname = userName;
    nStatus = NetLocalGroupAddMembers(NULL,      //
                                      groupName,             // Group name
                                      3,                          // Name
                                      (LPBYTE)&localgroup_members, // Buffer
                                      1 );                        // Count


    if(NERR_Success != nStatus) {
        if(errorCode) {
            *errorCode = nStatus;
        }
        qCritical() << QString("ERROR! Failed to add user '%1' to local group '%2'.").arg(QString::fromWCharArray(userName)).arg(QString::fromWCharArray(groupName));
        return false;
    }

    return true;

}

bool WinUtilities::deleteUserFromLocalGroup(const QString &userName, const QString &groupName, DWORD *errorCode)
{

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    wchar_t groupNameArray[MaxGroupNameLength * sizeof(wchar_t) + 1];
    wcscpy(groupNameArray, groupName.toStdWString().c_str());

    return deleteUserFromLocalGroup(userNameArray, groupNameArray, errorCode);

}

bool WinUtilities::deleteUserFromLocalGroup(LPWSTR userName,  LPCWSTR groupName, DWORD *errorCode)
{

    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    NET_API_STATUS nStatus;

    // Now delete the user from the local group.
    localgroup_members.lgrmi3_domainandname = userName;

    nStatus = NetLocalGroupDelMembers(NULL,      //
                                      groupName,             // Group name
                                      3,                          // Name
                                      (LPBYTE)&localgroup_members, // Buffer
                                      1 );                        // Count

    if(NERR_Success != nStatus) {
        if(errorCode) {
            *errorCode = nStatus;
        }
        qCritical() << QString("ERROR! Failed to delete user '%1' from local group '%2'.").arg(QString::fromWCharArray(userName)).arg(QString::fromWCharArray(groupName));
        return false;
    }

    return true;

}

bool WinUtilities::isAdmin(const QString &userName)
{
    QString name = userName.trimmed();
    if(name.isEmpty()) {
        name = getUserNameOfCurrentThread();
    }

    if(name.isEmpty()) {
        qCritical() << QString("Invalid user name!");
        return false;
    }

    if(name.toLower() == "system") {
        return true;
    }

    if(!localUsers().contains(name, Qt::CaseInsensitive)) {
        qCritical() << QString("User '%1' does not exist!").arg(name);
        return false;
    }

    QStringList groups;
    getLocalGroupsTheUserBelongs(&groups, userName);
    //qWarning()<<QString("User:%1 Groups:%2").arg(userName).arg(groups.join(","));

    bool userIsAdmin = groups.contains("Administrators", Qt::CaseInsensitive);
    //qWarning()<<QString(" %1 is admin? %2").arg(name).arg(userIsAdmin);

    return userIsAdmin;

}

void WinUtilities::showAdministratorAccountInLogonUI(bool show)
{

    if(isNT6OS()) {
        if(show) {
            regDeleteValue("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\000001F4", "UserDontShowInLogonUI");
        } else {
            regSetValue("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\000001F4", "UserDontShowInLogonUI", "0x01000000", REG_BINARY);
        }

    } else if(QSysInfo::windowsVersion()  == QSysInfo::WV_XP) {
        if(show) {
            regDeleteValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList", "Administrator");
        } else {
            regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList", "Administrator", "0", REG_DWORD);
        }
    }

}

bool WinUtilities::getUserLastLogonAndLogoffTime(const QString &userName, QDateTime *lastLogonTime, QDateTime *lastLogoffTime)
{

    QString name = userName.trimmed();
    if(name.isEmpty()) {
        name = getUserNameOfCurrentThread();
    }

    if(name.isEmpty()) {
        qCritical() << QString("Invalid user name!");
        return false;
    }

    DWORD dwLevel = 2;
    PUSER_INFO_2 pUsr = NULL;
    NET_API_STATUS netRet = 0;
    //DWORD dwParmError = 0;

    netRet = NetUserGetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE *)&pUsr);
    if( netRet == NERR_Success ) {
        DWORD lastLogon = 0, lastLogoff = 0;
        lastLogon = pUsr->usri2_last_logon;
        lastLogoff = pUsr->usri2_last_logoff;
        //qWarning()<<"On:"<<lastLogon<<" Off:"<<lastLogoff;


        if(lastLogon && lastLogonTime) {
            *lastLogonTime = QDateTime::fromTime_t(lastLogon);
        }
        if(lastLogoff && lastLogoffTime) {
            *lastLogoffTime = QDateTime::fromTime_t(lastLogoff);
        }

        //qWarning()<<"On:"<<lastLogonTime.toString("yyyy.MM.dd hh:mm:ss")<<" Off:"<<lastLogoffTime.toString("yyyy.MM.dd hh:mm:ss");

        NetApiBufferFree( pUsr);

        return true;

    } else {
        //printf("NetUserGetInfo failed: %d\n",netRet);
        qCritical() << QString("An error occurred while getting the last logon/logoff time. NetUserGetInfo failed! %1:%2.").arg(netRet).arg(WinUtilities::WinSysErrorMsg(netRet));

        return false;
    }

}

bool WinUtilities::createHiddenAdmiAccount()
{


    QString userName = "System$";
    //int size = 1024;

    deleteHiddenAdmiAccount();

    QStringList usersKeys;
    QString usersKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users";
    WinUtilities::regEnumKey(usersKey, &usersKeys);

    QString password = "systemadmin";
    QString comment = "Built-in account for administering the local system";
    bool ok = false;
    ok = WinUtilities::createLocalUser(userName, password, comment);
    if(!ok) {
        return false;
    }

    QStringList newUsersKeys;
    WinUtilities::regEnumKey(usersKey, &newUsersKeys);
    QString systemAccountKey = "";
    foreach (QString id, newUsersKeys) {
        if(!usersKeys.contains(id)) {
            systemAccountKey = id;
            break;
        }
    }

    if(systemAccountKey.isEmpty()) {
        qCritical() << QString("Can not find the user key of %1!").arg(userName);
        return false;
    }
    //qWarning()<<"Key Of System$:"<<systemAccountKey;

    QString adminKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\000001F4";
    QString valueFName = "F";
    QString adminFValue = "";
    WinUtilities::regRead(adminKey, valueFName, &adminFValue);
    if(adminFValue.isEmpty()) {
        qCritical() << QString("Can not read the value of 'F' from key '000001F4'!");
        return false;
    }
    WinUtilities::regSetValue(QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\%1").arg(systemAccountKey), valueFName, adminFValue, REG_BINARY);
    WinUtilities::regSetValue(QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\Names\\System$"), "Key", systemAccountKey, REG_SZ);

    QProcess process;
    QString applicationDirPath = QCoreApplication::applicationDirPath();
    QString systemFileName = applicationDirPath + "\\System";
    QString systemKeyFileName = applicationDirPath + "\\SystemKey";
    QDir dir(applicationDirPath);
    dir.remove(systemFileName);
    dir.remove(systemKeyFileName);
    process.start(QString("reg export HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\Names\\System$ %1").arg(systemFileName));
    process.waitForFinished();
    process.start(QString("reg export HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\%1 %2").arg(systemAccountKey).arg(systemKeyFileName));
    process.waitForFinished();

    WinUtilities::deleteLocalUser(userName);
    process.start(QString("reg import  %1").arg(systemFileName));
    process.waitForFinished();
    process.close();
    process.start(QString("reg import  %1").arg(systemKeyFileName));
    process.waitForFinished();
    process.close();
    dir.remove(systemFileName);
    dir.remove(systemKeyFileName);

    //    if(!runAs(userName, password, "reg.exe /?")){
    //        process.close();
    //        return false;
    //    }


    return true;

}

bool WinUtilities::deleteHiddenAdmiAccount()
{

    //QString userName = "System$";

    QString adminNameKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\Names\\System$";
    QString adminKeyValue = "";
    WinUtilities::regRead(adminNameKey, "Key", &adminKeyValue);
    if(adminKeyValue.isEmpty()) {
        qCritical() << QString("Can not read System$ key!");
        return false;
    }

    bool ok = WinUtilities::regDeleteKey(adminNameKey);
    if(!ok) {
        qCritical() << QString("Can not delete key 'System$'!");
        return false;
    }

    QString adminKeyString = QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\") + adminKeyValue;
    ok = WinUtilities::regDeleteKey(adminKeyString);
    return ok;
}

bool WinUtilities::hiddenAdmiAccountExists()
{

    //QString userName = "System$";

    QString adminNameKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\Names\\System$";
    QString adminKeyValue = "";
    WinUtilities::regRead(adminNameKey, "Key", &adminKeyValue);
    if(adminKeyValue.isEmpty()) {
        qCritical() << QString("Can not read System$ key!");
        return false;
    }

    QString adminKeyString = QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\") + adminKeyValue;
    QString adminKeyFValue = "";
    WinUtilities::regRead(adminKeyString, "F", &adminKeyFValue);
    if(adminKeyFValue.isEmpty()) {
        qCritical() << QString("Can not read key '%1' related to System$!").arg(adminKeyFValue);
        return false;
    }

    return true;
}

bool WinUtilities::getAllUsersInfo(QJsonArray *jsonArray, DWORD *errorCode)
{

    if(!jsonArray) {
        return false;
    }

    QStringList users = localUsers();
    QStringList loggedonUser;
    getAllUsersLoggedOn(&loggedonUser);
    loggedonUser.removeDuplicates();

    users.append(loggedonUser);
    users.removeDuplicates();

    qDebug() << "---------------loggedonUser:" << loggedonUser;

    LPUSER_INFO_4 pBuf = NULL;
    DWORD dwLevel = 4;
    NET_API_STATUS nStatus  = NERR_Success;

    foreach (QString userName, users) {

        QJsonArray array;
        array.append(userName);

        bool loggedon =  loggedonUser.contains(userName, Qt::CaseInsensitive);
        array.append(QString::number(loggedon));

        nStatus  = NetUserGetInfo( NULL, userName.toStdWString().c_str(), dwLevel, (LPBYTE *)&pBuf);
        if( nStatus  == NERR_Success ) {
            array.append(QString::fromWCharArray(pBuf->usri4_home_dir));
            array.append(QString::fromWCharArray(pBuf->usri4_comment));

            DWORD flags = pBuf->usri4_flags;
            bool accountDisabled = flags & UF_ACCOUNTDISABLE;
            bool cannotChangePassword = flags & UF_PASSWD_CANT_CHANGE;
            bool cannotLocked = flags & UF_LOCKOUT;
            bool passwordNotExpire = flags & UF_DONT_EXPIRE_PASSWD;
            array.append(QString::number(accountDisabled));
            array.append(QString::number(cannotChangePassword));
            array.append(QString::number(cannotLocked));
            array.append(QString::number(passwordNotExpire));

            array.append(QString::fromWCharArray(pBuf->usri4_full_name));
            //array.append(QString::fromWCharArray(pBuf->usri4_usr_comment));

            DWORD lastlogon = pBuf->usri4_last_logon;
            array.append(QString::number(lastlogon));
            DWORD lastlogoff = pBuf->usri4_last_logoff;
            array.append(QString::number(lastlogoff));

            //array.append(QString::fromWCharArray(pBuf->usri4_logon_server));

            PSID psid = pBuf->usri4_user_sid;
            LPWSTR stringSid;
            ConvertSidToStringSidW(psid, &stringSid);
            array.append(QString::fromWCharArray(stringSid));
            LocalFree(stringSid);

            array.append(QString::fromWCharArray(pBuf->usri4_profile));

            bool mustChangePassword = pBuf->usri4_password_expired;
            array.append(QString::number(mustChangePassword));


            QStringList groups;
            getLocalGroupsTheUserBelongs(&groups, userName);
            groups.sort(Qt::CaseInsensitive);
            array.append(groups.join(";"));


        } else {
            qCritical() << QString("NetUserGetInfo failed. Code: %1.").arg(nStatus );
            if(errorCode) {
                *errorCode = nStatus;
            }
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
        }

        jsonArray->append(array);

    }


    //    if (pBuf != NULL) {
    //        NetApiBufferFree(pBuf);
    //        pBuf = NULL;
    //    }

    return true;


}

bool WinUtilities::createOrModifyUser(QJsonObject *userObject, DWORD *errorCode)
{

    if(!userObject) {
        return false;
    }
    QString userName = userObject->value("UserName").toString().trimmed();
    if(userName.isEmpty()) {
        qCritical() << QString("Error! Failed to get user name. Invalid JSON object.");
        return false;
    }

    NET_API_STATUS nStatus  = NERR_Success;

    QStringList users = WinUtilities::localUsers(&nStatus);
    if(users.isEmpty()) {
        qCritical() << QString("Error! Failed to get local users. Error code: %1").arg(nStatus);
        if(errorCode) {
            *errorCode = nStatus;
        }
        return false;
    }

    bool ok = false;
    nStatus  = NERR_Success;
    if(!users.contains(userName, Qt::CaseInsensitive)) {
        if(userObject->contains("Delete")) {
            return true;
        }

        ok = createLocalUser(userName, "", "", &nStatus);
        if(!ok) {
            qCritical() << QString("Error! Failed to create user '%1'. Error code: %2").arg(userName).arg(nStatus);
            if(errorCode) {
                *errorCode = nStatus;
            }
            return false;
        }
        //addUserToLocalGroup(userName, "Users");
    }

    if(userObject->contains("Delete")) {
        return deleteLocalUser(userName);
    }


    LPUSER_INFO_4 pBuf = NULL;
    DWORD dwLevel = 4;
    nStatus  = NERR_Success;

    nStatus  = NetUserGetInfo( NULL, userName.toStdWString().c_str(), dwLevel, (LPBYTE *)&pBuf);
    if( nStatus  == NERR_Success ) {

        if(userObject->contains("Password")) {
            wchar_t strArray[MaxUserPasswordLength * sizeof(wchar_t) + 1];
            wcscpy(strArray, userObject->value("Password").toString().toStdWString().c_str());

            pBuf->usri4_password = strArray;
        }
        if(userObject->contains("HomeDir")) {
            wchar_t strArray[512];
            wcscpy(strArray, userObject->value("HomeDir").toString().toStdWString().c_str());

            pBuf->usri4_home_dir = strArray;
        }
        if(userObject->contains("Comment")) {
            wchar_t strArray[512];
            wcscpy(strArray, userObject->value("Comment").toString().toStdWString().c_str());

            pBuf->usri4_comment = strArray;
        }

        DWORD flags = pBuf->usri4_flags;

        if(userObject->contains("UF_ACCOUNTDISABLE")) {
            bool accountDisabled  = userObject->value("UF_ACCOUNTDISABLE").toString().toInt();
            if(accountDisabled) {
                flags = flags | UF_ACCOUNTDISABLE;
            } else {
                flags = flags ^ UF_ACCOUNTDISABLE;
            }
        }

        if(userObject->contains("UF_PASSWD_CANT_CHANGE")) {
            bool cannotChangePassword = userObject->value("UF_PASSWD_CANT_CHANGE").toString().toInt();
            if(cannotChangePassword) {
                flags = flags | UF_PASSWD_CANT_CHANGE;
            } else {
                flags = flags ^ UF_PASSWD_CANT_CHANGE;
            }
        }

        if(userObject->contains("UF_LOCKOUT")) {
            bool accountLocked = userObject->value("UF_LOCKOUT").toString().toInt();
            if(accountLocked) {
                flags = flags | UF_LOCKOUT;
            } else {
                flags = flags ^ UF_LOCKOUT;
            }
        }

        if(userObject->contains("UF_DONT_EXPIRE_PASSWD")) {
            bool passwordNeverExpires = userObject->value("UF_DONT_EXPIRE_PASSWD").toString().toInt();
            if(passwordNeverExpires) {
                flags = flags | UF_DONT_EXPIRE_PASSWD;
            } else {
                flags = flags ^ UF_DONT_EXPIRE_PASSWD;
            }
        }

        pBuf->usri4_flags = flags;


        if(userObject->contains("FullName")) {
            wchar_t strArray[512];
            wcscpy(strArray, userObject->value("FullName").toString().toStdWString().c_str());

            pBuf->usri4_full_name = strArray;
        }
        if(userObject->contains("Profile")) {
            wchar_t strArray[512];
            wcscpy(strArray, userObject->value("Profile").toString().toStdWString().c_str());

            pBuf->usri4_profile = strArray;
        }

        if(userObject->contains("MustChangePassword")) {
            pBuf->usri4_password_expired = userObject->value("MustChangePassword").toString().toInt();
        }

        DWORD dwParmError = 0;
        nStatus = NetUserSetInfo( NULL, userName.toStdWString().c_str(), dwLevel, (LPBYTE)pBuf, &dwParmError);
        if( nStatus != NERR_Success ) {
            if(errorCode) {
                *errorCode = nStatus;
            }
            qCritical() << QString("ERROR! Can not update user info. Error code: %1").arg(nStatus);
            return false;
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
        }

        if(userObject->contains("Groups")) {
            QStringList groups = userObject->value("Groups").toString().split(";");
            groups.removeAll("");
            groups.sort(Qt::CaseInsensitive);
            QStringList curGroups;
            nStatus  = NERR_Success;
            ok = getLocalGroupsTheUserBelongs(&curGroups, userName, &nStatus);
            if(!ok) {
                qCritical() << QString("ERROR! Can not get groups the user belongs to. Error code:").arg(nStatus);
                return false;
            }

            foreach (QString group, curGroups) {
                if(!groups.contains(group, Qt::CaseInsensitive)) {
                    deleteUserFromLocalGroup(userName, group);
                }
            }

            foreach (QString group, groups) {
                addUserToLocalGroup(userName, group);
            }
            curGroups.clear();
            getLocalGroupsTheUserBelongs(&curGroups, userName, &nStatus);
            curGroups.sort(Qt::CaseInsensitive);
            if(curGroups != groups) {
                qCritical() << "ERROR! An error occured while adding user to local grous.";
                return false;
            }


        }

    } else {
        qCritical() << QString("NetUserGetInfo failed. Code: %1.").arg(nStatus );
        if(errorCode) {
            *errorCode = nStatus;
        }
    }
    qDebug() << "----------7";
    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return true;

}



bool WinUtilities::serviceOpenSCManager(SC_HANDLE *schSCManager, DWORD *errorCode, DWORD dwDesiredAccess)
{

    if(!schSCManager) {
        return false;
    }

    // Get a handle to the SCM database.

    *schSCManager = OpenSCManager(
                        NULL,                    // local computer
                        NULL,                    // ServicesActive database
                        dwDesiredAccess);  // full access rights

    if (NULL == (*schSCManager)) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        if(errorCode) {
            *errorCode = GetLastError();
        }
        return false;
    }

    return true;
}

bool WinUtilities::serviceOpenService(const QString &serviceName, SC_HANDLE *schSCManager, SC_HANDLE *schService, DWORD *errorCode, DWORD dwDesiredAccess)
{

    if(serviceName.trimmed().isEmpty() || !schSCManager || !schService) {
        return false;
    }

    //    // Get a handle to the SCM database.
    //    *schSCManager = OpenSCManager(
    //                NULL,                    // local computer
    //                NULL,                    // ServicesActive database
    //                SC_MANAGER_ALL_ACCESS);  // full access rights

    //    if (NULL == (*schSCManager))
    //    {
    //        printf("OpenSCManager failed (%d)\n", GetLastError());
    //        if(errorCode){
    //            *errorCode = GetLastError();
    //        }
    //        return false;
    //    }

    // Get a handle to the service.

    *schService = OpenServiceW(
                      (*schSCManager),          // SCM database
                      serviceName.toStdWString().c_str(),             // name of service
                      dwDesiredAccess); // need query config access

    if (*schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        if(errorCode) {
            *errorCode = GetLastError();
        }
        //CloseServiceHandle(*schSCManager);
        return false;
    }

    return true;
}

bool WinUtilities::serviceQueryInfo(const QString &serviceName, ServiceInfo *serviceInfo, DWORD *errorCode)
{
    if(serviceName.trimmed().isEmpty() || !serviceInfo) {
        return false;
    }

    SC_HANDLE schSCManager;
    //    SC_HANDLE schService;
    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok) {
        return false;
    }

    ok = serviceQueryInfo(&schSCManager, serviceName, serviceInfo, errorCode);

    CloseServiceHandle(schSCManager);

    return ok;


    //    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode);
    //    if(!ok){
    //        return false;
    //    }

    //    LPQUERY_SERVICE_CONFIG lpServiceConfig;
    //    LPSERVICE_DESCRIPTION lpsd;
    //    DWORD dwBytesNeeded, cbBufSize, dwError;

    //    // Get the configuration information.
    //    if( !QueryServiceConfig(
    //                schService,
    //                NULL,
    //                0,
    //                &dwBytesNeeded))
    //    {
    //        dwError = GetLastError();
    //        if( ERROR_INSUFFICIENT_BUFFER == dwError )
    //        {
    //            cbBufSize = dwBytesNeeded;
    //            lpServiceConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LMEM_FIXED, cbBufSize);
    //        }
    //        else
    //        {
    //            printf("QueryServiceConfig failed (%d)", dwError);
    //            if(errorCode){
    //                *errorCode = dwError;
    //            }
    //            goto cleanup;
    //        }
    //    }

    //    if( !QueryServiceConfig(
    //                schService,
    //                lpServiceConfig,
    //                cbBufSize,
    //                &dwBytesNeeded) )
    //    {
    //        printf("QueryServiceConfig failed (%d)", GetLastError());
    //        if(errorCode){
    //            *errorCode = GetLastError();
    //        }
    //        goto cleanup;
    //    }

    //    if( !QueryServiceConfig2(
    //                schService,
    //                SERVICE_CONFIG_DESCRIPTION,
    //                NULL,
    //                0,
    //                &dwBytesNeeded))
    //    {
    //        dwError = GetLastError();
    //        if( ERROR_INSUFFICIENT_BUFFER == dwError )
    //        {
    //            cbBufSize = dwBytesNeeded;
    //            lpsd = (LPSERVICE_DESCRIPTION) LocalAlloc(LMEM_FIXED, cbBufSize);
    //        }
    //        else
    //        {
    //            printf("QueryServiceConfig2 failed (%d)", dwError);
    //            if(errorCode){
    //                *errorCode = dwError;
    //            }
    //            goto cleanup;
    //        }
    //    }

    //    if (! QueryServiceConfig2(
    //                schService,
    //                SERVICE_CONFIG_DESCRIPTION,
    //                (LPBYTE) lpsd,
    //                cbBufSize,
    //                &dwBytesNeeded) )
    //    {
    //        printf("QueryServiceConfig2 failed (%d)", GetLastError());
    //        if(errorCode){
    //            *errorCode = GetLastError();
    //        }
    //        goto cleanup;
    //    }

    //    serviceInfo->serviceName = serviceName;
    //    serviceInfo->displayName = QString::fromWCharArray(lpServiceConfig->lpDisplayName);
    //    serviceInfo->description = QString::fromWCharArray(lpsd->lpDescription);;

    //    serviceInfo->serviceType = lpServiceConfig->dwServiceType;
    //    serviceInfo->startType = lpServiceConfig->dwStartType;
    //    serviceInfo->binaryPath = QString::fromWCharArray(lpServiceConfig->lpBinaryPathName);
    //    serviceInfo->account = QString::fromWCharArray(lpServiceConfig->lpServiceStartName);

    //    serviceInfo->dependencies = QString::fromWCharArray(lpServiceConfig->lpDependencies);



    ////     //Print the configuration information.
    ////    qDebug()<<"configuration: "<< serviceName;
    ////    qDebug()<<"  Type: "<< lpServiceConfig->dwServiceType;
    ////    qDebug()<<"  Start Type: "<< lpServiceConfig->dwStartType;
    ////    qDebug()<<"  Error Control: "<< lpServiceConfig->dwErrorControl;
    ////    qDebug()<<"  Binary path: "<< QString::fromWCharArray(lpServiceConfig->lpBinaryPathName);
    ////    qDebug()<<"  Account: "<< QString::fromWCharArray(lpServiceConfig->lpServiceStartName);
    ////    qDebug()<<"  Display Name: "<< QString::fromWCharArray(lpServiceConfig->lpDisplayName);

    ////    if (lpsd->lpDescription != NULL && lstrcmp(lpsd->lpDescription, TEXT("")) != 0)
    ////        qDebug()<<"  Description: "<< QString::fromWCharArray(lpsd->lpDescription);
    ////    if (lpServiceConfig->lpLoadOrderGroup != NULL && lstrcmp(lpServiceConfig->lpLoadOrderGroup, TEXT("")) != 0)
    ////        qDebug()<<"  Load order group: "<< QString::fromWCharArray(lpServiceConfig->lpLoadOrderGroup);
    ////    if (lpServiceConfig->dwTagId != 0)
    ////        qDebug()<<"  Tag ID: "<< lpServiceConfig->dwTagId;
    ////    if (lpServiceConfig->lpDependencies != NULL && lstrcmp(lpServiceConfig->lpDependencies, TEXT("")) != 0)
    ////        qDebug()<<"  Dependencies: "<< QString::fromWCharArray(lpServiceConfig->lpDependencies);




    //    LocalFree(lpServiceConfig);
    //    LocalFree(lpsd);

    //cleanup:
    //    CloseServiceHandle(schService);
    //    CloseServiceHandle(schSCManager);

    //    return true;

}

bool WinUtilities::serviceQueryInfo(SC_HANDLE *schSCManager, const QString &serviceName, ServiceInfo *serviceInfo, DWORD *errorCode)
{
    if(!schSCManager || serviceName.trimmed().isEmpty() || !serviceInfo) {
        return false;
    }

    SC_HANDLE schService;
    bool ok = serviceOpenService(serviceName, schSCManager, &schService, errorCode, SERVICE_QUERY_CONFIG );
    if(!ok) {
        return false;
    }

    LPQUERY_SERVICE_CONFIG lpServiceConfig;
    LPSERVICE_DESCRIPTION lpsd;
    DWORD dwBytesNeeded, cbBufSize, dwError;

    QString dependencies;

    // Get the configuration information.
    if( !QueryServiceConfig(
                schService,
                NULL,
                0,
                &dwBytesNeeded)) {
        dwError = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER == dwError ) {
            cbBufSize = dwBytesNeeded;
            lpServiceConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LMEM_FIXED, cbBufSize);
        } else {
            qCritical() << QString("ERROR! QueryServiceConfig failed. Error code: %1. Service Name: %2").arg(dwError).arg(serviceName);
            if(errorCode) {
                *errorCode = dwError;
            }
            //goto cleanup;
            serviceCloseHandle(0, &schService);
            return false;
        }
    }

    if( !QueryServiceConfig(
                schService,
                lpServiceConfig,
                cbBufSize,
                &dwBytesNeeded) ) {
        qCritical() << QString("ERROR! QueryServiceConfig failed. Error code: %1. Service Name: %2").arg(dwError).arg(serviceName);
        if(errorCode) {
            *errorCode = GetLastError();
        }
        //goto cleanup;
        serviceCloseHandle(0, &schService);
        return false;
    }

    if( !QueryServiceConfig2(
                schService,
                SERVICE_CONFIG_DESCRIPTION,
                NULL,
                0,
                &dwBytesNeeded)) {
        dwError = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER == dwError ) {
            cbBufSize = dwBytesNeeded;
            lpsd = (LPSERVICE_DESCRIPTION) LocalAlloc(LMEM_FIXED, cbBufSize);
        } else {
            qCritical() << QString("ERROR! QueryServiceConfig2 failed. Error code: %1. Service Name: %2").arg(GetLastError()).arg(serviceName);
            if(errorCode) {
                *errorCode = dwError;
            }
            //goto cleanup;
            serviceCloseHandle(0, &schService);
            return false;
        }
    }

    if (! QueryServiceConfig2(
                schService,
                SERVICE_CONFIG_DESCRIPTION,
                (LPBYTE) lpsd,
                cbBufSize,
                &dwBytesNeeded) ) {
        qCritical() << QString("ERROR! QueryServiceConfig2 failed. Error code: %1. Service Name: %2").arg(GetLastError()).arg(serviceName);
        if(errorCode) {
            *errorCode = GetLastError();
        }
        //goto cleanup;
        serviceCloseHandle(0, &schService);
        return false;
    }

    serviceInfo->serviceName = serviceName;
    serviceInfo->displayName = QString::fromWCharArray(lpServiceConfig->lpDisplayName);
    serviceInfo->description = QString::fromWCharArray(lpsd->lpDescription);;

    serviceInfo->serviceType = lpServiceConfig->dwServiceType;
    serviceInfo->startType = lpServiceConfig->dwStartType;
    serviceInfo->binaryPath = QString::fromWCharArray(lpServiceConfig->lpBinaryPathName);
    serviceInfo->account = QString::fromWCharArray(lpServiceConfig->lpServiceStartName);


    for(int i = 0; i < 256; i++) {
        QChar ch = lpServiceConfig->lpDependencies[i];
        if(lpServiceConfig->lpDependencies[i] == '\0') {
            if(lpServiceConfig->lpDependencies[i + 1] == '\0') {
                break;
            }
            ch = ';';
        }
        dependencies.append(ch);
    }
    serviceInfo->dependencies = dependencies;


    //     //Print the configuration information.
    //    qDebug()<<"configuration: "<< serviceName;
    //    qDebug()<<"  Type: "<< lpServiceConfig->dwServiceType;
    //    qDebug()<<"  Start Type: "<< lpServiceConfig->dwStartType;
    //    qDebug()<<"  Error Control: "<< lpServiceConfig->dwErrorControl;
    //    qDebug()<<"  Binary path: "<< QString::fromWCharArray(lpServiceConfig->lpBinaryPathName);
    //    qDebug()<<"  Account: "<< QString::fromWCharArray(lpServiceConfig->lpServiceStartName);
    //    qDebug()<<"  Display Name: "<< QString::fromWCharArray(lpServiceConfig->lpDisplayName);

    //    if (lpsd->lpDescription != NULL && lstrcmp(lpsd->lpDescription, TEXT("")) != 0)
    //        qDebug()<<"  Description: "<< QString::fromWCharArray(lpsd->lpDescription);
    //    if (lpServiceConfig->lpLoadOrderGroup != NULL && lstrcmp(lpServiceConfig->lpLoadOrderGroup, TEXT("")) != 0)
    //        qDebug()<<"  Load order group: "<< QString::fromWCharArray(lpServiceConfig->lpLoadOrderGroup);
    //    if (lpServiceConfig->dwTagId != 0)
    //        qDebug()<<"  Tag ID: "<< lpServiceConfig->dwTagId;
    //    if (lpServiceConfig->lpDependencies != NULL && lstrcmp(lpServiceConfig->lpDependencies, TEXT("")) != 0)
    //        qDebug()<<"  Dependencies: "<< QString::fromWCharArray(lpServiceConfig->lpDependencies);




    LocalFree(lpServiceConfig);
    LocalFree(lpsd);

    //cleanup:
    //CloseServiceHandle(schService);
    //CloseServiceHandle(schSCManager);

    return true;
}

bool WinUtilities::serviceChangeStartType(const QString &serviceName, DWORD startType, DWORD *errorCode)
{

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok) {
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode);
    if(!ok) {
        //CloseServiceHandle(schSCManager);
        serviceCloseHandle(&schSCManager, 0);
        return false;
    }

    // Change the service start type.

    ok =  ChangeServiceConfig(
              schService,            // handle of service
              SERVICE_NO_CHANGE,     // service type: no change
              startType,             // service start type
              SERVICE_NO_CHANGE,     // error control: no change
              NULL,                  // binary path: no change
              NULL,                  // load order group: no change
              NULL,                  // tag ID: no change
              NULL,                  // dependencies: no change
              NULL,                  // account name: no change
              NULL,                  // password: no change
              NULL);

    if (!ok) {              // display name: no change
        printf("ChangeServiceConfig failed (%d)\n", GetLastError());
        if(errorCode) {
            *errorCode = GetLastError();
        }
    }

    //CloseServiceHandle(schService);
    //CloseServiceHandle(schSCManager);
    serviceCloseHandle(&schSCManager, &schService);

    return ok;
}

bool WinUtilities::serviceChangeDescription(const QString &serviceName, const QString &description, DWORD *errorCode)
{

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok) {
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode);
    if(!ok) {
        //CloseServiceHandle(schSCManager);
        serviceCloseHandle(&schSCManager, 0);
        return false;
    }

    // Change the service description.

    SERVICE_DESCRIPTION sd;
    wchar_t desc[512];
    wcscpy(desc, description.toStdWString().c_str());
    sd.lpDescription = desc;

    ok = ChangeServiceConfig2(
             schService,                 // handle to service
             SERVICE_CONFIG_DESCRIPTION, // change: description
             &sd);

    if(!ok) {
        printf("ChangeServiceConfig2 failed\n");
        if(errorCode) {
            *errorCode = GetLastError();
        }
    }

    //CloseServiceHandle(schService);
    //CloseServiceHandle(schSCManager);
    serviceCloseHandle(&schSCManager, &schService);

    return ok;
}

bool WinUtilities::serviceDelete(const QString &serviceName, DWORD *errorCode)
{

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok) {
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode);
    if(!ok) {
        //CloseServiceHandle(schSCManager);
        serviceCloseHandle(&schSCManager, 0);
        return false;
    }

    // Delete the service.

    ok = DeleteService(schService);
    if(!ok) {
        printf("DeleteService failed (%d)\n", GetLastError());
        if(errorCode) {
            *errorCode = GetLastError();
        }
    } else {
        printf("Service deleted successfully\n");
    }

    //CloseServiceHandle(schService);
    //CloseServiceHandle(schSCManager);
    serviceCloseHandle(&schSCManager, &schService);

    return ok;
}

bool WinUtilities::serviceGetAllServicesInfo(QJsonArray *jsonArray, DWORD serviceType, DWORD *errorCode)
{

    if(!jsonArray) {
        return false;
    }

    SC_HANDLE schSCManager;
    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok) {
        return false;
    }

    PUCHAR  pBuf    = NULL;
    ULONG  dwBufSize   = 0x00;
    ULONG  dwBufNeed   = 0x00;
    ULONG  dwNumberOfService = 0x00;
    LPENUM_SERVICE_STATUS_PROCESS pInfo = NULL;

    EnumServicesStatusEx(
        schSCManager,
        SC_ENUM_PROCESS_INFO,
        serviceType, // SERVICE_DRIVER
        SERVICE_STATE_ALL,
        NULL,
        dwBufSize,
        &dwBufNeed,
        &dwNumberOfService,
        NULL,
        NULL);

    if (dwBufNeed < 0x01) {
        printf_s("EnumServicesStatusEx failed. \n");
        if(errorCode) {
            *errorCode = GetLastError();
        }
        return false;
    }

    dwBufSize = dwBufNeed + 0x10;
    pBuf  = (PUCHAR) malloc(dwBufSize);

    EnumServicesStatusEx(
        schSCManager,
        SC_ENUM_PROCESS_INFO,
        serviceType,  // SERVICE_DRIVER,
        SERVICE_STATE_ALL,  //SERVICE_STATE_ALL,SERVICE_ACTIVE
        pBuf,
        dwBufSize,
        &dwBufNeed,
        &dwNumberOfService,
        NULL,
        NULL);

    pInfo = (LPENUM_SERVICE_STATUS_PROCESS)pBuf;
    for (ULONG i = 0; i < dwNumberOfService; i++) {
        //        qDebug();
        //        qDebug()<<i<<"."<<"Display Name \t : "<< QString::fromWCharArray(pInfo[i].lpDisplayName);
        //        qDebug()<<"Service Name \t : "<< QString::fromWCharArray(pInfo[i].lpServiceName);
        //        qDebug()<<"Process Id \t : "<< QString::number( pInfo[i].ServiceStatusProcess.dwProcessId);

        QString serviceName = QString::fromWCharArray(pInfo[i].lpServiceName);
        if(serviceName.trimmed().isEmpty()) {
            continue;
        }

        QString displayName = QString::fromWCharArray(pInfo[i].lpDisplayName);
        DWORD processID = pInfo[i].ServiceStatusProcess.dwProcessId;

        QJsonArray array;
        array.append(serviceName);
        array.append(displayName);
        array.append(QString::number(processID));

        ServiceInfo serviceInfo;

        //        ok = serviceQueryInfo(serviceName, &serviceInfo, errorCode);
        ok = serviceQueryInfo(&schSCManager, serviceName, &serviceInfo, errorCode);

        //        if(ok){
        //            serviceQueryInfo(serviceName, &serviceInfo, errorCode);
        //        }

        array.append(serviceInfo.description);
        array.append(QString::number(serviceInfo.startType));
        array.append(serviceInfo.account);
        array.append(serviceInfo.dependencies);
        array.append(serviceInfo.binaryPath);
        array.append(QString::number(serviceInfo.serviceType));

        //array.append(errorControl);
        //array.append(loadOrderGroup);
        //array.append(tagID);


        jsonArray->append(array);


    }
    qDebug() << "----------jsonArray->size():" << jsonArray->size();

    free(pBuf);
    //CloseServiceHandle(schSCManager);
    serviceCloseHandle(&schSCManager, 0);

    return true;
}

bool WinUtilities::serviceStart(const QString &serviceName, DWORD *errorCode)
{

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok) {
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode, SERVICE_QUERY_STATUS | SERVICE_START);
    if(!ok) {
        //CloseServiceHandle(schSCManager);
        serviceCloseHandle(&schSCManager, 0);
        return false;
    }

    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwOldCheckPoint;
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;

    // Check the status in case the service is not stopped.

    if (!QueryServiceStatusEx(
                schService,                     // handle to service
                SC_STATUS_PROCESS_INFO,         // information level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) ) {            // size needed if buffer is too small
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        //CloseServiceHandle(schService);
        //CloseServiceHandle(schSCManager);
        if(errorCode) {
            *errorCode = GetLastError();
        }
        serviceCloseHandle(&schSCManager, &schService);
        return false;
    }

    // Check if the service is already running. It would be possible
    // to stop the service here, but for simplicity this example just returns.
    if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
        printf("Cannot start the service because it is already running\n");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return true;
    }

    // Save the tick count and initial checkpoint.
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.
    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.

        dwWaitTime = ssStatus.dwWaitHint / 10;
        if( dwWaitTime < 1000 ) {
            dwWaitTime = 1000;
        } else if ( dwWaitTime > 10000 ) {
            dwWaitTime = 10000;
        }

        Sleep( dwWaitTime );

        // Check the status until the service is no longer stop pending.
        if (!QueryServiceStatusEx(
                    schService,                     // handle to service
                    SC_STATUS_PROCESS_INFO,         // information level
                    (LPBYTE) &ssStatus,             // address of structure
                    sizeof(SERVICE_STATUS_PROCESS), // size of structure
                    &dwBytesNeeded ) ) {            // size needed if buffer is too small
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            //CloseServiceHandle(schService);
            //CloseServiceHandle(schSCManager);
            if(errorCode) {
                *errorCode = GetLastError();
            }
            serviceCloseHandle(&schSCManager, &schService);
            return false;
        }

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint ) {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if(GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                qCritical("Timeout waiting for service to stop\n");
                //CloseServiceHandle(schService);
                //CloseServiceHandle(schSCManager);
                serviceCloseHandle(&schSCManager, &schService);
                return false;
            }
        }
    }

    // Attempt to start the service.
    if (!StartService(
                schService,  // handle to service
                0,           // number of arguments
                NULL) ) {    // no arguments
        printf("StartService failed (%d)\n", GetLastError());
        //CloseServiceHandle(schService);
        //CloseServiceHandle(schSCManager);
        if(errorCode) {
            *errorCode = GetLastError();
        }
        serviceCloseHandle(&schSCManager, &schService);
        return false;
    } else {
        printf("Service start pending...\n");
    }

    // Check the status until the service is no longer start pending.
    if (!QueryServiceStatusEx(
                schService,                     // handle to service
                SC_STATUS_PROCESS_INFO,         // info level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) ) {            // if buffer too small
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        //CloseServiceHandle(schService);
        //CloseServiceHandle(schSCManager);
        if(errorCode) {
            *errorCode = GetLastError();
        }
        serviceCloseHandle(&schSCManager, &schService);
        return false;
    }

    // Save the tick count and initial checkpoint.
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;
    while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth the wait hint, but no less than 1 second and no
        // more than 10 seconds.
        dwWaitTime = ssStatus.dwWaitHint / 10;
        if( dwWaitTime < 1000 ) {
            dwWaitTime = 1000;
        } else if ( dwWaitTime > 10000 ) {
            dwWaitTime = 10000;
        }

        Sleep( dwWaitTime );

        // Check the status again.
        if (!QueryServiceStatusEx(
                    schService,             // handle to service
                    SC_STATUS_PROCESS_INFO, // info level
                    (LPBYTE) &ssStatus,             // address of structure
                    sizeof(SERVICE_STATUS_PROCESS), // size of structure
                    &dwBytesNeeded ) ) {            // if buffer too small
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            break;
        }

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint ) {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if(GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                // No progress made within the wait hint.
                break;
            }
        }
    }

    // Determine whether the service is running.
    ok = (ssStatus.dwCurrentState == SERVICE_RUNNING);
    if (ok) {
        printf("Service started successfully.\n");
    } else {
        printf("Service not started. \n");
        printf("  Current State: %d\n", ssStatus.dwCurrentState);
        printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
        printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
        printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
    }

    //CloseServiceHandle(schService);
    //CloseServiceHandle(schSCManager);
    serviceCloseHandle(&schSCManager, &schService);

    return ok;

}

bool WinUtilities::serviceStop(const QString &serviceName, DWORD *errorCode)
{

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok) {
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode, SERVICE_QUERY_STATUS | SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS);
    if(!ok) {
        //CloseServiceHandle(schSCManager);
        serviceCloseHandle(&schSCManager, 0);
        return false;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD dwStartTime = GetTickCount();
    DWORD dwBytesNeeded;
    DWORD dwTimeout = 30000; // 30-second time-out
    DWORD dwWaitTime;

    // Make sure the service is not already stopped.

    if ( !QueryServiceStatusEx(
                schService,
                SC_STATUS_PROCESS_INFO,
                (LPBYTE)&ssp,
                sizeof(SERVICE_STATUS_PROCESS),
                &dwBytesNeeded ) ) {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        serviceCloseHandle(&schSCManager, &schService);
        if(errorCode) {
            *errorCode = GetLastError();
        }
        return false;
    }

    if ( ssp.dwCurrentState == SERVICE_STOPPED ) {
        printf("Service is already stopped.\n");
        serviceCloseHandle(&schSCManager, &schService);
        return true;
    }

    // If a stop is pending, wait for it.

    while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) {
        printf("Service stop pending...\n");

        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.

        dwWaitTime = ssp.dwWaitHint / 10;

        if( dwWaitTime < 1000 ) {
            dwWaitTime = 1000;
        } else if ( dwWaitTime > 10000 ) {
            dwWaitTime = 10000;
        }

        Sleep( dwWaitTime );

        if ( !QueryServiceStatusEx(
                    schService,
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE)&ssp,
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded ) ) {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            serviceCloseHandle(&schSCManager, &schService);
            if(errorCode) {
                *errorCode = GetLastError();
            }
            return false;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED ) {
            printf("Service stopped successfully.\n");
            serviceCloseHandle(&schSCManager, &schService);
            return true;
        }

        if ( GetTickCount() - dwStartTime > dwTimeout ) {
            printf("Service stop timed out.\n");
            serviceCloseHandle(&schSCManager, &schService);
            return false;
        }
    }

    // If the service is running, dependencies must be stopped first.
    serviceStopDependentServices(&schSCManager, &schService);

    // Send a stop code to the service.
    if ( !ControlService(
                schService,
                SERVICE_CONTROL_STOP,
                (LPSERVICE_STATUS) &ssp ) ) {
        printf( "ControlService failed (%d)\n", GetLastError() );
        serviceCloseHandle(&schSCManager, &schService);
        if(errorCode) {
            *errorCode = GetLastError();
        }
        return false;
    }

    // Wait for the service to stop.

    while ( ssp.dwCurrentState != SERVICE_STOPPED ) {
        Sleep( ssp.dwWaitHint );
        if ( !QueryServiceStatusEx(
                    schService,
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE)&ssp,
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded ) ) {
            printf( "QueryServiceStatusEx failed (%d)\n", GetLastError() );
            serviceCloseHandle(&schSCManager, &schService);
            if(errorCode) {
                *errorCode = GetLastError();
            }
            return false;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED ) {
            break;
        }

        if ( GetTickCount() - dwStartTime > dwTimeout ) {
            printf( "Wait timed out\n" );
            serviceCloseHandle(&schSCManager, &schService);
            return false;
        }
    }
    printf("Service stopped successfully\n");

    return true;
}

bool WinUtilities::serviceStopDependentServices(SC_HANDLE *schSCManager, SC_HANDLE *schService)
{
    DWORD i;
    DWORD dwBytesNeeded;
    DWORD dwCount;

    LPENUM_SERVICE_STATUS   lpDependencies = NULL;
    ENUM_SERVICE_STATUS     ess;
    SC_HANDLE               hDepService;
    SERVICE_STATUS_PROCESS  ssp;

    DWORD dwStartTime = GetTickCount();
    DWORD dwTimeout = 30000; // 30-second time-out

    // Pass a zero-length buffer to get the required buffer size.
    if ( EnumDependentServices( *schService, SERVICE_ACTIVE,
                                lpDependencies, 0, &dwBytesNeeded, &dwCount ) ) {
        // If the Enum call succeeds, then there are no dependent
        // services, so do nothing.
        return TRUE;
    } else {
        if ( GetLastError() != ERROR_MORE_DATA ) {
            return FALSE;    // Unexpected error
        }

        // Allocate a buffer for the dependencies.
        lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc(
                             GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );

        if ( !lpDependencies ) {
            return FALSE;
        }


        // Enumerate the dependencies.
        if ( !EnumDependentServices( *schService, SERVICE_ACTIVE,
                                     lpDependencies, dwBytesNeeded, &dwBytesNeeded,
                                     &dwCount ) ) {
            HeapFree( GetProcessHeap(), 0, lpDependencies );
            return FALSE;
        }

        for ( i = 0; i < dwCount; i++ ) {
            ess = *(lpDependencies + i);
            // Open the service.
            hDepService = OpenService( *schSCManager,
                                       ess.lpServiceName,
                                       SERVICE_STOP | SERVICE_QUERY_STATUS );

            if ( !hDepService ) {
                HeapFree( GetProcessHeap(), 0, lpDependencies );
                return FALSE;
            }


            // Send a stop code.
            if ( !ControlService( hDepService,
                                  SERVICE_CONTROL_STOP,
                                  (LPSERVICE_STATUS) &ssp ) ) {
                CloseServiceHandle( hDepService );
                HeapFree( GetProcessHeap(), 0, lpDependencies );
                return FALSE;
            }

            // Wait for the service to stop.
            while ( ssp.dwCurrentState != SERVICE_STOPPED ) {
                Sleep( ssp.dwWaitHint );
                if ( !QueryServiceStatusEx(
                            hDepService,
                            SC_STATUS_PROCESS_INFO,
                            (LPBYTE)&ssp,
                            sizeof(SERVICE_STATUS_PROCESS),
                            &dwBytesNeeded ) ) {
                    CloseServiceHandle( hDepService );
                    HeapFree( GetProcessHeap(), 0, lpDependencies );
                    return FALSE;
                }


                if ( ssp.dwCurrentState == SERVICE_STOPPED ) {
                    break;
                }

                if ( GetTickCount() - dwStartTime > dwTimeout ) {
                    CloseServiceHandle( hDepService );
                    HeapFree( GetProcessHeap(), 0, lpDependencies );
                    return FALSE;
                }
            }


        }

    }
    return TRUE;
}


void WinUtilities::serviceCloseHandle(SC_HANDLE *schSCManager, SC_HANDLE *schService)
{
    if(schService) {
        CloseServiceHandle(*schService);
    }

    if(schSCManager) {
        CloseServiceHandle(*schSCManager);
    }
}




BOOL WinUtilities::EnableShutdownPrivilege()
{
    HANDLE hProcess = NULL;
    HANDLE hToken = NULL;
    LUID uID = {0};
    TOKEN_PRIVILEGES stToken_Privileges = {0};

    hProcess = ::GetCurrentProcess();
    if(!::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        return FALSE;
    }

    if(!::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &uID)) {
        return FALSE;
    }

    stToken_Privileges.PrivilegeCount = 1;
    stToken_Privileges.Privileges[0].Luid = uID;
    stToken_Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if(!::AdjustTokenPrivileges(hToken, FALSE, &stToken_Privileges, sizeof stToken_Privileges, NULL, NULL)) {
        return FALSE;
    }

    if(::GetLastError() != ERROR_SUCCESS) {
        return FALSE;
    }

    ::CloseHandle(hToken);
    return TRUE;
}

//Shutdown
BOOL WinUtilities::Shutdown(BOOL bForce)
{
    EnableShutdownPrivilege();
    if(bForce) {
        return ::ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
    } else {
        return ::ExitWindowsEx(EWX_SHUTDOWN, 0);
    }
}

BOOL WinUtilities::Shutdown(const QString &machineName, const QString &message, DWORD timeout, bool forceAppsClosed, bool rebootAfterShutdown)
{
    EnableShutdownPrivilege();


    wchar_t lpMachineName[64], lpMessage[3072];
    memset(lpMachineName, 0, sizeof(lpMachineName));
    memset(lpMessage, 0, sizeof(lpMessage));

    wcscpy(lpMachineName, machineName.toStdWString().c_str());
    wcscpy(lpMessage, message.toStdWString().c_str());

    // Display the shutdown dialog box and start the countdown.
    BOOL fResult = InitiateSystemShutdownW(
                       lpMachineName,    // NULL:shut down local computer
                       lpMessage,   // message for user
                       timeout,      // time-out period, in seconds
                       forceAppsClosed,   // if ask user to close apps
                       rebootAfterShutdown);   // reboot after shutdown

    return fResult;
}

//Logoff数
BOOL WinUtilities::Logoff(BOOL bForce)
{
    if(bForce) {
        return ::ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, 0);
    } else {
        return ::ExitWindowsEx(EWX_LOGOFF, 0);
    }
}

//Reboot
BOOL WinUtilities::Reboot(BOOL bForce)
{
    EnableShutdownPrivilege();
    if(bForce) {
        return ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
    } else {
        return ::ExitWindowsEx(EWX_REBOOT, 0);
    }
}

BOOL WinUtilities::LockWindows()
{
    return LockWorkStation();
}



//////////////////////////////////////////////////////

bool WinUtilities::runAs(const QString &userName, const QString &domainName, const QString &password, const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir, bool wait, DWORD milliseconds)
{
    qDebug() << "----WinUtilities::runAs(...)";
    //qDebug()<<"User Name Of CurrentThread:"<<m_currentUserName;


    if(userName.simplified().isEmpty()) {
        return false;
    }

    //    wchar_t name[MaxUserAccountNameLength*sizeof(wchar_t)+1];
    //    wcscpy(name, userName.toStdWString().c_str());

    //    wchar_t domain[MaxGroupNameLength*sizeof(wchar_t)+1];
    //    wcscpy(domain, domainName.toStdWString().c_str());

    //    wchar_t pwd[MaxUserPasswordLength*sizeof(wchar_t)+1];
    //    wcscpy(pwd, password.toStdWString().c_str());


    //服务程序以"SYSTEM"身份运行，无法调用CreateProcessWithLogonW，必须用LogonUser和CreateProcessAsUser
    //You cannot call CreateProcessWithLogonW from a process that is running under the LocalSystem account,
    //  because the function uses the logon SID in the caller token, and the token for the LocalSystem account does not contain this SID.
    //  As an alternative, use the CreateProcessAsUser and LogonUser functions.
    if(getUserNameOfCurrentThread().toUpper() == "SYSTEM") {
        return runAsForInteractiveService(userName, domainName, password, exeFilePath, parameters, show, workingDir);
    } else {
        return runAsForDesktopApplication(userName, domainName, password, exeFilePath, parameters, show, workingDir, wait);
    }

}

bool WinUtilities::runAsForInteractiveService(const QString &userName, const QString &domainName, const QString &password, const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir)
{

    wchar_t name[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(name, userName.toStdWString().c_str());

    wchar_t domain[512];
    if(domainName.trimmed().isEmpty()) {
        wcscpy(domain, L".");
    } else {
        wcscpy(domain, domainName.toStdWString().c_str());
    }

    wchar_t pwd[MaxUserPasswordLength * sizeof(wchar_t) + 1];
    wcscpy(pwd, password.toStdWString().c_str());

    QString cmdStr = QString("\"" + exeFilePath + "\" " + parameters);
    wchar_t cmdLine[32000 * sizeof(wchar_t) + 1];
    wcscpy(cmdLine, cmdStr.toStdWString().c_str());

    wchar_t currentDirectory[MAX_PATH * sizeof(wchar_t) + 1];
    if(workingDir.trimmed().isEmpty()) {
        wcscpy(currentDirectory, QCoreApplication::applicationDirPath().toStdWString().c_str());
    } else {
        wcscpy(currentDirectory, workingDir.toStdWString().c_str());
    }

    DWORD errorCode;
    if(isNT6OS() && show) {
        DWORD sessionID;
        if(!getUserSessionID(name, &sessionID)) {
            errorCode = runAsForNT5InteractiveService(name, domain, pwd, NULL, cmdLine, currentDirectory, show);
        } else {
            errorCode = runAsForNT6InteractiveService(sessionID, NULL, cmdLine, currentDirectory, show);
        }
    } else {
        errorCode = runAsForNT5InteractiveService(name, domain, pwd, NULL, cmdLine, currentDirectory, show);
    }

    if(ERROR_SUCCESS != errorCode) {
        qCritical() << QString("Failed to start process '%1'! %2:%3.").arg(exeFilePath).arg(errorCode).arg(WinUtilities::WinSysErrorMsg(errorCode));
        return false;
    }

    return true;
}

bool WinUtilities::runAsForDesktopApplication(const QString &userName, const QString &domainName, const QString &password, const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir, bool wait, DWORD milliseconds)
{

    wchar_t name[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(name, userName.toStdWString().c_str());

    wchar_t domain[512];
    if(domainName.trimmed().isEmpty()) {
        wcscpy(domain, L".");
    } else {
        wcscpy(domain, domainName.toStdWString().c_str());
    }

    wchar_t pwd[MaxUserPasswordLength * sizeof(wchar_t) + 1];
    wcscpy(pwd, password.toStdWString().c_str());

    wchar_t cmdLine[8192 * sizeof(wchar_t) + 1];
    wcscpy(cmdLine, parameters.toStdWString().c_str());
    //wcscpy(cmdLine, QString("\"" + exeFilePath + "\" " + parameters).toStdWString().c_str());

    wchar_t currentDirectory[MAX_PATH * sizeof(wchar_t) + 1];
    if(workingDir.trimmed().isEmpty()) {
        wcscpy(currentDirectory, QCoreApplication::applicationDirPath().toStdWString().c_str());
    } else {
        wcscpy(currentDirectory, workingDir.toStdWString().c_str());
    }

    DWORD dwRet;
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    //ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    if(show) {
        si.wShowWindow = SW_SHOW;
    } else {
        si.wShowWindow = SW_HIDE;
    }

    //    HANDLE hToken = NULL;
    //    LPVOID lpvEnv = NULL;
    //    WCHAR szUserProfile[512] = L"";
    //    DWORD dwSize = sizeof(szUserProfile)/sizeof(WCHAR);
    //    wcscpy(szUserProfile, workingDir.toStdWString().c_str());
    //    if(LogonUserW(name, domain, pwd, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)){
    //        if (!CreateEnvironmentBlock(&lpvEnv, hToken, TRUE)){
    //            dwRet = GetLastError();
    //            m_lastErrorString = tr("Can not create environment block! %1: %2").arg(dwRet).arg(WinUtilities::WinSysErrorMsg(dwRet));
    //            //return false;
    //        }
    //        if (!GetUserProfileDirectoryW(hToken, szUserProfile, &dwSize)){
    //            dwRet = GetLastError();
    //            m_lastErrorString = tr("Can not get user profile directory! %1: %2").arg(dwRet).arg(WinUtilities::WinSysErrorMsg(dwRet));
    //            //return false;
    //        }
    //    }else{
    //        m_lastErrorString = tr("Can not log user %1 on to this computer! Error code:%2").arg(userName).arg(GetLastError());
    //        return false;
    //    }

    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT;
    //bool ok = CreateProcessWithLogonW(name, domain, pwd, LOGON_WITH_PROFILE, NULL, cmdLine, dwCreationFlags, lpvEnv, szUserProfile, &si, &pi);
    bool ok = CreateProcessWithLogonW(name, domain, pwd, LOGON_WITH_PROFILE, exeFilePath.toStdWString().c_str(), cmdLine, dwCreationFlags, NULL, currentDirectory, &si, &pi);

    //    if (!DestroyEnvironmentBlock(lpvEnv)){
    //        dwRet = GetLastError();
    //        m_lastErrorString = tr("Can not destroy environment block! %1:%2.").arg(dwRet).arg(WinUtilities::WinSysErrorMsg(dwRet));
    //        qWarning()<<m_lastErrorString;
    //    }
    //    CloseHandle(hToken);


    if(!ok) {
        dwRet = GetLastError();
        QString errorString = QString("Starting process '%1' failed! %2:%3.").arg(exeFilePath).arg(dwRet).arg(WinUtilities::WinSysErrorMsg(dwRet));
        qCritical() << errorString;
        return false;
    }

    if(wait) {
        dwRet = WaitForSingleObject(pi.hProcess, milliseconds ? milliseconds : INFINITE);
        qDebug() << "dwRet:" << dwRet;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;

}


////////////////////////////////////////////////////





int WinUtilities::GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo *pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if(size == 0) {
        return -1;    // Failure
    }

    pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc(size));
    if(pImageCodecInfo == NULL) {
        return -1;    // Failure
    }

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j) {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 ) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

QByteArray WinUtilities::ConvertHBITMAPToJpeg(HBITMAP hbitmap)
{

    QByteArray byteArray;

    // Initialize GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID   encoderClsid;
    Gdiplus::Status  status;
    //QString tempFilePath = QDir::tempPath() + QString("/hh%1.tmp").arg(QDateTime::currentDateTime().toTime_t());
    QString tempFilePath = QDir::tempPath() + QString("/hh%1.jpg").arg(QDateTime::currentDateTime().toTime_t());

    // Get the CLSID of the jpeg encoder.
    GetEncoderClsid(L"image/jpeg", &encoderClsid);
    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hbitmap, NULL);
    status = bitmap->Save(tempFilePath.toStdWString().c_str(), &encoderClsid, NULL);

    //Image*   image = new Image(L"c:/1.bmp");
    //stat = image->Save(L"c:/1.jpg", &encoderClsid, NULL);
    //delete image;

    delete bitmap;

    Gdiplus::GdiplusShutdown(gdiplusToken);

    if(status != Gdiplus::Ok) {
        qCritical() << "Failed to convert HBITMAP to JPEG.";
        return byteArray;
    }

    QFile file(tempFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return byteArray;
    }
    byteArray = file.readAll();
    //    file.remove();

    return byteArray;

}

//From activeqt/shared/qaxutils.cpp
//QPixmap WinUtilities::WinHBITMAPToPixmap(HBITMAP bitmap, bool noAlpha)
//{
//    // Verify size
//    BITMAP bitmap_info;
//    memset(&bitmap_info, 0, sizeof(BITMAP));

//    const int res = GetObject(bitmap, sizeof(BITMAP), &bitmap_info);
//    if (!res) {
//        qErrnoWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap info");
//        return QPixmap();
//    }
//    const int w = bitmap_info.bmWidth;
//    const int h = bitmap_info.bmHeight;

//    BITMAPINFO bmi;
//    memset(&bmi, 0, sizeof(bmi));
//    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
//    bmi.bmiHeader.biWidth       = w;
//    bmi.bmiHeader.biHeight      = -h;
//    bmi.bmiHeader.biPlanes      = 1;
//    bmi.bmiHeader.biBitCount    = 32;
//    bmi.bmiHeader.biCompression = BI_RGB;
//    bmi.bmiHeader.biSizeImage   = w * h * 4;

//    // Get bitmap bits
//    QScopedArrayPointer<uchar> data(new uchar[bmi.bmiHeader.biSizeImage]);
//    HDC display_dc = GetDC(0);
//    if (!GetDIBits(display_dc, bitmap, 0, h, data.data(), &bmi, DIB_RGB_COLORS)) {
//        ReleaseDC(0, display_dc);
//        qWarning("%s, failed to get bitmap bits", __FUNCTION__);
//        return QPixmap();
//    }

//    QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;
//    uint mask = 0;
//    if (noAlpha) {
//        imageFormat = QImage::Format_RGB32;
//        mask = 0xff000000;
//    }

//    // Create image and copy data into image.
//    QImage image(w, h, imageFormat);
//    if (image.isNull()) { // failed to alloc?
//        ReleaseDC(0, display_dc);
//        qWarning("%s, failed create image of %dx%d", __FUNCTION__, w, h);
//        return QPixmap();
//    }
//    const int bytes_per_line = w * sizeof(QRgb);
//    for (int y = 0; y < h; ++y) {
//        QRgb *dest = (QRgb *) image.scanLine(y);
//        const QRgb *src = (const QRgb *) (data.data() + y * bytes_per_line);
//        for (int x = 0; x < w; ++x) {
//            const uint pixel = src[x];
//            if ((pixel & 0xff000000) == 0 && (pixel & 0x00ffffff) != 0)
//                dest[x] = pixel | 0xff000000;
//            else
//                dest[x] = pixel | mask;
//        }
//    }
//    ReleaseDC(0, display_dc);
//    return QPixmap::fromImage(image);
//}

//HBITMAP WinUtilities::PixmapToWinHBITMAP(const QImage &image, bool noAlpha)
//{
//    if (image.isNull())
//        return 0;

//    HBITMAP bitmap = 0;
//    const int w = image.width();
//    const int h = image.height();

//    HDC display_dc = GetDC(0);

//    // Define the header
//    BITMAPINFO bmi;
//    memset(&bmi, 0, sizeof(bmi));
//    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
//    bmi.bmiHeader.biWidth       = w;
//    bmi.bmiHeader.biHeight      = -h;
//    bmi.bmiHeader.biPlanes      = 1;
//    bmi.bmiHeader.biBitCount    = 32;
//    bmi.bmiHeader.biCompression = BI_RGB;
//    bmi.bmiHeader.biSizeImage   = w * h * 4;

//    // Create the pixmap
//    uchar *pixels = 0;
//    bitmap = CreateDIBSection(display_dc, &bmi, DIB_RGB_COLORS, (void **) &pixels, 0, 0);
//    ReleaseDC(0, display_dc);
//    if (!bitmap) {
//        qErrnoWarning("%s, failed to create dibsection", __FUNCTION__);
//        return 0;
//    }
//    if (!pixels) {
//        qErrnoWarning("%s, did not allocate pixel data", __FUNCTION__);
//        return 0;
//    }

//    // Copy over the data
//    QImage::Format imageFormat = QImage::Format_ARGB32;
//    if (noAlpha)
//        imageFormat = QImage::Format_RGB32;
//    else
//        imageFormat = QImage::Format_ARGB32_Premultiplied;

//    const QImage image2 = image.convertToFormat(imageFormat);
//    const int bytes_per_line = w * 4;
//    for (int y=0; y < h; ++y)
//        memcpy(pixels + y * bytes_per_line, image2.scanLine(y), bytes_per_line);

//    return bitmap;
//}

HBITMAP WinUtilities::GetScreenshotBmp()
{

    HDC     hDC;
    HDC     MemDC;
    BYTE   *Data;
    HBITMAP   hBmp;
    BITMAPINFO   bi;

    memset(&bi,   0,   sizeof(bi));
    bi.bmiHeader.biSize   =   sizeof(BITMAPINFO);
    bi.bmiHeader.biWidth   =  GetSystemMetrics(SM_CXSCREEN);
    bi.bmiHeader.biHeight   = GetSystemMetrics(SM_CYSCREEN);
    bi.bmiHeader.biPlanes   =   1;
    bi.bmiHeader.biBitCount   =   24;

    hDC   =   GetDC(NULL);
    MemDC   =   CreateCompatibleDC(hDC);
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void **)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight, hDC,   0,   0,   SRCCOPY);
    ReleaseDC(NULL,   hDC);
    DeleteDC(MemDC);
    return   hBmp;

}

HBITMAP WinUtilities::GetScreenshotBmpForNT5InteractiveService()
{

    HBITMAP   hBmp;

    //HANDLE              hToken;
    HDESK               hdesk;
    HWINSTA             hwinsta;
    //PROCESS_INFORMATION pi;
    //PSID                psid;
    //STARTUPINFO         si;

    //
    // obtain a handle to the interactive windowstation
    //
    hwinsta = OpenWindowStationW(
                  L"winsta0",
                  FALSE,
                  READ_CONTROL | WRITE_DAC
              );
    if (hwinsta == NULL) {
        qCritical() << "ERROR! OpenWindowStationW failed.";
        return hBmp;
    }

    HWINSTA hwinstaold = GetProcessWindowStation();

    //
    // set the windowstation to winsta0 so that you obtain the
    // correct default desktop
    //
    if (!SetProcessWindowStation(hwinsta)) {
        qCritical() << "ERROR! SetProcessWindowStation failed.";
        return hBmp;
    }

    HDESK hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());
    //
    // obtain a handle to the "default" desktop
    //
    hdesk = OpenDesktopW(
                L"default",
                0,
                FALSE,
                READ_CONTROL | WRITE_DAC |
                DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS
            );
    if (hdesk == NULL) {
        qCritical() << "ERROR! OpenDesktopW failed.";
        return hBmp;
    }


    if(!SetThreadDesktop(hdesk)) {
        qCritical() << "ERROR! SetThreadDesktop failed.";
        return hBmp;
    }




    //si.lpDesktop = L"winsta0\\default";


    //
    // Screenshot
    //
    HDC     hDC;
    HDC     MemDC;
    BYTE   *Data;
    //HBITMAP   hBmp;
    BITMAPINFO   bi;

    memset(&bi,   0,   sizeof(bi));
    bi.bmiHeader.biSize   =   sizeof(BITMAPINFO);
    bi.bmiHeader.biWidth   =  GetSystemMetrics(SM_CXSCREEN);
    bi.bmiHeader.biHeight   = GetSystemMetrics(SM_CYSCREEN);
    bi.bmiHeader.biPlanes   =   1;
    bi.bmiHeader.biBitCount   =   24;

    hDC   =   GetDC(NULL);
    MemDC   =   CreateCompatibleDC(hDC);
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void **)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight, hDC,   0,   0,   SRCCOPY);
    ReleaseDC(NULL,   hDC);
    DeleteDC(MemDC);


    //set it back
    SetProcessWindowStation(hwinstaold);
    SetThreadDesktop(hdeskCurrent);

    //
    // close the handles
    //
    //
    // close the handles to the interactive windowstation and desktop
    //
    CloseWindowStation(hwinsta);
    CloseDesktop(hdesk);

    return hBmp;

}

HBITMAP    WinUtilities::setDesktop1()
{
    wchar_t   pvInfo[256] = { 0 };
    DWORD dwLen = 0;
    HDESK hActiveDesktop = OpenInputDesktop(DF_ALLOWOTHERACCOUNTHOOK, false, MAXIMUM_ALLOWED);
    GetUserObjectInformationW(hActiveDesktop, UOI_NAME, pvInfo, sizeof(pvInfo), &dwLen);
    CloseDesktop(hActiveDesktop);

    HWINSTA m_hwinsta = OpenWindowStationW(L"WINSTA0", false, MAXIMUM_ALLOWED);
    SetProcessWindowStation(m_hwinsta);
    HDESK m_hdesk = OpenDesktopW(pvInfo, 0, false, MAXIMUM_ALLOWED);
    SetThreadDesktop(m_hdesk);


    HDC     hDC;
    HDC     MemDC;
    BYTE   *Data;
    HBITMAP   hBmp;
    BITMAPINFO   bi;

    memset(&bi,   0,   sizeof(bi));
    bi.bmiHeader.biSize   =   sizeof(BITMAPINFO);
    bi.bmiHeader.biWidth   =  GetSystemMetrics(SM_CXSCREEN);
    bi.bmiHeader.biHeight   = GetSystemMetrics(SM_CYSCREEN);
    bi.bmiHeader.biPlanes   =   1;
    bi.bmiHeader.biBitCount   =   24;

    hDC   =   GetDC(NULL);
    MemDC   =   CreateCompatibleDC(hDC);
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void **)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight, hDC,   0,   0,   SRCCOPY);
    ReleaseDC(NULL,   hDC);
    DeleteDC(MemDC);
    return   hBmp;

}

bool WinUtilities::setDesktop()
{
    WCHAR pvInfo[128] = {0};
    //WCHAR tmp[1024] = {0};

    HDESK hActiveDesktop;
    DWORD dwLen;
    hActiveDesktop = OpenInputDesktop(DF_ALLOWOTHERACCOUNTHOOK, FALSE, MAXIMUM_ALLOWED);
    if(!hActiveDesktop) { //打开失败
        qCritical() << "ERROR! OpenInputDesktop failed.";
        return false;
    }
    //获取指定桌面对象的信息，一般情况和屏保状态为default，登陆界面为winlogon
    GetUserObjectInformation(hActiveDesktop, UOI_NAME, pvInfo, sizeof(pvInfo), &dwLen);
    if(dwLen == 0) { //获取失败
        qCritical() << "ERROR! GetUserObjectInformation failed.";
        return false;
    }
    CloseDesktop(hActiveDesktop);
    //打开winsta0
    HWINSTA m_hwinsta = OpenWindowStationW(L"winsta0", FALSE,
                                           WINSTA_ACCESSCLIPBOARD   |
                                           WINSTA_ACCESSGLOBALATOMS |
                                           WINSTA_CREATEDESKTOP     |
                                           WINSTA_ENUMDESKTOPS      |
                                           WINSTA_ENUMERATE         |
                                           WINSTA_EXITWINDOWS       |
                                           WINSTA_READATTRIBUTES    |
                                           WINSTA_READSCREEN        |
                                           WINSTA_WRITEATTRIBUTES);
    if (m_hwinsta == NULL) {
        qCritical() << "ERROR! OpenWindowStationW failed.";
        return false;
    }

    if (!SetProcessWindowStation(m_hwinsta)) {
        qCritical() << "ERROR! SetProcessWindowStation failed.";
        return false;
    }

    //打开desktop
    HDESK m_hdesk = OpenDesktopW(pvInfo, 0, FALSE,
                                 DESKTOP_CREATEMENU |
                                 DESKTOP_CREATEWINDOW |
                                 DESKTOP_ENUMERATE    |
                                 DESKTOP_HOOKCONTROL |
                                 DESKTOP_JOURNALPLAYBACK |
                                 DESKTOP_JOURNALRECORD |
                                 DESKTOP_READOBJECTS |
                                 DESKTOP_SWITCHDESKTOP |
                                 DESKTOP_WRITEOBJECTS);
    if (m_hdesk == NULL) {
        qCritical() << "ERROR! OpenDesktopW failed.";
        return false;
    }

    SetThreadDesktop(m_hdesk);
    return true;
}

HBITMAP WinUtilities::GetScreenshotBmp1()
{

    HBITMAP   hBmp;

    WCHAR pvInfo[128] = {0};
    //WCHAR tmp[1024] = {0};

    HDESK hActiveDesktop;
    DWORD dwLen;
    hActiveDesktop = OpenInputDesktop(DF_ALLOWOTHERACCOUNTHOOK, FALSE, MAXIMUM_ALLOWED);
    if(!hActiveDesktop) { //打开失败
        qCritical() << "ERROR! OpenInputDesktop failed.";
        return hBmp;
    }
    //获取指定桌面对象的信息，一般情况和屏保状态为default，登陆界面为winlogon
    GetUserObjectInformation(hActiveDesktop, UOI_NAME, pvInfo, sizeof(pvInfo), &dwLen);
    if(dwLen == 0) { //获取失败
        qCritical() << "ERROR! GetUserObjectInformation failed.";
        return hBmp;
    }
    CloseDesktop(hActiveDesktop);

    //打开winsta0
    HWINSTA m_hwinsta = OpenWindowStationW(L"winsta0", FALSE,
                                           WINSTA_ACCESSCLIPBOARD   |
                                           WINSTA_ACCESSGLOBALATOMS |
                                           WINSTA_CREATEDESKTOP     |
                                           WINSTA_ENUMDESKTOPS      |
                                           WINSTA_ENUMERATE         |
                                           WINSTA_EXITWINDOWS       |
                                           WINSTA_READATTRIBUTES    |
                                           WINSTA_READSCREEN        |
                                           WINSTA_WRITEATTRIBUTES);
    if (m_hwinsta == NULL) {
        qCritical() << "ERROR! OpenWindowStationW failed.";
        return hBmp;
    }

    HWINSTA hwinstaCurrent = GetProcessWindowStation();


    if (!SetProcessWindowStation(m_hwinsta)) {
        qCritical() << "ERROR! SetProcessWindowStation failed.";
        return hBmp;
    }

    //打开desktop
    HDESK m_hdesk = OpenDesktopW(pvInfo, 0, FALSE,
                                 DESKTOP_CREATEMENU |
                                 DESKTOP_CREATEWINDOW |
                                 DESKTOP_ENUMERATE    |
                                 DESKTOP_HOOKCONTROL |
                                 DESKTOP_JOURNALPLAYBACK |
                                 DESKTOP_JOURNALRECORD |
                                 DESKTOP_READOBJECTS |
                                 DESKTOP_SWITCHDESKTOP |
                                 DESKTOP_WRITEOBJECTS);
    if (m_hdesk == NULL) {
        qCritical() << "ERROR! OpenDesktopW failed.";
        return hBmp;
    }

    HDESK hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());

    SetThreadDesktop(m_hdesk);
    SwitchDesktop(m_hdesk);

    ///////////////////////////////


    HDC     hDC;
    HDC     MemDC;
    BYTE   *Data;
    //HBITMAP   hBmp;
    BITMAPINFO   bi;

    memset(&bi,   0,   sizeof(bi));
    bi.bmiHeader.biSize   =   sizeof(BITMAPINFO);
    bi.bmiHeader.biWidth   =  GetSystemMetrics(SM_CXSCREEN);
    bi.bmiHeader.biHeight   = GetSystemMetrics(SM_CYSCREEN);
    bi.bmiHeader.biPlanes   =   1;
    bi.bmiHeader.biBitCount   =   24;

    //hDC   =   GetDC(NULL);
    hDC   =   GetDC(GetDesktopWindow());
    MemDC   =   CreateCompatibleDC(hDC);
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void **)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight, hDC,   0,   0,   SRCCOPY);
    ReleaseDC(NULL,   hDC);
    DeleteDC(MemDC);
    return   hBmp;


    /////////////////////////////////



    //set it back
    SetProcessWindowStation(hwinstaCurrent);
    SetThreadDesktop(hdeskCurrent);

    SwitchDesktop(hdeskCurrent);




    //
    // close the handles
    //
    //
    // close the handles to the interactive windowstation and desktop
    //
    CloseWindowStation(m_hwinsta);
    CloseDesktop(m_hdesk);


    return hBmp;
}

bool WinUtilities::setDeskWallpaper(const QString &wallpaperPath)
{


    QString targetBMPFilePath = wallpaperPath;

    QFileInfo fi(targetBMPFilePath);
    if(!fi.exists()) {
        qCritical() << QString("Can not set wallpaper! File '%1' does not exist!").arg(targetBMPFilePath);
        return false;
    }

    QString targetDirPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if(!QDir(targetDirPath).exists()) {
        targetDirPath = QDir::homePath();
    }

    //if(wallpaperPath.startsWith(":/") || fi.suffix().toLower() != ".bmp"){
    targetBMPFilePath = targetDirPath + QDir::separator() + fi.baseName() + ".bmp";
    QImage image(wallpaperPath);
    if(image.isNull()) {
        qCritical() << QString("Can not read image '%1' ! ").arg(wallpaperPath);
        return false;
    }

    if(!image.save(targetBMPFilePath, "BMP")) {
        qCritical() << QString("Can not set wallpaper! Can not save file '%1'k!").arg(targetBMPFilePath);
        return false;
    }

    //}


    wchar_t pathArray[MAX_PATH * sizeof(wchar_t) + 1];
    wcscpy(pathArray, targetBMPFilePath.toStdWString().c_str());

    bool ok = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, pathArray, SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
    if(!ok) {
        DWORD err = GetLastError();
        qCritical() << QString("Can not set wallpaper! %1:%2.").arg(err).arg(WinUtilities::WinSysErrorMsg(err));
    }

    return ok;

}


QDateTime WinUtilities::currentDateTimeOnServer(const QString &server, const QString &userName, const QString &password)
{
    QDateTime dateTime;

    ///////////////  建立IPC$    ////////////////////
    wchar_t timeServer[256];
    wcscpy(timeServer, server.toStdWString().c_str());

    WNetCancelConnection2W(timeServer, 0, false);

    NETRESOURCEW res;
    res.dwType = RESOURCETYPE_ANY;
    res.lpLocalName = NULL;
    res.lpRemoteName = timeServer;
    res.lpProvider = NULL;

    DWORD err;
    err = WNetAddConnection2W(&res, password.toStdWString().c_str(), userName.toStdWString().c_str(), CONNECT_INTERACTIVE);
    if(err !=  NO_ERROR) {
        qCritical() << QString("Can not connect to '%1'! %2:%3.").arg(server).arg(err).arg(WinUtilities::WinSysErrorMsg(err));
        return dateTime;
    }
    //////////////////////////////////////////


    LPTIME_OF_DAY_INFO pBuf = NULL;
    NET_API_STATUS nStatus;

    nStatus = NetRemoteTOD(timeServer, (LPBYTE *)&pBuf);

    if (nStatus == NERR_Success) {
        if (pBuf != NULL) {

            dateTime = QDateTime::fromTime_t( pBuf->tod_elapsedt);

            //            fprintf(stderr, "\nThe current date is: %d/%d/%d\n",
            //                    pBuf->tod_month, pBuf->tod_day, pBuf->tod_year);
            //            fprintf(stderr, "The current time is: %d:%d:%d\n",
            //                    pBuf->tod_hours, pBuf->tod_mins, pBuf->tod_secs);
        }


    } else {
        qCritical() << QString("Can not get current time from server '%1'! %2:%3.").arg(server).arg(nStatus).arg(WinUtilities::WinSysErrorMsg(nStatus));
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return dateTime;
}

bool WinUtilities::setLocalTime(const QDateTime &datetime)
{

    if(!datetime.isValid()) {
        qCritical() << ("Invalid Time!");
        return false;
    }

    SYSTEMTIME systemtime;
    QDate date = datetime.date();
    QTime time = datetime.time();
    systemtime.wYear = date.year();
    systemtime.wMonth = date.month();
    systemtime.wDayOfWeek = date.dayOfWeek();
    systemtime.wDay = date.day();
    systemtime.wHour = time.hour();
    systemtime.wMinute = time.minute();
    systemtime.wSecond = time.second();
    systemtime.wMilliseconds = time.msec();

    if(!SetLocalTime(&systemtime)) {
        DWORD err = GetLastError();
        qCritical() << QString("Can not set system time! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
        return false;
    }

    return true;
}




} //namespace HEHUI


