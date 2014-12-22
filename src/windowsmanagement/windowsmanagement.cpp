/*
 ****************************************************************************
 * windowsmanagement.cpp
 *
 * Created on: 2009-5-1
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
 * Last Modified on: 2010-08-18
 * Last Modified by: 贺辉
 ***************************************************************************
 */






#include <QSettings>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QDir>
#include <QProcess>
#include <QImage>
#include <QStandardPaths>
#include <QVariant>

#include <QDebug>



#include "windowsmanagement.h"


#ifdef Q_OS_WIN32

#include <Lm.h>
#include <Tlhelp32.h>
#include <Lmjoin.h>

#include <Userenv.h>

//#include "autoit3.h"

const int MaxUserAccountNameLength = 20;
const int MaxUserPasswordLength = LM20_PWLEN;
const int MaxUserCommentLength = 256;
const int MaxGroupNameLength = 256;

#include "WindowsAPI.h"

#endif


using namespace std;




namespace HEHUI {


WindowsManagement::WindowsManagement(QObject *parent) :
    QObject(parent)
{

    m_lastErrorString = "";
    runningNT6OS = isNT6OS();
    m_isAdmin = isAdmin();

    location = No1_Branch_Factory;
    //    m_newComputerNameToBeUsed = "";


    //    test();


    //#if defined(Q_OS_WIN32)
    //                SetProcessWorkingSetSize(GetCurrentProcess(), 0xFFFFFFFF, 0xFFFFFFFF);
    //                //SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
    //#endif


}


bool WindowsManagement::addNewSitoyUserToLocalSystem(const QString &userName, const QString &userPassword, const QString &userComment, const QString &emails, const QString &dept){

    QMutexLocker locker(&mutex);


#ifdef Q_OS_WIN32
    m_lastErrorString = "";
    m_outputMsgs.clear();

    std::wstring userid = userName.toStdWString();
    std::wstring password = userPassword.toStdWString();
    std::wstring comment = userComment.toStdWString();



    wchar_t id[MaxUserAccountNameLength*sizeof(wchar_t)+1];
    wcscpy(id, userid.c_str());

    wchar_t pwd[MaxUserPasswordLength*sizeof(wchar_t)+1];
    wcscpy(pwd, password.c_str());

    wchar_t cmt[MaxUserCommentLength*sizeof(wchar_t)+1];
    wcscpy(cmt, comment.c_str());

    emit signalProgressUpdate(QString(tr("Adding user %1 to local system...").arg(userName)), 0);
    QCoreApplication::processEvents();
    if(!addUserToLocalSystem(id, pwd, cmt)){
        emit signalAddingUserJobDone(false);
        return false;
    }

    emit signalProgressUpdate(QString(tr("Adding user %1 to local 'Administrators' group...").arg(userName)), 15);
    QCoreApplication::processEvents();
    if(!addUserToLocalGroup(id, L"Administrators")){
        emit signalAddingUserJobDone(false);
        return false;
    }
    addUserToLocalGroup(id, L"Users");

    emit signalProgressUpdate(QString(tr("Save settings...")), 30);
    QCoreApplication::processEvents();
    QSettings settings(userInfoFilePath(), QSettings::IniFormat);
    settings.beginGroup(userName);
    settings.setValue("Dept", dept);
    settings.setValue("EmailAccount", emails.section("@", 0, 0));
    if(emails.contains("sitoydg.com", Qt::CaseInsensitive)){
        settings.setValue("IntEmail", 1);
    }else{
        settings.setValue("IntEmail", 0);
    }
    if(emails.contains("sitoy.com", Qt::CaseInsensitive)){
        settings.setValue("ExtEmail", 1);
    }else{
        settings.setValue("ExtEmail", 0);
    }
    settings.setValue("Location", quint16(location));

    settings.endGroup();

    bool ok = false;

    //    if(!m_newComputerNameToBeUsed.isEmpty()){
    //        emit signalProgressUpdate(QString(tr("Update computer name ...")), 45);
    //        QCoreApplication::processEvents();
    //        ok = setComputerName(m_newComputerNameToBeUsed.replace("_", "-").toStdWString().c_str());
    //        if(!ok){
    //            m_outputMsgs.append(lastErrorString);
    //        }
    //    }



    emit signalProgressUpdate(QString(tr("Join workgroup ...")), 70);
    QCoreApplication::processEvents();
    ok = joinWorkgroup(dept);
    if(!ok){
        m_outputMsgs.append(m_lastErrorString);
    }

    emit signalProgressUpdate(QString(tr("Set Starting up with M$ windows ...")), 85);
    QCoreApplication::processEvents();
    ok = setUserAutoLogin(userName, userPassword, true);
    if(!ok){
        m_outputMsgs.append(m_lastErrorString);
    }

    QString outlookInstalledPathString = outlookInstalledPath();
    if(QFileInfo(outlookInstalledPathString).exists()){
        ok = setStartupWithWin(outlookInstalledPathString, "", "Email", true);
    }else{
        ok = setStartupWithWin(runningNT6OS?"wlmail.exe":"msimn.exe" , "", "Email", true);
    }
    if(!ok){
        m_outputMsgs.append(m_lastErrorString);
    }
    //    ok = setStartupWithWin(QCoreApplication::applicationFilePath(), "", "", true);
    //    if(!ok){
    //        m_outputMsgs.append(lastErrorString);
    //    }

    emit signalProgressUpdate(QString(tr("Done!")), 100);
    QCoreApplication::processEvents();

#else
    //QMessageBox::information(this, tr("Invalid Operation"), tr(
    //		"This Function Is For Ducking M$ Windows Only!"));

    error = tr("This Function Is For Ducking M$ Windows Only!");
    emit signalAddingUserJobDone(false);
    return false;
#endif

    m_lastErrorString = "";
    emit signalAddingUserJobDone(true);

    return true;

}



QStringList WindowsManagement::outputMessages() const{
    return m_outputMsgs;
}


#ifdef Q_OS_WIN32
bool WindowsManagement::enablePrivilege(LPCTSTR privilegeName){
    m_lastErrorString = "";

    HANDLE hToken;
    BOOL rv;
    //TOKEN_PRIVILEGES priv = {1, {0, 0, SE_PRIVILEGE_ENABLED}};
    TOKEN_PRIVILEGES priv;
    LookupPrivilegeValue(0, privilegeName, &priv.Privileges[0].Luid);
    priv.PrivilegeCount = 1;
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
    AdjustTokenPrivileges(hToken, FALSE, &priv, sizeof(priv), 0, 0);
    rv = (GetLastError() == ERROR_SUCCESS);
    CloseHandle(hToken);

    if(!rv){
        m_lastErrorString = tr("Can Not Adjust Token Privileges!");
    }

    return rv;

}

bool WindowsManagement::isNT6OS()
{
    m_lastErrorString = "";

    OSVERSIONINFO  osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx (&osvi);
    if(osvi.dwMajorVersion > 5){
        return true;
    }

    m_lastErrorString = tr("Current OS is not NT6!");
    return false;

}

void WindowsManagement::freeMemory(){

#if defined(Q_OS_WIN32)
    //SetProcessWorkingSetSize(GetCurrentProcess(), 0xFFFFFFFF, 0xFFFFFFFF);
    SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
#endif

}

QString WindowsManagement::getEnvironmentVariable(const QString &environmentVariable){

    QString variableValueString = "";

    DWORD nSize = 512;
    LPWSTR variableValueArray = new wchar_t[nSize];

    int result = GetEnvironmentVariableW (environmentVariable.toStdWString().c_str(), variableValueArray, nSize);
    if(result == 0){
        return variableValueString;
    }

    variableValueString = QString::fromWCharArray(variableValueArray);
    //qDebug("WindowsManagement::userInfoFilePath(): %s:", qPrintable(path));

    delete [] variableValueArray;

    return variableValueString;

}


QString WindowsManagement::getExeFileVersion(const QString &exeFileName){
    QString versionString = "0.0.0.0";

    if(!QFileInfo(exeFileName).exists()){
        return versionString;
    }

    VS_FIXEDFILEINFO *VInfo;
    unsigned int i=GetFileVersionInfoSizeW(exeFileName.toStdWString().c_str() ,0);
    if (i)
    {
        wchar_t *ver=new wchar_t [i+1];
        int ok= GetFileVersionInfoW(exeFileName.toStdWString().c_str(), 0, i, ver);

        if (ok)
        {
            wchar_t nameBuffer[4];
            wcscpy(nameBuffer, L"\\");
            if (VerQueryValueW(ver, nameBuffer, (LPVOID*)&VInfo, &i))
            {
                QStringList versionStrings;
                versionStrings.append(QString::number(VInfo->dwFileVersionMS >> 16));
                versionStrings.append(QString::number(VInfo->dwFileVersionMS & 0x00ff));
                versionStrings.append(QString::number(VInfo->dwFileVersionLS >> 16));
                versionStrings.append(QString::number(VInfo->dwFileVersionLS & 0x00ff));
                versionString = versionStrings.join(".");
            }
        }

    }

    return versionString;

}



QStringList WindowsManagement::localUsers() {
    qDebug()<<"--WindowsManagement::localUsers()";

    m_lastErrorString = "";

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

    }
    while (nStatus == ERROR_MORE_DATA);

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
        pBuf = NULL;
    }

    return users;


}

QStringList WindowsManagement::localCreatedUsers() {
    //qDebug()<<"--WindowsManagement::localCreatedUsers()";

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

QString WindowsManagement::getUserNameOfCurrentThread() {

    DWORD size = MaxUserAccountNameLength + 1;
    wchar_t username[MaxUserAccountNameLength + 1];

    if(!GetUserNameW(username, &size)){
        m_lastErrorString = tr("Can not retrieve the name of the user associated with the current thread! Code:%1 ").arg(QString::number(GetLastError()));
        return QString("");
    }

    return QString::fromWCharArray(username);

}

QString WindowsManagement::WinSysErrorMsg(LONG winErrorCode){
    wchar_t buffer[8192];
    ZeroMemory(buffer, 8192);

    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, winErrorCode, 0, buffer, 8192, 0);
    return QString::fromWCharArray(buffer).simplified();
}

