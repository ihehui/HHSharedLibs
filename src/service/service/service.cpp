/*
 ****************************************************************************
 * service.cpp
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
 * Last Modified on: 2010-05-15
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#include "service.h"
#include "logdebug.h"

#ifdef Q_OS_UNIX
//    #include <stdio.h>
//    #include <string.h>
//    #include <stdlib.h>
//    #include <limits.h>
#endif


Service::Service(int argc, char **argv, const QString &serviceName, const QString &description, bool interactive)
    : QtService<QCoreApplication>(argc, argv, serviceName)
{

    setServiceDescription(description);
    setServiceFlags(QtServiceBase::Default);


    if(interactive){

#ifdef Q_OS_WIN
        enableInteractWithInputDesktop();
#endif

    }



#if defined(SERVICE_DEBUG)
    enableLog(true, serviceName, applicationDirPath() + "/log/" + serviceName);
#else
    for(int i = 0; i < argc; i++){
        if("-log" == QString(argv[i]).toLower()){
            enableLog(true, serviceName);
            break;
        }
    }
#endif


}

Service::~Service()
{


}

void Service::enableLog(bool enable, const QString &fileBaseName, const QString &baseDir)
{

    if(enable){
        bool printTostderr = false;
#ifdef DEBUG
        printTostderr = true;
#endif

        installMessageLogger(baseDir, fileBaseName, printTostderr);

    }else{
        uninstallMessageLogger();
    }

}

bool Service::getCurrentModuleFileName(QString *path)
{
    if(!path){return false;}

#ifdef Q_OS_WIN

    std::vector<WCHAR> buffer;
    DWORD size = MAX_PATH;

    while (true) {
        // Allocate buffer
        buffer.resize(size + 1);
        // Try to get file name
        DWORD ret = GetModuleFileName(NULL, &buffer[0], size);

        if (ret == 0) {
            return false;
        } else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            size += 128;
        } else {
            break;
        }
    }

    *path = QString::fromStdWString(&buffer[0]);

#else

    const int MAX_SIZE = 4096;
    char current_absolute_path[MAX_SIZE];

//    if (NULL == getcwd(current_absolute_path, MAX_SIZE)){
//        return false;
//    }

    int cnt = readlink("/proc/self/exe", current_absolute_path, MAX_SIZE);
    if (cnt < 0 || cnt >= MAX_SIZE)
    {
        return false;
    }

    *path = QString::fromLocal8Bit(current_absolute_path);
#endif


    return true;
}

QString Service::applicationDirPath()
{
    QString path = "";
    bool ok = getCurrentModuleFileName(&path);
    Q_ASSERT(ok);
    if(ok){
        QFileInfo fi(path);
        return fi.absolutePath();
    }

    return "";
}


#ifdef Q_OS_WIN

QString Service::WinSysErrorMsg(DWORD winErrorCode, DWORD dwLanguageId)
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

bool Service::runASCurrentConsoleProcessInActiveConsoleSession(const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir, bool wait, DWORD milliseconds)
{

    if(exeFilePath.trimmed().isEmpty() && parameters.trimmed().isEmpty()){
        qCritical() << QString("Starting process failed! Invalid file path and parameters!");
        return false;
    }

    QString cmdStr = QString("\"" + exeFilePath + "\" " + parameters);
    wchar_t cmdLine[32000 * sizeof(wchar_t) + 1];
    wcscpy(cmdLine, cmdStr.toStdWString().c_str());

    wchar_t currentDirectory[MAX_PATH * sizeof(wchar_t) + 1];
    if(workingDir.trimmed().isEmpty()) {
        wcscpy(currentDirectory, QCoreApplication::applicationDirPath().toStdWString().c_str());
    } else {
        wcscpy(currentDirectory, workingDir.toStdWString().c_str());
    }

    PROCESS_INFORMATION pi = {0};
    STARTUPINFO sti = {0};
    sti.cb = sizeof(STARTUPINFO);
    sti.dwFlags |= STARTF_USESHOWWINDOW;
    if(show) {
        sti.wShowWindow = SW_SHOW;
    } else {
        sti.wShowWindow = SW_HIDE;
    }

//    QString desktopName = "";
//    getDesktopName(getDesktop(""), &desktopName);
//    if(desktopName.toLower() == "winlogon"){
//        sti.lpDesktop = L"WinSta0\\Winlogon";
//        qDebug()<<"-----------winlogon";
//    }

//    wchar_t desktop[MAX_PATH * sizeof(wchar_t) + 1];
//    wcscpy(desktop, desktopName.toStdWString().c_str());
//    sti.lpDesktop = desktop;

    HANDLE token, userToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &token) == 0) {
        DWORD dwRet = GetLastError();
        QString errorString = QString("Starting process '%1' failed! 'OpenProcessToken' failed! %2:%3.").arg(exeFilePath).arg(dwRet).arg(WinSysErrorMsg(dwRet));
        qCritical() << errorString;
        return false;
    }

    if (DuplicateTokenEx(token,
                         MAXIMUM_ALLOWED,
                         0,
                         SecurityImpersonation,
                         TokenPrimary,
                         &userToken) == 0) {
        DWORD dwRet = GetLastError();
        QString errorString = QString("Starting process '%1' failed! 'DuplicateTokenEx' failed! %2:%3.").arg(exeFilePath).arg(dwRet).arg(WinSysErrorMsg(dwRet));
        qCritical() << errorString;
        return false;
    }

    DWORD sessionId = getActiveConsoleSessionId();
    if (SetTokenInformation(userToken,
                            (TOKEN_INFORMATION_CLASS) TokenSessionId,
                            &sessionId,
                            sizeof(sessionId)) == 0) {
        DWORD dwRet = GetLastError();
        QString errorString = QString("Starting process '%1' failed! 'SetTokenInformation' failed! %2:%3.").arg(exeFilePath).arg(dwRet).arg(WinSysErrorMsg(dwRet));
        qCritical() << errorString;
        return false;
    }

    // Fix Windows 8/8.1 UIAccess restrictions (Alt-Tab) for server as service
    // http://stackoverflow.com/questions/13972165/pressing-winx-alt-tab-programatically
    // For application we need to set /uiAccess='true' in linker manifest, sign binary
    // and run from "Program Files/"
    DWORD uiAccess  = 1; // Nonzero enables UI control
    if (SetTokenInformation(userToken,
                            (TOKEN_INFORMATION_CLASS) TokenUIAccess,
                            &uiAccess,
                            sizeof(uiAccess)) == 0) {
        qWarning()<<"Can't set UIAccess=1, ignore it";
    }

    bool handlesIsInherited = false;
    if (CreateProcessAsUserW(userToken, 0, cmdLine,
                             0, 0, handlesIsInherited, NORMAL_PRIORITY_CLASS, 0, 0, &sti, &pi) == 0) {
        DWORD dwRet = GetLastError();
        QString errorString = QString("Starting process '%1' failed! %2:%3.").arg(exeFilePath).arg(dwRet).arg(WinSysErrorMsg(dwRet));
        qCritical() << errorString;
        return false;
    }


    if(wait) {
        DWORD dwRet = WaitForSingleObject(pi.hProcess, milliseconds ? milliseconds : INFINITE);
        qDebug() << "dwRet:" << dwRet;
    }

    CloseHandle(userToken);
    CloseHandle(token);


    qDebug()<<"------------dwProcessId:"<<pi.dwProcessId;

    return true;
}

bool Service::enableInteractWithInputDesktop()
{
    return selectDesktop("");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD Service::getActiveConsoleSessionId()
{
    typedef DWORD (WINAPI *FUNC_WTSGetActiveConsoleSessionId)(void);

    FUNC_WTSGetActiveConsoleSessionId func = (FUNC_WTSGetActiveConsoleSessionId)GetProcAddress( GetModuleHandleW(L"Kernel32.dll"), "WTSGetActiveConsoleSessionId");
    if (0 == func) {
        qCritical("The WTSGetActiveConsoleSessionId function has not been found");
        return 0;
    }

    return func();
}

HDESK Service::getInputDesktop()
{
    return OpenInputDesktop(0, TRUE,
                            DESKTOP_CREATEMENU |
                            DESKTOP_CREATEWINDOW |
                            DESKTOP_ENUMERATE |
                            DESKTOP_HOOKCONTROL |
                            DESKTOP_WRITEOBJECTS |
                            DESKTOP_READOBJECTS |
                            DESKTOP_SWITCHDESKTOP |
                            GENERIC_WRITE);
}

HDESK Service::getDesktop(const QString &name)
{
    return OpenDesktopW(name.toStdWString().c_str(), 0, TRUE,
                        DESKTOP_CREATEMENU |
                        DESKTOP_CREATEWINDOW |
                        DESKTOP_ENUMERATE |
                        DESKTOP_HOOKCONTROL |
                        DESKTOP_WRITEOBJECTS |
                        DESKTOP_READOBJECTS |
                        DESKTOP_SWITCHDESKTOP |
                        GENERIC_WRITE);
}

bool Service::closeDesktop(HDESK hdesk)
{
    return CloseDesktop(hdesk) != 0;
}

bool Service::setDesktopToCurrentThread(HDESK newDesktop)
{
    bool ok = SetThreadDesktop(newDesktop);
    if(!ok){
        DWORD dwRet = GetLastError();
        QString errorString = QString("setDesktopToCurrentThread Failed! %1:%2.").arg(dwRet).arg(WinSysErrorMsg(dwRet));
        qCritical() << errorString;
    }

    return ok;
}

bool Service::selectDesktop(const QString &name)
{
    HDESK desktop;
    if (name.trimmed().isEmpty()) {
        desktop = getInputDesktop();
    } else {
        desktop = getDesktop(name);
    }

    bool result = setDesktopToCurrentThread(desktop) != 0;
    closeDesktop(desktop);

    return result;
}

bool Service::getCurrentDesktopName(QString *desktopName)
{
    HDESK inputDesktop = getInputDesktop();
    bool result = getDesktopName(inputDesktop, desktopName);
    closeDesktop(inputDesktop);
    return result;
}

bool Service::getThreadDesktopName(QString *desktopName)
{
    return getDesktopName(GetThreadDesktop(GetCurrentThreadId()), desktopName);
}

bool Service::getDesktopName(HDESK desktop, QString *desktopName)
{
    *desktopName = "";

    DWORD nameLength = 0;
    // Do not check returned value because the function will return FALSE always.
    GetUserObjectInformationW(desktop, UOI_NAME, 0, 0, &nameLength);

    if (nameLength != 0) {
        std::vector<WCHAR> name(nameLength);
        bool result = !!GetUserObjectInformationW(desktop,
                                                  UOI_NAME,
                                                  &name[0],
                nameLength,
                0);
        if (result) {
            *desktopName = QString::fromStdWString(&name[0]);
            return true;
        }
    }
    return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////



#endif




void Service::start()
{
#if defined(Q_OS_WIN)
    if ((QSysInfo::WindowsVersion & QSysInfo::WV_NT_based) &&
            (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA)) {
        logMessage( "Service GUI not allowed on Windows Vista. See the documentation for this example for more information.", QtServiceBase::Error );
        return;
    }
#endif




}

void Service::stop()
{

}

void Service::pause()
{

}

void Service::resume()
{

}

void Service::processCommand(int code)
{

}


