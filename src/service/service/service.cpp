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




Service::Service(int argc, char **argv, const QString &serviceName, const QString &description)
    : QtService<QCoreApplication>(argc, argv, serviceName)
{

    setServiceDescription(description);
    setServiceFlags(QtServiceBase::CanBeSuspended);


#if defined(QTSERVICE_DEBUG)
    installMessageLogger();
#else
    for(int i = 0; i < argc; i++) {
        if("-log" == QString(argv[i]).toLower()) {
            installMessageLogger();
        }
    }
#endif


}

Service::~Service()
{


}

//#ifdef Q_OS_WIN

//bool Service::runAs(const QString &userName, const QString &password, const QString &exeFilePath, const QString &parameters, bool show, const QString &workingDir, bool wait){
//    //qDebug()<<"----Service::runAs(...)";
//    //qDebug()<<"User Name Of CurrentThread:"<<getUserNameOfCurrentThread();


//    RunAsThread thread;
//    //thread.run();
//    QString errorMessage;
//    bool ret = thread.runAs(&errorMessage, userName, password, exeFilePath, parameters, show, workingDir);

//    logMessage(errorMessage, QtServiceBase::Information);
//    return ret;


//     if(userName.simplified().isEmpty()){
//         //error = tr("Invalid user name!");
//         return false;
//     }



//     HDESK hdeskCurrent;
//     HDESK hdesk;
//     HWINSTA hwinstaCurrent;
//     HWINSTA hwinsta;

//     hwinstaCurrent = GetProcessWindowStation();
//     if (hwinstaCurrent == NULL){
//     //LogEvent(_T("get window station err"));
//     return FALSE;
//     }

//     hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());
//     if (hdeskCurrent == NULL){
//     //LogEvent(_T("get window desktop err"));
//     return FALSE;
//     }

//     //打开winsta0
//     hwinsta = OpenWindowStationW(L"winsta0", FALSE,
//     WINSTA_ACCESSCLIPBOARD |
//     WINSTA_ACCESSGLOBALATOMS |
//     WINSTA_CREATEDESKTOP |
//     WINSTA_ENUMDESKTOPS |
//     WINSTA_ENUMERATE |
//     WINSTA_EXITWINDOWS |
//     WINSTA_READATTRIBUTES |
//     WINSTA_READSCREEN |
//     WINSTA_WRITEATTRIBUTES);
//     if (hwinsta == NULL){
//     //LogEvent(_T("open window station err"));

//     return FALSE;
//     }

//     if (!SetProcessWindowStation(hwinsta)){
//     //LogEvent(_T("Set window station err"));

//     return FALSE;
//     }

//     //打开desktop
//     hdesk = OpenDesktopW(L"default", 0, FALSE,
//     DESKTOP_CREATEMENU |
//     DESKTOP_CREATEWINDOW |
//     DESKTOP_ENUMERATE |
//     DESKTOP_HOOKCONTROL |
//     DESKTOP_JOURNALPLAYBACK |
//     DESKTOP_JOURNALRECORD |
//     DESKTOP_READOBJECTS |
//     DESKTOP_SWITCHDESKTOP |
//     DESKTOP_WRITEOBJECTS);
//     if (hdesk == NULL){
//     //LogEvent(_T("Open desktop err"));

//     return FALSE;
//     }

//     if(!SetThreadDesktop(hdesk)){
//         logMessage(QString("SetThreadDesktop Failed! %1").arg(GetLastError()), QtServiceBase::Error);
//        return false;
//     }
//     logMessage("SetThreadDesktop OK!", QtServiceBase::Information);







//     wchar_t name[userName.size()*sizeof(wchar_t)+1];
//     wcscpy(name, userName.toStdWString().c_str());

//     wchar_t domain[4];
//     wcscpy(domain, L".");

//     wchar_t pwd[password.size()*sizeof(wchar_t)+1];
//     wcscpy(pwd, password.toStdWString().c_str());


//    //服务程序以"SYSTEM"身份运行，无法调用CreateProcessWithLogonW，必须用LogonUser和CreateProcessAsUser
//    //You cannot call CreateProcessWithLogonW from a process that is running under the LocalSystem account,
//    //  because the function uses the logon SID in the caller token, and the token for the LocalSystem account does not contain this SID.
//    //  As an alternative, use the CreateProcessAsUser and LogonUser functions.
//    //if(getUserNameOfCurrentThread().toUpper() == "SYSTEM"){
//        HANDLE hToken = NULL;
//        if(!LogonUserW(name, domain, pwd, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)){
//            //error = tr("Can not log user %1 on to this computer! Error code:%2").arg(userName).arg(GetLastError());
//            return false;
//        }

