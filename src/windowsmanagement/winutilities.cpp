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
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDir>

#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus")


#include <Lm.h>
#include <Tlhelp32.h>
#include <Lmjoin.h>
#include <Userenv.h>

const int MaxUserAccountNameLength = 20;
const int MaxUserPasswordLength = LM20_PWLEN;
const int MaxUserCommentLength = 256;
const int MaxGroupNameLength = 256;





namespace HEHUI {


WinUtilities::WinUtilities() {
    // TODO Auto-generated constructor stub


}

WinUtilities::~WinUtilities() {
    // TODO Auto-generated destructor stub
}


QString WinUtilities::WinSysErrorMsg(DWORD winErrorCode, DWORD dwLanguageId){
    //    wchar_t buffer[8192];
    //    ZeroMemory(buffer, 8192);

    //    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, winErrorCode, 0, buffer, 8192, 0);
    //    return QString::fromWCharArray(buffer).simplified();



    HMODULE hLib= 0;
    DWORD dwFlags;
    OSVERSIONINFOW os;
    wchar_t buffer[8192];
    DWORD cbBuffer;

    ZeroMemory(buffer, 8192);
    cbBuffer = 0;


    if(winErrorCode >= 2100 && winErrorCode <= 2999){
        //Undocumented errors %NETWORK_ERROR_FIRST to %NETWORK_ERROR_LAST
        os.dwOSVersionInfoSize = sizeof(os);
        GetVersionExW(&os);
        if(os.dwPlatformId == VER_PLATFORM_WIN32_NT){
            hLib = LoadLibraryExW(L"NETMSG.DLL", 0, LOAD_LIBRARY_AS_DATAFILE);
        }
    }else if(winErrorCode >= 12000 && winErrorCode <= 12171){
        //Undocumented errors %INTERNET_ERROR_FIRST to %NTERNET_ERROR_LAST
        hLib = LoadLibraryExW(L"WININET.DLL", 0, LOAD_LIBRARY_AS_DATAFILE);
    }

    dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK;
    if(hLib){
        dwFlags = dwFlags | FORMAT_MESSAGE_FROM_HMODULE;
    }

    cbBuffer = FormatMessageW(dwFlags, hLib, winErrorCode, dwLanguageId, buffer, 8192, 0);
    if(hLib){
        FreeLibrary(hLib);
    }

    if(cbBuffer){
        return QString::fromWCharArray(buffer).simplified();
    }else{
        QString string = QString::number(winErrorCode, 16).toUpper();
        return QString(QObject::tr("Error:0x%1").arg(string.rightJustified(8, '0')));
    }


}

void WinUtilities::freeMemory(){

#if defined(Q_OS_WIN32)
    //SetProcessWorkingSetSize(GetCurrentProcess(), 0xFFFFFFFF, 0xFFFFFFFF);
    SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
#endif

}

QString WinUtilities::getComputerName(DWORD *errorCode){
    qDebug()<<"--WinUtilities::getComputerName()";

    QString computerName = "";
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    LPWSTR name = new wchar_t[size];

    if(GetComputerNameW(name, &size)){
        computerName = QString::fromWCharArray(name);
    }else{
        qDebug()<<QString("Can not get computer name! Error: %1").arg(GetLastError());
    }

    if(errorCode){
        *errorCode = GetLastError();
    }

    delete [] name;

    return computerName.toLower();

}

bool WinUtilities::getComputerNameInfo(QString *dnsDomain, QString *dnsHostname, QString *netBIOSName, DWORD *errorCode){

    bool ok = false;
    COMPUTER_NAME_FORMAT nameType;
    wchar_t buffer[512];
    ZeroMemory(buffer, 512);
    DWORD size = sizeof(buffer);

    if(dnsDomain){
        nameType = ComputerNameDnsDomain;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok){
            *dnsDomain = QString::fromWCharArray(buffer);
        }else{
            qDebug()<<QString("\nFailed to get dns domain! Error Code: %1").arg(GetLastError());
        }
    }

    if(dnsHostname){
        ZeroMemory(buffer, size);

        nameType = ComputerNameDnsHostname;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok){
            *dnsHostname = QString::fromWCharArray(buffer);
        }else{
            qDebug()<<QString("\nFailed to get dns hostname! Error Code: %1").arg(GetLastError());
        }
    }

    if(netBIOSName){
        ZeroMemory(buffer, size);

        nameType = ComputerNameNetBIOS;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok){
            *netBIOSName = QString::fromWCharArray(buffer);
        }else{
            qDebug()<<QString("\nFailed to get NetBIOS name! Error Code: %1").arg(GetLastError());
        }
    }

    if(errorCode){
        *errorCode = GetLastError();
    }

    return ok;
}