//When running on 64-bit Windows if you want to read a value specific to the 64-bit environment you have to suffix the HK... with 64 i.e. HKLM64.
bool WindowsManagement::parseRegKeyString(const QString &keyString, HKEY *rootKey, QString *subKeyString){
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

bool WindowsManagement::regOpen(const QString &key, HKEY *hKey, REGSAM samDesired){
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

bool WindowsManagement::regRead(HKEY hKey, const QString &valueName, QString *value){

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

bool WindowsManagement::regRead(const QString &key, const QString &valueName, QString *value, bool on64BitView){
    //qDebug()<<"--WindowsManagement::regRead(...) "<<" key:"<<key<<" valueName:"<<valueName;

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

bool WindowsManagement::regEnumVal(HKEY hKey, QStringList *valueNameList){
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

bool WindowsManagement::regEnumVal(const QString &key, QStringList *valueNameList, bool on64BitView){
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

bool WindowsManagement::regEnumKey(HKEY hKey, QStringList *keyNameList){
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

bool WindowsManagement::regEnumKey(const QString &key, QStringList *keyNameList, bool on64BitView){
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

bool WindowsManagement::regCreateKey(HKEY hKey, const QString &subKeyName, HKEY *hSubKey){

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

bool WindowsManagement::regCreateKey(const QString &key, const QString &subKeyName, HKEY *hSubKey, bool on64BitView){

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

bool WindowsManagement::regSetValue(HKEY hKey, const QString &valueName, const QString &value, DWORD valueType){

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

bool WindowsManagement::regSetValue(const QString &key, const QString &valueName, const QString &value, DWORD valueType, bool on64BitView){
    //qDebug()<<"--WindowsManagement::regSetValue(...) "<<" key:"<<key<<" valueName:"<<valueName<<" value:"<<value;

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

bool WindowsManagement::regDeleteKey(HKEY hKey, const QString &subKeyName, bool on64BitView){

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

bool WindowsManagement::regDeleteKey(const QString &key, bool on64BitView){

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

bool WindowsManagement::regDeleteValue(HKEY hKey, const QString &valueName){

    DWORD dwRet = RegDeleteValueW(hKey, valueName.toStdWString().c_str());
    if(dwRet != ERROR_SUCCESS){
        qCritical()<<"ERROR! RegDeleteValueW failed! "<<dwRet<<": "<<WinSysErrorMsg(dwRet);
        return false;
    }

    return true;
}

bool WindowsManagement::regDeleteValue(const QString &key, const QString &valueName, bool on64BitView){

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

void WindowsManagement::regCloseKey(HKEY hKey){
    RegCloseKey(hKey);
}

bool WindowsManagement::is64BitApplication(){
    return 8 == sizeof( void * );
    //return (sizeof(LPFN_ISWOW64PROCESS) == 8)? TRUE: FALSE;
}

bool WindowsManagement::isWow64()
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

bool WindowsManagement::isUserAutoLogin(){
    m_lastErrorString = "";

    QString value = "0";
    bool ok = regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "AutoAdminLogon", &value, true);
    if(!ok){
        return false;
    }
    qDebug()<<"value:"<<value;
    return value.toInt();
}

bool WindowsManagement::setUserAutoLogin(const QString &userName, const QString &password, bool autoLogin)
{

    m_lastErrorString = "";

    if(!m_isAdmin){
        m_lastErrorString = tr("Administrator Privilege Required!");
        return false;
    }

    QString key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";

    bool ok = regSetValue(key, "AutoAdminLogon", autoLogin?"1":"0", REG_SZ, true);
    if(!ok){
        m_lastErrorString = tr("Can not set 'AutoAdminLogon' for 'AutoAdminLogon'!");
    }

    ok = regSetValue(key, "DefaultUserName", autoLogin?userName:"", REG_SZ, true);
    if(!ok){
        m_lastErrorString = tr("Can not set 'DefaultUserName' for 'AutoAdminLogon'!");
    }

    ok = regSetValue(key, "DefaultPassword", autoLogin?password:"", REG_SZ, true);
    if(!ok){
        m_lastErrorString = tr("Can not set 'DefaultPassword' for 'AutoAdminLogon'!");
    }

    return ok;
}

bool WindowsManagement::initNewSitoyUser(){
    QMutexLocker locker(&mutex);

    qApp->processEvents();
    m_lastErrorString = "";
    m_outputMsgs.clear();


    QString userName = getUserNameOfCurrentThread();

    emit signalProgressUpdate(tr("Reading settings ..."), 10);

    QSettings *ini = new QSettings(userInfoFilePath(), QSettings::IniFormat, this);
    ini->beginGroup(userName);
    if(!ini->contains("Dept")){
        m_lastErrorString = tr("Can not find user info!");
        emit signalInitializingUserJobDone(false);
        return false;
    }

    QString dept = ini->value("Dept", "").toString();
    QString emailAccount = ini->value("EmailAccount", "").toString();
    bool hasIntEmail = ini->value("IntEmail", 0).toBool();
    bool hasExeEmail = ini->value("ExtEmail", 0).toBool();
    quint16 userLocation = ini->value("Location", quint16(No1_Branch_Factory)).toUInt();
    this->location = Location(userLocation);

    //    int userNameArraySize = userName.size() * sizeof(wchar_t) + 1;
    //    wchar_t * userNameArray = new wchar_t[userNameArraySize];
    //    userName.toWCharArray(userNameArray);
    wchar_t userNameArray[MaxUserAccountNameLength];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    emit signalProgressUpdate(tr( "Setting up email accounts ..."), 20);

    QString outlookInstalledPathString = outlookInstalledPath();
    if(QFileInfo(outlookInstalledPathString).exists()){
        emit signalProcessOutputUpdated(tr("Setting Up Outlook Mail Account"));

        if(hasIntEmail){addOutlookMailAccount(userName, dept, true, "", emailAccount);}
        if(hasExeEmail){addOutlookMailAccount(userName, dept, false, "", emailAccount);}
    }else{
        QString storeRoot;
        //if(homeDrive.UpperCase() == "C:\\" ){
        if(getDiskFreeSpace("G:\\")/(1024*1024*1024) > 5){
            storeRoot = "G:\\Email";
        }else if(getDiskFreeSpace("F:\\")/(1024*1024*1024) >= 5){
            storeRoot = "F:\\Email";
        }else if(getDiskFreeSpace("E:\\")/(1024*1024*1024) >= 5){
            storeRoot = "E:\\Email";
        }else if(getDiskFreeSpace("D:\\")/(1024*1024*1024) >= 5){
            storeRoot = "D:\\Email";
        }else{
            storeRoot = "C:\\Email";
        }

        //}
        //    qDebug()<<"storeRoot:"<<storeRoot;
        CreateDirectoryW(storeRoot.toStdWString().c_str(), NULL);

        emit signalProcessOutputUpdated(tr("Email Store Root:'%1'").arg(storeRoot));


        if(runningNT6OS){
            emit signalProcessOutputUpdated(tr("Setting Up Live Mail Account"));

            if(hasIntEmail){addLiveMailAccount(userName, dept, true, storeRoot, emailAccount);}
            if(hasExeEmail){addLiveMailAccount(userName, dept, false, storeRoot, emailAccount);}
        }else{
            emit signalProcessOutputUpdated(tr("Setting Up OE Mail Account"));

            if(hasIntEmail){addOEMailAccount(userName, dept, true, storeRoot, emailAccount);}
            if(hasExeEmail){addOEMailAccount(userName, dept, false, storeRoot, emailAccount);}
        }

        if(getFileSystemName(storeRoot).toUpper() == "NTFS"){
            emit signalProcessOutputUpdated(tr("Setting Up File Permissions"));

            QProcess process(this);
            process.setProcessChannelMode(QProcess::MergedChannels);
            if(runningNT6OS){
                process.start(QString("takeown.exe /R /D Y /F %1").arg(storeRoot));
                process.waitForFinished();
            }
            process.start(QString("cacls.exe %1 /T /E /C /G %2:F").arg(storeRoot + "/*").arg(userName));
            //process.start(QString("cacls.exe %1 /T /E /C /G Everyone:F").arg(storeRoot + "/*"));
            process.waitForFinished();
        }

    }


    emit signalProgressUpdate(tr( "Setting up IME ..."), 30);

    if(!setupIME()){
        //m_outputMsgs.append(tr("Can not setup IME"));
        m_outputMsgs.append(m_lastErrorString);
    }


    emit signalProgressUpdate(tr("Adding user to local 'Power Users' group..."), 40);

    if (!addUserToLocalGroup(userNameArray, L"Power Users")) {
        //m_outputMsgs.append(tr("Can not add user to local 'Power Users' group!"));
        m_outputMsgs.append(m_lastErrorString);
    }

    emit signalProgressUpdate(tr("Deleting user from local 'Administrators' group..."), 50);

    if (!deleteUserFromLocalGroup(userNameArray, L"Administrators")) {
        //m_outputMsgs.append(tr("Can not delete user from local 'Administrators' group!"));
        m_outputMsgs.append(m_lastErrorString);
    }

    emit signalProgressUpdate(tr("Connecting to net drive ..."), 60);

    if (!addConnectionToNetDrive()) {
        //m_outputMsgs.append(tr("Can not connect to net drive!"));
        m_outputMsgs.append(m_lastErrorString);
    }

    emit signalProgressUpdate(tr("Connecting to network printers ..."), 70);
    //    if (!addPrinterConnections(dept)) {
    //        //m_outputMsgs.append(tr("Can not connect to network printers!"));
    //        m_outputMsgs.append(error);
    //    }

    emit signalProgressUpdate(tr("Disable Application Starting with M$ windows ..."), 80);
    setStartupWithWin(QCoreApplication::applicationFilePath(), "", "", false);
    setStartupWithWin("" , "", "Email", false);


    emit signalProgressUpdate(tr( "Saving settings ..."), 90);

    ////////////
    ini->remove("");
    ini->endGroup();
    delete ini;
    ////////////

    emit signalProgressUpdate(tr( "Done! User '%1' initialized!").arg(userName), 100);

    m_lastErrorString = "";
    emit signalInitializingUserJobDone(true);
    return true;

}

bool WindowsManagement::userNeedInit(const QString &userName){

    QString m_userName = userName;

    if(m_userName.isEmpty()){
        m_userName = getUserNameOfCurrentThread();
    }

    QSettings ini(userInfoFilePath(), QSettings::IniFormat, this);
    //     ini.beginGroup(userName);
    //     if(!ini.contains("Dept")){
    //        return false;
    //     }
    m_lastErrorString = "";
    return ini.contains(m_userName+"/Dept");
}

float WindowsManagement::getDiskFreeSpace(const QString &directoryName){

    m_lastErrorString = "";

    ULARGE_INTEGER freeBytesAvailableToUser, totalBytes, totalFreeBytes;

    if(GetDiskFreeSpaceExW(directoryName.toStdWString().c_str(), &freeBytesAvailableToUser, &totalBytes, &totalFreeBytes))
    {
        return (float)freeBytesAvailableToUser.QuadPart;
    }else{
        m_lastErrorString = tr("Can not get disk free space!");
        return 0.0;
    }

}

QString WindowsManagement::lastError() const{
    return m_lastErrorString;
}

bool WindowsManagement::isAdmin(const QString &userName){
    QString name = userName.trimmed();
    if(name.isEmpty()){
        name = getUserNameOfCurrentThread();
    }

    if(name.isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        return false;
    }

    if(name.toLower() == "system"){
        return true;
    }

    if(!localUsers().contains(name, Qt::CaseInsensitive)){
        m_lastErrorString = tr("User '%1' does not exist!").arg(name);
        return false;
    }

    QStringList groups;
    getLocalGroupsTheUserBelongs(&groups, userName);
    //qWarning()<<QString("User:%1 Groups:%2").arg(userName).arg(groups.join(","));
    
    bool userIsAdmin = groups.contains("Administrators", Qt::CaseInsensitive);
    //qWarning()<<QString(" %1 is admin? %2").arg(name).arg(userIsAdmin);
    
    return userIsAdmin;

}

bool WindowsManagement::updateUserPassword(const QString &userName, const QString &password, bool activeIfAccountDisabled){
    qDebug()<<"--WindowsManagement::updateUserPassword(...) "<<"userName:"<<userName;
    m_lastErrorString = "";

    QString name = userName.trimmed();
    if(name.isEmpty()){
        name = getUserNameOfCurrentThread();
    }

    if(name.isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        return false;
    }

    //    if(!localUsers().contains(name, Qt::CaseInsensitive)){
    //        error = tr("User '%1' does not exist!").arg(name);
    //        return false;
    //    }


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
    if( netRet == NERR_Success )
    {
        //
        // The function was successful;
        //  set the usri3_password_expired value to a nonzero value.
        //  Call the NetUserSetInfo function.
        //
        wchar_t pwd[MaxUserPasswordLength*sizeof(wchar_t)+1];
        wcscpy(pwd, password.toStdWString().c_str());
        pUsr->usri1_password = pwd;

        if(activeIfAccountDisabled){
            DWORD flags = pUsr->usri1_flags;
            if(flags & UF_ACCOUNTDISABLE){
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
        if( netRet == NERR_Success ){
            printf("Password has been changed for user %S\n", name.toStdWString().c_str());
            result = true;

        }else {
            //printf("Error %d occurred.  Parm Error %d returned.\n", netRet, dwParmError);
            m_lastErrorString = tr("Error %1 occurred while updating the password. Parm Error %2 returned.").arg(netRet).arg(dwParmError);
            qCritical()<<m_lastErrorString;
            result = false;
        }
        //
        // Must free the buffer returned by NetUserGetInfo.
        //
        NetApiBufferFree( pUsr);
    }else{
        //printf("NetUserGetInfo failed: %d\n",netRet);
        m_lastErrorString = tr("An error occurred while updating the password. %1:%2.").arg(netRet).arg(WinSysErrorMsg(netRet));
        qCritical()<<m_lastErrorString;
        result = false;
    }

    return result;
}

bool WindowsManagement::setupUserAccountState(const QString &userName,  bool enableAccount){

    m_lastErrorString = "";
    QString name = userName.trimmed();

    if(name.isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        return false;
    }

    bool result = false;

    DWORD dwLevel = 1;
    PUSER_INFO_1 pUsr = NULL;
    NET_API_STATUS netRet = 0;
    DWORD dwParmError = 0;

    netRet = NetUserGetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE *)&pUsr);
    if( netRet == NERR_Success )
    {
        DWORD flags = pUsr->usri1_flags;
        if(enableAccount){
            if(flags & UF_ACCOUNTDISABLE){
                pUsr->usri1_flags = flags ^ UF_ACCOUNTDISABLE;
            }
        }else{
            pUsr->usri1_flags = flags | UF_ACCOUNTDISABLE;
        }

        netRet = NetUserSetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE)pUsr, &dwParmError);

        if( netRet == NERR_Success ){
            //printf("Password has been changed for user %S\n", name.toStdWString().c_str());
            m_lastErrorString = "";
            result = true;

        }else {
            //printf("Error %d occurred.  Parm Error %d returned.\n", netRet, dwParmError);
            m_lastErrorString = tr("Error %1 occurred while setting up the account. Parm Error %2 returned.").arg(netRet).arg(dwParmError);
            qCritical()<<m_lastErrorString;
            result = false;
        }

        NetApiBufferFree( pUsr);
    }else{
        //printf("NetUserGetInfo failed: %d\n",netRet);
        m_lastErrorString = tr("An error occurred while setting up the account. NetUserGetInfo failed: %1").arg(netRet);
        qCritical()<<m_lastErrorString;
        result = false;
    }

    return result;
}

WindowsManagement::UserAccountState WindowsManagement::getUserAccountState(const QString &userName){
    qDebug()<<"--WindowsManagement::getUserAccountState(...) "<<" userName:"<<userName;
    UserAccountState result = UAS_Unknown;
    m_lastErrorString = "";

    QString name = userName.trimmed();
    if(name.isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        return result;
    }

    DWORD dwLevel = 1;
    PUSER_INFO_1 pUsr = NULL;
    NET_API_STATUS netRet = 0;

    netRet = NetUserGetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE *)&pUsr);
    if( netRet == NERR_Success )
    {

        DWORD flags = pUsr->usri1_flags;

        if(flags & UF_ACCOUNTDISABLE){
            result = UAS_Disabled;
        }else{
            result = UAS_Enabled;
        }

    }else{
        //printf("NetUserGetInfo failed: %d\n",netRet);
        m_lastErrorString = tr("An error occurred while setting up the account. %1:%2.").arg(netRet).arg(WinSysErrorMsg(netRet));
        qDebug()<<m_lastErrorString;
    }

    return result;

}

bool WindowsManagement::getUserLastLogonAndLogoffTime(const QString &userName, QDateTime *lastLogonTime, QDateTime *lastLogoffTime){

    QString name = userName.trimmed();
    if(name.isEmpty()){
        name = getUserNameOfCurrentThread();
    }

    if(name.isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        return false;
    }

    DWORD dwLevel = 2;
    PUSER_INFO_2 pUsr = NULL;
    NET_API_STATUS netRet = 0;
    //DWORD dwParmError = 0;

    netRet = NetUserGetInfo( NULL, name.toStdWString().c_str(), dwLevel, (LPBYTE *)&pUsr);
    if( netRet == NERR_Success )
    {
        DWORD lastLogon = 0, lastLogoff = 0;
        lastLogon = pUsr->usri2_last_logon;
        lastLogoff = pUsr->usri2_last_logoff;
        //qWarning()<<"On:"<<lastLogon<<" Off:"<<lastLogoff;


        if(lastLogon && lastLogonTime){
            *lastLogonTime = QDateTime::fromTime_t(lastLogon);
        }
        if(lastLogoff && lastLogoffTime){
            *lastLogoffTime = QDateTime::fromTime_t(lastLogoff);
        }

        //qWarning()<<"On:"<<lastLogonTime.toString("yyyy.MM.dd hh:mm:ss")<<" Off:"<<lastLogoffTime.toString("yyyy.MM.dd hh:mm:ss");

        NetApiBufferFree( pUsr);

        return true;

    }else{
        //printf("NetUserGetInfo failed: %d\n",netRet);
        m_lastErrorString = tr("An error occurred while getting the last logon/logoff time. NetUserGetInfo failed! %1:%2.").arg(netRet).arg(WinSysErrorMsg(netRet));
        qDebug()<<m_lastErrorString;

        return false;
    }

}

QDateTime WindowsManagement::currentDateTimeOnServer(const QString &server, const QString &userName, const QString &password){
    m_lastErrorString = "";

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
    if(err !=  NO_ERROR){
        m_lastErrorString = tr("Can not connect to '%1'! %2:%3.").arg(server).arg(err).arg(WinSysErrorMsg(err));
        return dateTime;
    }
    //////////////////////////////////////////


    LPTIME_OF_DAY_INFO pBuf = NULL;
    NET_API_STATUS nStatus;

    nStatus = NetRemoteTOD(timeServer, (LPBYTE *)&pBuf);

    if (nStatus == NERR_Success)
    {
        if (pBuf != NULL)
        {

            dateTime = QDateTime::fromTime_t( pBuf->tod_elapsedt);

            //            fprintf(stderr, "\nThe current date is: %d/%d/%d\n",
            //                    pBuf->tod_month, pBuf->tod_day, pBuf->tod_year);
            //            fprintf(stderr, "The current time is: %d:%d:%d\n",
            //                    pBuf->tod_hours, pBuf->tod_mins, pBuf->tod_secs);
        }


    }else{
        m_lastErrorString = tr("Can not get current time from server '%1'! %2:%3.").arg(server).arg(nStatus).arg(WinSysErrorMsg(nStatus));
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return dateTime;
}

bool WindowsManagement::setLocalTime(const QDateTime &datetime){


    m_lastErrorString = "";

    if(!datetime.isValid()){
        m_lastErrorString = tr("Invalid Time!");
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

    if(!SetLocalTime(&systemtime)){
        DWORD err = GetLastError();
        m_lastErrorString = tr("Can not set system time! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
        return false;
    }

    return true;
}

void WindowsManagement::getLocalGroupsTheUserBelongs(QStringList *groups, const QString &userName){

    m_lastErrorString = "";

    Q_ASSERT(groups);
    if(!groups){
        m_lastErrorString = tr("Invalid QStringList pointer!");
        return;
    }

    QString name = userName.trimmed();
    if(name.isEmpty()){
        name = getUserNameOfCurrentThread();
    }

    if(name.isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        qCritical()<<m_lastErrorString;
        //return QStringList();
        return;
    }

    //    if(!localUsers().contains(name, Qt::CaseInsensitive)){
    //        error = tr("User '%1' does not exist!").arg(name);
    //        qCritical()<<error;
    //        return QStringList();
    //    }


    //    QStringList groups;

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
    //
    // If the call succeeds,
    //
    if (nStatus == NERR_Success)
    {
        LPLOCALGROUP_USERS_INFO_0 pTmpBuf;
        DWORD i;
        DWORD dwTotalCount = 0;

        if ((pTmpBuf = pBuf) != NULL)
        {
            //                fprintf(stderr, "\nLocal group(s):\n");
            //
            // Loop through the entries and
            //  print the names of the local groups
            //  to which the user belongs.
            //
            for (i = 0; i < dwEntriesRead; i++)
            {
                Q_ASSERT(pTmpBuf != NULL);

                if (pTmpBuf == NULL)
                {
                    fprintf(stderr, "An access violation has occurred\n");
                    break;
                }

                //wprintf(L"\t-- %s\n", pTmpBuf->lgrui0_name);
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
        if (dwEntriesRead < dwTotalEntries){
            //fprintf(stderr, "\nTotal entries: %d", dwTotalEntries);
            qDebug()<<"Total entries:"<<dwTotalEntries;
        }

        //
        // Otherwise, just print the total.
        //
        //printf("\nEntries enumerated: %d\n", dwTotalCount);
    }else{
        //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
        qCritical()<<"A system error has occurred:"<<nStatus;
        m_lastErrorString = tr("A system error has occurred! %1:%2.").arg(nStatus).arg(WinSysErrorMsg(nStatus));
    }
    //
    // Free the allocated memory.
    //
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);


    //    return groups;

}

void WindowsManagement::getGlobalGroupsTheUserBelongs(QStringList *groups, const QString &userName, const QString &serverName){

    m_lastErrorString = "";

    Q_ASSERT(groups);
    if(!groups){
        m_lastErrorString = tr("Invalid QStringList pointer!");
        return;
    }


    QString name = userName.trimmed();
    if(name.isEmpty()){
        name = getUserNameOfCurrentThread();
    }
    if(userName.isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        qCritical()<<m_lastErrorString;
        //        return QStringList();
        return;
    }

    //QStringList groups;

    LPGROUP_USERS_INFO_0 pBuf = NULL;
    DWORD dwLevel = 0;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;
    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()){
        pszServerName = serverName.toStdWString().c_str();
    }

    //
    // Call the NetUserGetGroups function, specifying level 0.
    //
    nStatus = NetUserGetGroups(pszServerName,
                               userName.toStdWString().c_str(),
                               dwLevel,
                               (LPBYTE*)&pBuf,
                               dwPrefMaxLen,
                               &dwEntriesRead,
                               &dwTotalEntries);
    //
    // If the call succeeds,
    //
    if (nStatus == NERR_Success)
    {
        LPGROUP_USERS_INFO_0 pTmpBuf;
        DWORD i;
        DWORD dwTotalCount = 0;

        if ((pTmpBuf = pBuf) != NULL)
        {
            //fprintf(stderr, "\nGlobal group(s):\n");
            //
            // Loop through the entries;
            //  print the name of the global groups
            //  to which the user belongs.
            //
            for (i = 0; i < dwEntriesRead; i++)
            {
                Q_ASSERT(pTmpBuf != NULL);

                if (pTmpBuf == NULL)
                {
                    fprintf(stderr, "An access violation has occurred\n");
                    m_lastErrorString += tr("An access violation has occurred!\n");
                    break;
                }

                //wprintf(L"\t-- %s\n", pTmpBuf->grui0_name);
                groups->append(QString::fromWCharArray(pTmpBuf->grui0_name));

                pTmpBuf++;
                dwTotalCount++;
            }
        }
        //
        // If all available entries were
        //  not enumerated, print the number actually
        //  enumerated and the total number available.
        //
        //if (dwEntriesRead < dwTotalEntries){
        //    fprintf(stderr, "\nTotal entries: %d", dwTotalEntries);
        //}
        //
        // Otherwise, just print the total.
        //
        //printf("\nEntries enumerated: %d\n", dwTotalCount);
    }else{
        switch(nStatus){
        case ERROR_ACCESS_DENIED:
            m_lastErrorString += tr("The user does not have access to the requested information.");
            break;
        case NERR_InvalidComputer:
            m_lastErrorString += tr("The computer name is invalid.");
            break;
        case NERR_UserNotFound:
            m_lastErrorString += tr("The user name could not be found.");
            break;
        default:
            m_lastErrorString += tr("A system error has occurred! %1:%2.").arg(nStatus).arg(WinSysErrorMsg(nStatus));
            break;
        }

        //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
    }
    //
    // Free the allocated buffer.
    //
    if (pBuf != NULL){
        NetApiBufferFree(pBuf);
    }

    //return groups;

}

QString WindowsManagement::userInfoFilePath(){
    DWORD nSize = 256;
    LPWSTR windowsDir = new wchar_t[nSize];

    int result = GetEnvironmentVariableW (L"WINDIR", windowsDir, nSize);
    if(result == 0){
        QString("c:/windows").toWCharArray(windowsDir);
        //windowsDir = L"c:/windows";
    }

    //QString path = QString::fromWCharArray(windowsDir) + "/system32/.sitoyusers" ;
    QString path = QString::fromWCharArray(windowsDir) + "/.sitoyusers" ;
    //qDebug("WindowsManagement::userInfoFilePath(): %s:", qPrintable(path));

    delete [] windowsDir;

    return path;
}

QStringList WindowsManagement::localGroups() {
    QStringList groups;

    //DWORD dwLevel = 0;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    DWORD i;
    //DWORD dwTotalCount = 0;
    NET_API_STATUS nStatus;


    LPLOCALGROUP_INFO_0 pBuf = NULL;
    LPLOCALGROUP_INFO_0 pTmpBuf;

    do // begin do
    {
        //
        nStatus = NetLocalGroupEnum(
                    NULL,
                    0,
                    (LPBYTE*)&pBuf,
                    dwPrefMaxLen,
                    &dwEntriesRead,
                    &dwTotalEntries,
                    &dwResumeHandle);
        //
        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
        {
            if ((pTmpBuf = pBuf) != NULL)
            {
                //
                for (i = 0; (i < dwEntriesRead); i++)
                {
                    Q_ASSERT(pTmpBuf != NULL);

                    if (pTmpBuf == NULL)
                    {
                        fprintf(stderr, "An access violation has occurred\n");
                        break;
                    }

                    wprintf(L"\t-- %s\n", pTmpBuf->lgrpi0_name);
                    groups.append(QString::fromWCharArray(pTmpBuf->lgrpi0_name));

                    pTmpBuf++;
                    //dwTotalCount++;
                }
            }
        }

        else{
            qDebug()<<"A system error has occurred!";
            //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
        }

        //
        if (pBuf != NULL)
        {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    }

    while (nStatus == ERROR_MORE_DATA); // end do

    if (pBuf != NULL)
        NetApiBufferFree(pBuf);

    //fprintf(stderr, "Total of %d groups\n\n", dwTotalCount);

    return groups;


}

bool WindowsManagement::addUserToLocalSystem(const QString &userName, const QString &userPassword, const QString &comment){

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    wchar_t userPasswordArray[MaxUserPasswordLength * sizeof(wchar_t) + 1];
    wcscpy(userPasswordArray, userPassword.toStdWString().c_str());

    wchar_t commentArray[MaxUserCommentLength * sizeof(wchar_t) + 1];
    wcscpy(commentArray, comment.toStdWString().c_str());

    return addUserToLocalSystem(userNameArray, userPasswordArray, commentArray);

}

bool WindowsManagement::addUserToLocalSystem(LPWSTR userName, LPWSTR userPassword, LPWSTR comment){

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


    if (nStatus == NERR_Success)
    {
        qDebug()<<"User '"<<userName<<"' successfully added to Local system.\n";
        //wprintf(stderr, "User %s has been successfully added on %s\n",
        //szUserName, szServerName);
        m_lastErrorString = "";
        return true;
    } else {
        //qDebug()<<"An Error occured while adding user '"<<userName<<"' to Local system!\n";
        //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
    }

    m_lastErrorString = tr("An Error occured while adding user '%1' to Local system! %2:%3.").arg(QString::fromWCharArray(userName)).arg(nStatus).arg(WinSysErrorMsg(nStatus));
    qDebug()<<m_lastErrorString;
    return false;

}

bool WindowsManagement::deleteUserFromLocalSystem(const QString &userName){

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    return deleteUserFromLocalSystem(userNameArray);

}

bool WindowsManagement::deleteUserFromLocalSystem(LPWSTR userName){

    NET_API_STATUS nStatus;

    nStatus = NetUserDel(NULL, userName);

    if (nStatus == NERR_Success) {
        //fwprintf(stderr, "User %s has been successfully deleted! \n", userName);
        qDebug()<<"User"<< userName<<" has been successfully deleted!";
        m_lastErrorString = "";
        return true;

    }
    else {
        //fprintf(stderr, "A system error has occurred: %d\n", nStatus);
        qDebug()<<"A system error has occurred: "<<nStatus;

        m_lastErrorString = tr("A system error has occurred! %1:%2.").arg(nStatus).arg(WinSysErrorMsg(nStatus));
        return false;
    }


}

bool WindowsManagement::addUserToLocalGroup(const QString &userName, const QString &groupName){

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    wchar_t groupNameArray[MaxGroupNameLength * sizeof(wchar_t) + 1];
    wcscpy(groupNameArray, groupName.toStdWString().c_str());

    return addUserToLocalGroup(userNameArray, groupNameArray);

}

bool WindowsManagement::addUserToLocalGroup(LPWSTR userName,  LPCWSTR groupName){

    m_lastErrorString = "";

    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    NET_API_STATUS err;

    // Now add the user to the local group.
    localgroup_members.lgrmi3_domainandname = userName;
    err = NetLocalGroupAddMembers(NULL,      //
                                  groupName,             // Group name
                                  3,                          // Name
                                  (LPBYTE)&localgroup_members, // Buffer
                                  1 );                        // Count

    switch ( err )
    {
    case NERR_Success:
        qDebug()<<"User '"<<userName<<"' successfully added to Local "<<groupName<<" Group.\n";
        //printf("User successfully added to Local Group.\n");

        return true;
        break;
    case ERROR_MEMBER_IN_ALIAS:
        //printf("User already in Local Group.\n");
        m_lastErrorString = tr("User is already in Local '%1' Group").arg(QString::fromWCharArray(groupName));
        qDebug()<< m_lastErrorString;
        return false;
        break;
    default:
        //printf("An error occured while adding User to Local Group '%s' Error code: %d\n", groupName, err);
        m_lastErrorString = tr("An error occured while adding user '%1' to local group '%2'! %3:%4.").arg(QString::fromWCharArray(userName)).arg(QString::fromWCharArray(groupName)).arg(err).arg(WinSysErrorMsg(err));
        qDebug()<< m_lastErrorString;

        return false;
        break;
    }

    //    return( err );
}

bool WindowsManagement::deleteUserFromLocalGroup(const QString &userName, const QString &groupName){

    wchar_t userNameArray[MaxUserAccountNameLength * sizeof(wchar_t) + 1];
    wcscpy(userNameArray, userName.toStdWString().c_str());

    wchar_t groupNameArray[MaxGroupNameLength * sizeof(wchar_t) + 1];
    wcscpy(groupNameArray, groupName.toStdWString().c_str());

    return deleteUserFromLocalGroup(userNameArray, groupNameArray);

}

bool WindowsManagement::deleteUserFromLocalGroup(LPWSTR userName,  LPCWSTR groupName){

    m_lastErrorString = "";

    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    NET_API_STATUS err;

    // Now delete the user from the local group.

    localgroup_members.lgrmi3_domainandname = userName;

    err = NetLocalGroupDelMembers(NULL,      //
                                  groupName,             // Group name
                                  3,                          // Name
                                  (LPBYTE)&localgroup_members, // Buffer
                                  1 );                        // Count

    switch ( err )
    {
    case NERR_Success:
        qDebug()<<"User '"<<userName<<"' successfully deleted from Local "<<groupName<<" Group.\n";
        //printf("User successfully added to Local Group.\n");
        return true;
        break;
    case ERROR_NO_SUCH_MEMBER:
        
        //qWarning()<<"User '"<<userName<<"' does not exist.";
        //printf("User does not exist.\n");
        m_lastErrorString = tr("User '%1' does not exist!").arg(QString::fromWCharArray(userName));
        qDebug()<< m_lastErrorString;

        return false;
        break;
    default:

        //qWarning()<<"Error occured while deleting user '"<<userName<<"' from Local "<<groupName<<" Group.\n";
        //printf("Error deleting User from Local Group: %d\n", err);

        m_lastErrorString = tr("An error occured while deleting user '%1' from local group '%2'! %3:%4.").arg(QString::fromWCharArray(userName)).arg(QString::fromWCharArray(groupName)).arg(err).arg(WinSysErrorMsg(err));
        qDebug()<< m_lastErrorString;
        return false;
        break;
    }

    //    return( err );

}

QStringList WindowsManagement::getMembersOfLocalGroup(const QString &groupName, const QString &serverName){

    m_lastErrorString = "";

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

    do
    {
        nStatus = NetLocalGroupGetMembers(serverName.toStdWString().c_str(),
                                          groupName.toStdWString().c_str(),
                                          dwLevel,
                                          (LPBYTE*)&pBuf,
                                          dwPrefMaxLen,
                                          &dwEntriesRead,
                                          &dwTotalEntries,
                                          &dwResumeHandle);

        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
        {
            if ((pTmpBuf = pBuf) != NULL)
            {
                for (i = 0; (i < dwEntriesRead); i++)
                {
                    Q_ASSERT(pTmpBuf != NULL);

                    if (pTmpBuf == NULL)
                    {
                        m_lastErrorString += tr("An access violation has occurred\n");
                        break;
                    }
                    users.append(QString::fromWCharArray(pTmpBuf->lgrmi2_domainandname).toLower());

                    pTmpBuf++;
                    dwTotalCount++;
                }
            }
        }else{
            m_lastErrorString += tr("A system error has occurred! %1:%2.").arg(nStatus).arg(WinSysErrorMsg(nStatus));
        }

        if (pBuf != NULL)
        {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    }while (nStatus == ERROR_MORE_DATA); // end do

    if (pBuf != NULL)
        NetApiBufferFree(pBuf);


    return users;

}

bool WindowsManagement::setComputerName(const QString &newComputerName) {

    m_lastErrorString = "";

    if(newComputerName.trimmed().isEmpty()){
        m_lastErrorString = tr("Invalid computer name!");
        return false;
    }

    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\Tcpip\\Parameters", "NV Hostname", newComputerName, REG_SZ, true);

    //if (SetComputerNameExW(ComputerNamePhysicalDnsHostname, computerName)){
    if (SetComputerNameW(newComputerName.toStdWString().c_str())){
        m_lastErrorString = "";
        return true;
    }else{
        qCritical()<< "Can not set computer name to " << newComputerName;
        m_lastErrorString = tr("Can not set computer name to '%1'").arg(newComputerName);
        return false;
    }

}

QString WindowsManagement::getComputerName(){
    qDebug()<<"--WindowsManagement::getComputerName()";

    m_lastErrorString = "";
    QString computerName = "";
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    LPWSTR name = new wchar_t[size];

    if(GetComputerNameW(name, &size)){
        computerName = QString::fromWCharArray(name);
    }else{
        m_lastErrorString = tr("Can not get computer name! Error: %1").arg(GetLastError());
    }

    delete [] name;

    return computerName.toLower();

}

bool WindowsManagement::getComputerNameInfo(QString *dnsDomain, QString *dnsHostname, QString *netBIOSName){

    m_lastErrorString = "";

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
            m_lastErrorString += tr("\nFailed to get dns domain! Error Code: %1").arg(GetLastError());
        }
    }

    if(dnsHostname){
        ZeroMemory(buffer, size);

        nameType = ComputerNameDnsHostname;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok){
            *dnsHostname = QString::fromWCharArray(buffer);
        }else{
            m_lastErrorString += tr("\nFailed to get dns hostname! Error Code: %1").arg(GetLastError());
        }
    }

    if(netBIOSName){
        ZeroMemory(buffer, size);

        nameType = ComputerNameNetBIOS;
        ok = GetComputerNameExW(nameType, buffer, &size);
        if(ok){
            *netBIOSName = QString::fromWCharArray(buffer);
        }else{
            m_lastErrorString += tr("\nFailed to get NetBIOS name! Error Code: %1").arg(GetLastError());
        }
    }

    return ok;
}

bool WindowsManagement::joinWorkgroup(const QString &workgroup){


    NET_API_STATUS err;
    err = NetJoinDomain(NULL, workgroup.toStdWString().c_str(), NULL, NULL, NULL, 0);

    switch ( err )
    {
    case NERR_Success:
        qDebug()<<"' Successfully join the workgroup '"<<workgroup<<"' .\n";
        //printf("Successfully join the workgroup.\n");

        m_lastErrorString = "";
        return true;
        break;
    case NERR_InvalidWorkgroupName:
        qDebug()<<"Invalid Workgroup Name!";
        //printf("Invalid Workgroup Name!\n");
        err = 0;

        m_lastErrorString = tr("Invalid Workgroup Name '%1'!").arg(workgroup);
        return false;
        break;
    default:
        qDebug()<<"An error occured while trying to join the workgroup '"<<workgroup<<"' .\n";
        //printf("Error occured while trying to join the workgroup: %d\n", err);

        m_lastErrorString = tr("An error occured while trying to join the workgroup '%1'! %2:%3. ").arg(workgroup).arg(err).arg(WinSysErrorMsg(err));
        return false;
        break;
    }

    return( err );

}

bool WindowsManagement::joinDomain(const QString &domainName, const QString &accountName, const QString &password, const QString &serverName){
    qDebug()<<"--WindowsManagement::joinDomain(...) "<<"domainName:"<<domainName<<" accountName:"<<accountName<<" password:"<<password;

    m_lastErrorString = "";

    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()){
        pszServerName = serverName.toStdWString().c_str();
    }
    //LPCWSTR lpAccount = QString(accountName + "@" + domainName).toStdWString().c_str();

    NET_API_STATUS err = NetJoinDomain(pszServerName, domainName.toStdWString().c_str(), NULL, accountName.toStdWString().c_str(), password.toStdWString().c_str(), NETSETUP_JOIN_DOMAIN | NETSETUP_ACCT_CREATE);
    switch(err){
    case NERR_Success:
        return true;
        break;
    case ERROR_INVALID_PARAMETER:
        m_lastErrorString = tr("Invalid parameter!");
        break;
    case ERROR_NO_SUCH_DOMAIN:
        m_lastErrorString = tr("No such domain!");
        break;
    case NERR_SetupAlreadyJoined:
        m_lastErrorString = tr("The computer is already joined to a domain!");
        break;
    case NERR_InvalidWorkgroupName:
        m_lastErrorString = tr("The specified workgroup name is not valid!");
        break;
    default:
        m_lastErrorString = tr("Failed to join a domain! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
        break;

    }

    return false;

}

bool WindowsManagement::unjoinDomain(const QString &accountName, const QString &password, const QString &serverName){

    m_lastErrorString = "";

    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()){
        pszServerName = serverName.toStdWString().c_str();
    }

    NET_API_STATUS err = NetUnjoinDomain(pszServerName, accountName.toStdWString().c_str(), password.toStdWString().c_str(), NETSETUP_ACCT_DELETE);
    switch(err){
    case NERR_Success:
        return true;
        break;
    case ERROR_INVALID_PARAMETER:
        m_lastErrorString = tr("A parameter is incorrect.");
        break;
    case NERR_SetupNotJoined:
        m_lastErrorString = tr("The computer is not currently joined to a domain.");
        break;
    case NERR_SetupDomainController:
        m_lastErrorString = tr("This computer is a domain controller and cannot be unjoined from a domain.");
        break;
    default:
        m_lastErrorString = tr("Failed to unjoin machine from the domain! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
        break;
    }

    return false;


}

QString WindowsManagement::getJoinInformation(bool *isJoinedToDomain, const QString &serverName){
    qDebug()<<"--WindowsManagement::getJoinInformation()";

    m_lastErrorString = "";

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
        m_lastErrorString = tr("Can not get join status information! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
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

bool WindowsManagement::renameMachineInDomain(const QString &newMachineName, const QString &accountName, const QString &password, const QString &serverName){

    m_lastErrorString = "";

    LPCWSTR pszServerName = NULL; // The server is the default local computer.
    if(!serverName.trimmed().isEmpty()){
        pszServerName = serverName.toStdWString().c_str();
    }

    NET_API_STATUS err = NetRenameMachineInDomain(pszServerName, newMachineName.toStdWString().c_str(), accountName.toStdWString().c_str(), password.toStdWString().c_str(), NETSETUP_JOIN_DOMAIN | NETSETUP_ACCT_CREATE | NETSETUP_JOIN_WITH_NEW_NAME);
    switch(err){
    case NERR_Success:
        return true;
        break;
    case ERROR_INVALID_PARAMETER:
        m_lastErrorString = tr("A parameter is incorrect.");
        break;
    case NERR_SetupNotJoined:
        m_lastErrorString = tr("The computer is not currently joined to a domain.");
        break;
    case NERR_SetupDomainController:
        m_lastErrorString = tr("This computer is a domain controller and cannot be unjoined from a domain.");
        break;
    default:
        m_lastErrorString = tr("Failed to rename machine in domain! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
        break;
    }

    return false;

}

void WindowsManagement::getAllUsersLoggedOn(QStringList *users, const QString &serverName){

    Q_ASSERT(users);
    m_lastErrorString = "";

    if(!users){
        m_lastErrorString = "Invalid QStringList pointer!";
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

    QString computerName = this->getComputerName();

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
                        m_lastErrorString += tr("An access violation has occurred\n");
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
            m_lastErrorString += tr("A system error has occurred: %1\n").arg(nStatus);
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


bool WindowsManagement::getLogonInfoOfCurrentUser(QString *userName, QString *domain, QString *logonServer){

    m_lastErrorString = "";

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


    //
    // Free the allocated memory.
    //
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);

    return ok;

}


bool WindowsManagement::addConnectionToNetDrive(){

    QString labelStr = "";
    QString pathStr = "";

    switch(location){
    case No1_Branch_Factory:
    case No2_Branch_Factory:
    case No3_Branch_Factory:
    case No4_Branch_Factory:
        labelStr = QString("S:");
        pathStr = QString("\\\\200.200.200.21\\Sys");
        break;
    case LEATHER_PRODUCTS_FACTORY_YD:
        labelStr = QString("Y:");
        pathStr = QString("\\\\193.168.2.1\\Sys");
        break;
    default:
        labelStr = QString("S:");
        pathStr = QString("\\\\200.200.200.21\\Sys");
        break;
    }

    //           QString labelStr = QString("S:");
    wchar_t label[3*sizeof(wchar_t)+1];
    wcscpy(label, labelStr.toStdWString().c_str());

    //           QString pathStr = QString("\\\\200.200.200.21\\Sys");
    wchar_t path[MAX_PATH*sizeof(wchar_t)+1];
    wcscpy(path, pathStr.toStdWString().c_str());


    DWORD err;
    NETRESOURCEW res;
    res.dwType = RESOURCETYPE_DISK;
    res.lpLocalName = label;
    res.lpRemoteName = path;
    res.lpProvider = NULL;

    err = WNetAddConnection2W(&res, NULL, NULL, CONNECT_UPDATE_PROFILE);

    switch(err) {
    case NO_ERROR:
        //printf("Net Drive successfully connected.\n");
        m_lastErrorString = "";
        return true;
        // break;
    case ERROR_ACCESS_DENIED:
        //printf("ACCESS DENIED.\n");
        qDebug()<<"ACCESS DENIED";
        m_lastErrorString = tr("ACCESS DENIED");
        return false;
        // break;
    case ERROR_NO_NETWORK:
        //printf("NO NETWORK.\n");
        qDebug()<<"NO NETWORK";
        m_lastErrorString = tr("NO NETWORK");
        return false;
    case ERROR_NO_NET_OR_BAD_PATH:
        //printf("NO NET OR BAD PATH.\n");
        qDebug()<<"NO NET OR BAD PATH";
        m_lastErrorString = tr("NO NET OR BAD PATH");
        return false;
    case ERROR_BAD_NET_NAME:
        //printf("BAD NET NAME.\n");
        qDebug()<<"BAD NET NAME";
        m_lastErrorString = tr("BAD NET NAME");
        return false;
    default:
        //printf("An error occured while connecting to network drive: %d\n", err);
        qDebug()<<"An error occured while connecting to network drive: " <<err;

        m_lastErrorString = tr("An error occured while connecting to network drive! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
        return false;
        // break;
    }

}

bool WindowsManagement::addPrinterConnections(const QString &department){

    m_lastErrorString = "";

    QString printerServer = "";
    QString printer1 = "";
    QString printer2 = "";

    if(location == LEATHER_PRODUCTS_FACTORY_YD){
        printerServer = QString("\\\\193.168.2.1\\");
    }else{
        printerServer = QString("\\\\200.200.200.3\\");
    }

    switch(location){
    case No1_Branch_Factory:
    {
        if(department.toLower() == "sales"){
            printer1 = "RICOHAfi2F01";
            printer2 = "RICOHAfi2F02";
        }else{
            QString deptPrefix = department.left(3).toLower();
            if(deptPrefix == "pmc" || deptPrefix == "acc" || deptPrefix == "pla" || deptPrefix == "pur"){
                printer1 = "RICOHA1045(3F)";
            }
            if(deptPrefix == "adm"){
                printer1 = "RICOHAfi(1f)";
                printer2 = "RICOHA2238c";
            }

        }
    }
        break;
    case No2_Branch_Factory:
    {
        printer1 = "RICOHAfi(new)";
        //            if(deptPrefix == "acc"){
        //              printer2 = "RICOHA2238c";
        //            }
    }
        break;
    case No3_Branch_Factory:
        printer1 = "RICOHAfiYQ";
        break;
        //case No4_Branch_Factory:
        //    printer1 = "RICOHAfiYQ";
        //    break;
    case LEATHER_PRODUCTS_FACTORY_YD:
        printer1 = "ar-350";
        break;
    default:
        break;
    }

    //    qDebug()<<"printerServer:"<<printerServer<<" printer1:"<<printer1;

    bool ok = false;
    if(!printer1.isEmpty()){
        QString printerStr = printerServer + printer1;
        wchar_t printer[MAX_PATH*sizeof(wchar_t)+1];
        wcscpy(printer, printerStr.toStdWString().c_str());
        ok = AddPrinterConnectionW(printer);
        if(!ok){
            //printf("Error occured while connecting to network printer '%s'! Error code: %d\n", printer, GetLastError());
            qDebug()<< QString("An error occured while connecting to network printer '%1'! Error code:%2").arg(printer1).arg(GetLastError());
            m_lastErrorString = tr("An error occured while connecting to network printer '%1'! Error code: %2").arg(printer1).arg(GetLastError());
        }

    }

    if(!printer2.isEmpty()){
        QString printerStr = printerServer + printer2;
        wchar_t printer[MAX_PATH*sizeof(wchar_t)+1];
        wcscpy(printer, printerStr.toStdWString().c_str());
        ok = AddPrinterConnectionW(printer);
        if(!ok){
            //printf("An error occured while connecting to network printer '%s'! %d\n", printer, GetLastError());
            qDebug()<< QString("An error occured while connecting to network printer '%1'! Error code:%2").arg(printer2).arg(GetLastError());
            m_lastErrorString += tr("\nAn error occured while connecting to network printer '%1'! Error code: %2").arg(printer2).arg(GetLastError());
        }

    }

    return ok;
}

bool WindowsManagement::setupIME(){
    qApp->processEvents();
    m_lastErrorString = "";

    if(QLocale::system().name() == "zh_CN"){
        return true;
    }

    QString wbID = "";
    QString keyName = "HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\Keyboard Layouts\\";

    QString imeFileKey = "Ime File";
    QString imeFileValue = "";
    //QString imeNameKey = "Layout Text" ;
    //QString imeNameValue = "";
    QStringList imeKeys;
    regEnumKey(keyName, &imeKeys);
    foreach (QString imeKey, imeKeys) {
        if(imeKey.length() != 8){
            continue;
        }
        QString imeKey2 = keyName + "\\" + imeKey;
        imeFileValue = "";
        regRead(imeKey2, imeFileKey, &imeFileValue);
        if(imeFileValue == "WB.IME"){
            wbID = imeKey;
            qDebug()<<"wbID:"<<wbID;
            break;
        }
    }

    if(wbID.isEmpty()){
        m_lastErrorString = tr("Can Not Find WB IME Key");
        return false;
    }

    regSetValue("HKEY_CURRENT_USER\\Keyboard Layout\\Preload", "1", "00000404", REG_SZ);
    regSetValue("HKEY_CURRENT_USER\\Keyboard Layout\\Preload", "2", wbID, REG_SZ);

    LoadKeyboardLayoutW(wbID.toStdWString().c_str(), 0x1);

    return true;

}

bool WindowsManagement::isStartupWithWin(const QString &applicationFilePath, const QString &parameters, const QString &valueNameString)
{
    m_lastErrorString = "";

    QString key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    QString valueName = "";
    if(valueNameString.trimmed().isEmpty()){
        valueName = applicationFilePath;
    }else{
        valueName = valueNameString;
    }

    QString valueString = "";
    regRead(key, valueName, &valueString, true);

    QString targetString = QDir::toNativeSeparators(applicationFilePath);
    if(!parameters.trimmed().isEmpty()){
        targetString += QString(" " + parameters);
    }

    if(valueString == targetString){
        return true;
    }else{
        return false;
    }

}

bool WindowsManagement::setStartupWithWin(const QString &applicationFilePath, const QString &parameters, const QString &valueNameString, bool startupWithWin)
{
    m_lastErrorString = "";

    if(!m_isAdmin){
        m_lastErrorString = tr("Administrator Privilege Required!");
        return false;
    }

    QString nativeApplicationFilePath = QDir::toNativeSeparators(applicationFilePath);

    QString key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    QString valueName;
    if(valueNameString.trimmed().isEmpty()){
        valueName = nativeApplicationFilePath;
    }else{
        valueName = valueNameString;
    }

    bool ok = false;
    if(startupWithWin){
        QString valueString = nativeApplicationFilePath;
        if(!parameters.trimmed().isEmpty()){
            valueString += QString(" " + parameters);
        }
        ok = regSetValue(key, valueName, valueString, REG_SZ, true);
    }else{
        ok = regDeleteValue(key, valueName, true);
    }

    if(!ok){
        m_lastErrorString = tr("Failure writing to the system registry! Can not %1 '%2' to start with M$ Windows!").arg(startupWithWin?tr("enable"):tr("disable")).arg(applicationFilePath);
        return false;
    }

    return true;

}


bool WindowsManagement::addOEMailAccount(const QString &userName, const QString &department, bool intEmail, const QString &storeRoot, const QString &accountName)
{
    qDebug()<<"----WindowsManagement::addOEMailAccount(...)";

    QString num = "" ;
    QString popServer = "";
    QString smtpServer = "";
    QString emailAddress = "" ;
    QString mailFolderPath = storeRoot + "\\" + userName;

    if (intEmail) {
        num = "00000002";
        emailAddress = accountName+"@sitoydg.com";
        popServer = "200.200.200.4" ;
        smtpServer = "200.200.200.4" ;
    }else{
        num = "00000001" ;
        emailAddress = accountName+"@sitoy.com"  ;
        popServer = "pop3.sitoy.com"  ;
        smtpServer = "smtp.sitoy.com" ;
    }

    QString key = "HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Account Manager";
    QString valueName = "Default Mail Account";
    QString value = num;
    regSetValue(key, valueName, value, REG_SZ, false);

    key = "HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Account Manager\\Accounts\\" + num;
    HKEY hKey;
    bool ok = regOpen(key, &hKey);
    if(!ok){
        m_lastErrorString = tr("Failed to open registry '%1'!").arg(key);
        return false;
    }

    regSetValue(hKey, QString("Account Name"), emailAddress, REG_SZ);
    regSetValue(hKey, QString("Connection Type"), QString("3"), REG_DWORD);
    regSetValue(hKey, QString("POP3 Prompt for Password"), QString("0"), REG_DWORD);
    regSetValue(hKey, QString("POP3 Server"), popServer, REG_SZ);
    regSetValue(hKey, QString("POP3 Use Sicily"), QString("0"), REG_DWORD);
    regSetValue(hKey, QString("POP3 User Name"), accountName, REG_SZ);
    regSetValue(hKey, QString("SMTP Display Name"), accountName, REG_SZ);
    regSetValue(hKey, QString("SMTP Email Address"), emailAddress, REG_SZ);
    regSetValue(hKey, QString("SMTP Server"), smtpServer, REG_SZ);
    if(!intEmail){
        regSetValue(hKey, QString("SMTP Port"), QString("465"), REG_DWORD);
        regSetValue(hKey, QString("SMTP Secure Connection"), QString("1"), REG_DWORD);
        regSetValue(hKey, QString("SMTP Use Sicily"), QString("2"), REG_DWORD);
    }
    regCloseKey(hKey);

    key = "HKEY_CURRENT_USER\\Identities";
    valueName = "Default User ID";
    QString defaultUserID = "";
    regRead(key, valueName, &defaultUserID);

    key = "HKEY_CURRENT_USER\\Identities\\" + defaultUserID + "\\Software\\Microsoft\\Outlook Express\\5.0";
    regSetValue(key, "Store Root", mailFolderPath, REG_SZ);

    key = "HKEY_CURRENT_USER\\Identities\\" + defaultUserID + "\\Software\\Microsoft\\Outlook Express\\5.0\\Mail";
    regSetValue(key, "Safe Attachments", QString("0"), REG_DWORD);

    CreateDirectoryW(storeRoot.toStdWString().c_str(), NULL);
    CreateDirectoryW(mailFolderPath.toStdWString().c_str(), NULL);

    m_lastErrorString = "";
    return true;
}

bool WindowsManagement::addLiveMailAccount(const QString &userName, const QString &department, bool intEmail, const QString &storeRoot, const QString &accountName)
{

    qDebug() << tr("Setting Up Live Mail Account");

    if(!runningNT6OS){
        m_lastErrorString = "Current OS is not NT6!";
        return false;
    }

    QString liveMailFolderPath = storeRoot + "\\" + userName;

    QString mailAccountFolderPath = "";
    QString mailAccountConfigFileName = "";
    QString displayAccountName = "";

    QString popServer = "";
    QString smtpServer = "";
    QString emailAddress = "" ;
    QString sMTPUseSicily = "";
    QString sMTPPort = "";
    QString sMTPSecureConnection = "";


    if (intEmail) {
        displayAccountName = userName + "(INT)";
        mailAccountFolderPath = liveMailFolderPath + "\\" + displayAccountName;
        mailAccountConfigFileName = "account{AAAAAAAA-3061-44FF-8BD4-AAAAAAAAAAAA}.oeaccount";

        emailAddress = accountName+"@sitoy.cn";
        popServer = "pop3.sitoy.com" ;
        smtpServer = "smtp.sitoy.com" ;
        sMTPUseSicily = "00000000";
        sMTPPort = "00000019";
        sMTPSecureConnection = "00000000";
    }else{
        displayAccountName = userName + "(EXT)";
        mailAccountFolderPath = liveMailFolderPath + "\\" + displayAccountName;
        mailAccountConfigFileName = "account{BBBBBBBB-3061-44FF-8BD4-BBBBBBBBBBBB}.oeaccount";

        emailAddress = accountName+"@sitoy.com"  ;
        popServer = "pop3.sitoy.com"  ;
        smtpServer = "smtp.sitoy.com" ;
        sMTPUseSicily = "00000002";
        sMTPPort = "000001d1";
        sMTPSecureConnection = "00000001";
    }

    CreateDirectoryW(storeRoot.toStdWString().c_str(), NULL);
    CreateDirectoryW(liveMailFolderPath.toStdWString().c_str(), NULL);
    //CreateDirectoryW(mailAccountFolderPath.toStdWString().c_str(), NULL);
    //CreateDir(mailAccountFolderPath + "\\Deleted Items");
    //CreateDir(mailAccountFolderPath + "\\Drafts");
    //CreateDir(mailAccountFolderPath + "\\Inbox");
    //CreateDir(mailAccountFolderPath + "\\Junk E-mail");
    //CreateDir(mailAccountFolderPath + "\\Sent Items");


    QStringList xml;
    xml.append("<?xml version=\"1.0\" encoding=\"utf-16\" ?>");

    xml.append("<MessageAccount>");

    xml.append("<Account_Name type=\"SZ\">" + displayAccountName + "</Account_Name>");
    xml.append("<Connection_Type type=\"DWORD\">00000003</Connection_Type>");
    xml.append("<POP3_Server type=\"SZ\">" + popServer + "</POP3_Server>");
    xml.append("<POP3_User_Name type=\"SZ\">" + accountName + "</POP3_User_Name>");
    xml.append("<POP3_Password2 type=\"BINARY\"></POP3_Password2>");
    xml.append( "<POP3_Port type=\"DWORD\">0000006e</POP3_Port>");
    xml.append("<POP3_Secure_Connection type=\"DWORD\">00000000</POP3_Secure_Connection>");
    xml.append( "<POP3_Timeout type=\"DWORD\">0000003c</POP3_Timeout>");
    xml.append( "<Leave_Mail_On_Server type=\"DWORD\">00000000</Leave_Mail_On_Server>");
    xml.append( "<Remove_When_Deleted type=\"DWORD\">00000000</Remove_When_Deleted>");
    xml.append( "<Remove_When_Expired type=\"DWORD\">00000000</Remove_When_Expired>");
    xml.append( "<POP3_Skip_Account type=\"DWORD\">00000000</POP3_Skip_Account>");
    xml.append( "<POP3_Prompt_for_Password type=\"DWORD\">00000000</POP3_Prompt_for_Password>");
    xml.append( "<SMTP_Server type=\"SZ\">" + smtpServer + "</SMTP_Server>");
    xml.append( "<SMTP_Use_Sicily type=\"DWORD\">" + sMTPUseSicily + "</SMTP_Use_Sicily>");
    xml.append("<SMTP_Port type=\"DWORD\">" + sMTPPort + "</SMTP_Port>");
    xml.append( "<SMTP_Secure_Connection type=\"DWORD\">" + sMTPSecureConnection + "</SMTP_Secure_Connection>");
    xml.append( "<SMTP_Timeout type=\"DWORD\">0000003c</SMTP_Timeout>");
    xml.append( "<SMTP_Display_Name type=\"SZ\">" + accountName + "</SMTP_Display_Name>");
    xml.append( "<SMTP_Email_Address type=\"SZ\">" + emailAddress + "</SMTP_Email_Address>");
    xml.append( "<SMTP_Split_Messages type=\"DWORD\">00000000</SMTP_Split_Messages>");

    xml.append( "</MessageAccount>");

    QString targetFilePath = liveMailFolderPath + "\\" + mailAccountConfigFileName;
    //QString targetFilePath = mailAccountFolderPath + "\\" + mailAccountConfigFileName;
    QFile file(targetFilePath);
    if(file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)){
        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("UTF-16"));
        out.setGenerateByteOrderMark(true);
        foreach(QString line, xml){
            out << line << endl;
        }
        xml.clear();
        file.flush();
        file.close();

    }else{
        qCritical()<<QString("Crirical Error: Can not write file '%1'").arg(targetFilePath);
        return false;
    }


    QString key = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows Live Mail";
    QString valueName = "Default Mail Account";
    QString value = mailAccountConfigFileName;
    regSetValue(key, valueName, value, REG_SZ);


    valueName = "Store Root";
    value = liveMailFolderPath;
    regSetValue(key, valueName, value, REG_SZ);

    key = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows Live Mail\\mail";
    valueName = "Safe Attachments";
    value = "0";
    regSetValue(key, valueName, value, REG_DWORD);

    m_lastErrorString = "";
    return true;
}

bool WindowsManagement::addOutlookMailAccount(const QString &userName, const QString &department, bool intEmail, const QString &storeRoot, const QString &accountName)
{
    //qDebug()<<"----WindowsManagement::addOutlookMailAccount(...)";

    qDebug() << tr("Setting Up Outlook Mail Account");

    QString num = "" ;
    QString miniUID = "" ;
    QString popServer = "";
    QString smtpServer = "";
    QString emailAddress = "" ;
    //QString mailFolderPath = storeRoot + "\\" + userName;

    if (intEmail) {
        num = "00000004";
        miniUID = "127761513";
        emailAddress = accountName+"@sitoy.cn";
        popServer = "pop3.sitoy.com" ;
        smtpServer = "pop3.sitoy.com" ;
    }else{
        num = "00000003" ;
        miniUID = "4196440948";
        emailAddress = accountName+"@sitoy.com"  ;
        popServer = "pop3.sitoy.com"  ;
        smtpServer = "smtp.sitoy.com" ;
    }


    QString key = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows Messaging Subsystem\\Profiles\\Outlook\\9375CFF0413111d3B88A00104B2A6676\\" + num;
    regSetValue(key, QString("Account Name"), stringToOutlookHexString(emailAddress), REG_BINARY);

    regSetValue(key, QString("clsid"), "{ED475411-B0D6-11D2-8C3B-00104B2A6676}", REG_SZ);
    //regSetValue(key, QString("Display Name"), stringToOutlookHexString(popServer), REG_BINARY);
    regSetValue(key, QString("Display Name"), stringToOutlookHexString(emailAddress), REG_BINARY);
    regSetValue(key, QString("Email"), stringToOutlookHexString(emailAddress), REG_BINARY);
    regSetValue(key, QString("Mini UID"), miniUID, REG_DWORD);
    regSetValue(key, QString("POP3 Server"), stringToOutlookHexString(popServer), REG_BINARY);
    regSetValue(key, QString("POP3 User"), stringToOutlookHexString(accountName), REG_BINARY);
    regSetValue(key, QString("SMTP Server"), stringToOutlookHexString(smtpServer), REG_BINARY);

    if(!intEmail){
        regSetValue(key, QString("SMTP Port"), QString("465"), REG_DWORD);
        regSetValue(key, QString("SMTP Use Auth"), QString("1"), REG_DWORD);
        regSetValue(key, QString("SMTP Use SSL"), QString("1"), REG_DWORD);
    }


    //CreateDirectoryW(storeRoot.toStdWString().c_str(), NULL);
    //CreateDirectoryW(mailFolderPath.toStdWString().c_str(), NULL);

    m_lastErrorString = "";
    return true;

}

QString WindowsManagement::stringToOutlookHexString(const QString &string){

    QByteArray byteArray;
    byteArray.resize(0);
    byteArray = byteArray.append(string.toUtf8());
    byteArray = byteArray.toHex();

    for(int i = 0; i<byteArray.size(); ){
        i+=2;
        byteArray.insert(i, "00");
        i+=2;
    }
    byteArray.append("0000");
    byteArray.prepend("0x");

    //qWarning()<<"temp:"<<byteArray;

    return byteArray;
}

QString WindowsManagement::outlookInstalledPath(){

    QString key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.EXE";
    QString value = "";
    regRead(key, "", &value);

    return value;
}

void WindowsManagement::cleanTemporaryFiles(){

    QString basepath = QDir::rootPath();
    QString tempPath = "", tempIEPath = "";
    if(runningNT6OS){
        basepath += "Users/";
        tempPath = "/AppData/Local/Temp";
        tempIEPath = "/AppData/Local/Microsoft/Windows/Temporary Internet Files";
    }else{
        basepath += "Documents and Settings/";
        tempPath = "/Local Settings/Temp";
        tempIEPath = "/Local Settings/Temporary Internet Files";

    }

    QStringList filters, ignoredFiles, ignoredDirs;
    filters << "*" << "*.*";
    ignoredFiles << "index.dat";
    ignoredDirs << "Temp" << "Temporary Internet Files";

    QDir dir(basepath);
    //foreach (QString dirName, dir.entryList(QDir::AllDirs | QDir::Hidden | QDir::System | QDir::Readable | QDir::Writable | QDir::NoDotAndDotDot)) {
    foreach (QString dirName, dir.entryList(QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot)) {

        qDebug()<<"dirname:"<<dirName;

        QString path = basepath + dirName + tempPath;
        emit signalProgressUpdate(tr("Deleting Temporary Files ..."), 0);
        deleteFiles(path, filters, ignoredFiles, ignoredDirs);

        path = basepath + dirName + tempIEPath;
        emit signalProgressUpdate(tr("Deleting Temporary Internet Files ..."), 0);
        deleteFiles(path, filters, ignoredFiles, ignoredDirs);
    }

}

void WindowsManagement::deleteFiles(const QString &path, const QStringList & nameFilters, const QStringList & ignoredFiles, const QStringList & ignoredDirs){
    qDebug()<<"--WindowsManagement::deleteFiles(...)"<<" Path:"<<path;

    QDir dir(path);
    if(!dir.exists()){
        return;
    }

    QStringList filters = nameFilters;
    if(filters.isEmpty()){
        filters << "*" << "*.*";
    }

    int steps = 100/(dir.count());
    emit signalProgressUpdate(tr("Deleting Files In '%1' ...").arg(path), 0);

    //qlonglong size = 0;
    //QStringList filters;
    //filters << "*" << "*.*";
    int i = 0;
    //foreach(QString file, dir.entryList(/*filters,*/QDir::Files|QDir::System|QDir::Hidden))
    foreach(QString file, dir.entryList(filters, QDir::Files | QDir::System | QDir::Hidden))
    {
        if(ignoredFiles.contains(file)){continue;}
        if(!dir.remove(file)){
            qDebug() << "Failed To Delete :" + path + QDir::separator() + file ;
        }

        emit signalProgressUpdate(tr("Deleting File '%1' ...").arg(file), (++i)*steps);
        qApp->processEvents();
    }

    foreach(QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden))
    {
        deleteFiles(path + QDir::separator() + subDir, filters, ignoredFiles, ignoredDirs);
    }

    if(!ignoredDirs.contains(dir.dirName())){
        dir.rmdir(path);
    }

}

void WindowsManagement::modifySystemSettings(){
    //    qDebug()<<"----WindowsManagement::modifySystemSettings()";

    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\lanmanserver\\parameters", "AutoShareServer", "1", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\lanmanserver\\parameters", "AutoShareWks", "1", REG_DWORD);
    //经典共享方式
    //RegWrite("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\Lsa", "forceguest", "0", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa", "forceguest", "0", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa", "restrictanonymous", "0", REG_DWORD);

    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\RemoteRegistry", "Start", "2", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\Schedule", "Start", "2", REG_DWORD);

    //Runas
    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\seclogon", "Start", "2", REG_DWORD);
    //Remote Desktop
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "AllowMultipleTSSessions", "1", REG_DWORD);

    //Disable Firewall
    if(isNT6OS()){
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\services\\SharedAccess\\Parameters\\FirewallPolicy\\FirewallRules", "RemoteDesktop-In-TCP", "v2.10|Action=Allow|Active=TRUE|Dir=In|Protocol=6|LPort=3389|App=System|Name=@FirewallAPI.dll,-28753|Desc=@FirewallAPI.dll,-28756|EmbedCtxt=@FirewallAPI.dll,-28752|", REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\services\\SharedAccess\\Parameters\\FirewallPolicy\\PublicProfile", "EnableFirewall", "0", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile", "EnableFirewall", "0", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\services\\SharedAccess\\Parameters\\FirewallPolicy\\DomainProfile", "EnableFirewall", "0", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\MpsSvc", "Start", "4", REG_DWORD);
    }else{
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile", "EnableFirewall", "0", REG_DWORD);
    }

    if(QSysInfo::windowsVersion() > QSysInfo::WV_2000){
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server", "fDenyTSConnections", "0", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server", "fSingleSessionPerUser", "0", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\TermService", "Start", "2", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\wscsvc", "Start", "4", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\TelnetServer\\1.0", "SecurityMechanism", "6", REG_DWORD);
    }else{
        regSetValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\TelnetServer\\1.0", "NTLM", "0", REG_DWORD);
    }

    if(QSysInfo::windowsVersion() == QSysInfo::WV_XP){
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Licensing Core", "EnableConcurrentSessions", "1", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "EnableConcurrentSessions", "1", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "AllowMultipleTSSessions", "1", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services", "MaxInstanceCount", "5", REG_DWORD);

    }
    
    if(QSysInfo::windowsVersion() == QSysInfo::WV_WINDOWS7){
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\iphlpsvc", "Start", "4", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\CscService", "Start", "4", REG_DWORD);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\SSDPSRV", "Start", "4", REG_DWORD);
    }

    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\tvnserver", "Start", "2", REG_DWORD);
    regDeleteValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "tvncontrol");
    
    //Disable AVG IDS Agent
    //regDeleteValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\services\\AVGIDSAgent");


}

QString WindowsManagement::getFileSystemName(const QString &rootPath){
    m_lastErrorString = "";

    QString path = "";
    QRegExp rxp;
    rxp.setCaseSensitivity(Qt::CaseInsensitive);
    if(rootPath.startsWith("\\\\")){
        rxp.setPattern("^\\\\\\\\([a-zA-Z0-9.]+\\\\{1}){2}");
    }else{
        rxp.setPattern("^[a-zA-Z]:(\\\\|/)");
    }
    if(rxp.indexIn(rootPath) != -1){
        path = rxp.cap(0);
    }else{
        m_lastErrorString = tr("Invalid Root Path '%1' !").arg(rootPath);
        qCritical()<<QString("Invalid Root Path '%1' !").arg(rootPath)<<" "<<rxp.errorString();
        return "";
    }


    DWORD size = 256;
    wchar_t * fileSystemNameBuffer = new wchar_t[size];

    bool ok = GetVolumeInformationW(path.toStdWString().c_str(), NULL, 0, NULL, NULL, NULL, fileSystemNameBuffer, size);
    if(!ok){
        qDebug()<<"Failed to get volume information! Error Code:"<<GetLastError();
    }

    QString fileSystemName = QString::fromWCharArray(fileSystemNameBuffer);
    delete [] fileSystemNameBuffer;

    qDebug()<<QString("File System Name Of '%1':").arg(path)<<fileSystemName;

    return fileSystemName;

}

bool WindowsManagement::getTokenByProcessName(HANDLE &hToken, const QString &processName, bool justQuery){

    if (processName.trimmed().isEmpty()) {
        return false;
    }

    HANDLE hProcessSnap = NULL;
    BOOL bRet = FALSE;
    PROCESSENTRY32W pe32 = { 0 };

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return (FALSE);
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            //if (wcscmp(pe32.szExeFile, processName.toStdWString().c_str()) == 0) {
            if (QString::fromWCharArray(pe32.szExeFile).toLower() == processName.toLower()) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
                                              pe32.th32ProcessID);
                bRet = OpenProcessToken(hProcess, justQuery?TOKEN_QUERY:TOKEN_ALL_ACCESS, &hToken);
                CloseHandle(hProcessSnap);
                //qWarning()<<"~~~~~~~~~~~~~~~~~~~~~~~";
                return (bRet);
            }
            //qWarning()<<"~~~"<<QString::fromWCharArray(pe32.szExeFile);
        } while (Process32Next(hProcessSnap, &pe32));
        bRet = TRUE;
    } else
        bRet = FALSE;
    CloseHandle(hProcessSnap);
    return (bRet);

}

QList<HANDLE> WindowsManagement::getTokenListByProcessName(const QString &processName, bool justQuery){

    QList<HANDLE> tokenList;

    if (processName.trimmed().isEmpty()) {
        return tokenList;
    }

    HANDLE hProcessSnap = NULL;
    PROCESSENTRY32W pe32 = { 0 };

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return tokenList;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            //if (wcscmp(pe32.szExeFile, processName.toStdWString().c_str()) == 0) {
            if (QString::fromWCharArray(pe32.szExeFile).toLower() == processName.toLower()) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
                                              pe32.th32ProcessID);
                HANDLE hToken = 0;
                bool bRet = OpenProcessToken(hProcess, justQuery?TOKEN_QUERY:TOKEN_ALL_ACCESS, &hToken);
                CloseHandle(hProcessSnap);

                if(bRet){
                    tokenList.append(hToken);
                }else{
                    qCritical()<<"Error! Process Found, but can not get the token!";
                }
                qDebug()<<"--tokenList.size():"<<tokenList.size();
            }
            qDebug()<<"~~Exe File:"<<QString::fromWCharArray(pe32.szExeFile);
        } while (Process32Next(hProcessSnap, &pe32));

    }

    CloseHandle(hProcessSnap);

    return tokenList;

}

