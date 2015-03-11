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

#include <windows.h>





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











} //namespace HEHUI