QString WinUtilities::getJoinInformation(bool *isJoinedToDomain, const QString &serverName, DWORD *errorCode){
    qDebug()<<"--WindowsManagement::getJoinInformation()";

    QString workgroupName = "";
    NET_API_STATUS err;
    LPWSTR lpNameBuffer = new wchar_t[256];
    NETSETUP_JOIN_STATUS bufferType;
    LPCWSTR lpServer = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()){
        lpServer = serverName.toStdWString().c_str();
    }

    err = NetGetJoinInformation(lpServer, &lpNameBuffer, &bufferType);
    if(err == NERR_Success){
        workgroupName = QString::fromWCharArray(lpNameBuffer);
    }else{
        qDebug()<<QString("Can not get join status information! %1:%2.").arg(err).arg(WinUtilities::WinSysErrorMsg(err));
    }

    if(errorCode){
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

    if(isJoinedToDomain){
        *isJoinedToDomain = (bufferType == NetSetupDomainName)?true:false;
    }

    return workgroupName;

}



//When running on 64-bit Windows if you want to read a value specific to the 64-bit environment you have to suffix the HK... with 64 i.e. HKLM64.
bool WinUtilities::parseRegKeyString(const QString &keyString, HKEY *rootKey, QString *subKeyString){
    QString tempStr = keyString.trimmed();
    if(tempStr.isEmpty() || (!rootKey) || (!subKeyString) ){return false;}

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

    if(rootKeysHash.keys().contains(rootKeyString)){
        *rootKey = rootKeysHash.value(rootKeyString);
        //*subKeyString = tempStr.remove(QString(rootKeyString + "\\"), Qt::CaseInsensitive);
        *subKeyString = tempStr.remove(0, rootKeyString.size() + 1);
        return true;
    }else{
        return false;
    }

}