QString WindowsManagement::getAccountNameOfProcess(HANDLE &hToken){

    m_lastErrorString = "";
    QString accountName = "";

    if(!hToken){
        m_lastErrorString = tr("Invalid Process Token!");
        return accountName;
    }

    SID_NAME_USE peUse;

    bool isok = false;
    DWORD size = 256;
    wchar_t buf[256];
    wchar_t accountNamebuf[256];
    wchar_t domainNamebuf[256];
    DWORD dwNumBytesRet;
    DWORD dwNumBytesRet1;

    isok = GetTokenInformation(hToken, TokenUser, &buf, size, &dwNumBytesRet);
    if (isok) {
        dwNumBytesRet = size;
        dwNumBytesRet1 = size;
        isok = LookupAccountSidW(NULL, (DWORD *) (*(DWORD *) buf), accountNamebuf, &dwNumBytesRet, domainNamebuf, &dwNumBytesRet1, &peUse);
        if (isok) {
            accountName = QString::fromWCharArray(accountNamebuf);
            qDebug()<<"Account Name Of Process:"<<accountName;
            return accountName;
        }else{
            DWORD err = GetLastError();
            m_lastErrorString = tr("Can not get account name of process! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
        }
    }

    return accountName;

}

QString WindowsManagement::getAccountNameOfProcess(const QString &processName){

    HANDLE hToken;
    getTokenByProcessName(hToken, processName);
    return getAccountNameOfProcess(hToken);
    

    //    QList<HANDLE> list = getTokenListByProcessName(processName);
    //    qWarning()<<"~~~~list.size():"<<list.size();
    //    foreach(HANDLE token, list){
    //        getAccountNameOfProcess(token);
    //        CloseHandle(token);
    //    }
    
}

void WindowsManagement::showAdministratorAccountInLogonUI(bool show){

    if(runningNT6OS){
        if(show){
            regDeleteValue("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\000001F4", "UserDontShowInLogonUI");
        }else{
            regSetValue("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\000001F4", "UserDontShowInLogonUI", "0x01000000", REG_BINARY);
        }

    }else if(QSysInfo::windowsVersion()  == QSysInfo::WV_XP){
        if(show){
            regDeleteValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList", "Administrator");
        }else{
            regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList", "Administrator", "0", REG_DWORD);
        }
    }

}

bool WindowsManagement::createHiddenAdmiAccount(){

    m_lastErrorString = "";

    QString userName = "System$";
    //int size = 1024;

    deleteHiddenAdmiAccount();

    QStringList usersKeys;
    QString usersKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users";
    regEnumKey(usersKey, &usersKeys);

    QString password = "systemadmin";
    QString comment = "Built-in account for administering the local system";
    bool ok = false;
    ok = addUserToLocalSystem(userName, password, comment);
    if(!ok){
        return false;
    }

    QStringList newUsersKeys;
    regEnumKey(usersKey, &newUsersKeys);
    QString systemAccountKey = "";
    foreach (QString id, newUsersKeys) {
        if(!usersKeys.contains(id)){
            systemAccountKey = id;
            break;
        }
    }

    if(systemAccountKey.isEmpty()){
        m_lastErrorString = tr("Can not find the user key of %1!").arg(userName);
        return false;
    }
    //qWarning()<<"Key Of System$:"<<systemAccountKey;

    QString adminKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\000001F4";
    QString valueFName = "F";
    QString adminFValue = "";
    regRead(adminKey, valueFName, &adminFValue);
    if(adminFValue.isEmpty()){
        m_lastErrorString = tr("Can not read the value of 'F' from key '000001F4'!");
        return false;
    }
    regSetValue(QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\%1").arg(systemAccountKey), valueFName, adminFValue, REG_BINARY);
    regSetValue(QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\Names\\System$"), "Key", systemAccountKey, REG_SZ);

    QProcess process(this);
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

    deleteUserFromLocalSystem(userName);
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

bool WindowsManagement::deleteHiddenAdmiAccount(){

    m_lastErrorString = "";

    //QString userName = "System$";

    QString adminNameKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\Names\\System$";
    QString adminKeyValue = "";
    regRead(adminNameKey, "Key", &adminKeyValue);
    if(adminKeyValue.isEmpty()){
        m_lastErrorString = tr("Can not read System$ key!");
        return false;
    }

    bool ok = regDeleteKey(adminNameKey);
    if(!ok){
        m_lastErrorString = tr("Can not delete key 'System$'!");
        return false;
    }

    QString adminKeyString = QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\") + adminKeyValue;
    ok = regDeleteKey(adminKeyString);
    return ok;
}

bool WindowsManagement::hiddenAdmiAccountExists(){

    m_lastErrorString = "";
    //QString userName = "System$";

    QString adminNameKey = "HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\Names\\System$";
    QString adminKeyValue = "";
    regRead(adminNameKey, "Key", &adminKeyValue);
    if(adminKeyValue.isEmpty()){
        m_lastErrorString = tr("Can not read System$ key!");
        return false;
    }

    QString adminKeyString = QString("HKEY_LOCAL_MACHINE\\SAM\\SAM\\Domains\\Account\\Users\\") + adminKeyValue;
    QString adminKeyFValue = "";
    regRead(adminKeyString, "F", &adminKeyFValue);
    if(adminKeyFValue.isEmpty()){
        m_lastErrorString = tr("Can not read key '%1' related to System$!").arg(adminKeyFValue);
        return false;
    }

    return true;
}

bool WindowsManagement::setupUSBStorageDevice(bool enableRead, bool enableWrite){

    m_lastErrorString = "";

    //Service
    if(enableRead){
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "Start", "3", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "Start", "3", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "Start", "3", REG_DWORD);

    }else{
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS-", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\USBSTOR", "Start", "4", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS-", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "Start", "4", REG_DWORD);

        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "ImagePath", "system32\\DRIVERS\\USBSTOR.SYS-", REG_EXPAND_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002\\Services\\USBSTOR", "Start", "4", REG_DWORD);
    }

    //Policies
    //CD-ROM
    //regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f56308-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Read", enableRead?"0":"1", REG_DWORD);
    //regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f56308-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Write", enableWrite?"0":"1", REG_DWORD);

    //Removable Disk
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Read", enableRead?"0":"1", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Write", enableWrite?"0":"1", REG_DWORD);
    //Portable Storage Devices
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Read", enableRead?"0":"1", REG_DWORD);
    regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Write", enableWrite?"0":"1", REG_DWORD);


    regSetValue("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies", "WriteProtect", enableWrite?"0":"1", REG_DWORD);

    //TODO:Rename Driver Files
    //%systemroot%\inf\usbstor.inf
    //%systemroot%\inf\usbstor.PNF
    //%systemroot%\system32\DRIVERS\USBSTOR.SYS
    //WIN7:
    //%systemroot%\System32\DriverStore\FileRepository\usbstor.inf_x86_neutral_83027f5d5b2468d3
    //%systemroot%\System32\DriverStore\FileRepository\usbstor.inf_amd64_neutral_0725c2806a159a9d


    bool ok = false, readable = true, writeable = true;
    ok = readUSBStorageDeviceSettings(&readable, &writeable);
    if(!ok){
        m_lastErrorString = tr("Can not read registry!");
        qCritical()<<m_lastErrorString;
        return false;
    }

    if((enableRead == readable) && (enableWrite == writeable)){
       return true;
    }

    return true;

}

bool WindowsManagement::readUSBStorageDeviceSettings(bool *readable, bool *writeable){

    m_lastErrorString = "";

    bool ok = false;
    QString imagePath = "", start = "";
    regRead("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "ImagePath", &imagePath);
    ok = regRead("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\USBSTOR", "Start", &start);
    if(!ok){
        return false;
    }
    if(imagePath.toUpper() != "SYSTEM32\\DRIVERS\\USBSTOR.SYS" || start == "4"){
        if(readable){*readable = false;}
        if(writeable){*writeable = false;}
        return true;
    }

    //Removable Disk
    QString deny_Read = "", deny_Write = "";
    regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Read", &deny_Read);
    regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", "Deny_Write", &deny_Write);
    if(deny_Read == "1"){
        if(readable){*readable = false;}
        if(writeable){*writeable = false;}
        return true;
    }
    if(deny_Write == "1"){
        if(readable){*readable = false;}
        if(writeable){*writeable = true;}
        return true;
    }

    //Portable Storage Devices
    //regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Read", &deny_Read);
    //regRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\RemovableStorageDevices\\{6AC27878-A6FA-4155-BA85-F98F491D4F33}", "Deny_Write", &deny_Write);

    QString writeProtect = "";
    regRead("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies", "WriteProtect", &writeProtect);
    if(writeProtect == "1"){
        if(readable){*readable = true;}
        if(writeable){*writeable = false;}
    }

    return true;
}



bool WindowsManagement::setupProgrames(bool enable){

    m_lastErrorString = "";

    if(enable){

        regDeleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\QQ.exe");
        regDeleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\TXPlatform.exe");
        regDeleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\TM.exe");
        regDeleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\Timwp.exe");
        regDeleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\QQPI.exe");
        regDeleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\TXOPShow.exe");

    }else{

        QString debugger = "shutdown.exe -s -f -t 0 -c ";

        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\QQ.exe", "Debugger", debugger, REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\TXPlatform.exe", "Debugger", debugger, REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\Timwp.exe", "Debugger", debugger, REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\QQPI.exe", "Debugger", debugger, REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\TXOPShow.exe", "Debugger", debugger, REG_SZ);

        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\TM.exe", "Debugger", debugger, REG_SZ);

        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\QQGame.exe", "Debugger", debugger, REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\QQGameDl.exe", "Debugger", debugger, REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\Accel.exe", "Debugger", debugger, REG_SZ);
        regSetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\QQGwp.exe", "Debugger", debugger, REG_SZ);

    }

    return true;

}

bool WindowsManagement::setDeskWallpaper(const QString &wallpaperPath){

    m_lastErrorString = "";

    QString targetBMPFilePath = wallpaperPath;

    QFileInfo fi(targetBMPFilePath);
    if(!fi.exists()){
        m_lastErrorString = tr("Can not set wallpaper! File '%1' does not exist!").arg(targetBMPFilePath);
        return false;
    }

    //    DWORD nSize = 256;
    //    LPWSTR windowsDir = new wchar_t[nSize];
    //    int result = GetEnvironmentVariableW (L"WINDIR", windowsDir, nSize);
    //    if(result == 0){
    //        QString(QDir::rootPath() + "windows").toWCharArray(windowsDir);
    //    }
    //    QString targetDirPath = QString::fromWCharArray(windowsDir) ;
    QString targetDirPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if(!QDir(targetDirPath).exists()){
        targetDirPath = QDir::homePath();
    }

    //if(wallpaperPath.startsWith(":/") || fi.suffix().toLower() != ".bmp"){
    targetBMPFilePath = targetDirPath + QDir::separator() + fi.baseName() + ".bmp";
    QImage image(wallpaperPath);
    if(image.isNull()){
        m_lastErrorString = tr("Can not read image '%1' ! ").arg(wallpaperPath);
        return false;
    }

    if(!image.save(targetBMPFilePath, "BMP")){
        m_lastErrorString = tr("Can not set wallpaper! Can not save file '%1'k!").arg(targetBMPFilePath);
        return false;
    }

    //}


    wchar_t pathArray[MAX_PATH * sizeof(wchar_t) + 1];
    wcscpy(pathArray, targetBMPFilePath.toStdWString().c_str());

    bool ok = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, pathArray, SPIF_SENDWININICHANGE| SPIF_UPDATEINIFILE);
    if(!ok){
        DWORD err = GetLastError();
        m_lastErrorString = QString("Can not set wallpaper! %1:%2.").arg(err).arg(WinSysErrorMsg(err));
    }

    return ok;

}












void WindowsManagement::setLocation(Location location){
    this->location = location;
}

//////////////////////////////////////////////////////

bool WindowsManagement::runAs(const QString &userName, const QString &domainName, const QString &password, const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir, bool wait, DWORD milliseconds){
    qDebug()<<"----WindowsManagement::runAs(...)";
    //qDebug()<<"User Name Of CurrentThread:"<<getUserNameOfCurrentThread();

    m_lastErrorString = "";

    if(userName.simplified().isEmpty()){
        m_lastErrorString = tr("Invalid user name!");
        return false;
    }

    //     if(!QFileInfo(exeFilePath).exists()){
    //         error = tr("Can not find file '%1'!").arg(exeFilePath);
    //         return false;
    //     }

    wchar_t name[MaxUserAccountNameLength*sizeof(wchar_t)+1];
    wcscpy(name, userName.toStdWString().c_str());

    wchar_t domain[MaxGroupNameLength*sizeof(wchar_t)+1];
    wcscpy(domain, domainName.toStdWString().c_str());

    wchar_t pwd[MaxUserPasswordLength*sizeof(wchar_t)+1];
    wcscpy(pwd, password.toStdWString().c_str());


    //服务程序以"SYSTEM"身份运行，无法调用CreateProcessWithLogonW，必须用LogonUser和CreateProcessAsUser
    //You cannot call CreateProcessWithLogonW from a process that is running under the LocalSystem account,
    //  because the function uses the logon SID in the caller token, and the token for the LocalSystem account does not contain this SID.
    //  As an alternative, use the CreateProcessAsUser and LogonUser functions.
    if(getUserNameOfCurrentThread().toUpper() == "SYSTEM"){
        //        HANDLE hToken = NULL;
        //        if(!LogonUserW(name, domain, pwd, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)){
        //            m_lastErrorString = tr("Can not log user %1 on to this computer! Error code:%2").arg(userName).arg(GetLastError());
        //            return false;
        //        }

        //        QString cmdStr = QString("\"" + exeFilePath + "\" " + parameters);
        //        wchar_t cmdLine[32000*sizeof(wchar_t)+1];
        //        wcscpy(cmdLine, cmdStr.toStdWString().c_str());

        //        STARTUPINFO si;
        //        PROCESS_INFORMATION pi;
        //        ZeroMemory(&si, sizeof(STARTUPINFO));
        //        si.cb= sizeof(STARTUPINFO);

        //        wchar_t desktop[64];
        //        wcscpy(desktop, "WinSta0\\Default");
        //        si.lpDesktop = desktop;
        //        si.dwFlags = STARTF_USESHOWWINDOW;
        //        if(show){
        //            si.wShowWindow = SW_SHOW;
        //        }else{
        //            si.wShowWindow = SW_HIDE;
        //        }


        //        bool ok = CreateProcessAsUserW(hToken, exeFilePath.toStdWString().c_str(), cmdLine, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL,workingDir.toStdWString().c_str(), &si,&pi);
        //        DWORD dwRet = GetLastError();
        //        CloseHandle(hToken);
        //        if(!ok){
        //            m_lastErrorString = tr("Starting process '%1' failed! Error Code:%2").arg(exeFilePath).arg(dwRet);
        //            qWarning()<<m_lastErrorString;
        //            return false;
        //        }else{
        //            return true;
        //        }

        return runAsForInteractiveService(userName, domainName, password, exeFilePath, parameters, show, workingDir);

    }else{
        return runAsForDesktopApplication(userName, domainName, password, exeFilePath, parameters, show, workingDir, wait);
    }

}

bool WindowsManagement::runAsForInteractiveService(const QString &userName, const QString &domainName, const QString &password, const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir){
    m_lastErrorString = "";

    wchar_t name[MaxUserAccountNameLength*sizeof(wchar_t)+1];
    wcscpy(name, userName.toStdWString().c_str());

    wchar_t domain[512];
    if(domainName.trimmed().isEmpty()){
        wcscpy(domain, L".");
    }else{
        wcscpy(domain, domainName.toStdWString().c_str());
    }

    wchar_t pwd[MaxUserPasswordLength*sizeof(wchar_t)+1];
    wcscpy(pwd, password.toStdWString().c_str());

    QString cmdStr = QString("\"" + exeFilePath + "\" " + parameters);
    wchar_t cmdLine[32000*sizeof(wchar_t)+1];
    wcscpy(cmdLine, cmdStr.toStdWString().c_str());

    wchar_t currentDirectory[MAX_PATH*sizeof(wchar_t)+1];
    if(workingDir.trimmed().isEmpty()){
        wcscpy(currentDirectory, QCoreApplication::applicationDirPath().toStdWString().c_str());
    }else{
        wcscpy(currentDirectory, workingDir.toStdWString().c_str());
    }

    DWORD errorCode;
    if(isNT6OS() && show){
        DWORD sessionID;
        if(!getUserSessionID(name, &sessionID)){
            errorCode = runAsForNT5InteractiveService(name, domain, pwd, NULL, cmdLine, currentDirectory, show);
        }else{
            errorCode = runAsForNT6InteractiveService(sessionID, NULL, cmdLine, currentDirectory, show);
        }
    }else{
        errorCode = runAsForNT5InteractiveService(name, domain, pwd, NULL, cmdLine, currentDirectory, show);
    }

    if(ERROR_SUCCESS != errorCode){
        m_lastErrorString = tr("Failed to start process '%1'! %2:%3.").arg(exeFilePath).arg(errorCode).arg(WinSysErrorMsg(errorCode));
        return false;
    }

    return true;
}

bool WindowsManagement::runAsForDesktopApplication(const QString &userName, const QString &domainName, const QString &password, const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir, bool wait, DWORD milliseconds){
    m_lastErrorString = "";

    wchar_t name[MaxUserAccountNameLength*sizeof(wchar_t)+1];
    wcscpy(name, userName.toStdWString().c_str());

    wchar_t domain[512];
    if(domainName.trimmed().isEmpty()){
        wcscpy(domain, L".");
    }else{
        wcscpy(domain, domainName.toStdWString().c_str());
    }

    wchar_t pwd[MaxUserPasswordLength*sizeof(wchar_t)+1];
    wcscpy(pwd, password.toStdWString().c_str());

    wchar_t cmdLine[8192*sizeof(wchar_t)+1];
    wcscpy(cmdLine, parameters.toStdWString().c_str());
    //wcscpy(cmdLine, QString("\"" + exeFilePath + "\" " + parameters).toStdWString().c_str());

    wchar_t currentDirectory[MAX_PATH*sizeof(wchar_t)+1];
    if(workingDir.trimmed().isEmpty()){
        wcscpy(currentDirectory, QCoreApplication::applicationDirPath().toStdWString().c_str());
    }else{
        wcscpy(currentDirectory, workingDir.toStdWString().c_str());
    }

    DWORD dwRet;
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    //ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb= sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    if(show){
        si.wShowWindow = SW_SHOW;
    }else{
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
//            m_lastErrorString = tr("Can not create environment block! %1: %2").arg(dwRet).arg(WinSysErrorMsg(dwRet));
//            //return false;
//        }
//        if (!GetUserProfileDirectoryW(hToken, szUserProfile, &dwSize)){
//            dwRet = GetLastError();
//            m_lastErrorString = tr("Can not get user profile directory! %1: %2").arg(dwRet).arg(WinSysErrorMsg(dwRet));
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
//        m_lastErrorString = tr("Can not destroy environment block! %1:%2.").arg(dwRet).arg(WinSysErrorMsg(dwRet));
//        qWarning()<<m_lastErrorString;
//    }
//    CloseHandle(hToken);


    if(!ok){
        dwRet = GetLastError();
        m_lastErrorString = tr("Starting process '%1' failed! %2:%3.").arg(exeFilePath).arg(dwRet).arg(WinSysErrorMsg(dwRet));
        qWarning()<<m_lastErrorString;
        return false;
    }

    if(wait){
        dwRet = WaitForSingleObject(pi.hProcess, milliseconds?milliseconds:INFINITE);
        qDebug()<<"dwRet:"<<dwRet;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;

}


////////////////////////////////////////////////////


//void WindowsManagement::setNewComputerNameToBeUsed(const QString &computerName){
//    this->m_newComputerNameToBeUsed = computerName;
//}


void WindowsManagement::test(){


    //    setDeskWallpaper("C:\\WINDOWS\\system32\\wallpaper.bmp");

    //    qWarning()<<"hui:"<<getUserAccountState("hui");
    //    qWarning()<<"yan:"<<getUserAccountState("yan");
    //    qWarning()<<"HelpAssistant:"<<getUserAccountState("HelpAssistant");
    //    qWarning()<<"a:"<<getUserAccountState("a");



    //qWarning()<<20*(QDateTime::currentDateTime().toString("z").toUInt());


    //    qWarning()<<"17 Time"<<currentDateTimeOnServer("\\\\200.200.200.17").toString("yyyy-MM-dd hh:mm:ss");
    //    qWarning()<<"2 Time"<<currentDateTimeOnServer("\\\\200.200.200.2").toString("yyyy-MM-dd hh:mm:ss");
    //    qWarning()<<"setLocalTime:"<<setLocalTime(currentDateTimeOnServer("\\\\200.200.200.2"));

    //    QPair<QDateTime, QDateTime> pair = getUserLastLogonAndLogoffTime("b");
    //    QDateTime lastLogoffTime = pair.second;
    //    qWarning()<<"lastLogonTime"<<pair.first.toString("yyyy-MM-dd hh:mm:ss");
    //    qWarning()<<"lastLogoffTime:"<<pair.second.toString("yyyy-MM-dd hh:mm:ss");


    //qWarning()<<outlookInstalledPath();


    //    qWarning()<<getEnvironmentVariable("ALLUSERSPROFILE");

    //getUserLastLogonAndLogoffTime("");

    //    HANDLE hToken;
    //    getTokenByProcessName(hToken, "explorer.exe");

    //        QList<HANDLE> list = getTokenListByProcessName("qtcreator.exe");
    //        //qWarning()<<"~~~~list.size():"<<list.size();
    //        foreach(HANDLE token, list){
    //            getAccountNameOfProcess(token);
    //            CloseHandle(token);
    //        }



}















#endif



//void WindowsManagement::setLastErrorString(const QString &errorString){
//    this->lastErrorString = errorString;
//}














} //namespace HEHUI
