#include "wmiquery.h"

#include <QVariant>
#include <QDebug>


#include <comdef.h>
#include <Wbemidl.h>

#include "activex/qaxtypes.h"
#include "winutilities.h"

# pragma comment(lib, "wbemuuid.lib")

namespace HEHUI {


WMIQuery::WMIQuery(QObject *parent) : QObject(parent)
{

    m_isNull = true;
    m_pLocator = NULL;


    HRESULT hr = S_OK;
    hr = CoInitializeEx(0,COINIT_MULTITHREADED);

    hr =  CoInitializeSecurity(
                NULL,
                -1,                          // COM authentication
                NULL,                        // Authentication services
                NULL,                        // Reserved
                RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
                RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
                NULL,                        // Authentication info
                EOAC_NONE,                   // Additional capabilities
                NULL                         // Reserved
                );


    if ((S_OK != hr) && (RPC_E_TOO_LATE !=hr))
    {
        CoUninitialize();
        qCritical()<<"ERROR! Failed to initialize security."<<hr<<" "<<WinUtilities::WinSysErrorMsg(hr);

        return ;
    }

    //// initial locator to WMI
    hr = CoCreateInstance(
                CLSID_WbemLocator,
                0,
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, (LPVOID *) &m_pLocator);

    if (FAILED(hr))
    {
        CoUninitialize();
        qCritical()<<"ERROR! Failed to create IWbemLocator object."<<hr<<" "<<WinUtilities::WinSysErrorMsg(hr);

        return ;
    }


    ///
    InitializeCriticalSection(&m_csLock);

    m_isNull = false;

}

WMIQuery::~WMIQuery()
{

    if (m_pLocator)
    {
        m_pLocator->Release();
    }
    CoUninitialize();

    DeleteCriticalSection(&m_csLock);

}

bool WMIQuery::isNull(){
    return m_isNull;
}

QList<QVariantList> WMIQuery::queryValues(const QString &queryString, const QString &propertiesToRetrieve,
                              const QString &wmiNamespace, const QString &server){

    QList<QVariantList> resultList;
    if(m_isNull || propertiesToRetrieve.trimmed().isEmpty()){
        return resultList;
    }
    QStringList properties = propertiesToRetrieve.split(",");

    HRESULT hr = S_OK;
    // Connect to WMI through the IWbemLocator::ConnectServer method
    QString strNetworkResource = "//" + server + "/" + wmiNamespace;
    IWbemServices *pSvc = NULL;

    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hr = m_pLocator->ConnectServer(
        QStringToBSTR(strNetworkResource), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (e.g. Kerberos)
        0,                       // Context object
        &pSvc                    // pointer to IWbemServices proxy
        );

    if (FAILED(hr))
    {
        qCritical()<<"ERROR! ConnectServer failed. "<<hr<<" "<<WinUtilities::WinSysErrorMsg(hr);

        return resultList;
    }

    // Set security levels on the proxy

    hr = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
        );

    if (FAILED(hr))
    {
        pSvc->Release();
        qCritical()<<"ERROR! Could not set proxy blanket. "<<hr<<" "<<WinUtilities::WinSysErrorMsg(hr);

        return resultList;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;

//		bstr_t("WQL"),
//		bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),
//		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,

    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        QStringToBSTR(queryString),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hr))
    {
        pSvc->Release();
        qCritical()<<"ERROR! Query for operating system name failed. "<<hr<<" "<<WinUtilities::WinSysErrorMsg(hr);

        return resultList;
    }

    // Get the data from the query
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;
        VariantInit(&vtProp);

        QVariantList propertyValueList;
        foreach (QString property, properties) {
            // Get the value of the VendorSpecific property
            hr = pclsObj->Get(property.toStdWString().c_str(), 0, &vtProp, 0, 0);
            propertyValueList.append(VARIANTToQVariant(vtProp, 0));
            VariantClear(&vtProp);
        }
        resultList.append(propertyValueList);

    }

    // Cleanup
    pSvc->Release();
    pEnumerator->Release();
    pclsObj->Release();

    return resultList;
}




} //namespace HEHUI
