
#INCLUDE THIS ONCE

#INCLUDE ONCE "activeds.inc"

#INCLUDE ONCE "ADO.INC"

'#INCLUDE ONCE "sddl.inc"
#INCLUDE ONCE "afxsid.inc"
#INCLUDE ONCE "CAfxTime.inc"

' Functions to manipulate local accounts using ADs
'-------------------------------------------------------------------
'#INCLUDE Once "AdsHlp.inc" ' library by Jos¨¦ Roca


'ADO
GLOBAL oAD_Connection AS ADOConnection
GLOBAL oAD_Command AS ADOCommand
'global as ADORecordset
GLOBAL bAD_BindFlags AS DWORD ' Bind flags


GLOBAL sAD_UserId AS WSTRINGZ * 256
GLOBAL sAD_Password AS WSTRINGZ * 256
GLOBAL sAD_HostServer AS WSTRINGZ * 256

GLOBAL sAD_LastErrorString AS WSTRINGZ * 1024
GLOBAL lAD_LastErrorCode AS LONG

GLOBAL oAD_RootDSE AS IADs
GLOBAL sAD_DNSDomain AS WSTRINGZ * 256
GLOBAL oAD_RootContainer AS IADsContainer

'Global oADOpenDSObject As IADsOpenDSObject
 




' #FUNCTION# ====================================================================================================================
' Name...........: AD_Open
' Description ...: Opens a connection to Active Directory.
' Syntax.........: AD_Open([sAD_UserIdParam = "", sAD_PasswordParam = "", sAD_HostServerParam = "", $iAD_Security = 0)
' Parameters ....: sAD_UserIdParam - Optional: UserId credential for authentication. This has to be a valid domain user
'                  sAD_PasswordParam - Optional: Password for authentication
'                  sAD_HostServerParam - Optional: Name of Domain Controller if you want to connect to a different domain e.g. DC-Server1.microsoft.com
'                  |If you want to connect to a Global Catalog append port 3268 e.g. DC-Server1.microsoft.com:3268
'                  iAD_Security - Optional: Specifies the security settings to be used. Can be a combination of the following:
'                  |1: %ADS_SECURE_AUTHENTICATION (default)
'                  |2: Sets the connection property "Encrypt Password" to True to encrypt userid and password
'                  |2: The channel is encrypted using Secure Sockets Layer (SSL). AD requires that the Certificate Server be installed to support SSL
' Return values .: Success - 1
'                  Failure - 0, SetError:
'                  |1 - Creation of the COM object to the AD failed.
'                  |2 - Parameter $sAD_PasswordParam is required when $sAD_UserIdParam is specified.
'                  |3 - Creation of the oAD_RootDSE object failed.
'                  |4 - Creation of the oAD_RootContainer object failed.
'                  |x - For Windows Vista and later: Win32 error code (decimal). To get detailed error information call function _AD_GetLastADSIError
' Remarks .......: To close the connection to the Active Directory, use the AD_Close function.
'+
'                  AD_Open will use the alternative credentials $sAD_UserIdParam and $sAD_PasswordParam if passed as parameters.
'                  $sAD_UserIdParam has to be in one of the following forms (assume the samAccountName = DJ)
'                  * Windows Login Name   e.g. "DJ"
'                  * NetBIOS Login Name   e.g. "<DOMAIN>\DJ"
'                  * User Principal Name  e.g. "DJ@domain.com"
'                  All other name formats have NOT been successfully tested (see section "Link").
'+
'                  Connection to an alternate domain (not the domain your computer is a member of) or if your computer is not a domain member
'                  requires $sAD_DNSDomainParam, $sAD_HostServerParam and $sAD_ConfigurationParam as FQDN as well as $sAD_UserIdParam and $sAD_PasswordParam.
'                  Example:
'                  $sAD_DNSDomainParam = "DC=subdomain,DC=example,DC=com"
'                  $sAD_HostServerParam = "servername.subdomain.example.com"
'                  $sAD_ConfigurationParam = "CN=Configuration,DC=subdomain,DC=example,DC=com"
'+
'                  The COM error handler will be initialized only if there doesn't already exist another error handler.
'+
'                  If you specify $sAD_UserIdParam as NetBIOS Login Name or User Principal Name and the OS is Windows Vista or later then _AD_Open will try to
'                  verify the userid/password.
'                  @error will be set to the Win32 error code (decimal). To get detailed error information please call _AD_GetlastADSIError.
'                  For all other OS or if userid is specified as Windows Login Name @error=8.
'                  This is OS dependant because Windows XP doesn't return useful error information.
'                  For Windows Login Name all OS return success even when an error occures. This seems to be caused by secure authentification.
'+
'                  $iAD_Security = 2 activates LDAP/SSL. LDAP/SSL uses port 636 by default.
'                  Note that an SSL server certificate must be configured properly in order to use SSL.
'+
'                  If you want to connect to a specific DC in the current domain the just provide $sAD_HostServerParam and let $sAD_DNSDomainParam and $sAD_ConfigurationParam be blank.
' Related .......: AD_Close
' Link ..........: http://msdn.microsoft.com/en-us/library/cc223499(PROT.10).aspx (Simple Authentication), http://msdn.microsoft.com/en-us/library/aa746471(VS.85).aspx (ADO)
' Example .......:
' ===============================================================================================================================
'FUNCTION AD_Open ALIAS "AD_Open" (BYVAL sAD_UserIdParam AS WSTRING, BYVAL sAD_PasswordParam AS WSTRING, BYVAL sAD_HostServerParam AS WSTRING, BYVAL iAD_SecurityParam AS DWORD) EXPORT AS LONG
FUNCTION AD_Open ALIAS "AD_Open" (BYREF sAD_UserIdParam AS WSTRINGz, BYREF sAD_PasswordParam AS WSTRINGz, BYREF sAD_HostServerParam AS WSTRINGz, BYVAL iAD_SecurityParam AS DWORD) EXPORT AS LONG

    sAD_UserId = sAD_UserIdParam
    sAD_Password = sAD_PasswordParam
    sAD_HostServer = sAD_HostServerParam

    DIM iAD_Security AS DWORD
    iAD_Security = iAD_SecurityParam

	'msgbox "sAD_UserId " & sAD_UserId & $CRLF & "sAD_Password " & sAD_Password & $CRLF & "sAD_HostServer " & sAD_HostServer 