bool WinUtilities::regOpen(const QString &key, HKEY *hKey, REGSAM samDesired){
    if(!hKey){return false;}

    HKEY rootKey;
    QString subKeyString;
    if(!parseRegKeyString(key, &rootKey, &subKeyString)){
        qCritical()<<"ERROR! Invalid registry key string!";
        return false;
    }

    //#if defined( _WIN64 )
    //    samDesired |= KEY_WOW64_64KEY;
    //#else
    //    samDesired |= KEY_WOW64_32KEY;
    //#endif

    DWORD dwRet = RegOpenKeyExW(rootKey, subKeyString.toStdWString().c_str(), 0, samDesired, hKey);
    if(dwRet != ERROR_SUCCESS){
        qCritical()<<"ERROR! RegOpenKeyExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regRead(HKEY hKey, const QString &valueName, QString *value){

    if(!value){return false;}

    DWORD dwRet;
    DWORD dwType;
    DWORD bufferSize = 8192;
    dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, &dwType, 0, &bufferSize);
    if(dwRet != ERROR_SUCCESS){
        qCritical()<<"ERROR! RegQueryValueExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
        //RegCloseKey(hKey);
        return false;
    }

    switch (dwType) {
    case REG_DWORD:
    {
        DWORDLONG lResult = 0;
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)&lResult, &bufferSize);
        *value = QString::number(lResult);
    }
        break;
    case REG_BINARY:
    {
        char buffer[8192];
        ZeroMemory(buffer, 8192);
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
        QByteArray ba;
        for(DWORD i=0;i<bufferSize;i++){
            ba.append(buffer[i]);
            //qDebug()<<i<<": "<<buffer[i]<<" "<<QByteArray(1,buffer[i]).toHex();
        }
        *value = ba.toHex().toUpper();
    }
        break;
    case REG_SZ:
    case REG_EXPAND_SZ:
    {
        wchar_t buffer[8192];
        ZeroMemory(buffer, 8192);
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
        *value = QString::fromWCharArray(buffer);
    }
        break;
    case REG_MULTI_SZ:
    {
        wchar_t buffer[8192];
        ZeroMemory(buffer, 8192);
        dwRet = RegQueryValueExW(hKey, valueName.toStdWString().c_str(), 0, 0, (LPBYTE)buffer, &bufferSize);
        int len = bufferSize / sizeof(wchar_t) - 1;
        QByteArray ba;
        for(int i=0;i<len;i++){
            if(buffer[i] == '\0'){
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
        qCritical()<<"ERROR! Unknown data type: "<<dwType;
        //RegCloseKey(hKey);
        return false;
        break;
    }

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regRead(const QString &key, const QString &valueName, QString *value, bool on64BitView){
    //qDebug()<<"--WinUtilities::regRead(...) "<<" key:"<<key<<" valueName:"<<valueName;

    if(!value){return false;}

    REGSAM samDesired = KEY_READ;
    if(on64BitView){
        samDesired |= KEY_WOW64_64KEY;
    }else{
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)){return false;}

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

bool WinUtilities::regEnumVal(HKEY hKey, QStringList *valueNameList){
    if(!valueNameList){return false;}

    DWORD dwRet;
    DWORD dwIndex= 0;

    DWORD valueNameLen = 8192;
    wchar_t valueName[8192];
    ZeroMemory(valueName, 8192);

    do{
        dwRet = RegEnumValueW(hKey, dwIndex, valueName, &valueNameLen, 0, 0, 0, 0);
        if(dwRet == ERROR_SUCCESS){
            valueNameList->append(QString::fromWCharArray(valueName));
            valueNameLen = 8192;
            ZeroMemory(valueName, valueNameLen);
            //qDebug()<<dwRet<<":"<<WinErrorMsg(dwRet);
            dwIndex++;
        }else if(dwRet == ERROR_NO_MORE_ITEMS){
            break;
        }else{
            qCritical()<<"ERROR! RegEnumValueW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
            //RegCloseKey(hKey);
            return false;
        }

    }while(dwRet == ERROR_SUCCESS);

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regEnumVal(const QString &key, QStringList *valueNameList, bool on64BitView){
    if(!valueNameList){return false;}

    REGSAM samDesired = KEY_READ;
    if(on64BitView){
        samDesired |= KEY_WOW64_64KEY;
    }else{
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)){return false;}

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

bool WinUtilities::regEnumKey(HKEY hKey, QStringList *keyNameList){
    if(!keyNameList){return false;}

    DWORD dwRet;
    DWORD dwIndex= 0;

    DWORD keyNameLen = 8192;
    wchar_t keyName[8192];
    ZeroMemory(keyName, 8192);

    do{
        dwRet = RegEnumKeyExW(hKey, dwIndex, keyName, &keyNameLen, 0, 0, 0, 0);
        if(dwRet == ERROR_SUCCESS){
            keyNameList->append(QString::fromWCharArray(keyName));
            keyNameLen = 8192;
            ZeroMemory(keyName, keyNameLen);
            dwIndex++;
        }else if(dwRet == ERROR_NO_MORE_ITEMS){
            break;
        }else{
            qCritical()<<"ERROR! RegEnumKeyExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
            //RegCloseKey(hKey);
            return false;
        }

    }while(dwRet == ERROR_SUCCESS);

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regEnumKey(const QString &key, QStringList *keyNameList, bool on64BitView){
    if(!keyNameList){return false;}

    REGSAM samDesired = KEY_WRITE|KEY_READ;
    if(on64BitView){
        samDesired |= KEY_WOW64_64KEY;
    }else{
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)){return false;}

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

bool WinUtilities::regCreateKey(HKEY hKey, const QString &subKeyName, HKEY *hSubKey){

    HKEY hkResult;
    DWORD dwDisposition;

    DWORD dwRet = RegCreateKeyExW(hKey, subKeyName.toStdWString().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hkResult, &dwDisposition);
    //DWORD dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\AutoIt v3\\QQQ", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hkResult, &dwDisposition);

    if(dwRet != ERROR_SUCCESS){
        qCritical()<<"ERROR! RegCreateKeyExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
        //RegCloseKey(hKey);
        return false;
    }

    if(hSubKey){
        *hSubKey = hkResult;
    }

    //RegCloseKey(hKey);
    return true;
}

bool WinUtilities::regCreateKey(const QString &key, const QString &subKeyName, HKEY *hSubKey, bool on64BitView){

    REGSAM samDesired = KEY_WRITE|KEY_READ;
    if(on64BitView){
        samDesired |= KEY_WOW64_64KEY;
    }else{
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)){return false;}

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

bool WinUtilities::regSetValue(HKEY hKey, const QString &valueName, const QString &value, DWORD valueType){

    if(!hKey){
        qCritical()<<"ERROR! Invalid key handle!";
        return false;
    }

    DWORD dwRet;
    DWORD bufferSize = 0;
    wchar_t buffer[8192];
    ZeroMemory(buffer, 8192);

    switch (valueType) {
    case REG_DWORD:
    {
        bufferSize = sizeof(DWORD);
        buffer[0] = value.toULong();
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE*)buffer, bufferSize);
    }
        break;
    case REG_BINARY:
    {
        //bufferSize = value.size() / 2;
        QByteArray ba = QByteArray::fromHex(value.toLatin1());
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE*)ba.data(), ba.size());
    }
        break;
    case REG_SZ:
    case REG_EXPAND_SZ:
    {
        bufferSize = (value.size()) * sizeof(wchar_t);
        wcscpy(buffer, value.toStdWString().c_str());
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE*)buffer, bufferSize);
    }
        break;
    case REG_MULTI_SZ:
    {
        int len = value.size() + 1;
        bufferSize = len * sizeof(wchar_t);
        wcscpy(buffer, value.toStdWString().c_str());
        for(int i=0;i<len;i++){
            if(buffer[i] == '\n'){
                buffer[i] = '\0';
            }
        }
        dwRet = RegSetValueExW(hKey, valueName.toStdWString().c_str(), 0, valueType, (BYTE*)buffer, bufferSize);
    }
        break;
    default:
        qCritical()<<"ERROR! Unknown data type!";
        return false;
        break;
    }

    if(dwRet != ERROR_SUCCESS){
        qCritical()<<"ERROR! RegSetValueExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regSetValue(const QString &key, const QString &valueName, const QString &value, DWORD valueType, bool on64BitView){
    //qDebug()<<"--WinUtilities::regSetValue(...) "<<" key:"<<key<<" valueName:"<<valueName<<" value:"<<value;

    REGSAM samDesired = KEY_WRITE;
    if(on64BitView){
        samDesired |= KEY_WOW64_64KEY;
    }else{
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)){return false;}

    bool ret = regSetValue(hKey, valueName, value, valueType);
    RegCloseKey(hKey);
    return ret;
}

