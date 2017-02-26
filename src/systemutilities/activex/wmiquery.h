#ifndef WMIQUERY_H
#define WMIQUERY_H

#include <QObject>
#include "../systemutilitieslib.h"

#include <Windows.h>



class IWbemLocator;


namespace HEHUI
{


class SYSUTIL_LIB_API WMIQuery : public QObject
{
    Q_OBJECT
public:
    explicit WMIQuery(QObject *parent = 0);
    ~WMIQuery();

    bool isNull();

    QList<QVariantList> queryValues(const QString &queryString, const QString &propertiesToRetrieve,
                                    const QString &wmiNamespace = "root/CIMV2", DWORD *errorCode = 0,
                                    const QString &server = "."
                                   );



private:

    bool m_isNull;

    IWbemLocator *m_pLocator;
    CRITICAL_SECTION m_csLock;


};

} //namespace HEHUI

#endif // WMIQUERY_H