'''''''''''''''''''''''''''''''''''''

    ' // Create a Connection object
    oAD_Connection = NEWCOM "ADODB.Connection"
    IF ISNOTHING(oAD_Connection) THEN
        SetError(1, "Creation of the COM object to the AD failed")
        FUNCTION = 0
        EXIT FUNCTION
    END IF

    ' ConnectionString Property (ADO): http://msdn.microsoft.com/en-us/library/ms675810.aspx
    DIM adoConnectionString AS WSTRING
    adoConnectionString = "Provider=ADsDSOObject"
    'oAD_Connection.ConnectionString = "Provider=ADsDSOObject" ' Sets Service providertype
    IF sAD_UserId <> "" THEN
        IF sAD_Password = "" THEN
			  AD_Close()
			  
           SetError(2, "Parameter sAD _PasswordParam is required when sAD _UserIdParam is specified")								
           FUNCTION = 0
           EXIT FUNCTION
        END IF


        'oAD_Connection.Properties("User ID") = sAD_UserId 'Authenticate User
        'oAD_Connection.Properties("Password") = sAD_Password ' Authenticate User
        'oAD_Connection.Properties("Encrypt Password") = 1 ' Encrypts userid and password
        adoConnectionString = "Provider=ADsDSOObject;User ID=" & sAD_UserId & ";Password=" & sAD_Password

        bAD_BindFlags = %ADS_SERVER_BIND
        IF (iAD_Security AND 2) = 2 THEN bAD_BindFlags = (bAD_BindFlags OR %ADS_USE_SSL)

        'If userid is the Windows login name then set the flag for secure authentification
        IF VERIFY("\", sAD_UserId) <> 0 AND VERIFY("@", sAD_UserId) <> 0 THEN bAD_BindFlags = (bAD_BindFlags OR %ADS_SECURE_AUTHENTICATION)

        'oAD_Connection.Properties("ADSI Flag") = bAD_BindFlags
        adoConnectionString = adoConnectionString & ";ADSI Flag=" & STR$(bAD_BindFlags)

    END IF

    oAD_Connection.ConnectionString = adoConnectionString
    ' ADO Open Method: http://msdn.microsoft.com/en-us/library/ms676505.aspx
    oAD_Connection.Open() ' Open connection to AD


    ' ADO Command object as global
    ' Create an ADO command object
    oAD_Command = NEWCOM "ADODB.Command"
    IF ISFALSE ISOBJECT(oAD_Command) THEN
		  AD_Close()
		  
        SetError(1, "Creation of the COM object to the AD failed")		  
        FUNCTION = 0
        EXIT FUNCTION
    END IF
    oAD_Command.ActiveConnection = oAD_Connection
    'oAD_Command.Properties("Page Size") = 1000

''''''''''''''''''''''''''''''''''''''''

    DIM hr AS LONG

''''''''''''''''''''''''''''''''''

    IF sAD_UserId = "" THEN
        hr = ADsOpenObject(BYREF "LDAP://" & sAD_HostServer & "/RootDSE", BYVAL %Null, BYVAL %Null, %ADS_SECURE_AUTHENTICATION, $IID_IADs, BYREF oAD_RootDSE)
    ELSE
        hr = ADsOpenObject(BYREF "LDAP://" & sAD_HostServer & "/RootDSE", BYREF sAD_UserId, BYREF sAD_Password, %ADS_SECURE_AUTHENTICATION, $IID_IADs, BYREF oAD_RootDSE)
    END IF

    IF FAILED(hr) THEN
		  AD_Close()
		  
        SetError(3, OBJRESULT$(hr))
        #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
		  'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error:	" & FUNCNAME$		  				 
		  FUNCTION = %FALSE
        EXIT FUNCTION
    END IF

    sAD_DNSDomain = VARIANT$$(oAD_RootDSE.Get("defaultNamingContext"))   ' Retrieve the current AD domain name

''''''''''''''''''''''''''''''''''


''''''''''''''''''''''''''''''''''

    IF sAD_UserId = "" THEN
        hr = ADsOpenObject(BYREF "LDAP://" & sAD_HostServer, BYVAL %Null, BYVAL %Null, %ADS_SECURE_AUTHENTICATION, $IID_IADsContainer, BYREF oAD_RootContainer)
    ELSE
        hr = ADsOpenObject(BYREF "LDAP://" & sAD_HostServer, BYREF sAD_UserId, BYREF sAD_Password, %ADS_SECURE_AUTHENTICATION, $IID_IADsContainer, BYREF oAD_RootContainer)
    END IF

    IF FAILED(hr) THEN
		  AD_Close()
		  
        SetError(4, OBJRESULT$(hr))
        #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
		  'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error:	" & FUNCNAME$
        FUNCTION = 0
        EXIT FUNCTION
    END IF


''''''''''''''''''''''''''''''''''

'''''''''''''''''''''''''''''''''''''
'   hr = ADsGetObject(ByRef "LDAP:", $IID_IADsOpenDSObject, ByRef oADOpenDSObject)
'   If FAILED(hr) Then
'      #Debug Print "Error! " & OBJRESULT$(hr) & " Code:" & Str$(hr) & " Function:" & FuncName$
'      MsgBox OBJRESULT$(hr) & $CrLf & "Code:" & Str$(hr), %MB_ICONERROR, "Error:IID_IADsOpenDSObject"
'      'MsgBox FORMAT$(hr)

'      Function = 0
'      Exit Function
'    End If

'''''''''''''''''''''''''''''''''''''





    'FUNCTION = 1
	 FUNCTION = %TRUE

END FUNCTION


FUNCTION AD_Close ALIAS "AD_Close" () EXPORT AS LONG

    IF ISOBJECT(oAD_Connection) THEN
	    oAD_Connection.Close() ' Close Connection
	 END IF
    oAD_Connection = NOTHING
    oAD_Command = NOTHING

    sAD_UserId = ""
    sAD_Password = ""
    sAD_HostServer = ""

    sAD_LastErrorString = ""
    lAD_LastErrorCode = 0

    oAD_RootDSE = NOTHING
    sAD_DNSDomain = ""
    oAD_RootContainer = NOTHING
    'oADOpenDSObject = Nothing

END FUNCTION


FUNCTION AD_GetLastErrorCode ALIAS "AD_GetLastErrorCode" () EXPORT AS LONG
   FUNCTION = lAD_LastErrorCode
END FUNCTION

FUNCTION AD_GetLastErrorString ALIAS "AD_GetLastErrorString" () EXPORT AS WSTRING
   FUNCTION = sAD_LastErrorString + $$NUL
END FUNCTION

'FUNCTION SetError(BYVAL errorCode AS LONG, BYVAL errorString AS WSTRING) AS LONG
FUNCTION SetError(BYVAL errorCode AS LONG, BYREF errorString AS WSTRINGz) AS LONG
   lAD_LastErrorCode = errorCode
   sAD_LastErrorString = errorString
END FUNCTION


FUNCTION AD_LastError() AS LONG
        DIM lError AS DWORD
        DIM sErrorBuf AS WSTRINGZ * 256
        DIM dwErrorBufLen AS DWORD
        dwErrorBufLen = 256

        DIM sNameBuf AS WSTRINGZ * 256
        DIM dwNameBufLen AS DWORD
        dwNameBufLen = 256

        DIM hr AS LONG
        hr = ADsGetLastError(BYREF lError, BYREF sErrorBuf, dwErrorBufLen, BYREF sNameBuf, dwNameBufLen)
        IF FAILED(hr) THEN
				#DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code: 0x" & HEX$(hr, 8) & " Function:" & FUNCNAME$
				'MSGBOX OBJRESULT$(hr) & $CRLF & "Code: 0x" & HEX$(hr, 8) & $CRLF & " Function:" & FUNCNAME$, %MB_ICONERROR, "ADSI Error"

            'sAD_LastErrorString = OBJRESULT$(hr)
            'lAD_LastErrorCode = hr
            SetError(hr, OBJRESULT$(hr))
            FUNCTION = hr
            EXIT FUNCTION
        END IF


        'sAD_LastErrorString = OBJRESULT$(lError)
        'lAD_LastErrorCode = lError
        SetError(lError, OBJRESULT$(lError))
        #DEBUG PRINT sAD_LastErrorString & " Code:" & STR$(lError)
        'MsgBox sAD_LastErrorString & $CrLf & "Code:" & Str$(lError), %MB_ICONERROR, "Error:" & FuncName$

        FUNCTION = lError

END FUNCTION


function AD_DefaultNamingContext alias "AD_DefaultNamingContext" () export as wstring
	function = sAD_DNSDomain + $$NUL
END FUNCTION


' #INTERNAL_USE_ONLY#============================================================================================================
' Name...........: AD_ObjGet
' Description ...: Returns an LDAP object from a FQDN.
'                  Will use the alternative credentials $sAD_UserId/$sAD_Password if they are set.
' Syntax.........: AD_ObjGet(sAD_FQDN, riidParam)
' Parameters ....: sAD_FQDN - Fully Qualified Domain name of the object for which the LDAP object will be returned.
'                  riidParam - OPTIONAL: object GUID
' Return values .: LDAP object
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......: This function is used internally
' Related .......:
' Link ..........: http://msdn.microsoft.com/en-us/library/aa772247(VS.85).aspx (ADS_AUTHENTICATION_ENUM Enumeration)
' Example .......:
' ===============================================================================================================================
'FUNCTION AD_ObjGet(BYVAL sAD_FQDN AS WSTRING, OPTIONAL BYVAL riidParam AS GUID) AS IDISPATCH
FUNCTION AD_ObjGet(BYREF sAD_FQDN AS WSTRINGz, OPTIONAL BYVAL riidParam AS GUID) AS IDISPATCH



   LOCAL obj AS IDISPATCH
   LOCAL adsPatch AS WSTRINGZ * 256, riid as GUID
   adsPatch = sAD_FQDN
	riid = riidParam
	IF riid = GUID$("{00000000-0000-0000-0000-000000000000}") THEN
      riid = $IID_IADs
      'MSGBOX "riid: " & GUIDTXT$(riid)
   END IF

'    If sAD_UserId = "" Then
'        Return ADsGetObject(sAD_FQDN, $IID_IADs, ByRef obj)
'    Else
'        Return oAD_OpenDS.OpenDSObject($sAD_FQDN, $sAD_UserId, $sAD_Password, $__bAD_BindFlags)
'    End If

     DIM hr AS LONG
    IF sAD_UserId = "" THEN
        hr = ADsOpenObject(BYREF adsPatch, BYVAL %Null, BYVAL %Null, %ADS_SECURE_AUTHENTICATION, BYREF riid, BYREF obj)
    ELSE
        hr = ADsOpenObject(BYREF adsPatch, BYREF sAD_UserId, BYREF sAD_Password, %ADS_SECURE_AUTHENTICATION, BYREF riid, BYREF obj)
    END IF

    IF FAILED(hr) THEN
        SetError(3, OBJRESULT$(hr))
        #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code: 0x" & HEX$(hr, 8) & " Function:" & FUNCNAME$
        'MSGBOX OBJRESULT$(hr) & $CRLF & "Code: 0x" & HEX$(hr, 8) & $CRLF & " Function:" & FUNCNAME$, %MB_ICONERROR, "ADSI Error"
        'MsgBox FORMAT$(hr)

        'Function = %Null
        'Exit Function
    END IF

    FUNCTION = obj

END FUNCTION   '==>AD_ObjGet



' #FUNCTION# ====================================================================================================================
' Name...........: AD_ObjectExists
' Description ...: Returns 1 if exactly one object exists for the given property in the local Active Directory Tree.
' Syntax.........: AD_ObjectExists([$sAD_Object = @UserName[, $sAD_Property = ""]])
' Parameters ....: sAD_ObjectParam   - Object (user, computer, group, OU) to check 
'                  sAD_PropertyParam - Property to check. If omitted the function tries to determine whether to use sAMAccountname or FQDN
' Return values .: Success - 1, Exactly one object exists for the given property in the local Active Directory Tree
'                  Failure - 0, sets @error to:
'                  |-1 - Parameter is required
'                  |1 - No object found for the specified property
'                  |x - More than one object found for the specified property. x is the number of objects found
' Author ........:
' Modified.......:
' Remarks .......: Checking on a computer account requires a "$" (dollar) appended to the sAMAccountName.
'                  To check the existence of an OU use the FQDN of the OU as first parameter because an OU has no SamAccountName.
' Related .......:
' Link ..........:
' Example .......:
' ===============================================================================================================================
FUNCTION AD_ObjectExists alias "AD_ObjectExists" (BYVAL sAD_ObjectParam AS WSTRING, BYVAL sAD_PropertyParam AS WSTRING) export AS LONG
'FUNCTION AD_ObjectExists alias "AD_ObjectExists" (BYREF sAD_ObjectParam AS WSTRINGZ, BYREF sAD_PropertyParam AS WSTRINGZ) export AS LONG

    SetError(0, "")

     LOCAL sAD_Object AS WSTRINGZ * 512, sAD_Property AS WSTRINGZ * 512
     sAD_Object = sAD_ObjectParam
     sAD_Property = sAD_PropertyParam

    IF sAD_Object = "" THEN
        SetError( - 1, "Parameter is required")
        FUNCTION = 0
        EXIT FUNCTION
    END IF

    IF sAD_Property = "" THEN
        sAD_Property = "samAccountName"
        IF MID$(sAD_Object, 3, 1) = "=" THEN sAD_Property = "distinguishedName"
    END IF

   oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_DNSDomain & ">;(" & sAD_Property & "=" & sAD_Object & ");ADsPath;subtree"

    LOCAL oAD_RecordSet AS ADORecordset
    oAD_RecordSet = oAD_Command.Execute ' Retrieve the ADsPath for the object, if it exists
    'msgbox AdoGetErrorInfo(oAD_Connection, OBJRESULT)


    IF ISOBJECT(oAD_RecordSet) THEN
        IF oAD_RecordSet.RecordCount = 1 THEN
            'MSGBOX "RecordSet.RecordCount = 1"
            FUNCTION = 1
            EXIT FUNCTION
        ELSEIF oAD_RecordSet.RecordCount > 1 THEN
            'MSGBOX "RecordSet.RecordCount > 1"
            SetError(oAD_RecordSet.RecordCount, "More than one object found for the specified property.")
            FUNCTION = 0
            EXIT FUNCTION
        ELSE
            'MSGBOX "RecordSet.RecordCount < 1"
            SetError(1, "No object found for the specified property.")
            FUNCTION = 0
            EXIT FUNCTION
        END IF
    ELSE
        'MSGBOX "oAD _RecordSet not object"
        SetError(1, "No object found for the specified property.")
        FUNCTION = 0
        EXIT FUNCTION
    END IF

    FUNCTION = 0

END FUNCTION   '==>AD_ObjectExists



' #FUNCTION# ====================================================================================================================
' Name...........: AD_RenameObject
' Description ...: Renames an object within an OU.
' Syntax.........: AD_RenameObject($sAD_Object, $sAD_CN)
' Parameters ....: sAD_ObjectParam - Object (user, group, computer) to rename (FQDN or sAMAccountName)
'                  sAD_CNParam - New Name (relative name) of the object in the current OU without CN=
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by MoveHere function (Missing permission etc.)
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......: Renames an object within the same OU. You can not move objects to another OU with this function.
' Related .......: _AD_MoveObject, _AD_DeleteObject
' Link ..........: http://msdn.microsoft.com/en-us/library/aa705991(v=VS.85).aspx
' Example .......: Yes
' ===============================================================================================================================
'Function AD_RenameObject alias "AD_RenameObject" (byval sAD_ObjectParam as wstring, byval sAD_CNParam as wstring) export as long
Function AD_RenameObject alias "AD_RenameObject" (BYREF sAD_ObjectParam as wstringz, BYREF sAD_CNParam as wstringz) export as long

   SetError(0, "")

   LOCAL sAD_Object AS WSTRINGZ * 512, sAD_CN as WSTRINGZ * 512
   sAD_Object = sAD_ObjectParam
	sAD_CN = sAD_CNParam
	
   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(1, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF


   IF MID$(sAD_Object, 3, 1) <> "=" THEN sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
   LOCAL oAD_Object AS IADs, oAD_OU AS IADsContainer
   oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADs)
   oAD_OU = AD_ObjGet(oAD_Object.Parent, $IID_IADsContainer) ' Get the object of the OU/CN where the object resides
   sAD_CN = "CN=" & sAD_CN
   oAD_OU.MoveHere("LDAP://" & sAD_HostServer & "/" & sAD_Object, sAD_CN)
	
	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

End Function   '==>AD_RenameObject


' #FUNCTION# ====================================================================================================================
' Name...........: AD_MoveObject
' Description ...: Moves an object to another OU.
' Syntax.........: AD_MoveObject($sAD_OU, $sAD_Object[, $sAD_DisplayName = ""])
' Parameters ....: sAD_OUParam - Target OU for the object move (FQDN)
'                  sAD_ObjectParam - Object (user, group, computer) to move (FQDN or sAMAccountName)
'                  sAD_CNParam - Optional: New Name of the object in the target OU. Common Name or RDN (Relative Distinguished Name) like "Lastname Firstname" without leading "CN="
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_OU does not exist
'                  |2 - $sAD_Object does not exist
'                  |3 - Object already exists in the target OU
'                  |x - Error returned by MoveHere function (Missing permission etc.)
' Author ........: water
' Modified.......:
' Remarks .......: You must escape commas in $sAD_Object with a backslash. E.g. "CN=Lastname\, Firstname,OU=..."
' Related .......: _AD_RenameObject, _AD_DeleteObject
' Link ..........: http://msdn.microsoft.com/en-us/library/aa705991(v=VS.85).aspx
' Example .......: Yes
' ===============================================================================================================================
'Function AD_MoveObject alias "AD_MoveObject" (byval sAD_OUParam as wstring, byval sAD_ObjectParam as wstring, opt byval sAD_CNParam as wstring) export as long
Function AD_MoveObject alias "AD_MoveObject" (BYREF sAD_OUParam as wstringz, BYREF sAD_ObjectParam as wstringz, opt BYREF sAD_CNParam as wstringz) export as long

   SetError(0, "")

   LOCAL sAD_OU AS WSTRINGZ * 512, sAD_Object AS WSTRINGZ * 512, sAD_CN as WSTRINGZ * 512
	sAD_OU = sAD_OUParam
   sAD_Object = sAD_ObjectParam
	sAD_CN = sAD_CNParam

	IF ISFALSE AD_ObjectExists(sAD_OU, "distinguishedName") THEN
      SetError(1, "OU '" & sAD_OU & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error-" & FUNCNAME$
      FUNCTION = 0
      EXIT FUNCTION
   END IF

   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(2, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error-" & FUNCNAME$
      FUNCTION = 0
      EXIT FUNCTION
   END IF

	If Mid$(sAD_Object, 3, 1) <> "=" Then sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
	If sAD_CN = "" Then
		sAD_CN = "CN=" & AD_GetObjectAttribute(sAD_Object, "cn")
	'Else
	'	sAD_CN = "CN=" & AD_FixSpecialChars(sAD_CN) ' escape all special characters
	End If
	
	If AD_ObjectExists(sAD_CN & "," & sAD_OU, "distinguishedName") Then 
		SetError(3, "Object '" & sAD_Object & "' already exists in the target OU!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error-" & FUNCNAME$
      FUNCTION = 0
      EXIT FUNCTION
	end if

	Local oAD_OU as IADsContainer
	oAD_OU = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_OU, $IID_IADsContainer) ' Pointer to the destination container
	oAD_OU.MoveHere("LDAP://" & sAD_HostServer & "/" & sAD_Object, sAD_CN)
	
 	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

End Function   '==>AD_MoveObject


' #FUNCTION# ====================================================================================================================
' Name...........: AD_DeleteObject
' Description ...: Deletes the specified object.
' Syntax.........: AD_DeleteObject($sAD_Object, $sAD_Class)
' Parameters ....: sAD_ObjectParam - Object (user, group, computer etc.) to delete (FQDN or sAMAccountName)
'                  sAD_ClassParam - The schema class object to delete ("user", "computer", "group", "contact" etc). Can be derived using _AD_GetObjectClass().
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by Delete function (Missing permission etc.)
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......:
' Related .......: _AD_RenameObject, _AD_MoveObject
' Link ..........: http://msdn.microsoft.com/en-us/library/aa705988(v=VS.85).aspx
' Example .......: Yes
' ===============================================================================================================================
'FUNCTION AD_DeleteObject ALIAS "AD_DeleteObject" (BYVAL sAD_ObjectParam AS WSTRING, BYVAL sAD_ClassParam AS WSTRING) EXPORT AS LONG
FUNCTION AD_DeleteObject ALIAS "AD_DeleteObject" (BYREF sAD_ObjectParam AS WSTRINGz, BYREF sAD_ClassParam AS WSTRINGz) EXPORT AS LONG

   SetError(0, "")

   LOCAL sAD_Object AS WSTRINGZ * 512, sAD_Class AS WSTRINGZ * 256
   sAD_Object = sAD_ObjectParam
	sAD_Class = sAD_ClassParam

   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(1, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF


   IF MID$(sAD_Object, 3, 1) <> "=" THEN sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
   LOCAL oAD_Object AS IADs, oAD_OU AS IADsContainer, sAD_CN AS WSTRING
   oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADs)
   oAD_OU = AD_ObjGet(oAD_Object.Parent, $IID_IADsContainer) ' Get the object of the OU/CN where the object resides
   sAD_CN = "CN=" & AD_GetObjectAttribute(sAD_Object, "cn")
   oAD_OU.Delete(sAD_Class, sAD_CN)

	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

END FUNCTION   '==>AD_DeleteObject


' #FUNCTION# ====================================================================================================================
' Name...........: AD_UnlockObject
' Description ...: Unlocks an AD object (user account, computer account).
' Syntax.........: AD_UnlockObject($sAD_Object)
' Parameters ....: sAD_ObjectParam - User account or computer account to unlock (sAMAccountName or FQDN)
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: water
' Modified.......:
' Remarks .......:
' Related .......: AD_IsObjectLocked, AD_GetObjectsLocked
' Link ..........: http://www.rlmueller.net/Programs/IsUserLocked.txt
' Example .......: Yes
' ===============================================================================================================================
'Function AD_UnlockObject alias "AD_UnlockObject" (byval sAD_ObjectParam as wstring) export as long
Function AD_UnlockObject alias "AD_UnlockObject" (BYREF sAD_ObjectParam as wstringz) export as long

   SetError(0, "")

   LOCAL sAD_Object AS WSTRINGZ * 512
   sAD_Object = sAD_ObjectParam

   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(1, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF
	
	If Mid$(sAD_Object, 3, 1) <> "=" Then sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
	local oAD_Object as IADsUser, iAD_UAC as long
	oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADsUser)
	oAD_Object.IsAccountLocked = %False
	oAD_Object.SetInfo
	
	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

End Function   '==>_AD_UnlockObject


' #FUNCTION# ====================================================================================================================
' Name...........: AD_EnableObject
' Description ...: Enable or disables an AD object (user account, computer account).
' Syntax.........: AD_DisableObject(sAD_Object)
' Parameters ....: sAD_Object - User account or computer account to enable or disable (sAMAccountName or FQDN)
'                  bEnableObject - enable or disable
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: water
' Modified.......:
' Remarks .......:
' Related .......: _AD_IsObjectDisabled, _AD_EnableObject, _AD_GetObjectsDisabled
' Link ..........: http://www.wisesoft.co.uk/scripts/vbscript_enable-disable_user_account_1.aspx
' Example .......: Yes
' ===============================================================================================================================
'Function AD_EnableObject alias "AD_EnableObject" (byval sAD_ObjectParam as wstring, byval bEnableObject as long) export as long
Function AD_EnableObject alias "AD_EnableObject" (BYREF sAD_ObjectParam as wstringz, byval bEnableObject as long) export as long

   SetError(0, "")

   LOCAL sAD_Object AS WSTRINGZ * 512
   sAD_Object = sAD_ObjectParam

   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(1, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF
	
	If Mid$(sAD_Object, 3, 1) <> "=" Then sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
	local oAD_Object as IADsUser, iAD_UAC as long
	oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADsUser)
	iAD_UAC = VARIANT#( oAD_Object.Get("userAccountControl") )
	if bEnableObject THEN
		'iAD_UAC = iAD_UAC AND (NOT %ADS_UF_DONT_EXPIRE_PASSWD)
		oAD_Object.Put("userAccountControl", (iAD_UAC AND (NOT %ADS_UF_ACCOUNTDISABLE)) )
	else
		oAD_Object.Put("userAccountControl", (iAD_UAC or %ADS_UF_ACCOUNTDISABLE) )
	END IF 
	oAD_Object.SetInfo
	
	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

End Function   '==>_AD_DisableObject


' #FUNCTION# ====================================================================================================================
' Name...........: AD_IsObjectDisabled
' Description ...: Returns 1 if the object (user account, computer account) is disabled.
' Syntax.........: AD_IsObjectDisabled([$sAD_Object = @Username])
' Parameters ....: sAD_ObjectParam - Object to check (default = @Username). Can be specified as Fully Qualified Domain Name (FQDN) or sAMAccountName
' Return values .: Success - 1, Specified object is disabled
'                  Failure - 0, sAD_Object is not disabled
'                  Error - -1, sAD_Object could not be found
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......: A $ sign must be appended to the computer name to create a correct sAMAccountName e.g. @ComputerName & "$"
' Related .......: _AD_DisableObject, _AD_EnableObject, _AD_GetObjectsDisabled
' Link ..........:
' Example .......: Yes
' ===============================================================================================================================
'Function AD_IsObjectDisabled alias "AD_IsObjectDisabled" (byval sAD_ObjectParam as wstring)  export as long
Function AD_IsObjectDisabled alias "AD_IsObjectDisabled" (BYREF sAD_ObjectParam as wstringz)  export as long

   SetError(0, "")

   LOCAL sAD_Object AS WSTRINGZ * 512
   sAD_Object = sAD_ObjectParam

   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(1, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = -1
      EXIT FUNCTION
   END IF
	
	If Mid$(sAD_Object, 3, 1) <> "=" Then sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
	local oAD_Object as IADsUser, iAD_UAC as long
	oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADsUser)
	iAD_UAC = VARIANT#( oAD_Object.Get("userAccountControl") )


	If (iAD_UAC and %ADS_UF_ACCOUNTDISABLE) = %ADS_UF_ACCOUNTDISABLE Then 
		FUNCTION = 1
	else
		SetError(0, "Object '" & sAD_Object & "' is not disabled!")
		FUNCTION = 0	
	end if
	
End Function   '==>AD_IsObjectDisabled



' #FUNCTION# ====================================================================================================================
' Name...........: AD_SetAccountExpire
' Description ...: Modifies the specified user or computer account expiration date or sets the account to never expire.
' Syntax.........: AD_SetAccountExpire($sAD_Object, $sAD_DateTime)
' Parameters ....: sAD_ObjectParam - User or computer account to set expiration date/time (sAMAccountName or FQDN)
'                  sAD_DateTimeParam - Expiration date in format: yyyy-mm-dd (local time) or "01/01/1970" to never expire
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: KenE
' Modified.......: water
' Remarks .......: Use the following syntax for the date/time:
'                  01/01/1970 = never expire
'                  yyyy-mm-dd hh:mm:ss= "international format" - always works
'                  xx/xx/xx <time> = "localized format" - the format depends on the local date/time settings
' Related .......:
' Link ..........: http://www.rlmueller.net/AccountExpires.htm
' Example .......: Yes
' ===============================================================================================================================
'Function AD_SetAccountExpire alias "AD_SetAccountExpire" (byval sAD_ObjectParam as wstring, byval sAD_DateTimeParam as wstring) export as long
Function AD_SetAccountExpire alias "AD_SetAccountExpire" (BYREF sAD_ObjectParam as wstringz, BYREF sAD_DateTimeParam as wstringz) export as long

   SetError(0, "")

   LOCAL sAD_Object AS WSTRINGZ * 512, sAD_DateTime as WSTRINGZ * 512
   sAD_Object = sAD_ObjectParam
	sAD_DateTime = sAD_DateTimeParam

   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(1, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF
	
	local afxTime as IAfxTime
	afxTime = NewAfxTime()
	afxTime.FromString(sAD_DateTime)
	
	If Mid$(sAD_Object, 3, 1) <> "=" Then sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
	local oAD_Object as IADsUser
	oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADsUser)
	'oAD_Object.Put("accountExpires", afxTime.ToVarDate())
	oAD_Object.AccountExpirationDate = afxTime.ToVariantDate() 
	oAD_Object.SetInfo
	
	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

End Function   '==>AD_SetAccountExpire


' #FUNCTION# ====================================================================================================================
' Name...........: AD_SetPasswordExpire
' Description ...: Modifies specified users password to expire or not.
' Syntax.........: AD_SetPasswordExpire(sAD_ObjectParam, bEnableExpire)
' Parameters ....: sAD_ObjectParam - User account to enable or disable password expiration (sAMAccountName or FQDN)
'						 bEnableExpire - enable or disable password expiration
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: KenE
' Modified.......: water
' Remarks .......:
' Related .......: _AD_IsPasswordExpired, _AD_GetPasswordExpired, _AD_GetPasswordDontExpire, _AD_SetPassword, _AD_EnablePasswordChange,  _AD_DisablePasswordChange, _AD_GetPasswordInfo, _AD_EnablePasswordExpire
' Link ..........:
' Example .......: Yes
' ===============================================================================================================================
'Function AD_SetPasswordExpire alias "AD_SetPasswordExpire" (byval sAD_ObjectParam as wstring, byval bEnableExpire as long) export as long
Function AD_SetPasswordExpire alias "AD_SetPasswordExpire" (BYREF sAD_ObjectParam as wstringz, byval bEnableExpire as long) export as long

   SetError(0, "")

   LOCAL sAD_Object AS WSTRINGZ * 512
   sAD_Object = sAD_ObjectParam

   IF ISFALSE AD_ObjectExists(sAD_Object, "") THEN
      SetError(1, "Object '" & sAD_Object & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF
	
	If Mid$(sAD_Object, 3, 1) <> "=" Then sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
	local oAD_Object as IADsUser, iAD_UAC as long
	oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADsUser)
	iAD_UAC = VARIANT#( oAD_Object.Get("userAccountControl") )
	if bEnableExpire THEN
		'iAD_UAC = iAD_UAC AND (NOT %ADS_UF_DONT_EXPIRE_PASSWD)
		oAD_Object.Put("userAccountControl", (iAD_UAC AND (NOT %ADS_UF_DONT_EXPIRE_PASSWD)) )
	else
		oAD_Object.Put("userAccountControl", (iAD_UAC or %ADS_UF_DONT_EXPIRE_PASSWD) )
	END IF 
	oAD_Object.SetInfo
	
	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

End Function   '==>AD_SetPasswordExpire


'---------------------------------------------------------------------------------------------------------------------------
' #FUNCTION# ====================================================================================================================
' Name...........: AD_SetUserCannotChangePassword
' Description ...: Enable or disables the 'User Cannot Change Password' option
' Syntax.........: AD_SetUserCannotChangePassword(sAD_UserSAMAccount, bCannotChangePassword)
' Parameters ....: sAD_UserSAMAccount - User account to enable or disable changing the password (sAMAccountName)
'                  bCannotChangePassword - If allowing the user to change their password, 0:user can not change the password 
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: He Hui
' Modified.......: He Hui
' Remarks .......:
' Related .......: _AD_IsPasswordExpired, _AD_GetPasswordExpired, _AD_GetPasswordDontExpire, _AD_SetPassword, _AD_DisablePasswordExpire, _AD_EnablePasswordExpire, AD_GetUserCannotChangePassword, _AD_GetPasswordInfo
' Link ..........: Example VBS see: http://msdn.microsoft.com/en-us/library/aa746399.aspx or How to use Visual Basic and ADsSecurity.dll to properly order ACEs in an ACL.
' Example .......: Yes
' ===============================================================================================================================
FUNCTION AD_SetUserCannotChangePassword alias "AD_SetUserCannotChangePassword" (BYREF sAD_UserSAMAccountParam AS WSTRINGZ, byval bCannotChangePassword AS DWORD) export as long
	    
		 
	 LOCAL sAD_UserSAMAccount AS WSTRINGZ * 128
    sAD_UserSAMAccount = sAD_UserSAMAccountParam	 
		 
		 
	 IF ISFALSE AD_ObjectExists(sAD_UserSAMAccount, "") THEN
      SetError(1, "Object '" & sAD_UserSAMAccount & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
    END IF	 
		 		 
	 DIM strADsPath AS WSTRINGZ*1024
    strADsPath = "WinNT://"
    strADsPath += sAD_HostServer
    strADsPath += "/"
    strADsPath += sAD_UserSAMAccount
	 strADsPath += ",user"
	 
	 DIM hr AS LONG
	 DIM oADsUser AS IADsUser
	 
	 oADsUser = AD_ObjGet(strADsPath, $IID_IADsUser)
	 
	' hr = ADsOpenObject(BYREF strADsPath, BYREF sAD_UserId, BYREF sAD_Password, %ADS_SECURE_AUTHENTICATION, $IID_IADsUser, BYREF ads)
   ' IF FAILED(hr) THEN	  
   '     SetError(3, OBJRESULT$(hr))
   '     #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code: 0x" & HEX$(hr, 8) & " Function:" & FUNCNAME$
   '     'MSGBOX OBJRESULT$(hr) & $CRLF & "Code: 0x" & HEX$(hr, 8) & $CRLF & " Function:" & FUNCNAME$, %MB_ICONERROR, "ADSI Error"	  				 
	'	  FUNCTION = 0
   '     EXIT FUNCTION
   ' END IF
	 
	 DIM dFlags AS DWORD
    dFlags = VARIANT#(oADsUser.Get("userFlags"))
    if bCannotChangePassword THEN
		dFlags = dFlags OR %ADS_UF_PASSWD_CANT_CHANGE
    else
      dFlags = dFlags AND (dFlags XOR %ADS_UF_PASSWD_CANT_CHANGE)
    END IF

    'Perform the change.
    oADsUser.Put("userFlags", dFlags)

    'Commit the change.
    oADsUser.SetInfo()
	 
	IF OBJRESULT <> %S_OK THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

END FUNCTION


'---------------------------------------------------------------------------------------------------------------------------
' #FUNCTION# ====================================================================================================================
' Name...........: AD_GetUserCannotChangePassword
' Description ...: Enable or disables the 'User Cannot Change Password' option
' Syntax.........: AD_GetUserCannotChangePassword(sAD_UserSAMAccount, bCannotChangePassword)
' Parameters ....: _IN_  sAD_UserSAMAccount - User account to enable or disable changing the password (sAMAccountName)
'                  _OUT_ bCannotChangePassword - If allowing the user to change their password, 0:user can not change the password 
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: He Hui
' Modified.......: He Hui
' Remarks .......:
' Related .......: _AD_IsPasswordExpired, _AD_GetPasswordExpired, _AD_GetPasswordDontExpire, _AD_SetPassword, _AD_DisablePasswordExpire, AD_SetUserCannotChangePassword, _AD_GetPasswordInfo
' Link ..........: Example VBS see: http://msdn.microsoft.com/en-us/library/aa746449(v=vs.85).aspx or How to use Visual Basic and ADsSecurity.dll to properly order ACEs in an ACL.
' Example .......: Yes
' ===============================================================================================================================
FUNCTION AD_GetUserCannotChangePassword alias "AD_GetUserCannotChangePassword" (BYREF sAD_UserSAMAccount AS WSTRINGZ, BYREF bCannotChangePassword AS DWORD) export as long
	    
	 IF ISFALSE AD_ObjectExists(sAD_UserSAMAccount, "") THEN
      SetError(1, "Object '" & sAD_UserSAMAccount & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
    END IF	 
		 		 
	 DIM strADsPath AS WSTRINGZ*1024
    strADsPath = "WinNT://"
    strADsPath += sAD_HostServer
    strADsPath += "/"
    strADsPath += sAD_UserSAMAccount
	 strADsPath += ",user"
	 
	 DIM hr AS LONG
	 DIM oADsUser AS IADsUser
	 
	 oADsUser = AD_ObjGet(strADsPath, $IID_IADsUser)
 	 
	 If (VARIANT#(oADsUser.Get("userFlags")) And %ADS_UF_PASSWD_CANT_CHANGE) <> 0 Then
        bCannotChangePassword = 1
    Else
        bCannotChangePassword = 0
    End If
	 
	IF OBJRESULT <> %S_OK THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

END FUNCTION


'!!!!!!!!!!!!!!!!!!!!!!INVALID!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
'---------------------------------------------------------------------------------------------------------------------------
' #FUNCTION# ====================================================================================================================
' Name...........: AD_SetUserPasswordChange
' Description ...: Enable or disables the 'User Cannot Change Password' option
' Syntax.........: AD_SetUserPasswordChange(sAD_Object, bCanChangePassword)
' Parameters ....: sAD_ObjectParam - User account to enable or disable changing the password (sAMAccountName or FQDN)
'                  bCanChangePassword - If allowing the user to change their password, 0:user can not change the password 
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: KenE
' Modified.......: water
' Remarks .......:
' Related .......: _AD_IsPasswordExpired, _AD_GetPasswordExpired, _AD_GetPasswordDontExpire, _AD_SetPassword, _AD_DisablePasswordExpire, _AD_EnablePasswordExpire, _AD_DisablePasswordChange, _AD_GetPasswordInfo
' Link ..........: Example VBS see: http://gallery.technet.microsoft.com/ScriptCenter/en-us/ced14c6c-d16a-4cd8-b7d1-90d716c0445f or How to use Visual Basic and ADsSecurity.dll to properly order ACEs in an ACL.
' Example .......: Yes
' ===============================================================================================================================
'Function AD_SetUserPasswordChange alias "AD_SetUserPasswordChange" (byval sAD_ObjectParam as wstring, byval bCanChangePassword as long) export as long
Function AD_SetUserPasswordChange alias "AD_SetUserPasswordChange" (BYREF sAD_ObjectParam as wstringz, byval bCanChangePassword as long) export as long

	local USER_CHANGE_PASSWORD as wstring
	USER_CHANGE_PASSWORD = "{ab721a53-1e2f-11d0-9819-00aa0040529b}" ' See: http://msdn.microsoft.com/en-us/library/cc223637(PROT.13).aspx
	local ADS_ACETYPE_ACCESS_ALLOWED_OBJECT, ADS_ACETYPE_ACCESS_DENIED_OBJECT, oldACETYPE, newACETYPE as integer
	ADS_ACETYPE_ACCESS_ALLOWED_OBJECT = &H5
	ADS_ACETYPE_ACCESS_DENIED_OBJECT = &H6
	
	if bCanChangePassword THEN
		oldACETYPE = ADS_ACETYPE_ACCESS_DENIED_OBJECT
		newACETYPE = ADS_ACETYPE_ACCESS_ALLOWED_OBJECT
	else
		oldACETYPE = ADS_ACETYPE_ACCESS_ALLOWED_OBJECT
		newACETYPE = ADS_ACETYPE_ACCESS_DENIED_OBJECT
	END IF

   SetError(0, "")

   DIM sAD_Object AS WSTRINGZ * 256
   sAD_Object = sAD_ObjectParam

   If isfalse AD_ObjectExists(sAD_Object, "") Then
      SetError(1, "Object does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   end if

   If Mid$(sAD_Object, 3, 1) <> "=" Then
       sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
   end if

   local sAD_Self, sAD_Everyone as wstring
   sAD_Self = "NT AUTHORITY\SELF"
   sAD_Everyone = "EVERYONE"


   LOCAL oAD_Object AS IADsUser
   oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADsUser)
   local oAD_SD as  IADsSecurityDescriptor, tempVariant as Variant
   tempVariant = oAD_Object.Get("nTSecurityDescriptor")
   oAD_SD = tempVariant
   Local oAD_DACL as IADsAccessControlList 'IADsAccessControlList
   oAD_DACL = oAD_SD.DiscretionaryAcl
   ' Search for ACE's for Change Password and modify

   local oAD_ACE as  IADsAccessControlEntry
   'local aceCount as long
   'aceCount = oAD_DACL.AceCount
	
	local bAD_Self, bAD_Everyone, bAD_Modified AS INTEGER
	bAD_Self = bAD_Everyone = bAD_Modified = 0


   local  pDisp as IDispatch, pUnk as IUnknown, pEnum as IEnumVARIANT, iid AS GUID, hr as long, varValue as variant, nFetch as long
   pUnk = oAD_DACL.NewEnum_
   iid = $IID_IEnumVARIANT
   hr = pUnk.QueryInterface(byref iid, byref pEnum )
   IF FAILED(hr) THEN
      SetError(2, OBJRESULT$(hr))
      #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
      'MSGBOX OBJRESULT$(hr) & $CRLF & "Code: &H" & HEX$(hr, 8), %MB_ICONERROR, "ADSI Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF

   hr = pEnum.Next(1, varValue, nFetch)
   while (hr = %S_OK)
       if nFetch = 1 THEN
           if variantvt(varValue) <> %VT_DISPATCH THEN
               exit loop
           END IF

           pDisp = varValue
			  local iid2 as GUID
			  iid2 = $IID_IADsAccessControlEntry
         hr = pDisp.QueryInterface(byref iid2, byref oAD_ACE)
			
           if SUCCEEDED(hr) THEN
               If UCASE$(oAD_ACE.objectType) = UCASE$(USER_CHANGE_PASSWORD) Then
                   If UCASE$(oAD_ACE.Trustee) = sAD_Self Then
                       If oAD_ACE.AceType = oldACETYPE Then				
                           oAD_ACE.AceType = newACETYPE					
									'oAD_DACL.RemoveAce(oAD_ACE)
                       End If
                   End If
                   If UCASE$(oAD_ACE.Trustee) = sAD_Everyone Then
                       If oAD_ACE.AceType = oldACETYPE Then
                           oAD_ACE.AceType = newACETYPE
									'oAD_DACL.RemoveAce(oAD_ACE)
                       End If
                   End If
               End If

           END IF
           VariantClear(varValue)
			  			  
			      If UCASE$(oAD_ACE.objectType) = UCASE$(USER_CHANGE_PASSWORD) Then
                   If UCASE$(oAD_ACE.Trustee) = sAD_Self Then	
							End If
               End If			  
			  
       END IF

       hr = pEnum.Next(1, varValue, nFetch)
   WEND


	'oAD_SD.DiscretionaryAcl = oAD_DACL
	oAD_Object.Put("ntSecurityDescriptor", oAD_SD)
   oAD_Object.SetInfo

	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF


End Function   '==>AD_SetUserPasswordChange


'---------------------------------------------------------------------------------------------------------------------------
' #FUNCTION# ====================================================================================================================
' Name...........: AD_GetUserPasswordChange
' Description ...: Get the 'User Cannot Change Password' option enabled or disabled
' Syntax.........: AD_GetUserPasswordChange(sAD_Object, bCanChangePassword)
' Parameters ....: sAD_ObjectParam - User account to enable or disable changing the password (sAMAccountName or FQDN)
'                  bCanChangePassword - If allowing the user to change their password, user can not change the password:0
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_Object does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: KenE
' Modified.......: water
' Remarks .......:
' Related .......: _AD_IsPasswordExpired, _AD_GetPasswordExpired, _AD_GetPasswordDontExpire, _AD_SetPassword, _AD_DisablePasswordExpire, _AD_EnablePasswordExpire, _AD_DisablePasswordChange, _AD_GetPasswordInfo
' Link ..........: Example VBS see: http://gallery.technet.microsoft.com/ScriptCenter/en-us/ced14c6c-d16a-4cd8-b7d1-90d716c0445f or How to use Visual Basic and ADsSecurity.dll to properly order ACEs in an ACL.
' Example .......: Yes
' ===============================================================================================================================
'Function AD_GetUserPasswordChange alias "AD_SetUserPasswordChange" (byval sAD_ObjectParam as wstring, BYREF bCanChangePassword as long) export as long
Function AD_GetUserPasswordChange alias "AD_GetUserPasswordChange" (BYREF sAD_ObjectParam as wstringz, BYREF bCanChangePassword as long) export as long

	local USER_CHANGE_PASSWORD as wstring
	USER_CHANGE_PASSWORD = "{ab721a53-1e2f-11d0-9819-00aa0040529b}" ' See: http://msdn.microsoft.com/en-us/library/cc223637(PROT.13).aspx
	local ADS_ACETYPE_ACCESS_ALLOWED_OBJECT, ADS_ACETYPE_ACCESS_DENIED_OBJECT, oldACETYPE, newACETYPE as integer
	ADS_ACETYPE_ACCESS_ALLOWED_OBJECT = &H5
	'ADS_ACETYPE_ACCESS_DENIED_OBJECT = &H6
	

   SetError(0, "")

   DIM sAD_Object AS WSTRINGZ * 256
   sAD_Object = sAD_ObjectParam

   If isfalse AD_ObjectExists(sAD_Object, "") Then
      SetError(1, "Object does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   end if

   If Mid$(sAD_Object, 3, 1) <> "=" Then
       sAD_Object = AD_SamAccountNameToFQDN(sAD_Object) ' sAMAccountName provided
   end if

   local sAD_Self, sAD_Everyone as wstring
   'sAD_Self = "NT AUTHORITY\SELF"
   sAD_Everyone = "EVERYONE"


   LOCAL oAD_Object AS IADsUser
   oAD_Object = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_Object, $IID_IADsUser)
   local oAD_SD as  IADsSecurityDescriptor, tempVariant as Variant
   tempVariant = oAD_Object.Get("nTSecurityDescriptor")
   oAD_SD = tempVariant
   Local oAD_DACL as IADsAccessControlList 'IADsAccessControlList
   oAD_DACL = oAD_SD.DiscretionaryAcl
   ' Search for ACE's for Change Password and modify

   local oAD_ACE as  IADsAccessControlEntry
   local  pDisp as IDispatch, pUnk as IUnknown, pEnum as IEnumVARIANT, iid AS GUID, hr as long, varValue as variant, nFetch as long
   pUnk = oAD_DACL.NewEnum_
   iid = $IID_IEnumVARIANT
   hr = pUnk.QueryInterface(byref iid, byref pEnum )
   IF FAILED(hr) THEN
      SetError(2, OBJRESULT$(hr))
      #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
      'MSGBOX OBJRESULT$(hr) & $CRLF & "Code: &H" & HEX$(hr, 8), %MB_ICONERROR, "ADSI Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF

   hr = pEnum.Next(1, varValue, nFetch)

   while (hr = %S_OK)
       if nFetch = 1 THEN
           if variantvt(varValue) <> %VT_DISPATCH THEN
               exit loop
           END IF

           pDisp = varValue
			  local iid2 as GUID
			  iid2 = $IID_IADsAccessControlEntry
           hr = pDisp.QueryInterface(byref iid2, byref oAD_ACE)
			
           if SUCCEEDED(hr) THEN
               If UCASE$(oAD_ACE.objectType) = UCASE$(USER_CHANGE_PASSWORD) Then
                   'If UCASE$(oAD_ACE.Trustee) = sAD_Self Then							 
                    '   If oAD_ACE.AceType = ADS_ACETYPE_ACCESS_ALLOWED_OBJECT Then
							'	  bCanChangePassword = 1
							'	elseif oAD_ACE.AceType = ADS_ACETYPE_ACCESS_DENIED_OBJECT THEN
							'	  bCanChangePassword = 0	  
                     '  End If
                   'End If
						 
                   If UCASE$(oAD_ACE.Trustee) = sAD_Everyone Then
                       If oAD_ACE.AceType = ADS_ACETYPE_ACCESS_ALLOWED_OBJECT Then
								  bCanChangePassword = 1
								else
								  bCanChangePassword = 0	  
                       End If

							  VariantClear(varValue)
							  EXIT LOOP
                   End If
						 
               End If

           END IF
           VariantClear(varValue)

       END IF

       hr = pEnum.Next(1, varValue, nFetch)
   WEND


	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF


End Function   '==>AD_GetUserPasswordChange



' #FUNCTION# ====================================================================================================================
' Name...........: AD_GetObjectAttribute
' Description ...: Returns the specified attribute for the named object.
' Syntax.........: _AD_GetObjectAttribute(sAD_ObjectParam, sAD_AttributeParam)
' Parameters ....: sAD_ObjectParam - sAMAccountName or FQDN of the object the attribute should be retrieved from
'                  sAD_AttributeParam - Attribute to be retrieved
' Return values .: Success - Value for the given attribute
'                  Failure - "", sets @error to:
'                  |1 - sAD_ObjectParam does not exist
'                  |2 - $sAD_Attribute does not exist for sAD_ObjectParam. @Extended is set to the error returned by AD
'                  |3 - Function Call Failed
'                  |4 - No values for the specified property
'                  |5 - Failed to create PowerArray object
'                  |6 - Failed to convert SID to string sid

' Author ........:
' Modified.......:
' Remarks .......: If the attribute is a single-value the function returns a string otherwise it returns an array.
'                  To get decrypted attributes (GUID, SID, dates etc.) please see _AD_GetObjectProperties.
' Related .......: AD_ModifyAttribute, AD_GetObjectProperties
' Link ..........:
' Example .......:
' ===============================================================================================================================
'FUNCTION AD_GetObjectAttribute ALIAS "AD_GetObjectAttribute" (BYVAL sAD_ObjectParam AS WSTRING, BYVAL sAD_AttributeParam AS WSTRING) EXPORT AS WSTRING
FUNCTION AD_GetObjectAttribute ALIAS "AD_GetObjectAttribute" (BYREF sAD_ObjectParam AS WSTRINGz, BYREF sAD_AttributeParam AS WSTRINGz) EXPORT AS WSTRING

    SetError(0, "")

    DIM sAD_Object AS WSTRINGZ * 512
    sAD_Object = sAD_ObjectParam
    DIM sAD_Attribute AS WSTRINGZ * 256
    sAD_Attribute = sAD_AttributeParam


   LOCAL oAD_Object AS IADsPropertyList
   LOCAL oAD_PropertyEntry AS IADsPropertyEntry 'IADsPropertyList

    IF AD_ObjectExists(sAD_Object, "") = 0 THEN
        SetError(1, "Object '" & sAD_Object & "' could not be found!")
        #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
        'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
        FUNCTION = $$NUL
        EXIT FUNCTION
    END IF

    LOCAL sAD_Property AS WSTRING
    sAD_Property = "sAMAccountName"
    IF MID$(sAD_Object, 3, 1) = "=" THEN sAD_Property = "distinguishedName" ' FQDN provided
    oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_DNSDomain & ">;(" & sAD_Property & "=" & sAD_Object & ");ADsPath;subtree"
    LOCAL oAD_RecordSet AS ADORecordset
    oAD_RecordSet = oAD_Command.Execute ' Retrieve the ADsPath for the object
    LOCAL fld AS ADOField
    fld = oAD_RecordSet.fields.Item(0)
    LOCAL sAD_LDAPEntry AS VARIANT
    sAD_LDAPEntry = fld.value

    'Local sAD_LDAPEntry = oAD_RecordSet.fields(0).value
    oAD_Object = AD_ObjGet(VARIANT$$(sAD_LDAPEntry), $IID_IADsPropertyList) ' Retrieve the COM Object


    LOCAL oAD_ADSObject AS IADs, iid AS GUID, hr AS LONG
    iid = $IID_IADs
    hr = oAD_Object.QueryInterface(BYREF iid, BYREF oAD_ADSObject)
    IF FAILED(hr) THEN
          SetError(3, OBJRESULT$(hr))
          #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
          'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
          FUNCTION = $$NUL
          EXIT FUNCTION
    END IF
    oAD_ADSObject.GetInfo()

   LOCAL oAD_TempVariant AS VARIANT
    oAD_TempVariant = oAD_Object.GetPropertyItem(sAD_Attribute, %ADSTYPE_UNKNOWN)
    oAD_PropertyEntry = oAD_TempVariant
    IF NOT ISOBJECT(oAD_PropertyEntry) THEN
        SetError(4, "No values for the specified property '" & sAD_Property & "'.")
        #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
        'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
        FUNCTION = $$NUL
        EXIT FUNCTION
    END IF



    LOCAL  propertyEntryValues AS IPOWERARRAY
    propertyEntryValues = CLASS "PowerArray"
    propertyEntryValues.CopyFromVariant(oAD_PropertyEntry.Values)
    LOCAL lb AS LONG, ub AS LONG
    lb = propertyEntryValues.LBound(1)
    ub = propertyEntryValues.UBound(1)
    'MSGBOX "oAD_Item.Name: " & oAD_Item.Name & "    " & " LBOUND: " & STR$(propertyEntryValues.LBound(1) ) & "    " & "UBound: " & STR$(propertyEntryValues.UBound(1) )

    DIM resultArray( ub ) AS WSTRING
    LOCAL vAD_PropertyValue AS IADsPropertyValue
    LOCAL valueIndex AS LONG, valueType AS LONG
    FOR valueIndex = lb TO ub

      propertyEntryValues.VALUEGET(BYREF oAD_TempVariant, valueIndex)
      vAD_PropertyValue = oAD_TempVariant
      valueType = vAD_PropertyValue.ADsType

      SELECT CASE valueType
         CASE %ADSTYPE_CASE_IGNORE_STRING
            resultArray(valueIndex) = vAD_PropertyValue.CaseIgnoreString

         CASE %ADSTYPE_INTEGER
            resultArray(valueIndex) = STR$(vAD_PropertyValue.Integer)

         CASE %ADSTYPE_LARGE_INTEGER

                LOCAL largeInteger AS DISPATCH 'IADsLargeInteger
                largeInteger = vAD_PropertyValue.LargeInteger

                DIM HiPart AS DOUBLE, LoPart AS DOUBLE
                OBJECT GET largeInteger.HighPart TO HiPart
                OBJECT GET largeInteger.LowPart TO LoPart
                'MSGBOX "HiPart: " & STR$(HiPart) & " LoPart: " & STR$(LoPart)
					 'msgbox str$(VARIANT#(HiPart) * (2^32) + VARIANT#(LoPart))
                IF HiPart = 0 OR LoPart = 0 OR LoPart = -1 THEN
                    FUNCTION = $$NUL
                    EXIT FUNCTION
                END IF

                IF sAD_Attribute = "accountExpires" OR sAD_Attribute = "badPasswordTime" OR sAD_Attribute = "lastLogoff" OR sAD_Attribute = "lastLogon" _
                      OR sAD_Attribute = "pwdLastSet" OR sAD_Attribute = "uSNChanged" OR sAD_Attribute = "uSNCreated" OR sAD_Attribute = "whenChanged" OR sAD_Attribute = "whenCreated" THEN

                    DIM pFileTime AS FILETIME
                    pFileTime.dwHighDateTime = HiPart
                    pFileTime.dwLowDateTime = LoPart

                    LOCAL vTime2 AS IPOWERTIME
                    LET vTime2 = CLASS "PowerTime"
                    vTime2.FileTime = pFileTime.qDateTime
                    vTime2.ToLocalTime

                    resultArray(valueIndex) = vTime2.DateString & " " & vTime2.TimeStringFull
                ELSE
                    DIM dValue AS DOUBLE
                    'dValue = VAL( "&H" & HEX$(VARIANT#(HiPart)) & HEX$(VARIANT#(LoPart)) )
                    dValue = VARIANT#(HiPart) * (2^32) + VARIANT#(LoPart)
                    resultArray(valueIndex) = STR$(dValue)
                END IF

         CASE %ADSTYPE_OCTET_STRING 'SID
                LOCAL valuesArray AS IPOWERARRAY
                ' Create the PowerArray object
                valuesArray = CLASS "PowerArray"
                IF ISFALSE(ISOBJECT(valuesArray)) THEN
                    SetError(5, "Failed to create PowerArray object!")
                    #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
                    'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
                    FUNCTION = $$NUL
                    EXIT FUNCTION
                END IF
                ' Copy the variant to the PowerArray
                valuesArray.CopyFromVariant(vAD_PropertyValue.OctetString)

                LOCAL lb1 AS LONG, ub1 AS LONG , subscripts AS LONG
                lb1 = valuesArray.LBound(1)
                ub1 = valuesArray.UBound(1)
                'subscripts = valuesArray.SUBSCRIPTS()
                'MSGBOX "SUBSCRIPTS:" & STR$(SUBSCRIPTS) & "  LBOUND: " & STR$(lb1 ) & "  UBound: " & STR$(ub1 )

                DIM vByte AS BYTE
                DIM n AS LONG
                DIM byteArray(ub1) AS BYTE
                FOR n = lb1 TO ub1
                    valuesArray.ValueGet(BYREF vByte, n)
                    'MSGBOX "VariantVT:" & STR$(VARIANTVT(a)) & "       " &  STR$( (a) )
                    byteArray(n) = vByte
                NEXT n

                LOCAL bRes AS LONG
                IF IsValidSid(BYVAL VARPTR( byteArray(0)) ) THEN
                    DIM pSID AS SID PTR
                    pSID = VARPTR( byteArray(0))

                    ' // Convert the SID to a readable string format
                    LOCAL pStringSid AS WSTRINGZ PTR

                    bRes = ConvertSidToStringSidW(BYVAL pSID, pStringSid)
                    IF bRes = 0 THEN
                        SetError(6, "Failed to convert SID to string sid!")
                        #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
                        'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
                        FUNCTION = $$NUL
                        EXIT FUNCTION
                    ELSE
                        resultArray(valueIndex) = @pStringSid
                        bRes = LocalFree(pStringSid)
                    END IF
                ELSE 'GUID
                    DIM pGUID AS GUID PTR
                    pGUID = VARPTR( byteArray(0))
                    resultArray(valueIndex) = GUIDTXT$(@pGUID)
                END IF

         CASE %ADSTYPE_DN_STRING
            resultArray(valueIndex) = vAD_PropertyValue.DNString

         CASE %ADSTYPE_UTC_TIME
                LOCAL vbDate AS DOUBLE, ST AS SYSTEMTIME, FT AS FILETIME
                vbDate = vAD_PropertyValue.UTCTime

                IF VariantTimeToSystemTime(vbDate, ST) = %FALSE THEN
                    SetError(3, "Function 'VariantTimeToSystemTime' Call Failed!")
                    #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
                    'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
                    FUNCTION = $$NUL
                    EXIT FUNCTION
                END IF
                IF SystemTimeToFileTime(ST, FT) = 0 THEN
                    SetError(3, "Function Call Failed!")
                    #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
                    'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
                    FUNCTION = $$NUL
                    EXIT FUNCTION
                END IF

                LOCAL vTime AS IPOWERTIME
                LET vTime = CLASS "PowerTime"
                vTime.FileTime = FT.qDateTime
                vTime.ToLocalTime

                resultArray(valueIndex) = vTime.DateString & " " & vTime.TimeStringFull

         CASE %ADSTYPE_BOOLEAN
            resultArray(valueIndex) = STR$(vAD_PropertyValue.Boolean)

         CASE %ADSTYPE_NT_SECURITY_DESCRIPTOR
                DIM vSecurityDescriptor AS IADsSecurityDescriptor
                vSecurityDescriptor = vAD_PropertyValue.SecurityDescriptor
                resultArray(valueIndex) = "Control:" & STR$(vSecurityDescriptor.Control) & ", " & _
                            "Group:" & vSecurityDescriptor.Group & ", " & _
                            "Owner:" & vSecurityDescriptor.Owner & ", " & _
                            "Revision:" & STR$(vSecurityDescriptor.Revision)

          CASE ELSE
            resultArray(valueIndex) = vAD_PropertyValue.PrintableString

        END SELECT

    NEXT



   'MSGBOX "resultArray: " & JOIN$(resultArray(), ";")

   FUNCTION = JOIN$(resultArray(), ";") + $$NUL

END FUNCTION '==>AD_GetObjectAttribute


' #FUNCTION# ====================================================================================================================
' Name...........: AD_ModifyAttribute
' Description ...: Modifies an attribute of the given object to the value specified.
' Syntax.........: AD_ModifyAttribute(sAD_Object, sAD_Attribute[, sAD_Value = ""[, iAD_Option = 1]])
' Parameters ....: sAD_Object - Object (user, group ...) to add/delete/modify an attribute (sAMAccountName or FQDN)
'                  sAD_Attribute - Attribute to add/delete/modify
'                  sAD_ValueParam - Optional: Value to modify the attribute to. Use a blank string ("") to delete the attribute (default).
'                  +sAD_Value can be a single value (as a string) or a multi-value (as a one-dimensional array)
'                  iAD_Option - Optional: Indicates the mode of modification: Append, Replace, Remove, and Delete
'                  |1 - CLEAR: remove all the property value(s) from the object (default when $sAD_value = "")
'                  |2 - UPDATE: replace the current value(s) with the specified value(s)
'                  |3 - APPEND: append the specified value(s) to the existing values(s)
'                  |4 - DELETE: delete the specified value(s) from the object
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - sAD_ObjectParam does not exist
'                  |2 - Creation of the COM object failed!
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........:
' Modified.......:
' Remarks .......:
' Related .......: AD_GetObjectAttribute, AD_GetObjectProperties
' Link ..........: http://msdn.microsoft.com/en-us/library/aa746353(VS.85).aspx (ADS_PROPERTY_OPERATION_ENUM Enumeration)
' Example .......: Yes
' ===============================================================================================================================
'FUNCTION AD_ModifyAttribute ALIAS "AD_ModifyAttribute" (BYVAL sAD_ObjectParam AS WSTRING, BYVAL sAD_AttributeParam AS WSTRING, BYVAL sAD_ValueParam AS WSTRING, OPT BYVAL iAD_Option AS LONG) EXPORT AS LONG
FUNCTION AD_ModifyAttribute ALIAS "AD_ModifyAttribute" (BYREF sAD_ObjectParam AS WSTRINGz, BYREF sAD_AttributeParam AS WSTRINGz, BYREF sAD_ValueParam AS WSTRINGz, OPT BYVAL iAD_Option AS LONG) EXPORT AS LONG

   SetError(0, "")

   DIM sAD_Object AS WSTRINGZ * 512, sAD_Attribute AS WSTRINGZ * 256, sAD_Value as WSTRINGZ * 512
   sAD_Object = sAD_ObjectParam
   sAD_Attribute = sAD_AttributeParam
	sAD_Value = sAD_ValueParam

    IF AD_ObjectExists(sAD_Object, "") = 0 THEN
      SetError(1, "Object '" & sAD_Object & "' could not be found!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF


    LOCAL sAD_Property AS WSTRING
    sAD_Property = "sAMAccountName"
    IF MID$(sAD_Object, 3, 1) = "=" THEN sAD_Property = "distinguishedName" ' FQDN provided
    oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_DNSDomain & ">;(" & sAD_Property & "=" & sAD_Object & ");ADsPath;subtree"
    LOCAL oAD_RecordSet AS ADORecordset
    oAD_RecordSet = oAD_Command.Execute ' Retrieve the ADsPath for the object
    LOCAL fld AS ADOField
    fld = oAD_RecordSet.fields.Item(0)
    LOCAL sAD_LDAPEntry AS VARIANT
    sAD_LDAPEntry = fld.value

    LOCAL oAD_Object AS IADs
    oAD_Object = AD_ObjGet(VARIANT$$(sAD_LDAPEntry), $IID_IADs) ' Retrieve the COM Object
    IF ISFALSE ISOBJECT(oAD_Object) THEN
      SetError(2, "Creation of the COM object failed!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF
   oAD_Object.GetInfo()


    IF sAD_Value = "" THEN
        oAD_Object.PutEx(1, sAD_Attribute, 0) ' CLEAR: remove all the property value(s) from the object
    ELSEIF iAD_Option = 3 THEN
        oAD_Object.PutEx(3, sAD_Attribute, sAD_Value) ' APPEND: append the specified value(s) to the existing values(s)
    'Else If IsArray(sAD_Value) Then
    ELSEIF iAD_Option = 2 THEN
        oAD_Object.PutEx(2, sAD_Attribute, sAD_Value) ' UPDATE: replace the current value(s) with the specified value(s)
    ELSE
        oAD_Object.Put(sAD_Attribute, sAD_Value) ' sets the value(s) of an attribute
    END IF
    oAD_Object.SetInfo

   IF AD_LastError() THEN
      FUNCTION = 0
      EXIT FUNCTION
   END IF


   FUNCTION = 1

END FUNCTION   '==>AD_ModifyAttribute



' #FUNCTION# ====================================================================================================================
' Name...........: AD_CreateOU
' Description ...: Creates a child OU in the specified parent OU.
' Syntax.........: AD_CreateOU(sAD_ParentOU, sAD_OU)
' Parameters ....: sParentOUDN - Parent OU where the new OU will be created (DN,e.g. OU=ParentOU)
'                  sOUName - OU to create in the the parent OU (Name without leading "OU=")
' Return values .: Success - 1
'                  Failure - 0, sets lAD_LastErrorCode to:
'                  |1 - sOUName is empty
'                  |2 - Parent OU does not exist
'                  |3 - OU already exists
'                  |4 - Can not create IADsOU object
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: hehui
' Modified.......: hehui
' Remarks .......: This does not create any attributes for the OU. Use function _AD_ModifyAttribute.
' Related .......: AD_CreateUser, AD_CreateGroup, AD_AddUserToGroup, AD_RemoveUserFromGroup
' Link ..........:
' Example .......: AD_CreateOU("OU=ParentOU", "ChildOU")
' ===============================================================================================================================
'FUNCTION AD_CreateOU ALIAS "AD_CreateOU" (BYVAL sParentOUDN AS WSTRING, BYVAL sOUName AS WSTRING) EXPORT AS LONG
FUNCTION AD_CreateOU ALIAS "AD_CreateOU" (BYREF sParentOUDN AS WSTRINGz, BYREF sOUName AS WSTRINGz) EXPORT AS LONG

    SetError(0, "")

    DIM sParentOUDNString AS WSTRINGZ * 256
    sParentOUDNString = sParentOUDN

    DIM sOUNameString AS WSTRINGZ * 256
    sOUNameString = sOUName

    IF sOUNameString = "" THEN
        'sAD_LastErrorString = "Invalid OU Name!"
        'lAD_LastErrorCode = 1
        SetError(1, "Invalid OU Name!")
        #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
        'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
        FUNCTION = 0
        EXIT FUNCTION
    END IF

    DIM ouDN AS WSTRING
    IF sParentOUDNString = "" THEN
      ouDN = "OU=" & sOUNameString
    ELSE
      ouDN = "OU=" & sOUNameString & "," & sParentOUDNString

      LOCAL parentOUFQDN$
      parentOUFQDN$ = sParentOUDNString & "," & sAD_DNSDomain
      IF ISFALSE(AD_ObjectExists(parentOUFQDN$, "distinguishedName")) THEN
         'sAD_LastErrorString = "Parent OU does not exist!"
         'lAD_LastErrorCode = 2
         SetError(2, "OU '" & parentOUFQDN$ & "' does not exist!")
         #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
         'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
         FUNCTION = 0
         EXIT FUNCTION
      END IF

    END IF

    LOCAL ouFQDN$
    ouFQDN$ = ouDN & "," & sAD_DNSDomain
    IF ISTRUE(AD_ObjectExists(ouFQDN$, "distinguishedName")) THEN
          'sAD_LastErrorString = "OU already exists!"
          'lAD_LastErrorCode = 3
          SetError(3, "OU '" & ouFQDN$ & "' already exists!")
          #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
          'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
          FUNCTION = 0
          EXIT FUNCTION
    END IF


    DIM oAD_OU AS IADsOU
    oAD_OU = oAD_RootContainer.Create("organizationalUnit", ouDN)

    IF NOT ISOBJECT(oAD_OU) THEN
        'sAD_LastErrorString = "Failed to create OU!"
        'lAD_LastErrorCode = 4
        SetError(4, "Failed to create OU!")
        FUNCTION = 0
        EXIT FUNCTION
    END IF

    oAD_OU.SetInfo()

    IF AD_LastError() THEN
        FUNCTION = 0
        EXIT FUNCTION
    END IF


    FUNCTION = 1

END FUNCTION   '==>AD_CreateOU



' #FUNCTION# ====================================================================================================================
' Name...........: AD_GetAllOUs
' Description ...: Retrieves an array of OUs. The paths are separated by the '\' character.
' Syntax.........: AD_GetAllOUs([sAD_Root = ""[, sAD_Separator = ","]])
' Parameters ....: sAD_RootParam - Optional: OU (FQDN) where to start in the AD tree (default = "", equals "start at the AD root")
'                  sAD_SeparatorParam - Optional: Single character to separate the OU names (default = ",")
'                  sAD_SubOUSeparatorParam - Optional: Single character to separate the sub OU names (default = "\")
' Return values .: Success - string with OU names separated by ","
'                  |0 - ... \name of grandfather OU\name of father OU\name of son OU
'                  |1 - Distinguished Name (FQDN) of the son OU
'                  Failure - "", sets @error to:
'                  |1 - No OUs found
'                  |2 - Specified sAD_Root does not exist
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......: If an OU contains spaces the sorting is wrong and might lead to problems in further processing.
'                  Please have a look at http://www.autoitscript.com/forum/topic/106163-active-directory-udf/page__view__findpost__p__943892
' Related .......: _AD_GetObjectsInOU
' Link ..........:
' Example .......: Yes
' ===============================================================================================================================
'Function AD_GetAllOUs alias "AD_GetAllOUs" (byval sAD_RootParam as wstring, opt byval sAD_SeparatorParam as wstring, byval sAD_SubOUSeparatorParam as wstring ) export as wstring
Function AD_GetAllOUs alias "AD_GetAllOUs" (BYREF sAD_RootParam as wstringz, opt BYREF sAD_SeparatorParam as wstringz, BYREF sAD_SubOUSeparatorParam as wstringz ) export as wstring

    SetError(0, "")

    local sAD_Root AS WSTRINGZ * 256, sAD_Separator AS WSTRINGZ * 4, sAD_SubOUSeparator AS WSTRINGZ * 4
    sAD_Root = sAD_RootParam
    sAD_Separator = sAD_SeparatorParam
	 sAD_SubOUSeparator = sAD_SubOUSeparatorParam


	If sAD_Root = "" Then
		sAD_Root = sAD_DNSDomain
	Else
		If AD_ObjectExists(sAD_Root, "distinguishedName") = 0 Then
			SetError(2, "Object '" & sAD_Root & "' does not exist!")
			#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
			'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
			FUNCTION = $$NUL
			EXIT FUNCTION 
		end if
	End If
	
	If sAD_Separator <= " " Or Len(sAD_Separator) > 1 Then sAD_Separator = ","
	If sAD_SubOUSeparator <= " " Or Len(sAD_SubOUSeparator) > 1 Then sAD_SubOUSeparator = "\"

	oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_Root & ">;" & "(objectCategory=organizationalUnit)" & ";distinguishedName;subtree"
	LOCAL oAD_RecordSet AS ADORecordset 
	oAD_RecordSet = oAD_Command.Execute
	Local iRecordCount as long
	iRecordCount = oAD_RecordSet.RecordCount
	If iRecordCount = 0 Then 
			SetError(1, "No OUs found!")
			#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
			'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
			FUNCTION = $$NUL
			EXIT FUNCTION 
	end if
	
	dim aAD_OUs(1 to iRecordCount) as wstring, iCount2 as long
	iCount2 = 1
	
	LOCAL fld AS ADOField, sOUName AS wstring
	oAD_RecordSet.MoveFirst
	Do Until oAD_RecordSet.EOF
		'aAD_OUs(iCount2, 1) = oAD_RecordSet.Fields("distinguishedName").Value
		fld = oAD_RecordSet.fields.Item(0)
		sOUName = CLIP$(RIGHT variant$$(fld.value), Len(sAD_DNSDomain) + 1) 
		
		replace ",OU=" with sAD_SubOUSeparator in sOUName
		replace "OU=" with "" in sOUName
		if VERIFY(sAD_SubOUSeparator, sOUName) = 0 THEN 'Sub OU
			dim aAD_TempArray(256) as wstring, iCount as integer, tempStr as wstring, n#
			PARSE sOUName, aAD_TempArray(), sAD_SubOUSeparator
			iCount = ubound(aAD_TempArray())
			sOUName = aAD_TempArray(0)
			for n#=1 to iCount
				tempStr = aAD_TempArray(n#)
				if tempStr = "" THEN
					exit for
				END IF
				sOUName = tempStr & sAD_SubOUSeparator & sOUName
			NEXT n# 
		END IF
		
		aAD_OUs(iCount2) = sOUName
	
		iCount2 += 1
		if iCount2 > iRecordCount THEN
			EXIT LOOP
		END IF
		
		oAD_RecordSet.MoveNext
	loop
  
	
	function = JOIN$(aAD_OUs(), sAD_Separator) + $$NUL


End Function   '==>AD_GetAllOUs


' #FUNCTION# ====================================================================================================================
' Name...........: AD_GetObjectsInOU
' Description ...: Returns a filtered array of objects and attributes for a given OU 
' Syntax.........: AD_GetObjectsInOU($sAD_OU[, $sAD_Filter = "(name=*)"[, $iAD_SearchScope = 2[, $sAD_DataToRetrieve =  "sAMAccountName"[, $sAD_SortBy = "sAMAccountName"[, $bAD_Count = False]]]]])
' Parameters ....: sAD_OUParam - The OU to retrieve from (FQDN) (default = "", equals "search the whole AD tree")
'                  sAD_FilterParam - Optional: An additional LDAP filter if required (default = "(name=*)")
'                  sAD_DataToRetrieveParam -  A comma-seperated list of attributes to retrieve (default = "sAMAccountName").
'                  |More than one attribute will create a 2-dimensional array
'                  sAD_ItemSeparatorParam - Single character to separate the items (default = $CRLF)
'                  sAD_AttributeSeparatorParam - Single character to separate the item attributes (default = "|")

'						  $sAD_SortBy - Optional: name of the attribute the resulting array will be sorted upon (default = "sAMAccountName").
'                  |To completely suppress sorting (even the default sort) set this parameter to "". This improves performance when doing large queries
'                  $iAD_SearchScope - Optional: 0 = base, 1 = one-level, 2 = sub-tree (default)
' Return values .: Success - One or two dimensional array of objects and attributes in the given OU. First entry is for the given OU itself
'                  Failure - "", sets @error to:
'                  |1 - Specified OU does not exist
'                  |2 - No records returned from Active Directory. $sAD_DataToRetrieve is invalid (attribute may not exist)
'                  |3 - No records returned from Active Directory. $sAD_Filter didn't return a record
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......: Multi-value attributes are returned as string with the pipe character (|) as separator.
'+
'                  The default filter returns an array including one record for the OU itself. To exclude the OU use a different filter that doesn't include the OU
'                  e.g. "(&(objectcategory=person)(objectclass=user)(name=*))"
'+
'                  To make sure that all properties you specify in $sAD_DataToRetrieve exist in the AD you can use _AD_ObjectExistsInSchema.
'+
'                  The following examples illustrate the use of the escaping mechanism in the LDAP filter:
'                    (o=Parens R Us \28for all your parenthetical needs\29)
'                    (cn=*\2A*)
'                    (filename=C:\5cMyFile)
'                    (bin=\00\00\00\04)
'                    (sn=Lu\c4\8di\c4\87)
'                  The first example shows the use of the escaping mechanism to represent parenthesis characters.
'                  The second shows how to represent a "*" in a value, preventing it from being interpreted as a substring indicator.
'                  The third illustrates the escaping of the backslash character.
'                  The fourth example shows a filter searching for the four-byte value 0x00000004, illustrating the use of the escaping mechanism to
'                  represent arbitrary data, including NUL characters.
'                  The final example illustrates the use of the escaping mechanism to represent various non-ASCII UTF-8 characters.
' Related .......: AD_GetAllOUs
' Link ..........:
' Example .......: 
    'AD_GetObjectsInOU("OU=TestOU1,DC=test,DC=local", "(&(objectcategory=person)(objectclass=user)(cn=" & "test" & "*))" , "sAMAccountName,distinguishedName,objectSid", ";", "|")
    'AD_GetObjectsInOU("OU=TestOU1,DC=test,DC=local", "(&(objectcategory=person)(objectclass=user)(sAMAccountName=" & "test" & "*)(displayName=Tes*))" , "memberOf", ";", "|")
    'AD_GetObjectsInOU("DC=test,DC=local", "(&(objectcategory=person)(objectclass=user)(sAMAccountName=" & "he" & "*))" , "lastLogon", ";", "|")

' ===============================================================================================================================
'Function AD_GetObjectsInOU alias "AD_GetObjectsInOU" (BYVAL sAD_OUParam as wstring, BYVAL sAD_FilterParam as wstring,  BYVAL sAD_DataToRetrieveParam as wstring, _
'																			 BYVAL sAD_ItemSeparatorParam as wstring, BYVAL sAD_AttributeSeparatorParam as wstring ) export as wstring
'                                                     ' byval sAD_SortByParam as wstring, byval iAD_SearchScopeParam as long) export as wstring
Function AD_GetObjectsInOU alias "AD_GetObjectsInOU" (BYVAL spBuffer AS WSTRINGZ PTR, BYVAL pdBufferSize AS DWORD PTR, BYREF sAD_OUParam as wstringz, BYREF sAD_FilterParam as wstringz,  BYREF sAD_DataToRetrieveParam as wstringz, _
																			 BYREF sAD_ItemSeparatorParam as wstringz, BYREF sAD_AttributeSeparatorParam as wstringz ) export as LONG														  
	
	SetError(0, "")	

    local sAD_OU AS WSTRINGZ * 256, sAD_Filter AS WSTRINGZ * 256, sAD_DataToRetrieve as WSTRINGZ * 256
	 local sAD_ItemSeparator as WSTRINGZ * 2, sAD_AttributeSeparator as WSTRINGZ * 2
	 'local sAD_SortBy as WSTRINGZ * 256, iAD_SearchScope AS long
	  
	sAD_OU = sAD_OUParam
	sAD_Filter = sAD_FilterParam
	sAD_DataToRetrieve = sAD_DataToRetrieveParam

	sAD_ItemSeparator = sAD_ItemSeparatorParam
	sAD_AttributeSeparator = sAD_AttributeSeparatorParam
			
	'iAD_SearchScope = iAD_SearchScopeParam
	'sAD_SortBy = sAD_SortByParam
	
	
	If sAD_OU = "" Then
		sAD_OU = sAD_DNSDomain
	Else
		If AD_ObjectExists(sAD_OU, "distinguishedName") = 0 Then
			SetError(1, "OU '" & sAD_OU & "' does not exist!")
			#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
			'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
			FUNCTION = 0
			EXIT FUNCTION  
		end if
			
	End If

	'if sAD_Filter = "" THEN sAD_Filter = "(name=*)"
	if sAD_Filter = "" THEN sAD_Filter = "(sAMAccountName=*)"
	if sAD_DataToRetrieve = "" THEN sAD_DataToRetrieve =  "sAMAccountName"
	'If sAD_ItemSeparator <= "" Or sAD_ItemSeparator <= "," or sAD_ItemSeparator <= ";" or Len(sAD_ItemSeparator) > 1 Then sAD_ItemSeparator = $crlf
	If sAD_ItemSeparator <= "" Or sAD_ItemSeparator <= "," or sAD_ItemSeparator <= ";" Then sAD_ItemSeparator = $crlf
	If sAD_AttributeSeparator <= "" or sAD_AttributeSeparator = "," or sAD_AttributeSeparator = ";" Or Len(sAD_AttributeSeparator) > 2 Then sAD_AttributeSeparator = "|"
	
	'if iAD_SearchScope = 0 THEN iAD_SearchScope = 2
	'if sAD_SortBy = "" THEN sAD_SortBy = "sAMAccountName"

	
	'Local iCount2 as long
	'sAD_DataToRetrieve = StringStripWS(sAD_DataToRetrieve, 8)
	REPLACE " " WITH "" IN sAD_DataToRetrieve

	'oAD_Command.Properties("Searchscope") = iAD_SearchScope
	oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_OU & ">;" & sAD_Filter & ";" & sAD_DataToRetrieve '& ";subtree"

	'oAD_Command.Properties("Sort On") = sAD_SortBy
	LOCAL oAD_RecordSet AS ADORecordset 
	oAD_RecordSet = oAD_Command.Execute
	If Not IsObject(oAD_RecordSet) Then 
		SetError(2, "No records returned from Active Directory! sAD_DataToRetrieve is invalid (attribute may not exist)")
		#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
		'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
		FUNCTION = 0
		EXIT FUNCTION  
	end if

	Local iRecordCount as long
	iRecordCount = oAD_RecordSet.RecordCount	
	If iRecordCount = 0 Then 
		SetError(3, "No records returned from Active Directory! sAD_Filter didn't return a record")
		#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
		'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
		FUNCTION = 0	
		EXIT FUNCTION
	end if

	dim fld AS ADOField, aAD_Objects(1 to iRecordCount) as wstringZ*8192, iCount1 as long, iCount2 as long
	'If bAD_Count Then Return oAD_RecordSet.RecordCount

	If VERIFY(",", sAD_DataToRetrieve) = 0 Then
		
		dim aAD_DataToRetrieveArray( PARSECOUNT(sAD_DataToRetrieve) ) as wstringZ*64
		PARSE sAD_DataToRetrieve, aAD_DataToRetrieveArray(), ","
		'aAD_DataToRetrieve = StringSplit(sAD_DataToRetrieve, ",")
		'Local aAD_Objects[iCount1 + 1][aAD_DataToRetrieve[0]]
		'aAD_Objects[0][0] = iCount1
		'aAD_Objects[0][1] = aAD_DataToRetrieve[0]
		'iCount2 = 1
		'oAD_RecordSet.MoveFirst
		
		'dim aAD_Objects(1 to iCount1) as wstring
		dim vValue as variant
		iCount1 = 0
		iCount2 = 1
		
		oAD_RecordSet.MoveFirst
		Do Until oAD_RecordSet.EOF
			'dim tempString as wstring
			dim tempStringArray(0 To PARSECOUNT(sAD_DataToRetrieve)-1) as wstringZ*4096
			For iCount1 = 0 To PARSECOUNT(sAD_DataToRetrieve)-1
				fld = oAD_RecordSet.fields.Item(iCount1)
				'vValue = fld.Value
				'tempString = tempString & convertADOVariantToString(fld.Value, aAD_DataToRetrieveArray(iCount1)) & sAD_AttributeSeparator
				tempStringArray(iCount1) = convertADOVariantToString(fld.Value, aAD_DataToRetrieveArray(iCount1))
			Next iCount1
			'tempString = CLIP$(RIGHT tempString, 1) 
			'aAD_Objects(iCount2) = tempString
			aAD_Objects(iCount2) = JOIN$(tempStringArray(), sAD_AttributeSeparator) + $$NUL  
			
			iCount2 += 1
			'tempString = ""
			
			if iCount2 > iRecordCount THEN
				EXIT LOOP
			END IF
			
			oAD_RecordSet.MoveNext

		loop
		

	Else
		'Local aAD_Objects[iCount1 + 1]
		'aAD_Objects[0] = UBound(aAD_Objects) - 1
		iCount2 = 1
		oAD_RecordSet.MoveFirst

		Do Until oAD_RecordSet.EOF

			fld = oAD_RecordSet.fields.Item(0)
			'vValue = fld.Value
			aAD_Objects(iCount2) = convertADOVariantToString(fld.Value, sAD_DataToRetrieve)
			
			oAD_RecordSet.MoveNext
			iCount2 += 1
		loop
	
	End If
	'oAD_Command.Properties("Sort On") = "" ' Reset sort property
	
		
	DIM dataString AS WSTRING, dataLength AS DWORD	
	dataString = JOIN$(aAD_Objects(), sAD_ItemSeparator) + $$NUL  
	dataLength = LEN(dataString)
	
	If @pdBufferSize < dataLength Then
		   @pdBufferSize = dataLength
			
			SetError(1, "No enough buffer!")
			#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
			'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
			FUNCTION = 0
			EXIT FUNCTION  
	end if
		
	@spBuffer = dataString
	@pdBufferSize = dataLength
	function = 1

End Function   '==>AD_GetObjectsInOU


' #FUNCTION# ====================================================================================================================
' Name...........: convertADOVariantToString
' Description ...: Convert ADO variant value to string
' Syntax.........: convertADOVariantToString(vValue, sAD_Attribute)
' Parameters ....: vValue - variant value from ADO
'                  sAD_Attribute - AD object attribute  which vValue is assigned to           
' Return values .: Success - string 
'                  Failure - "", sets @error to:
'                  |1 - Failed to create PowerArray object
'                  |2 - Failed to convert SID to string sid
' Author ........: hehui
' Modified.......: hehui
' Remarks .......: 
'                  
' Related .......: AD_GetObjectsInOU
' Link ..........:
' Example .......: Yes
' ===============================================================================================================================
function convertADOVariantToString(byval vValue as variant, byval sAD_Attribute as wstring ) as wstring
'function convertADOVariantToString(byval vValue as variant, BYREF sAD_Attribute as wstringz ) as wstring

	local variantType as long, result as wstring, valuesArraySeparator as wstringZ*2
	variantType = variantvt(vValue)
	valuesArraySeparator = ";"

	select CASE variantType
		case %VT_BSTR
			result = variant$$(vValue)
		case %VT_DISPATCH
			       LOCAL largeInteger AS DISPATCH 'IADsLargeInteger
                largeInteger = vValue

                DIM HiPart AS DOUBLE, LoPart AS DOUBLE
                OBJECT GET largeInteger.HighPart TO HiPart
                OBJECT GET largeInteger.LowPart TO LoPart

                IF HiPart = 0 OR LoPart = 0 OR LoPart = -1 THEN
                    FUNCTION = $$NUL
                    EXIT FUNCTION
                END IF

                IF sAD_Attribute = "accountExpires" OR sAD_Attribute = "badPasswordTime" _ 
							OR sAD_Attribute = "lastLogoff" OR sAD_Attribute = "lastLogon"  OR sAD_Attribute = "lastLogonTimestamp" OR sAD_Attribute = "lockoutTime"  _
                     OR sAD_Attribute = "pwdLastSet" OR sAD_Attribute = "uSNChanged" OR sAD_Attribute = "uSNCreated" OR sAD_Attribute = "whenChanged" OR sAD_Attribute = "whenCreated" THEN

                    DIM pFileTime AS FILETIME
                    pFileTime.dwHighDateTime = HiPart
                    pFileTime.dwLowDateTime = LoPart

                    LOCAL vTime2 AS IPOWERTIME
                    LET vTime2 = CLASS "PowerTime"
                    vTime2.FileTime = pFileTime.qDateTime
                    vTime2.ToLocalTime

                    result = vTime2.DateString & " " & vTime2.TimeStringFull
                ELSE
                    DIM dValue AS DOUBLE
                    'dValue = VAL( "&H" & HEX$(VARIANT#(HiPart)) & HEX$(VARIANT#(LoPart)) )
                    dValue = VARIANT#(HiPart) * (2^32) + VARIANT#(LoPart)
                    result = STR$(dValue)
                END IF 
					     
		case %VT_I4
			result = str$(variant#(vValue))
			
		case 8204 'objectClass-OID; dSCorePropagationData-GeneralizedTime
		''----------------------------
			LOCAL valuesArray AS IPOWERARRAY
		  ' Create the PowerArray object
			valuesArray = CLASS "PowerArray"
			IF ISFALSE(ISOBJECT(valuesArray)) THEN
				SetError(1, "Failed to create PowerArray object!")
				FUNCTION = $$NUL
				EXIT FUNCTION
			END IF
			' Copy the variant to the PowerArray
			valuesArray.CopyFromVariant(vValue)

			LOCAL lb AS LONG, ub AS LONG , subscripts AS LONG
			lb = valuesArray.LBound(1)
			ub = valuesArray.UBound(1)
			subscripts = valuesArray.SUBSCRIPTS()
			'MSGBOX "SUBSCRIPTS:" & STR$(SUBSCRIPTS) & "  LBOUND: " & STR$(lb ) & "  UBound: " & STR$(ub )


			local a AS VARIANT, n AS LONG
			FOR n = lb TO ub
				valuesArray.ValueGet(BYREF a, n)
				'MSGBOX "VariantVT:" & STR$(VARIANTVT(a)) & "       " &  STR$( VARIANT#(a) )
				if sAD_Attribute = "objectClass" or sAD_Attribute = "memberOf" or sAD_Attribute = "description" THEN
					result = result & variant$$(a) & valuesArraySeparator
				elseif sAD_Attribute = "dSCorePropagationData" then
					LOCAL dSCorePropagationDataTime AS IAfxTime
					dSCorePropagationDataTime = NewAfxTime()
					dSCorePropagationDataTime.FromVarDate(a)
					dSCorePropagationDataTime.ToLocalTime
					result = result & dSCorePropagationDataTime.DateString & " " & dSCorePropagationDataTime.TimeStringFull & ","
				END IF

			NEXT n
			result = CLIP$(RIGHT result, 1) 
		''----------------------------- 
		
		case 8209 'objectGUID-OctetString; objectSid-Sid; nTSecurityDescriptor-NTSecurityDescriptor
			if sAD_Attribute =  "nTSecurityDescriptor" THEN
				result = $$NUL
				EXIT SELECT         
			END IF
			
			 LOCAL sidValuesArray AS IPOWERARRAY
			 ' Create the PowerArray object
			 sidValuesArray = CLASS "PowerArray"
			 IF ISFALSE(ISOBJECT(sidValuesArray)) THEN
				  SetError(1, "Failed to create PowerArray object!")
				  #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
				  'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
				  FUNCTION = $$NUL
				  EXIT FUNCTION
			 END IF
			 ' Copy the variant to the PowerArray
			 sidValuesArray.CopyFromVariant(vValue)

			 LOCAL lbSID AS LONG, ubSID AS LONG ', subscripts AS LONG
			 lbSID = sidValuesArray.LBound(1)
			 ubSID = sidValuesArray.UBound(1)

			 DIM vByte AS BYTE
			 DIM nSID AS LONG
			 DIM byteArray(ubSID) AS BYTE
			 FOR nSID = lbSID TO ubSID
				  sidValuesArray.ValueGet(BYREF vByte, nSID)
				  byteArray(nSID) = vByte
			 NEXT nSID

			 LOCAL bRes AS LONG
			 IF IsValidSid(BYVAL VARPTR( byteArray(0)) ) THEN
				  DIM pSID AS SID PTR
				  pSID = VARPTR( byteArray(0))

				  ' // Convert the SID to a readable string format
				  LOCAL pStringSid AS WSTRINGZ PTR

				  bRes = ConvertSidToStringSidW(BYVAL pSID, pStringSid)
				  IF bRes = 0 THEN
						SetError(2, "Failed to convert SID to string sid!")
						#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
						'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
						FUNCTION = $$NUL
						EXIT FUNCTION
				  ELSE
						result = @pStringSid
						bRes = LocalFree(pStringSid)
				  END IF
			 ELSE 'GUID
				  DIM pGUID AS GUID PTR
				  pGUID = VARPTR( byteArray(0))
				  result = GUIDTXT$(@pGUID)
			 END IF 
					 
		case %VT_DATE
			LOCAL afxTime AS IAfxTime
			afxTime = NewAfxTime()
			afxTime.FromVarDate(vValue)
			afxTime.ToLocalTime
			result = afxTime.DateString & " " & afxTime.TimeStringFull

		CASE ELSE
			result = $$NUL
	END SELECT
	
	'msgbox "variantvt(vValue):	" & str$(variantType) & $crlf & "result:	" & result
	
	function = result + $$NUL  
	
end function   '==>convertADOVariantToString


' #FUNCTION# ====================================================================================================================
' Name...........: AD_CreateUser
' Description ...: Creates and activates a user in the specified OU.
' Syntax.........: AD_CreateUser(sAD_OU, sAD_User, sAD_CN)
' Parameters ....: sAD_OU - OU to create the user in. Form is "OU=sampleou,OU=sampleparent,DC=sampledomain1,DC=sampledomain2"
'                  sAD_User - Username, form is SamAccountName without leading 'CN='
'                  sAD_CNParam - Common Name (without CN=) or RDN (Relative Distinguished Name) like "Lastname Firstname"
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - sAD_User already exists
'                  |2 - sAD_OU does not exist
'                  |3 - sAD_CN is missing
'                  |4 - sAD_User is missing
'                  |5 - sAD_User could not be created.
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......: This function only sets sAMAccountName (= sAD_User) and userPrincipalName (e.g. sAD_User@microsoft.com).
'                  All other attributes have to be set using function _AD_ModifyAttribute
' Related .......: AD_CreateOU, AD_CreateGroup, AD_AddUserToGroup, AD_RemoveUserFromGroup
' Link ..........:
' Example .......: Yes
' ===============================================================================================================================
'FUNCTION AD_CreateUser ALIAS "AD_CreateUser" (BYVAL sAD_OU AS WSTRING, BYVAL sAD_User AS WSTRING, OPT BYVAL sAD_CNParam AS WSTRING) EXPORT AS LONG
FUNCTION AD_CreateUser ALIAS "AD_CreateUser" (BYREF sAD_OU AS WSTRINGz, BYREF sAD_User AS WSTRINGz, OPT BYREF sAD_CNParam AS WSTRINGz) EXPORT AS LONG

   SetError(0, "")

   DIM sAD_CN AS WSTRINGZ * 256
   sAD_CN = sAD_CNParam

    IF sAD_User = "" THEN
        SetError(4, "Parameter sAD_User is missing!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
    END IF

    IF sAD_CN = "" THEN
        sAD_CN = sAD_User

        'SetError(3, "Parameters sAD_CN is missing!")
      '#DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      'FUNCTION = 0
      'EXIT FUNCTION
    END IF
    'sAD_CN = _AD_FixSpecialChars(sAD_CN)

    IF AD_ObjectExists(sAD_User, "") THEN
      SetError(1, "User '" & sAD_User & "' already exists!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF

    IF ISFALSE AD_ObjectExists(sAD_OU, "distinguishedName") THEN
        SetError(2, "OU '" & sAD_OU & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
    END IF


    LOCAL oAD_OU AS IADsContainer
    oAD_OU = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_OU, $IID_IADsContainer) ' Retrieve the COM Object
    IF ISFALSE ISOBJECT(oAD_OU) THEN
      SetError(5, "Creation of the COM object failed!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF

    LOCAL oAD_User AS IADsUser
    oAD_User = oAD_OU.Create("User", "CN=" & sAD_CN)
    IF ISFALSE(ISOBJECT(oAD_User)) THEN
      SetError(5, "Creation of the COM object failed!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
    END IF

    oAD_User.put("sAMAccountName", sAD_User)

    LOCAL tempStr AS WSTRING
    tempStr = sAD_DNSDomain
    REPLACE "DC=" WITH "" IN tempStr
    REPLACE "," WITH "." IN tempStr
    tempStr = TRIM$(tempStr)
    oAD_User.put("userPrincipalName", sAD_User & "@" & tempStr)
    oAD_User.put("pwdLastSet", -1) ' Set password to not expired

    oAD_User.SetInfo
    IF OBJRESULT <> 0 THEN
        FUNCTION = 0
		  exit function
    END IF


    'Activate User
    LOCAL userActCtrl AS INTEGER
    userActCtrl = VARIANT#(oAD_User.Get("userAccountControl"))
    userActCtrl = userActCtrl AND (NOT %ADS_UF_ACCOUNTDISABLE)
    oAD_User.put("userAccountControl", userActCtrl)


    oAD_User.SetInfo
	 
	 
	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

    SET oAD_User = NOTHING
    SET oAD_OU = NOTHING



END FUNCTION   '==>AD_CreateUser


' #FUNCTION# ====================================================================================================================
' Name...........: AD_SetPassword
' Description ...: Sets a user's password, or clears it if no password is passed to the function.
' Syntax.........: AD_SetPassword($sAD_User[, $sAD_Password=""[, $iAD_Expired = 0]])
' Parameters ....: $sAD_User - User for which to set the password (FQDN or sAMAccountName)
'                  $sAD_Password - Optional: Password to be set for $sAD_User. If $sAD_Password is "" then the password will be cleared (default)
'                  $iAD_Expired - Optional: 1 = the password has to be changed at next logon (Default = 0)
' Return values .: Success - 1
'                  Failure - 0, sets @error to:
'                  |1 - $sAD_User does not exist
'                  |x - Error returned by SetInfo function (Missing permission etc.)
' Author ........: KenE
' Modified.......: water
' Remarks .......:
' Related .......: _AD_IsPasswordExpired, _AD_GetPasswordExpired, _AD_GetPasswordDontExpire, _AD_DisablePasswordExpire, _AD_EnablePasswordExpire, _AD_EnablePasswordChange,  _AD_DisablePasswordChange, _AD_GetPasswordInfo
' Link ..........:
' Example .......: Yes
' ===============================================================================================================================
'FUNCTION AD_SetPassword ALIAS "AD_SetPassword" (BYVAL sAD_UserParam AS WSTRING, BYVAL sAD_Password AS WSTRING, OPT BYVAL iAD_Expired AS long) EXPORT AS LONG
FUNCTION AD_SetPassword ALIAS "AD_SetPassword" (BYREF sAD_UserParam AS WSTRINGz, BYREF sAD_Password AS WSTRINGz, OPT BYVAL iAD_Expired AS long) EXPORT AS LONG

    SetError(0, "")

    DIM sAD_User AS WSTRINGZ * 256
   sAD_User = sAD_UserParam


    IF ISFALSE AD_ObjectExists(sAD_User, "") THEN
      SetError(1, "User '" & sAD_User & "' does not exist!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF



    IF MID$(sAD_User, 3, 1) <> "=" THEN
        sAD_User = AD_SamAccountNameToFQDN(sAD_User) ' sAMACccountName provided
    END IF

    LOCAL oAD_User AS IADsUser
    oAD_User = AD_ObjGet("LDAP://" & sAD_HostServer & "/" & sAD_User, $IID_IADsUser) ' Retrieve the COM Object
    IF ISFALSE ISOBJECT(oAD_User) THEN
      SetError(5, "Creation of the COM object failed!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = 0
      EXIT FUNCTION
   END IF

    oAD_User.SetPassword(sAD_Password)
    IF iAD_Expired THEN oAD_User.Put("pwdLastSet", 0)
    oAD_User.SetInfo()
	 
	IF OBJRESULT <> 0 THEN
      FUNCTION = 0
	else
		function = 1	
   END IF

END FUNCTION   '==>AD_SetPassword


' #FUNCTION# ====================================================================================================================
' Name...........: AD_SamAccountNameToFQDN
' Description ...: Returns a Fully Qualified Domain Name (FQDN) from a SamAccountName.
' Syntax.........: AD_SamAccountNameToFQDN([$sAD_SamAccountName = @UserName])
' Parameters ....: $sAD_SamAccountName - Optional: Security Accounts Manager (SAM) account name (default = @UserName)
' Return values .: Success - Fully Qualified Domain Name (FQDN)
'                  Failure - "", sets @error to:
'                  |1 - No record returned from Active Directory. $sAD_SamAccountName not found
' Author ........: Jonathan Clelland
' Modified.......: water
' Remarks .......: A $ sign must be appended to the computer name to generate the FQDN for a sAMAccountName e.g. @ComputerName & "$".
'                  The function escapes the following special characters (# and /). Commas in CN= or OU= have to be escaped by you.
'                  If $sAD_SamAccountName is already a FQDN then the function returns $sAD_SamAccountName unchanged and without raising an error.
' Related .......: AD_FQDNToSamAccountName
' Link ..........:
' Example .......: Yes
' ===============================================================================================================================
'FUNCTION AD_SamAccountNameToFQDN(BYVAL sAD_SamAccountName AS WSTRING) AS WSTRING
FUNCTION AD_SamAccountNameToFQDN(BYREF sAD_SamAccountName AS WSTRINGz) AS WSTRING

    IF MID$(sAD_SamAccountName, 3, 1) = "=" THEN
        FUNCTION = sAD_SamAccountName ' already a FQDN. Return unchanged
        EXIT FUNCTION
    END IF

    oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_DNSDomain & ">;(sAMAccountName=" & sAD_SamAccountName & ");distinguishedName;subtree"
    LOCAL oAD_RecordSet AS ADORecordset
   oAD_RecordSet = oAD_Command.Execute ' Retrieve the ADsPath for the object, if it exists
    IF NOT ISOBJECT(oAD_RecordSet) OR oAD_RecordSet.RecordCount = 0 THEN
      SetError(1, "No record returned from Active Directory!")
      #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
      'MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
      FUNCTION = $$NUL
      EXIT FUNCTION
    END IF

    LOCAL fld AS ADOField
   fld = oAD_RecordSet.fields.Item(0)
   LOCAL sAD_LDAPEntry AS VARIANT
   sAD_LDAPEntry = fld.value

    LOCAL sAD_FQDN AS WSTRING
    sAD_FQDN = VARIANT$$(sAD_LDAPEntry)

    ' _AD_FixSpecialChars(sAD_FQDN, 0, "/#")

    FUNCTION = sAD_FQDN

END FUNCTION   '==>AD_SamAccountNameToFQDN



















FUNCTION ComputerName ALIAS "ComputerName" () EXPORT AS WSTRING

  LOCAL szTmp AS STRINGZ * %MAX_PATH
  LOCAL sTmp AS WSTRING

  HOST NAME TO sTmp
  sTmp = TRIM$(sTmp)

  IF LEN(sTmp) = 0 THEN
     GetComputerName szTmp, 48
     sTmp = szTmp
  END IF

  'Function = LCase$(sTmp)
  FUNCTION = sTmp + $$NUL

END FUNCTION


function UserNameOfCurrentThread alias "UserNameOfCurrentThread" () export as wstring

	local buff as wstringz*(512)
   GetUserNameW(buff, SIZEOF(buff))
   FUNCTION = buff

END FUNCTION



'FUNCTION ADs_AddUserToGroup (strComputer AS STRING, strUser AS STRING, strGroup AS STRING) AS LONG
'  LOCAL hr AS LONG
'  LOCAL colAccounts AS IADsContainer
'  LOCAL objGroup AS IADsGroup
'  LOCAL objUser AS IADsUser
'  LOCAL strADsPath AS STRING

'  strADsPath = UCODE$("WinNT://" & strComputer & "")
'  hr = ADsGetObject(BYVAL STRPTR(strADsPath), $IID_IADsContainer, colAccounts)
'  IF hr <> 0 THEN
'     FUNCTION = hr
'     EXIT FUNCTION
'  END IF

'  objUser = colAccounts.GetObject(UCODE$("user"), UCODE$(strUser))
'  IF ISFALSE(ISOBJECT(objUser)) THEN
'     FUNCTION = -1
'     EXIT FUNCTION
'  END IF

'  objGroup = colAccounts.GetObject(UCODE$("group"), UCODE$(strGroup))
'  IF ISFALSE(ISOBJECT(objGroup)) THEN
'     FUNCTION = -2
'     EXIT FUNCTION
'  END IF

'  IF objGroup.IsMember(objUser.ADsPath) = 0 THEN
'     objGroup.Add(objUser.ADsPath)
'  END IF

'  IF objGroup.IsMember(objUser.ADsPath) = -1 THEN
'     FUNCTION = 0
'  ELSE
'     FUNCTION = -3
'  END IF

'  SET objUser = NOTHING
'  SET objGroup = NOTHING
'  SET colAccounts = NOTHING

'END FUNCTION


'FUNCTION ADs_RemoveUserFromGroup (strComputer AS STRING, strUser AS STRING, strGroup AS STRING) AS LONG
'  LOCAL hr AS LONG
'  LOCAL colAccounts AS IADsContainer
'  LOCAL objGroup AS IADsGroup
'  LOCAL objUser AS IADsUser
'  LOCAL strADsPath AS STRING

'  strADsPath = UCODE$("WinNT://" & strComputer & "")
'  hr = ADsGetObject(BYVAL STRPTR(strADsPath), $IID_IADsContainer, colAccounts)
'  IF hr <> 0 THEN
'     FUNCTION = hr
'     EXIT FUNCTION
'  END IF

'  objUser = colAccounts.GetObject(UCODE$("user"), UCODE$(strUser))
'  IF ISFALSE(ISOBJECT(objUser)) THEN
'     FUNCTION = -1
'     EXIT FUNCTION
'  END IF

'  objGroup = colAccounts.GetObject(UCODE$("group"), UCODE$(strGroup))
'  IF ISFALSE(ISOBJECT(objGroup)) THEN
'     FUNCTION = -2
'     EXIT FUNCTION
'  END IF

'  IF objGroup.IsMember(objUser.ADsPath) = -1 THEN
'     objGroup.Remove(objUser.ADsPath)
'  END IF

'  IF objGroup.IsMember(objUser.ADsPath) = 0 THEN
'     FUNCTION = 0
'  ELSE
'     FUNCTION = -3
'  END IF

'  SET objUser = NOTHING
'  SET objGroup = NOTHING
'  SET colAccounts = NOTHING

'END FUNCTION


'FUNCTION ADs_DeleteUser (strComputer AS STRING, strUser AS STRING) AS LONG
'  LOCAL hr AS LONG
'  LOCAL objComp AS IADsContainer
'  LOCAL strADsPath AS STRING

'  strADsPath = UCODE$("WinNT://" & strComputer & "")
'  hr = ADsGetObject(BYVAL STRPTR(strADsPath), $IID_IADsContainer, objComp)
'  IF hr <> 0 THEN
'     FUNCTION = hr
'     EXIT FUNCTION
'  END IF

'  objComp.Delete (UCODE$("user"), UCODE$(strUser))

'  IF OBJRESULT = 0 THEN
'     FUNCTION = 0
'  ELSE
'     FUNCTION = -1
'  END IF

'  SET objComp = NOTHING

'END FUNCTION


'FUNCTION GenName (nLength AS INTEGER, nSymbol AS INTEGER) AS STRING
'  DIM i AS INTEGER, j AS INTEGER, iMax AS INTEGER
'  DIM s AS STRING, sSymbols AS STRING

'  sSymbols = "#!+- _="

'  IF nLength = 0 THEN nLength = 1
'  nLength = nLength + 3
'  iMax = nLength \ 3

'  RANDOMIZE TIMER

'  FOR i = 1 TO iMax
'     s = s & CHR$(RND(65, 90))
'     s = s & CHR$(RND(97, 122))
'     s = s & CHR$(RND(48, 57))
'     IF nSymbol > 0 THEN
'        j = RND(0, 12)
'        IF j > 0 AND j <= 6 THEN
'          s = s & MID$(sSymbols, j, 1)
'        END IF
'     END IF
'  NEXT

'  FUNCTION = LEFT$(s, nLength)
'END FUNCTION








'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'--------------------------------------------------------------------------------
'FUNCTION AD_GetObjectAttribute2 ALIAS "AD _GetObjectAttribute2" (BYVAL sAD_ObjectParam AS WSTRING, BYVAL sAD_AttributeParam AS WSTRING) EXPORT AS WSTRING

'    SetError(0, "")

'    DIM sAD_Object AS WSTRINGZ * 512
'    sAD_Object = sAD_ObjectParam
'    DIM sAD_Attribute AS WSTRINGZ * 256
'    sAD_Attribute = sAD_AttributeParam


'    DIM sAD_Property AS WSTRING
'    sAD_Property = "sAMAccountName"
'    IF MID$(sAD_Object, 3, 1) = "=" THEN
'        sAD_Property = "distinguishedName" ' FQDN provided
'    END IF

'    IF ISFALSE(AD_ObjectExists(sAD_Object, sAD_Property)) THEN
'        SetError(1, "Object '" & sAD_Object & "' does not exist!")
'        #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
'         MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
'         FUNCTION = ""
'         EXIT FUNCTION
'    END IF

'     oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_DNSDomain & ">;(" & sAD_Property & "=" & sAD_Object & ");ADsPath;subtree"
'    LOCAL oAD_RecordSet AS ADORecordset
'    oAD_RecordSet = oAD_Command.Execute ' Retrieve the ADsPath for the object

'    IF (NOT ISOBJECT(oAD_RecordSet)) OR (ISOBJECT(oAD_RecordSet) AND oAD_RecordSet.RecordCount < 1) THEN
'        SetError(2, "No object found for the specified property.")
'        FUNCTION = ""
'        EXIT FUNCTION
'    END IF

'    LOCAL fld AS ADOField
'     fld = oAD_RecordSet.fields.Item(0)
'    LOCAL sAD_LDAPEntry AS VARIANT
'    sAD_LDAPEntry = fld.value
'    'MsgBox "sAD_LDAPEntry: " & Variant$$(sAD_LDAPEntry)

'     LOCAL oAD_Object AS IADs
'     oAD_Object = AD_ObjGet(VARIANT$$(sAD_LDAPEntry))
'     'MsgBox  "ADsPath:" & oAD_Object.ADsPath


'     LOCAL variantValue AS VARIANT
'     variantValue = oAD_Object.Get(sAD_Attribute)
'     MSGBOX "VariantVT(variantValue): " & STR$(VARIANTVT(variantValue))

'     'MsgBox "Variant$$(variantValue): " & Str$( Variant#(variantValue) )


'    LOCAL result AS WSTRING
'     LOCAL variantType AS LONG
'     variantType = VARIANTVT(variantValue)
'     SELECT CASE variantType
'          CASE %VT_I4
'                result = STR$(VARIANT#(variantValue))

'          CASE %VT_BSTR
'                result = VARIANT$$(variantValue)

'          CASE %VT_DATE
'               LOCAL vbDate AS DOUBLE, ST AS SYSTEMTIME, FT AS FILETIME
'               vbDate = VARIANT#(variantValue)

'               IF VariantTimeToSystemTime(vbDate, ST) = %FALSE THEN
'               SetError(3, "Function 'VariantTimeToSystemTime' Call Failed!")
'               #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
'               MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
'               FUNCTION = ""
'               EXIT FUNCTION
'               END IF
'               IF SystemTimeToFileTime(ST, FT) = 0 THEN
'               SetError(3, "Function Call Failed!")
'               #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
'               MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
'               FUNCTION = ""
'               EXIT FUNCTION
'               END IF

'               LOCAL vTime AS IPOWERTIME
'               LET vTime = CLASS "PowerTime"
'                    vTime.FileTime = FT.qDateTime
'               vTime.ToLocalTime

'               result = vTime.DateString & " " & vTime.TimeStringFull

'          CASE %VT_DISPATCH
'                IF sAD_Attribute = "accountExpires" OR sAD_Attribute = "badPasswordTime" OR sAD_Attribute = "lastLogoff" OR sAD_Attribute = "lastLogon" _
'                     OR sAD_Attribute = "pwdLastSet" OR sAD_Attribute = "uSNChanged" OR sAD_Attribute = "uSNCreated" OR sAD_Attribute = "whenChanged" OR sAD_Attribute = "whenCreated" THEN

'                     LOCAL largeInteger AS DISPATCH
'                     largeInteger = variantValue

'                     DIM HiDate AS DOUBLE, LoDate AS DOUBLE
'                     OBJECT GET largeInteger.HighPart TO HiDate
'                     OBJECT GET largeInteger.LowPart TO LoDate
'                     IF HiDate = 0 OR LoDate = 0 THEN
'                          FUNCTION = ""
'                          EXIT FUNCTION
'                     END IF

''               DIM dDate AS DOUBLE
''               dDate = VAL( "&H" & HEX$(VARIANT#(HiDate)) & HEX$(VARIANT#(LoDate)) )
''               dDate = VARIANT#(HiDate) * (2^32) + VARIANT#(LoDate)

'                     DIM pFileTime AS FILETIME
'                     pFileTime.dwHighDateTime = HiDate
'                     pFileTime.dwLowDateTime = LoDate

'                     LOCAL vTime2 AS IPOWERTIME
'                     LET vTime2 = CLASS "PowerTime"
'                     vTime2.FileTime = pFileTime.qDateTime
'                     vTime2.ToLocalTime

'                     result = vTime2.DateString & " " & vTime2.TimeStringFull
'                ELSE'IF sAD_Attribute = "nTSecurityDescriptor" THEN
'                    SetError(4, "Returned value is not supported type!")
'                    #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
'                    MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
'                    FUNCTION = ""
'                    EXIT FUNCTION
'                END IF


'          'case
'          'case
'          'case

'          CASE ELSE
'                SetError(4, "Returned value is not supported type!")
'                #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
'                MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
'                FUNCTION = ""
'                EXIT FUNCTION
'     END SELECT


''----------------------------
''  LOCAL valuesArray AS IPOWERARRAY
''  ' Create the PowerArray object
''  valuesArray = CLASS "PowerArray"
''  IF ISFALSE(ISOBJECT(valuesArray)) THEN
''    ? "Failed to create PowerArray object"
''      FUNCTION = ""
''      EXIT FUNCTION
''  END IF
''  ' Copy the variant to the PowerArray
''  valuesArray.CopyFromVariant(variantValue)

''  LOCAL lb AS LONG, ub AS LONG , subscripts AS LONG
''  lb = valuesArray.LBound(1)
''  ub = valuesArray.UBound(1)
''  subscripts = valuesArray.SUBSCRIPTS()
''  MSGBOX "SUBSCRIPTS:" & STR$(SUBSCRIPTS) & "  LBOUND: " & STR$(lb ) & "  UBound: " & STR$(ub )


''  DIM a AS VARIANT
''     DIM n AS LONG
''  FOR n = lb TO ub
''       valuesArray.ValueGet(BYREF a, n)
''     MSGBOX "VariantVT:" & STR$(VARIANTVT(a)) & "       " &  STR$( VARIANT#(a) )

''  NEXT n
''-----------------------------



'    'result =  Variant$$(value)
'    MSGBOX "result: " & result & "  size: " & STR$(LEN(result))
'    FUNCTION = result

'END FUNCTION   '==>AD_GetObjectAttribute2
'--------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------------------
'TYPE SAMAccountType
'     typeName AS WSTRINGZ * 256
'     typeValue AS LONG
'END TYPE

'TYPE AD_UAC
'     uacValue AS LONG
'     uacName AS WSTRINGZ * 256
'END TYPE
'' #FUNCTION# ====================================================================================================================
'' Name...........: AD_GetObjectProperties
'' Description ...: Returns a two-dimensional array of all or selected properties and their values of an object in readable form.
'' Syntax.........: _AD_GetObjectProperties(sAD_ObjectParam, sAD_PropertiesParam)
'' Parameters ....: sAD_ObjectParam -  SamAccountName or FQDN of the object properties from (e.g. computer, user, group ...) (default = @Username)
''                  |Can be of type object as well. Useful to get properties for a schema or configuration object (see _AD_ListRootDSEAttributes)
''                  sAD_PropertiesParam -  Comma separated list of properties to return (default = "" = return all properties)
'' Return values .: Success - Returns a two-dimensional array with all properties and their values of an object in readable form
''                  Failure - "" or property name, sets @error to:
''                  |1 - sAD_ObjectParam could not be found
''                  |2 - No values for the specified property. The property in error is returned as the function result
'' Author ........:
'' Modified.......:
'' Remarks .......: Dates are returned in format: YYYY/MM/DD HH:MM:SS local time of the calling user (AD stores all dates in UTC - Universal Time Coordinated)
''                  Exception: AD internal dates like "whenCreated", "whenChanged" and "dSCorePropagationData". They are returned as UTC
''                  NT Security Descriptors are returned as: Control:nn, Group:Domain\Group, Owner:Domain\Group, Revision:nn
''                  No error is returned if there are properties in sAD_PropertiesParam that are not available for the selected object
''+
''                  Properties are returned in alphabetical order. If sAD_PropertiesParam is set to "samaccountname,displayname" the returned array will contain
''                  displayname as the first and samaccountname as the second row.
'' Related .......:
'' Link ..........: http://www.autoitscript.com/forum/index.php?showtopic=49627&view=findpost&p=422402, http://msdn.microsoft.com/en-us/library/ms675090(VS.85).aspx
'' Example .......:
'' ===============================================================================================================================
'FUNCTION AD_GetObjectProperties ALIAS "AD _GetObjectProperties" (BYVAL sAD_ObjectParam AS WSTRING, BYVAL sAD_PropertiesParam AS WSTRING) EXPORT AS LONG
''Function AD_GetObjectProperties Alias "AD_GetObjectProperties" (ByVal sAD_ObjectParam As wString, ByVal sAD_PropertiesParam As wString, , byref sAD_ObjectPropertiesParam() as wstring) Export As IPowerArray


'    SetError(0, "")

'    DIM sAD_Object AS WSTRINGZ * 512
'    sAD_Object = sAD_ObjectParam
'    DIM sAD_Properties AS WSTRINGZ * 256
'    sAD_Properties = sAD_PropertiesParam


'    'Local $aAD_ObjectProperties[1][2], $oAD_Object
'      DIM aAD_ObjectProperties(1, 2) AS WSTRING
'     LOCAL oAD_Object AS IADsPropertyList

'    'Local $oAD_Item, $oAD_PropertyEntry, $oAD_Value, $iCount3, $xAD_Dummy
'     ' Data Type Mapping between Active Directory and LDAP
'     ' http://msdn.microsoft.com/en-us/library/aa772375(VS.85).aspx

'    LOCAL oAD_Item AS IADsPropertyEntry 'IADsPropertyEntry
'     LOCAL oAD_Value AS IDISPATCH '
'    LOCAL oAD_PropertyEntry AS IADsPropertyEntry 'IADsPropertyList

'    LOCAL xAD_Dummy AS VARIANT


''    Local Const %ADSTYPE_DN_STRING = 1
''    Local Const %ADSTYPE_CASE_IGNORE_STRING = 3
''    Local Const %ADSTYPE_BOOLEAN = 6
''    Local Const %ADSTYPE_INTEGER = 7
''    Local Const %ADSTYPE_OCTET_STRING = 8
''    Local Const %ADSTYPE_UTC_TIME = 9
''    Local Const %ADSTYPE_LARGE_INTEGER = 10
''    Local Const %ADSTYPE_NT_SECURITY_DESCRIPTOR = 25
''    Local Const %ADSTYPE_UNKNOWN = 26

''    dim aAD_SAMAccountType as IPowerArray
''    aAD_SAMAccountType = Class "PowerArray"
''    aAD_SAMAccountType.VALUESET(byref "DOMAIN _OBJECT", 0, 0)
''    aAD_SAMAccountType.VALUESET(byref 0x0#, 0, 1)

''   dim aAD_UAC as IPowerArray
''   aAD_UAC = Class "PowerArray"
''   aAD_UAC.VALUESET(0x00000001#, 0, 0)
''   aAD_UAC.VALUESET("SCRIPT", 0, 1)



'     DIM aAD_SAMAccountType(12) AS SAMAccountType
'     DIM aAD_UAC(21) AS AD_UAC

''     aAD_SAMAccountType[12][2] = [["DOMAIN _OBJECT", 0x0], ["GROUP _OBJECT", 0x10000000], ["NON _SECURITY _GROUP _OBJECT", 0x10000001], _
''            ["ALIAS _OBJECT", 0x20000000], ["NON _SECURITY _ALIAS _OBJECT", 0x20000001], ["USER _OBJECT", 0x30000000], ["NORMAL _USER _ACCOUNT", 0x30000000], _
''            ["MACHINE _ACCOUNT", 0x30000001], ["TRUST _ACCOUNT", 0x30000002], ["APP _BASIC _GROUP", 0x40000000], ["APP _QUERY _GROUP", 0x40000001], _
''            ["ACCOUNT _TYPE _MAX", 0x7fffffff]]

''    Local aAD_UAC[21][2] = [[0x00000001, "SCRIPT"], [0x00000002, "ACCOUNTDISABLE"], [0x00000008, "HOMEDIR _REQUIRED"], [0x00000010, "LOCKOUT"], [0x00000020, "PASSWD _NOTREQD"], _
''            [0x00000040, "PASSWD _CANT _CHANGE"], [0x00000080, "ENCRYPTED _TEXT _PASSWORD _ALLOWED"], [0x00000100, "TEMP _DUPLICATE _ACCOUNT"], [0x00000200, "NORMAL _ACCOUNT"], _
''            [0x00000800, "INTERDOMAIN _TRUST _ACCOUNT"], [0x00001000, "WORKSTATION _TRUST _ACCOUNT"], [0x00002000, "SERVER _TRUST _ACCOUNT"], [0x00010000, "DONT _EXPIRE _PASSWD"], _
''            [0x00020000, "MNS _LOGON _ACCOUNT"], [0x00040000, "SMARTCARD _REQUIRED"], [0x00080000, "TRUSTED _FOR _DELEGATION"], [0x00100000, "NOT _DELEGATED"], _
''            [0x00200000, "USE _DES _KEY _ONLY"], [0x00400000, "DONT _REQUIRE _PREAUTH"], [0x00800000, "PASSWORD _EXPIRED"], [0x01000000, "TRUSTED _TO _AUTHENTICATE _FOR _DELEGATION"]]
'          REPLACE " " WITH "" IN sAD_Properties
'     sAD_Properties = "," & sAD_Properties & ","
''''    If Not IsObject(sAD_Object) Then
'          IF AD_ObjectExists(sAD_Object, "") = 0 THEN
'                     SetError(1, "Object '" & sAD_Object & "' could not be found!")
'                     #DEBUG PRINT "Error! " & sAD_LastErrorString & " Function:" & FUNCNAME$
'                     MSGBOX sAD_LastErrorString, %MB_ICONERROR, "Error"
'                     FUNCTION = 0
'                     EXIT FUNCTION
'          END IF

'          LOCAL sAD_Property AS WSTRING
'          sAD_Property = "sAMAccountName"
'          IF MID$(sAD_Object, 3, 1) = "=" THEN sAD_Property = "distinguishedName" ' FQDN provided
'          oAD_Command.CommandText = "<LDAP://" & sAD_HostServer & "/" & sAD_DNSDomain & ">;(" & sAD_Property & "=" & sAD_Object & ");ADsPath;subtree"
'          LOCAL oAD_RecordSet AS ADORecordset
'          oAD_RecordSet = oAD_Command.Execute ' Retrieve the ADsPath for the object
'            LOCAL fld AS ADOField
'            fld = oAD_RecordSet.fields.Item(0)
'            LOCAL sAD_LDAPEntry AS VARIANT
'            sAD_LDAPEntry = fld.value

'          'Local sAD_LDAPEntry = oAD_RecordSet.fields(0).value
'          oAD_Object = AD_ObjGet(VARIANT$$(sAD_LDAPEntry), $IID_IADsPropertyList) ' Retrieve the COM Object
'          MSGBOX "sAD _LDAPEntry: " & VARIANT$$(sAD_LDAPEntry)
''''    Else
''''        oAD_Object = sAD_Object
''''   End If

'        LOCAL oAD_ADSObject AS IADs, iid AS GUID, hr AS LONG
'        iid = $IID_IADs
'        hr = oAD_Object.QueryInterface(BYREF iid, BYREF oAD_ADSObject)
'        IF FAILED(hr) THEN
'            SetError(3, OBJRESULT$(hr))
'            #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
'            MSGBOX OBJRESULT$(hr) & $CRLF & "Code: &H" & HEX$(hr, 8), %MB_ICONERROR, "ADSI Error"
'                FUNCTION = 0
'                EXIT FUNCTION
'        END IF

'        oAD_ADSObject.GetInfo()

'     'oAD_Object.PurgePropertyList()
'     LOCAL iCount1 AS LONG, iCount2 AS LONG, iCount3 AS LONG
'     iCount1 = oAD_Object.PropertyCount()
'      MSGBOX "PropertyCount:" & STR$(iCount1)

'    LOCAL oAD_TempVariant AS VARIANT

'    FOR iCount2 = 0 TO iCount1 - 1
'        oAD_TempVariant = oAD_Object.Item(iCount2)
'        oAD_Item = oAD_TempVariant
'        IF NOT (sAD_Properties = ",," OR VERIFY(sAD_Properties, "," & oAD_Item.Name & ",") > 0) THEN ITERATE
'        oAD_TempVariant = oAD_Object.GetPropertyItem(oAD_Item.Name, %ADSTYPE_UNKNOWN)
'          oAD_PropertyEntry = oAD_TempVariant
'        IF NOT ISOBJECT(oAD_PropertyEntry) THEN
'               SetError(2, "No values for the specified property '" & oAD_Item.Name & "'.")
'            #DEBUG PRINT "Error! " & OBJRESULT$(hr) & " Code:" & STR$(hr) & " Function:" & FUNCNAME$
'            MSGBOX OBJRESULT$(hr) & $CRLF & "Code: &H" & HEX$(hr, 8), %MB_ICONERROR, "ADSI Error"
'                FUNCTION = 0
'                EXIT FUNCTION
'        ELSE

'            LOCAL  propertyEntryValues AS IPOWERARRAY
'            propertyEntryValues = CLASS "PowerArray"
'            propertyEntryValues.CopyFromVariant(oAD_PropertyEntry.Values)
'            LOCAL lb AS LONG, ub AS LONG
'            lb = propertyEntryValues.LBound(1)
'            ub = propertyEntryValues.UBound(1)
'            'MSGBOX "oAD_Item.Name: " & oAD_Item.Name & "    " & " LBOUND: " & STR$(propertyEntryValues.LBound(1) ) & "    " & "UBound: " & STR$(propertyEntryValues.UBound(1) )

'                LOCAL vAD_PropertyValue AS IADsPropertyValue
'                LOCAL valueIndex AS LONG
'            FOR valueIndex = lb TO ub
'                    REDIM aAD_ObjectProperties(UBOUND(aAD_ObjectProperties, 1) + 1, 2)
'                    iCount3 = UBOUND(aAD_ObjectProperties, 1) - 1
'                    aAD_ObjectProperties(iCount3, 0) = oAD_Item.Name

'                    propertyEntryValues.VALUEGET(BYREF oAD_TempVariant, valueIndex)
'                    vAD_PropertyValue = oAD_TempVariant
'                    MSGBOX "ADsType: " & STR$(vAD_PropertyValue.ADsType) & "    " & vAD_PropertyValue.CaseIgnoreString




'                NEXT


''            For each vAD_PropertyValue In oAD_PropertyEntry.Values
''                ReDim aAD_ObjectProperties[UBound(aAD_ObjectProperties, 1) + 1][2]
''                iCount3 = UBound(aAD_ObjectProperties, 1) - 1
''                aAD_ObjectProperties[iCount3][0] = oAD_Item.Name
''                If oAD_Item.ADsType = %ADSTYPE_CASE_IGNORE_STRING Then
''                    aAD_ObjectProperties[iCount3][1] = vAD_PropertyValue.CaseIgnoreString
''                ElseIf oAD_Item.ADsType = %ADSTYPE_INTEGER Then
''                    If oAD_Item.Name = "sAMAccountType" Then
''                        For iCount4 = 0 To 11
''                            If vAD_PropertyValue.Integer = aAD_SAMAccountType[iCount4][1] Then
''                                aAD_ObjectProperties[iCount3][1] = aAD_SAMAccountType[iCount4][0]
''                                Exit For
''                            End If
''                        Next
''                    ElseIf oAD_Item.Name = "userAccountControl" Then
''                        aAD_ObjectProperties[iCount3][1] = vAD_PropertyValue.Integer & " = "
''                        For iCount4 = 0 To 20
''                            If BitAND(vAD_PropertyValue.Integer, aAD_UAC[iCount4][0]) = aAD_UAC[iCount4][0] Then
''                                aAD_ObjectProperties[iCount3][1] & = aAD_UAC[iCount4][1] & " - "
''                            End If
''                        Next
''                        If StringRight(aAD_ObjectProperties[iCount3][1], 3) = " - " Then aAD_ObjectProperties[iCount3][1] = StringTrimRight(aAD_ObjectProperties[iCount3][1], 3)
''                    Else
''                        aAD_ObjectProperties[iCount3][1] = vAD_PropertyValue.Integer
''                    End If
''                ElseIf oAD_Item.ADsType = %ADSTYPE_LARGE_INTEGER Then
''                    If oAD_Item.Name = "pwdLastSet" Or oAD_Item.Name = "accountExpires" Or oAD_Item.Name = "lastLogonTimestamp" Or oAD_Item.Name = "badPasswordTime" Or oAD_Item.Name = "lastLogon" Or oAD_Item.Name = "lockoutTime" Then
''                        If vAD_PropertyValue.LargeInteger.LowPart = 0 And vAD_PropertyValue.LargeInteger.HighPart = 0 Then
''                            aAD_ObjectProperties[iCount3][1] = "1601/01/01 00:00:00"
''                        Else
''                            Local sAD_Temp = DllStructCreate("dword low;dword high")
''                            DllStructSetData(sAD_Temp, "Low", vAD_PropertyValue.LargeInteger.LowPart)
''                            DllStructSetData(sAD_Temp, "High", vAD_PropertyValue.LargeInteger.HighPart)
''                            Local sAD_Temp2 = _Date_Time_FileTimeToSystemTime(DllStructGetPtr(sAD_Temp))
''                            Local sAD_Temp3 = _Date_Time_SystemTimeToTzSpecificLocalTime(DllStructGetPtr(sAD_Temp2))
''                            aAD_ObjectProperties[iCount3][1] = _Date_Time_SystemTimeToDateTimeStr(sAD_Temp3, 1)
''                        End If
''                    Else
''                        aAD_ObjectProperties[iCount3][1] = __AD_LargeInt2Double(vAD_PropertyValue.LargeInteger.LowPart, vAD_PropertyValue.LargeInteger.HighPart)
''                    End If
''                ElseIf oAD_Item.ADsType = %ADSTYPE_OCTET_STRING Then
''                    xAD_Dummy = DllStructCreate("byte[56]")
''                    DllStructSetData(xAD_Dummy, 1, vAD_PropertyValue.OctetString)
''                    ' objectSID etc. See: http://msdn.microsoft.com/en-us/library/aa379597(VS.85).aspx
''                    ' objectGUID etc. See: http://www.autoitscript.com/forum/index.php?showtopic=106163&view=findpost&p=767558
''                    If _Security__IsValidSid(DllStructGetPtr(xAD_Dummy)) Then
''                        aAD_ObjectProperties[iCount3][1] = _Security__SidToStringSid(DllStructGetPtr(xAD_Dummy)) ' SID
''                    Else
''                        aAD_ObjectProperties[iCount3][1] = _WinAPI_StringFromGUID(DllStructGetPtr(xAD_Dummy)) ' GUID
''                    End If
''                ElseIf oAD_Item.ADsType = %ADSTYPE_DN_STRING Then
''                    aAD_ObjectProperties[iCount3][1] = vAD_PropertyValue.DNString
''                ElseIf oAD_Item.ADsType = %ADSTYPE_UTC_TIME Then
''                    Local iAD_DateTime = vAD_PropertyValue.UTCTime
''                    aAD_ObjectProperties[iCount3][1] = StringLeft(iAD_DateTime, 4) & "/" & StringMid(iAD_DateTime, 5, 2) & "/" & StringMid(iAD_DateTime, 7, 2) & _
''                            " " & StringMid(iAD_DateTime, 9, 2) & ":" & StringMid(iAD_DateTime, 11, 2) & ":" & StringMid(iAD_DateTime, 13, 2)
''                ElseIf oAD_Item.ADsType = %ADSTYPE_BOOLEAN Then
''                    If vAD_PropertyValue.Boolean = 0 Then
''                        aAD_ObjectProperties[iCount3][1] = "False"
''                    Else
''                        aAD_ObjectProperties[iCount3][1] = "True"
''                    End If
''                ElseIf oAD_Item.ADsType = %ADSTYPE_NT_SECURITY_DESCRIPTOR Then
''                    oAD_Value = vAD_PropertyValue.SecurityDescriptor
''                    aAD_ObjectProperties[iCount3][1] = "Control:" & oAD_Value.Control & ", " & _
''                            "Group:" & oAD_Value.Group & ", " & _
''                            "Owner:" & oAD_Value.Owner & ", " & _
''                            "Revision:" & oAD_Value.Revision
''                Else
''                    aAD_ObjectProperties[iCount3][1] = "Has the unknown ADsType: " & oAD_Item.ADsType
''                End If
''            Next


'        END IF
'    NEXT

''    aAD_ObjectProperties[0][0] = UBound(aAD_ObjectProperties, 1) - 1
''    aAD_ObjectProperties[0][1] = UBound(aAD_ObjectProperties, 2)
''    _ArraySort(aAD_ObjectProperties, 0, 1)

'     FUNCTION = 1

'END FUNCTION   '==>AD_GetObjectProperties
'----------------------------------------------------------------------------------------------------------------