bool WinUtilities::regDeleteKey(HKEY hKey, const QString &subKeyName, bool on64BitView){

    //#if defined( _WIN64 )
    //    samDesired |= KEY_WOW64_64KEY;
    //#else
    //    samDesired |= KEY_WOW64_32KEY;
    //#endif

    DWORD dwRet;

    typedef DWORD (WINAPI *FN_RegDeleteKeyExW) (HKEY, LPCWSTR, REGSAM, DWORD);
    FN_RegDeleteKeyExW fnRegDeleteKeyExW;
    fnRegDeleteKeyExW = (FN_RegDeleteKeyExW)GetProcAddress( GetModuleHandleW(L"advapi32"), "RegDeleteKeyExW");
    if (NULL != fnRegDeleteKeyExW){
        REGSAM samDesired;
        if(on64BitView){
            samDesired = KEY_WOW64_64KEY;
        }else{
            samDesired = KEY_WOW64_32KEY;
        }
        dwRet = fnRegDeleteKeyExW(hKey, subKeyName.toStdWString().c_str(), samDesired, 0);
    }else{
        dwRet = RegDeleteKeyW(hKey, subKeyName.toStdWString().c_str());
    }

    if(dwRet != ERROR_SUCCESS){
        qCritical()<<"ERROR! RegDeleteKeyExW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regDeleteKey(const QString &key, bool on64BitView){

    HKEY rootKey;
    QString subKeyString;
    if(!parseRegKeyString(key, &rootKey, &subKeyString)){
        qCritical()<<"ERROR! Invalid registry key string!";
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

bool WinUtilities::regDeleteValue(HKEY hKey, const QString &valueName){

    DWORD dwRet = RegDeleteValueW(hKey, valueName.toStdWString().c_str());
    if(dwRet != ERROR_SUCCESS){
        qCritical()<<"ERROR! RegDeleteValueW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WinUtilities::regDeleteValue(const QString &key, const QString &valueName, bool on64BitView){

    REGSAM samDesired = KEY_WRITE;
    if(on64BitView){
        samDesired |= KEY_WOW64_64KEY;
    }else{
        samDesired |= KEY_WOW64_32KEY;
    }
    HKEY hKey;
    if(!regOpen(key, &hKey, samDesired)){return false;}

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

void WinUtilities::regCloseKey(HKEY hKey){
    RegCloseKey(hKey);
}

bool WinUtilities::is64BitApplication(){
    return 8 == sizeof( void * );
    //return (sizeof(LPFN_ISWOW64PROCESS) == 8)? TRUE: FALSE;
}

bool WinUtilities::isWow64()
{
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandleW(L"kernel32"), "IsWow64Process");
    if (NULL != fnIsWow64Process){
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    }
    return bIsWow64;
}

QString WinUtilities::getUserNameOfCurrentThread(DWORD *errorCode) {

    DWORD size = MaxUserAccountNameLength + 1;
    wchar_t username[MaxUserAccountNameLength + 1];

    if(!GetUserNameW(username, &size)){
        qDebug()<<QString("Can not retrieve the name of the user associated with the current thread! Code:%1 ")
                  .arg(QString::number(GetLastError()));

        if(errorCode){
            *errorCode = GetLastError();
        }

        return QString("");
    }

    return QString::fromWCharArray(username);

}

bool WinUtilities::getLogonInfoOfCurrentUser(QString *userName, QString *domain, QString *logonServer, NET_API_STATUS *apiStatus){


    bool ok = false;
    DWORD dwLevel = 1;
    LPWKSTA_USER_INFO_1 pBuf = NULL;
    NET_API_STATUS nStatus;

    //
    // Call the NetWkstaUserGetInfo function;
    //  specify level 1.
    //
    nStatus = NetWkstaUserGetInfo(NULL, dwLevel,(LPBYTE *)&pBuf);
    //
    // If the call succeeds, print the information
    //  about the logged-on user.
    //
    if (nStatus == NERR_Success)
    {
        if (pBuf != NULL)
        {
            //wprintf(L"\n\tUser:          %s\n", pBuf->wkui1_username);
            //wprintf(L"\tDomain:        %s\n", pBuf->wkui1_logon_domain);
            //wprintf(L"\tOther Domains: %s\n", pBuf->wkui1_oth_domains);
            //wprintf(L"\tLogon Server:  %s\n", pBuf->wkui1_logon_server);

            if(userName){
                *userName = QString::fromWCharArray(pBuf->wkui1_username);
            }
            if(domain){
                *domain = QString::fromWCharArray(pBuf->wkui1_logon_domain);
            }
            if(logonServer){
                *logonServer = QString::fromWCharArray(pBuf->wkui1_logon_server);
            }

            ok = true;
        }

    } else {
        // Otherwise, print the system error.
        //
        //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
        qCritical()<<QString("A system error has occurred: %1").arg(nStatus);
    }

    if(apiStatus){
        *apiStatus = nStatus;
    }

    //
    // Free the allocated memory.
    //
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);

    return ok;

}

void WinUtilities::getAllUsersLoggedOn(QStringList *users, const QString &serverName, DWORD *apiStatus){

    Q_ASSERT(users);

    if(!users){
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
    do // begin do
    {
        nStatus = NetWkstaUserEnum(serverNameArray,
                                   dwLevel,
                                   (LPBYTE*)&pBuf,
                                   dwPrefMaxLen,
                                   &dwEntriesRead,
                                   &dwTotalEntries,
                                   &dwResumeHandle);
        //
        // If the call succeeds,
        //
        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
        {
            if ((pTmpBuf = pBuf) != NULL)
            {
                //
                // Loop through the entries.
                //
                for (i = 0; (i < dwEntriesRead); i++)
                {
                    Q_ASSERT(pTmpBuf != NULL);

                    if (pTmpBuf == NULL)
                    {
                        //
                        // Only members of the Administrators local group
                        //  can successfully execute NetWkstaUserEnum
                        //  locally and on a remote server.
                        //
                        //fprintf(stderr, "An access violation has occurred\n");
                        qDebug()<<QString("An access violation has occurred\n");
                        break;
                    }
                    //
                    // Print the user logged on to the workstation.
                    //
                    //wprintf(L"\t-- %s\n", pTmpBuf->wkui0_username);
                    QString wkui1_username = QString::fromWCharArray(pTmpBuf->wkui1_username).toLower();
                    if(wkui1_username == computerName + "$"){continue;}

                    QString wkui1_logon_domain = QString::fromWCharArray(pTmpBuf->wkui1_logon_domain).toLower();
                    if(wkui1_logon_domain == computerName){
                        users->append(wkui1_username);
                    }else{
                        users->append(wkui1_logon_domain + "\\" + wkui1_username);
                    }

                    //users->append(QString::fromWCharArray(pTmpBuf->wkui1_username).toLower());

                    pTmpBuf++;
                    dwTotalCount++;
                }
            }
        }
        else{
            //
            // Otherwise, indicate a system error.
            //
            //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
            qDebug()<<QString("A system error has occurred: %1\n").arg(nStatus);
        }

        //
        // Free the allocated memory.
        //
        if (pBuf != NULL)
        {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    }
    //
    // Continue to call NetWkstaUserEnum while
    //  there are more entries.
    //
    while (nStatus == ERROR_MORE_DATA); // end do

    if(apiStatus){
        *apiStatus = nStatus;
    }

    //
    // Check again for allocated memory.
    //
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);

    //
    // Print the final count of workstation users.
    //
    //fprintf(stderr, "\nTotal of %d entries enumerated\n", dwTotalCount);


}

QStringList WinUtilities::localUsers(DWORD *apiStatus) {
    qDebug()<<"--WinUtilities::localUsers()";

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
                              (LPBYTE*) & pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries,
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
            qCritical()<<"A system error has occurred:"<<nStatus;
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }

    }while (nStatus == ERROR_MORE_DATA);

    if(apiStatus){
        *apiStatus = nStatus;
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
        pBuf = NULL;
    }

    return users;

}

QStringList WinUtilities::localCreatedUsers() {
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

bool WinUtilities::serviceOpenSCManager(SC_HANDLE *schSCManager, DWORD *errorCode){

    if(!schSCManager){
        return false;
    }

    // Get a handle to the SCM database.

    *schSCManager = OpenSCManager(
                NULL,                    // local computer
                NULL,                    // ServicesActive database
                SC_MANAGER_ALL_ACCESS);  // full access rights

    if (NULL == (*schSCManager))
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        if(errorCode){
            *errorCode = GetLastError();
        }
        return false;
    }

    return true;
}

bool WinUtilities::serviceOpenService(const QString &serviceName, SC_HANDLE *schSCManager, SC_HANDLE *schService, DWORD *errorCode){

    if(serviceName.trimmed().isEmpty() || !schSCManager || !schService){
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
                SERVICE_QUERY_CONFIG); // need query config access

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        if(errorCode){
            *errorCode = GetLastError();
        }
        CloseServiceHandle(*schSCManager);
        return false;
    }

    return true;
}

bool WinUtilities::serviceQueryInfo(const QString &serviceName, ServiceInfo *serviceInfo, DWORD *errorCode){
    if(serviceName.trimmed().isEmpty() || !serviceInfo){return false;}

    SC_HANDLE schSCManager;
//    SC_HANDLE schService;
    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok){
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

bool WinUtilities::serviceQueryInfo(SC_HANDLE *schSCManager, const QString &serviceName, ServiceInfo *serviceInfo, DWORD *errorCode){
    if(!schSCManager || serviceName.trimmed().isEmpty() || !serviceInfo){return false;}

    SC_HANDLE schService;
    bool ok = serviceOpenService(serviceName, schSCManager, &schService, errorCode);
    if(!ok){
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
                &dwBytesNeeded))
    {
        dwError = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER == dwError )
        {
            cbBufSize = dwBytesNeeded;
            lpServiceConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LMEM_FIXED, cbBufSize);
        }
        else
        {
            printf("QueryServiceConfig failed (%d)", dwError);
            if(errorCode){
                *errorCode = dwError;
            }
            goto cleanup;
        }
    }

    if( !QueryServiceConfig(
                schService,
                lpServiceConfig,
                cbBufSize,
                &dwBytesNeeded) )
    {
        printf("QueryServiceConfig failed (%d)", GetLastError());
        if(errorCode){
            *errorCode = GetLastError();
        }
        goto cleanup;
    }

    if( !QueryServiceConfig2(
                schService,
                SERVICE_CONFIG_DESCRIPTION,
                NULL,
                0,
                &dwBytesNeeded))
    {
        dwError = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER == dwError )
        {
            cbBufSize = dwBytesNeeded;
            lpsd = (LPSERVICE_DESCRIPTION) LocalAlloc(LMEM_FIXED, cbBufSize);
        }
        else
        {
            printf("QueryServiceConfig2 failed (%d)", dwError);
            if(errorCode){
                *errorCode = dwError;
            }
            goto cleanup;
        }
    }

    if (! QueryServiceConfig2(
                schService,
                SERVICE_CONFIG_DESCRIPTION,
                (LPBYTE) lpsd,
                cbBufSize,
                &dwBytesNeeded) )
    {
        printf("QueryServiceConfig2 failed (%d)", GetLastError());
        if(errorCode){
            *errorCode = GetLastError();
        }
        goto cleanup;
    }

    serviceInfo->serviceName = serviceName;
    serviceInfo->displayName = QString::fromWCharArray(lpServiceConfig->lpDisplayName);
    serviceInfo->description = QString::fromWCharArray(lpsd->lpDescription);;

    serviceInfo->serviceType = lpServiceConfig->dwServiceType;
    serviceInfo->startType = lpServiceConfig->dwStartType;
    serviceInfo->binaryPath = QString::fromWCharArray(lpServiceConfig->lpBinaryPathName);
    serviceInfo->account = QString::fromWCharArray(lpServiceConfig->lpServiceStartName);


    for(int i=0;i<256;i++){
        QChar ch = lpServiceConfig->lpDependencies[i];
        if(lpServiceConfig->lpDependencies[i] == '\0'){
            if(lpServiceConfig->lpDependencies[i+1] == '\0'){break;}
            ch = ';';
        }
        dependencies.append(ch);

        qDebug()<<i<<":"<<ch;
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

cleanup:
    CloseServiceHandle(schService);
//    CloseServiceHandle(schSCManager);

    return true;
}

