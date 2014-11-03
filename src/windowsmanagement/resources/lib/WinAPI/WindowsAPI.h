// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 WINDOWSAPI_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// WINDOWSAPI_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef WINDOWSAPI_EXPORTS
#define WINDOWSAPI_API __declspec(dllexport)
#else
#define WINDOWSAPI_API __declspec(dllimport)
#endif




#define WINSTA_ALL (WINSTA_ACCESSCLIPBOARD  | WINSTA_ACCESSGLOBALATOMS | \
	WINSTA_CREATEDESKTOP    | WINSTA_ENUMDESKTOPS      | \
	WINSTA_ENUMERATE        | WINSTA_EXITWINDOWS       | \
	WINSTA_READATTRIBUTES   | WINSTA_READSCREEN        | \
	WINSTA_WRITEATTRIBUTES  | DELETE                   | \
	READ_CONTROL            | WRITE_DAC                | \
	WRITE_OWNER)

#define DESKTOP_ALL (DESKTOP_CREATEMENU      | DESKTOP_CREATEWINDOW  | \
	DESKTOP_ENUMERATE       | DESKTOP_HOOKCONTROL   | \
	DESKTOP_JOURNALPLAYBACK | DESKTOP_JOURNALRECORD | \
	DESKTOP_READOBJECTS     | DESKTOP_SWITCHDESKTOP | \
	DESKTOP_WRITEOBJECTS    | DELETE                | \
	READ_CONTROL            | WRITE_DAC             | \
	WRITE_OWNER)

#define GENERIC_ACCESS (GENERIC_READ    | GENERIC_WRITE | \
	GENERIC_EXECUTE | GENERIC_ALL)


//// 此类是从 WindowsAPI.dll 导出的
//class WINDOWSAPI_API CWindowsAPI {
//public:
//	CWindowsAPI(void);
//	// TODO: 在此添加您的方法。
//};
//
//extern WINDOWSAPI_API int nWindowsAPI;

//WINDOWSAPI_API int fnWindowsAPI(void);



#ifdef __cplusplus
extern "C"
{
#endif

	// Functionality:
	//    Start a interactive process in another user's session. Only for NT5.1 and later system's service.
	//    The user must have been logged on.
	// Parameters:
	//    0) [in] dwSessionID: Session ID of target desktop to show the process window .
	//	  ...
	// Returned value:
	//    Error Code.
	WINDOWSAPI_API int runAsForNT6InteractiveService(DWORD dwSessionID, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPCWSTR lpWorkingDir, bool bShowWindow);


	// Functionality:
	//    Get User SessionID.
	// Parameters:
	//    0) [in] lpszUsername: User Name.
	//	  1) [in][out] pSessionID: User SessionID.
	// Returned value:
	//    true if user SessionID found, or false.
	WINDOWSAPI_API bool getUserSessionID(LPCWSTR lpszUserName, DWORD *pSessionID);


	WINDOWSAPI_API int runAsForNT5InteractiveService(LPWSTR lpszUsername, LPWSTR lpszDomain, LPWSTR lpszPassword, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPCWSTR lpWorkingDir, bool bShowWindow);


	WINDOWSAPI_API BOOL ObtainSid(
		HANDLE hToken,           // Handle to an process access token.
		PSID   *psid             // ptr to the buffer of the logon sid
		);

	WINDOWSAPI_API void RemoveSid(
		PSID *psid               // ptr to the buffer of the logon sid
		);

	WINDOWSAPI_API BOOL AddTheAceWindowStation(
		HWINSTA hwinsta,         // handle to a windowstation
		PSID    psid             // logon sid of the process
		);

	WINDOWSAPI_API  BOOL AddTheAceDesktop(
		HDESK hdesk,             // handle to a desktop
		PSID  psid               // logon sid of the process
		);




#ifdef __cplusplus
}
#endif