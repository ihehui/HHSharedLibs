#RESOURCE VERSIONINFO

#RESOURCE FILEFLAGS %VS_FF_SPECIALBUILD
#RESOURCE FILEVERSION 2014, 11, 28, 1
'#RESOURCE PRODUCTVERSION 2012, 10, 29, 1

#RESOURCE STRINGINFO "0804", "04B0"

#RESOURCE VERSION$ "Comments",         "ADSI Wrapper DLL"
#RESOURCE VERSION$ "CompanyName",      "HeHui Inc."
#RESOURCE VERSION$ "FileDescription",  "ADSI Wrapper"
'#RESOURCE VERSION$ "FileVersion",      "2014.10.23.1"
#RESOURCE VERSION$ "InternalName",     "ADSI"
#RESOURCE VERSION$ "LegalCopyright",   "Copyright (C) 2007 - 2014 He Hui"
#RESOURCE VERSION$ "LegalTrademarks",  "Built By HeHui"
#RESOURCE VERSION$ "OriginalFilename", "ADSI.DLL"
'#RESOURCE VERSION$ "PrivateBuild",     "Private info"
#RESOURCE VERSION$ "ProductName",      "ADSI Wrapper"
#RESOURCE VERSION$ "ProductVersion",   "2014.11.28.1"
'#RESOURCE VERSION$ "SpecialBuild",     "Special info"


#COMPILE DLL


''''''''''''''''''----------''''''''''''''''''''''''''
#DIM ALL
%USEMACROS = 1
#INCLUDE ONCE "modules\adsi.inc"
''''''''''''''''''----------''''''''''''''''''''''''''


GLOBAL ghInstance AS DWORD
FUNCTION LIBMAIN (BYVAL hInstance   AS LONG, _
                  BYVAL fwdReason   AS LONG, _
                  BYVAL lpvReserved AS LONG) AS LONG
    SELECT CASE fwdReason
    CASE %DLL_PROCESS_ATTACH : ghInstance = hInstance
                               FUNCTION = 1
    CASE %DLL_PROCESS_DETACH : FUNCTION = 1
    CASE %DLL_THREAD_ATTACH  : FUNCTION = 1
    CASE %DLL_THREAD_DETACH  : FUNCTION = 1
    END SELECT
END FUNCTION


