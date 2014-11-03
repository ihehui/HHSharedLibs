#RESOURCE VERSIONINFO

#RESOURCE FILEFLAGS %VS_FF_SPECIALBUILD
#RESOURCE FILEVERSION 2012, 11, 2, 1
'#RESOURCE PRODUCTVERSION 2012, 10, 29, 1

#RESOURCE STRINGINFO "0804", "04B0"

#RESOURCE VERSION$ "Comments",         "ADSI Wrapper DLL"
#RESOURCE VERSION$ "CompanyName",      "HeHui Inc."
#RESOURCE VERSION$ "FileDescription",  "ADSI Wrapper"
'#RESOURCE VERSION$ "FileVersion",      "2012.10.29.1"
#RESOURCE VERSION$ "InternalName",     "ADSI"
#RESOURCE VERSION$ "LegalCopyright",   "Copyright (C) 2007 - 2012 He Hui"
#RESOURCE VERSION$ "LegalTrademarks",  "Built By HeHui"
#RESOURCE VERSION$ "OriginalFilename", "ADSI.DLL"
'#RESOURCE VERSION$ "PrivateBuild",     "Private info"
#RESOURCE VERSION$ "ProductName",      "ADSI Wrapper"
#RESOURCE VERSION$ "ProductVersion",   "2012.11.2.1"
'#RESOURCE VERSION$ "SpecialBuild",     "Special info"







   #COMPILE EXE

    '#COMPILE DLL


''''''''''''''''''----------''''''''''''''''''''''''''
#DIM ALL
%USEMACROS = 1
#INCLUDE ONCE "modules\adsi.inc"
''''''''''''''''''----------''''''''''''''''''''''''''


FUNCTION test2() AS WSTRING

    DIM a AS WSTRINGZ*10, b AS WSTRING
    a= "A"
    b = ""

     FUNCTION = a
    'function = $$NUL


END FUNCTION



FUNCTION testADSI () AS LONG

'    MSGBOX ComputerName()
 '   MSGBOX UserNameOfCurrentThread()
  '  EXIT FUNCTION




     IF ad_open("hehui", "0000....~", "200.200.200.118", 0 ) = 0 THEN
     'IF ad_open("dgadmin", "dmsto&*(", "200.200.200.106", 0 ) = 0 THEN
        MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
		  exit function
     END IF


   ' MSGBOX AD_GetObjectAttribute("test", "objectGUID")

    'IF AD_CreateUser("OU=TestOU,OU=DG,DC=test,DC=local", "test1", "TestCN1") = 0 THEN
    '   MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
    'END IF

    'if AD_SetPassword("test4", "123456") = 0 then
    '    MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
   ' end if


   ' IF AD_DeleteObject("Test1", "user")  = 0 THEN
   '     MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
   ' END IF


'  IF AD_MoveObject("OU=DG,DC=test,DC=local", "Test1")  = 0 THEN
 '       MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
 '   END IF


   ' IF AD_RenameObject("test3", "Test")  = 0 THEN
  '      MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
   ' END IF

   'AD_ModifyAttribute("test4", "telephoneNumber", "12345", 0)

   ' IF AD_SetAccountExpire("test3", "2013-04-08")  = 0 THEN
   '     MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
   ' END IF

  '  IF AD_EnableObject("test1", 0)  = 0 THEN
  '      MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
  '  END IF

   ' IF AD_IsObjectDisabled("test3")  = 0 THEN
    '    MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
   ' END IF


   ' AD_SetUserPasswordChange("test1", 0)

DIM ous AS STRING
ous = AD_GetAllOUs("")
'ous = test2()
   MSGBOX "LEN:" + STR$(LEN(ous)) + " sizeof:" + STR$(SIZEOF(ous)) + $CRLF + ous


ous += $NUL
    MSGBOX "LEN:" + STR$(LEN(ous)) + " sizeof:" + STR$(SIZEOF(ous)) + $CRLF + ous




      'MSGBOX AD_GetAllOUs("")
   'MSGBOX AD_GetObjectsInOU("DC=sitoy,DC=group", "(&(objectcategory=person)(objectclass=user)(cn=" & "hehui" & "*))" , "sAMAccountName,distinguishedName,objectSid", ";", "|")

   'MSGBOX AD_GetObjectsInOU("OU=TestOU1,DC=sitoy,DC=group", "(&(objectcategory=person)(objectclass=user)(cn=" & "test" & "*))" , "sAMAccountName,distinguishedName,objectSid", ";", "|")
   ' MSGBOX AD_GetObjectsInOU("OU=TestOU1,DC=test,DC=local", "(&(objectcategory=person)(objectclass=user)(sAMAccountName=" & "test" & "*)(displayName=Tes*))" , "memberOf", ";", "|")
    'MSGBOX AD_GetObjectsInOU("DC=sitoy,DC=group", "(&(objectcategory=person)(objectclass=user)(sAMAccountName=" & "he" & "*))" , "lastLogon", ";", "|")




    'test()

END FUNCTION

''''''''''----------------------------------------------------------

#IF %PB_EXE

FUNCTION PBMAIN () AS LONG
    CALL testADSI()
END FUNCTION

#ELSE
''''''''''----------------------------------------------------------
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

#ENDIF