bool WinUtilities::serviceChangeStartType(const QString &serviceName, DWORD startType, DWORD *errorCode){

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok){
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode);
    if(!ok){
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

    if (!ok)                // display name: no change
    {
        printf("ChangeServiceConfig failed (%d)\n", GetLastError());
        if(errorCode){
            *errorCode = GetLastError();
        }
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return ok;
}

bool WinUtilities::serviceChangeDescription(const QString &serviceName, const QString &description, DWORD *errorCode){

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok){
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode);
    if(!ok){
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

    if(!ok)
    {
        printf("ChangeServiceConfig2 failed\n");
        if(errorCode){
            *errorCode = GetLastError();
        }
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return ok;
}

bool WinUtilities::serviceDelete(const QString &serviceName, DWORD *errorCode){

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok){
        return false;
    }
    ok = serviceOpenService(serviceName, &schSCManager, &schService, errorCode);
    if(!ok){
        return false;
    }

    // Delete the service.

    ok = DeleteService(schService);
    if(!ok)
    {
        printf("DeleteService failed (%d)\n", GetLastError());
        if(errorCode){
            *errorCode = GetLastError();
        }
    }
    else printf("Service deleted successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return ok;
}

bool WinUtilities::serviceGetAllServicesInfo(QJsonArray *jsonArray, DWORD serviceType, DWORD *errorCode){

    if(!jsonArray){return false;}

    SC_HANDLE schSCManager;
    bool ok = serviceOpenSCManager(&schSCManager, errorCode);
    if(!ok){
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

    if (dwBufNeed < 0x01)
    {
        printf_s("EnumServicesStatusEx failed. \n");
        if(errorCode){
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
    for (ULONG i=0; i<dwNumberOfService; i++)
    {
//        qDebug();
//        qDebug()<<i<<"."<<"Display Name \t : "<< QString::fromWCharArray(pInfo[i].lpDisplayName);
//        qDebug()<<"Service Name \t : "<< QString::fromWCharArray(pInfo[i].lpServiceName);
//        qDebug()<<"Process Id \t : "<< QString::number( pInfo[i].ServiceStatusProcess.dwProcessId);

        QString serviceName = QString::fromWCharArray(pInfo[i].lpServiceName);
        if(serviceName.trimmed().isEmpty()){continue;}

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
    qDebug()<<"----------jsonArray->size():"<<jsonArray->size();

    free(pBuf);
    CloseServiceHandle(schSCManager);

    return true;
}










int WinUtilities::GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

QByteArray WinUtilities::ConvertHBITMAPToJpeg(HBITMAP hbitmap){

    QByteArray byteArray;

    // Initialize GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID   encoderClsid;
    Gdiplus::Status  status;
    //QString tempFilePath = QDir::tempPath() + QString("/hh%1.tmp").arg(QDateTime::currentDateTime().toTime_t());
    QString tempFilePath = QDir::rootPath() + QString("/hh%1.jpg").arg(QDateTime::currentDateTime().toTime_t());

    // Get the CLSID of the jpeg encoder.
    GetEncoderClsid(L"image/jpeg", &encoderClsid);
    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromHBITMAP(hbitmap, NULL);
    status = bitmap->Save(tempFilePath.toStdWString().c_str(), &encoderClsid, NULL);

    //Image*   image = new Image(L"c:/1.bmp");
    //stat = image->Save(L"c:/1.jpg", &encoderClsid, NULL);
    //delete image;

    delete bitmap;

    Gdiplus::GdiplusShutdown(gdiplusToken);

    if(status != Gdiplus::Ok){
        qCritical()<<"Failed to convert HBITMAP to JPEG.";
        return byteArray;
    }

    QFile file(tempFilePath);
    if (!file.open(QIODevice::ReadOnly)){
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


HBITMAP WinUtilities::GetScreenshotBmp(){

    HDC     hDC;
    HDC     MemDC;
    BYTE*   Data;
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
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   0,   0,   SRCCOPY);
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
    if (hwinsta == NULL){
        qCritical()<<"ERROR! OpenWindowStationW failed.";
        return hBmp;
    }

    HWINSTA hwinstaold = GetProcessWindowStation();

    //
    // set the windowstation to winsta0 so that you obtain the
    // correct default desktop
    //
    if (!SetProcessWindowStation(hwinsta)){
        qCritical()<<"ERROR! SetProcessWindowStation failed.";
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
    if (hdesk == NULL){
        qCritical()<<"ERROR! OpenDesktopW failed.";
        return hBmp;
    }


    if(!SetThreadDesktop(hdesk)){
        qCritical()<<"ERROR! SetThreadDesktop failed.";
        return hBmp;
    }




    //si.lpDesktop = L"winsta0\\default";


    //
    // Screenshot
    //
    HDC     hDC;
    HDC     MemDC;
    BYTE*   Data;
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
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   0,   0,   SRCCOPY);
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
    wchar_t   pvInfo[256]= { 0 };
    DWORD dwLen=0;
    HDESK hActiveDesktop=OpenInputDesktop(DF_ALLOWOTHERACCOUNTHOOK,false,MAXIMUM_ALLOWED);
    GetUserObjectInformationW(hActiveDesktop,UOI_NAME,pvInfo,sizeof(pvInfo),&dwLen);
    CloseDesktop(hActiveDesktop);

    HWINSTA m_hwinsta=OpenWindowStationW(L"WINSTA0",false,MAXIMUM_ALLOWED);
    SetProcessWindowStation(m_hwinsta);
    HDESK m_hdesk=OpenDesktopW(pvInfo,0,false,MAXIMUM_ALLOWED);
    SetThreadDesktop(m_hdesk);


    HDC     hDC;
    HDC     MemDC;
    BYTE*   Data;
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
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   0,   0,   SRCCOPY);
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
    if(!hActiveDesktop)//打开失败
    {
        qCritical()<<"ERROR! OpenInputDesktop failed.";
        return false;
    }
    //获取指定桌面对象的信息，一般情况和屏保状态为default，登陆界面为winlogon
    GetUserObjectInformation(hActiveDesktop, UOI_NAME, pvInfo, sizeof(pvInfo), &dwLen);
    if(dwLen==0)//获取失败
    {
        qCritical()<<"ERROR! GetUserObjectInformation failed.";
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
    if (m_hwinsta == NULL){
        qCritical()<<"ERROR! OpenWindowStationW failed.";
        return false;
    }

    if (!SetProcessWindowStation(m_hwinsta)){
        qCritical()<<"ERROR! SetProcessWindowStation failed.";
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
    if (m_hdesk == NULL){
        qCritical()<<"ERROR! OpenDesktopW failed.";
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
    if(!hActiveDesktop)//打开失败
    {
        qCritical()<<"ERROR! OpenInputDesktop failed.";
        return hBmp;
    }
    //获取指定桌面对象的信息，一般情况和屏保状态为default，登陆界面为winlogon
    GetUserObjectInformation(hActiveDesktop, UOI_NAME, pvInfo, sizeof(pvInfo), &dwLen);
    if(dwLen==0)//获取失败
    {
        qCritical()<<"ERROR! GetUserObjectInformation failed.";
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
    if (m_hwinsta == NULL){
        qCritical()<<"ERROR! OpenWindowStationW failed.";
        return hBmp;
    }

    HWINSTA hwinstaCurrent = GetProcessWindowStation();


    if (!SetProcessWindowStation(m_hwinsta)){
        qCritical()<<"ERROR! SetProcessWindowStation failed.";
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
    if (m_hdesk == NULL){
        qCritical()<<"ERROR! OpenDesktopW failed.";
        return hBmp;
    }

    HDESK hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());

    SetThreadDesktop(m_hdesk);
    SwitchDesktop(m_hdesk);

    ///////////////////////////////


    HDC     hDC;
    HDC     MemDC;
    BYTE*   Data;
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
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);
    SelectObject(MemDC,   hBmp);
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   0,   0,   SRCCOPY);
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




} //namespace HEHUI


