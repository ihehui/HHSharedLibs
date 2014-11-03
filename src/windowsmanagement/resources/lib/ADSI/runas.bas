 
 
 ''''''''''''''''''----------''''''''''''''''''''''''''
#DIM ALL
%USEMACROS = 1
#INCLUDE ONCE "Windows.inc"
''''''''''''''''''----------''''''''''''''''''''''''''


   




'  LogonUser() requires that the caller has the following permission
'  Permission                        Display Name
'  --------------------------------------------------------------------
'  SE_TCB_NAME                      Act as part of the operating system

'  CreateProcessAsUser() requires that the caller has the following permissions
'  Permission                        Display Name
'  ---------------------------------------------------------------
'  SE_ASSIGNPRIMARYTOKEN_NAME       Replace a process level token
'  SE_INCREASE_QUOTA_NAME           Increase quotas
  




'********************************************************************

'                   RunAsUser for Windows 2000 and Later
'********************************************************************
Function W2KRunAsUser(ByVal UserName As WString, _
        ByVal Password As WString, _
        ByVal DomainName As WString, _
        ByVal CommandLine As WString, _
        ByVal CurrentDirectory As WString) export As Long
'MsgBox RunAsUser("admin2", "", "", "C:\Windows\system32\notepad.exe", "C:\Windows\system32\")

    Dim si As STARTUPINFO
    Dim pi As PROCESS_INFORMATION
    
    Dim wUser As WStringZ*256
    Dim wDomain As WStringZ*256
    Dim wPassword As WStringZ*256
    Dim wCommandLine As WStringZ*256
    Dim wCurrentDir As WStringZ*256
    
    Dim Result As Long
    
    si.cb = Len(si)
        
    wUser = UserName
    wDomain = DomainName
    wPassword = Password
    wCommandLine = CommandLine
    wCurrentDir = CurrentDirectory
    
    Result = CreateProcessWithLogonW(wUser, wDomain, wPassword, _
          %LOGON_WITH_PROFILE, _
			"", wCommandLine, _
          %CREATE_DEFAULT_ERROR_MODE, 0&, wCurrentDir, si, pi)

    If Result <> 0 Then
        CloseHandle pi.hThread
        CloseHandle pi.hProcess
        FUNCTION = 0
    Else
        FUNCTION = ERR
        MsgBox "CreateProcessWithLogonW() failed with error " & str$(GetLastError())
		  
    End If

End Function




Function RunAsUser(ByVal UserName As WString, _
                ByVal Password As WString, _
                ByVal DomainName As WString, _
                ByVal CommandLine As WString, _
                ByVal CurrentDirectory As WString) As Long

    Dim w2kOrAbove As Byte
    Dim osinfo As OSVERSIONINFO
    Dim Result As Long
    Dim uErrorMode As Long
    
    ' Determine if system is Windows 2000 or later
    osinfo.dwOSVersionInfoSize = Len(osinfo)
    osinfo.szCSDVersion = Space$(128)
    GetVersionExA osinfo
    w2kOrAbove = _
        (osinfo.dwPlatformId = %VER_PLATFORM_WIN32_NT And _
         osinfo.dwMajorVersion >= 5)
    'If (w2kOrAbove) Then
        Result = W2KRunAsUser(UserName, Password, DomainName, _
                    CommandLine, CurrentDirectory)
    
    'End If
    RunAsUser = Result
End Function


FUNCTION PBMAIN () AS LONG
   ' CALL RunAsUser("test", "testtest", "", "notepad.exe", "c:/windows/system32")
	 CALL RunAsUser("hehui", "12345678", "", "notepad.exe", "c:/windows/system32")
END FUNCTION