//        QString cmdStr = QString("\"" + exeFilePath + "\" " + parameters);
//        wchar_t cmdLine[cmdStr.size()*sizeof(wchar_t)+1];
//        wcscpy(cmdLine, cmdStr.toStdWString().c_str());

//        STARTUPINFOW si;
//        PROCESS_INFORMATION pi;
//        ZeroMemory(&si, sizeof(STARTUPINFOW));
//        si.cb= sizeof(STARTUPINFOW);
//        si.lpDesktop = NULL;
//        //si.lpDesktop = L"winsta0\\default";
//        //si.lpDesktop = L"";
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
//            //error = tr("Starting process '%1' failed! Error Code:%2").arg(exeFilePath).arg(dwRet);
//            return false;
//        }else{
//            return true;
//        }

//    //}


//    return true;



//}


//int Service::changeToDefaultDesktop(){

////    HDESK               hdesk;
////    HWINSTA             hwinsta;

////    //
////    // obtain a handle to the interactive windowstation
////    //
////    hwinsta = OpenWindowStationW(
////         L"winsta0",
////         FALSE,
////         READ_CONTROL | WRITE_DAC
////         );
////    if (hwinsta == NULL)
////         return RTN_ERROR;

////    //HWINSTA hwinstaold = GetProcessWindowStation();
////    hwinstaold = GetProcessWindowStation();


////    //
////    // set the windowstation to winsta0 so that you obtain the
////    // correct default desktop
////    //
////    if (!SetProcessWindowStation(hwinsta))
////         return RTN_ERROR;

////    //
////    // obtain a handle to the "default" desktop
////    //
////    hdesk = OpenDesktopW(
////         L"default",
////         0,
////         FALSE,
////         READ_CONTROL | WRITE_DAC |
////         DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS
////         );
////    if (hdesk == NULL)
////         return RTN_ERROR;

////    if(!SetThreadDesktop(hdesk)){
////        return RTN_ERROR;

////    }

////    //
////    // close the handles to the interactive windowstation and desktop
////    //
////    CloseWindowStation(hwinsta);

////    CloseDesktop(hdesk);

//////    SetProcessWindowStation(hwinstaold); //set it back






//    WCHAR pvInfo[128] = {0};
////    WCHAR tmp[1024] = {0};

////    if(szName != NULL)
////    lstrcpy(pvInfo, szName);
////    else
////    {

//    HDESK hActiveDesktop;
//    DWORD dwLen;
//    hActiveDesktop = OpenInputDesktop(DF_ALLOWOTHERACCOUNTHOOK, FALSE, MAXIMUM_ALLOWED);
//    if(!hActiveDesktop)//打开失败
//    {
//    return 0;
//    }
//    //获取指定桌面对象的信息，一般情况和屏保状态为default，登陆界面为winlogon
//    GetUserObjectInformation(hActiveDesktop, UOI_NAME, pvInfo, sizeof(pvInfo), &dwLen);
//    if(dwLen==0)//获取失败
//    {
//    return 0;
//    }
//    CloseDesktop(hActiveDesktop);
//    //打开winsta0
//    HWINSTA m_hwinsta = OpenWindowStationW(L"winsta0", FALSE,
//    WINSTA_ACCESSCLIPBOARD   |
//    WINSTA_ACCESSGLOBALATOMS |
//    WINSTA_CREATEDESKTOP     |
//    WINSTA_ENUMDESKTOPS      |
//    WINSTA_ENUMERATE         |
//    WINSTA_EXITWINDOWS       |
//    WINSTA_READATTRIBUTES    |
//    WINSTA_READSCREEN        |
//    WINSTA_WRITEATTRIBUTES);
//    if (m_hwinsta == NULL){
//    return 0;
//    }

//    if (!SetProcessWindowStation(m_hwinsta)){
//    return 0;
//    }

//    //打开desktop
//    //HDESK m_hdesk = OpenDesktop(pvInfo, 0, FALSE,
//    HDESK m_hdesk = OpenDesktopW(L"default", 0, FALSE,
//    DESKTOP_CREATEMENU |
//    DESKTOP_CREATEWINDOW |
//    DESKTOP_ENUMERATE    |
//    DESKTOP_HOOKCONTROL |
//    DESKTOP_JOURNALPLAYBACK |
//    DESKTOP_JOURNALRECORD |
//    DESKTOP_READOBJECTS |
//    DESKTOP_SWITCHDESKTOP |
//    DESKTOP_WRITEOBJECTS);
//    if (m_hdesk == NULL){
//    return 0;
//    }

//    SetThreadDesktop(m_hdesk);
//    return 1;
//    //}





//}

//void Service::resetDesktop(){

//    SetProcessWindowStation(hwinstaold); //set it back

//}

//#endif




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


