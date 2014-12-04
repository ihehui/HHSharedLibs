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





FUNCTION SetUserCannotChangePassword2(BYREF pwszDomain AS WSTRING, BYREF pwszUser AS WSTRING, BYREF pwszUserCred AS WSTRING, BYREF pwszPassword AS WSTRING, fCannotChangePassword AS DWORD) AS DWORD

 
	 If "" = pwszDomain OR "" = pwszUser Then
		FUNCTION = 0
		EXIT FUNCTION
	 END IF
	 
	 
    
    DIM hr AS LONG
	 hr = %S_OK
	 
    DIM ads AS IADs
	 DIM pvObj AS DWORD
	 LOCAL vObj AS VARIANT

    DIM strADsPath AS WSTRINGZ*1024
    strADsPath = "WinNT://"
    strADsPath += pwszDomain
    strADsPath += "/"
    strADsPath += pwszUser
	 strADsPath += ",user"
	 
	 DIM sAD_UserId AS WSTRINGZ*1024
	 sAD_UserId = pwszUser
	 
	 DIM sAD_Password AS WSTRINGZ*1024
	 sAD_Password = pwszPassword


    ads = AD_ObjGet(strADsPath, $IID_IADs)
	  'hr = ADsOpenObject(BYREF "LDAP://" & pwszDomain & "/" & pwszUser, BYREF sAD_UserId, BYREF sAD_Password, %ADS_SECURE_AUTHENTICATION, $IID_IADs, BYREF pads)

		  
	IF ISNOTHING(ads) THEN EXIT FUNCTION
		  
    'if SUCCEEDED(hr) THEN
   
        'DIM svar AS Variant
		  DIM svar AS DWORD
        svar = VARIANT#(ads.Get("userFlags"))

      
            if fCannotChangePassword THEN
                svar = svar OR %ADS_UF_PASSWD_CANT_CHANGE
            else
                svar = svar AND (svar XOR %ADS_UF_PASSWD_CANT_CHANGE)
            END IF

            'Perform the change.
            ads.Put("userFlags", svar)

            'Commit the change.
            ads.SetInfo()
				
				hr = OBJRESULT
		  
    'ELSE
	'	    #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
	'	    MSGBOX "FUNCTION: " & FUNCNAME$	& $CRLF & STR$(hr) & $CRLF & OBJRESULT$(hr), %MB_ICONERROR, "Error" 	    
    'END IF

    FUNCTION = hr


End FUNCTION



FUNCTION testADSI () AS LONG

'    MSGBOX ComputerName()
 '   MSGBOX UserNameOfCurrentThread()
  '  EXIT FUNCTION


     IF ad_open("hehui", "000...", "200.200.198.198", 0 ) = 0 THEN
     'IF ad_open("dgadmin", "dmsto&*(", "200.200.200.106", 0 ) = 0 THEN
        MSGBOX AD_GetLastErrorString(), %MB_ICONERROR, "Error"
		  exit function
     END IF





local cc as DWORD

local  ok AS long


msgbox "1"
ok = AD_GetUserPasswordChange("d", cc)
msgbox "ok:"+str$(ok) & $CRLF & "cc:"+str$(cc)

msgbox "2"

ok = AD_GetUserCannotChangePassword("d", cc)
msgbox "ok:"+str$(ok) & $CRLF & "cc:"+str$(cc)

ok = AD_SetUserCannotChangePassword("d", 0)
msgbox "ok:"+str$(ok)


ok = AD_GetUserCannotChangePassword("d", cc)
msgbox "ok:"+str$(ok) & $CRLF & "cc:"+str$(cc)


ok = AD_GetUserPasswordChange("d", cc)
msgbox "ok:"+str$(ok) & $CRLF & "cc:"+str$(cc)

exit function



local  change AS long
change = 100
AD_GetUserPasswordChange("c", byref change)
msgbox "change:" + STR$(change)

AD_SetUserPasswordChange("c", 0)

AD_GetUserPasswordChange("c", byref change)
msgbox "change:" + STR$(change)
exit function

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




      'MSGBOX AD_GetAllOUs("")
   'MSGBOX AD_GetObjectsInOU("DC=sitoy,DC=group", "(&(objectcategory=person)(objectclass=user)(cn=" & "hehui" & "*))" , "sAMAccountName,distinguishedName,objectSid", ";", "|")

   'MSGBOX AD_GetObjectsInOU("OU=TestOU1,DC=sitoy,DC=group", "(&(objectcategory=person)(objectclass=user)(cn=" & "test" & "*))" , "sAMAccountName,distinguishedName,objectSid", ";", "|")
   ' MSGBOX AD_GetObjectsInOU("OU=TestOU1,DC=test,DC=local", "(&(objectcategory=person)(objectclass=user)(sAMAccountName=" & "test" & "*)(displayName=Tes*))" , "memberOf", ";", "|")
    'MSGBOX AD_GetObjectsInOU("DC=sitoy,DC=group", "(&(objectcategory=person)(objectclass=user)(sAMAccountName=" & "he" & "*))" , "lastLogon", ";", "|")



       dim filter as wstringz*1024, dataToRetrieve as wstringz*1024
        filter = "(&(objectcategory=person)(objectclass=user)(sAMAccountName=hui*))"
        dataToRetrieve = "sAMAccountName,displayName,userWorkstations,telephoneNumber,description,objectGUID,objectSid"
        MSGBOX "Result:" + $CRLF + AD_GetObjectsInOU("DC=sitoy,DC=group", filter , dataToRetrieve, ";", "|")



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
