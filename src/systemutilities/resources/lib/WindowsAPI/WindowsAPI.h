
#ifndef __WINDOWS_API_H__
#define __WINDOWS_API_H__


#include <windows.h>



#ifdef __cplusplus
extern "C"
{
#endif

// Functionality:
//    Get User SessionID.
// Parameters:
//    0) [in] lpszUsername: User Name.
//	  1) [in][out] pSessionID: User SessionID.
// Returned value:
//    true if user SessionID found, or false.
extern bool getUserSessionID(LPCWSTR lpszUserName, DWORD *pSessionID);

// Functionality:
//    Start a interactive process in another user's session. Only for NT6 and later system's service.
//    The user must have been logged on.
// Parameters:
//    0) [in] dwSessionID: Session ID of target desktop to show the process window .
//	  ...
// Returned value:
//    Error Code.
extern int runAsForNT6InteractiveService(DWORD dwSessionID, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPCWSTR lpWorkingDir, bool bShowWindow);


// Functionality:
//    Start a interactive process in another user's session. Only for NT5 system's service.
//    The user must have been logged on.
// Parameters:
//    0) [in] lpszUsername: A pointer to a null-terminated string that specifies the name of the user. This is the name of the user account to log on to .
//    1) [in] lpszDomain: A pointer to a null-terminated string that specifies the name of the domain or server whose account database contains the lpszUsername account .
//    2) [in] lpszPassword: A pointer to a null-terminated string that specifies the plaintext password for the user account specified by lpszUsername .
//    3) [in] lpApplicationName: The name of the module to be executed .
//    4) [in] lpCommandLine: The command line to be executed .
//    5) [in] lpWorkingDir: The full path to the current directory for the process .
//    6) [in] bShowWindow: Hides or show the GUI window .
//	  ...
// Returned value:
//    Error Code.
extern int runAsForNT5InteractiveService(LPWSTR lpszUsername, LPWSTR lpszDomain, LPWSTR lpszPassword, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPCWSTR lpWorkingDir, bool bShowWindow);


// Functionality:
//    Start a interactive process in another user's session. Only for desktop application .
// Parameters:
//    0) [in] lpszUsername: A pointer to a null-terminated string that specifies the name of the user. This is the name of the user account to log on to .
//    1) [in] lpszDomain: A pointer to a null-terminated string that specifies the name of the domain or server whose account database contains the lpszUsername account .
//    2) [in] lpszPassword: A pointer to a null-terminated string that specifies the plaintext password for the user account specified by lpszUsername .
//    3) [in] lpApplicationName: The name of the module to be executed .
//    4) [in] lpCommandLine: The command line to be executed .
//    5) [in] lpWorkingDir: The full path to the current directory for the process .
//    6) [in] bShowWindow: Hides or show the GUI window .
//    7) [in] bWait: If wait for the process to exit .
//    8) [in] dwMilliseconds: Milliseconds to wait for .
//	  ...
// Returned value:
//    Error Code.
extern int runAsForDesktopApplication(LPWSTR lpszUsername, LPWSTR lpszDomain, LPWSTR lpszPassword, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPCWSTR lpWorkingDir, bool bShowWindow = true, bool bWait = false, DWORD dwMilliseconds = 0);




// 取得CPU厂商（Vendor）.
//
// result: 成功时返回字符串的长度（一般为12）。失败时返回0.
// pvendor: 接收厂商信息的字符串缓冲区。至少为13字节.
extern int cpu_getVendor(char *pvendor);

// 取得CPU商标（Brand）.
//
// result: 成功时返回字符串的长度（一般为48）。失败时返回0.
// pbrand: 接收商标信息的字符串缓冲区。至少为49字节.
extern int cpu_getName(char *pbrand);

// 取得CPU序列号（PSN）.
extern void cpu_getPSN(char *pPSN);




//取得物理硬盘型号和序列号
extern BOOL GetPhysicDriveSerialNumber(char *pModelNo, char *pSerialNo, unsigned int driveIndex = 0);




#ifdef __cplusplus
}
#endif

#endif //__WINDOWS_API_H__
