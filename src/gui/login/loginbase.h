/*
 ****************************************************************************
 * loginbase.h
 *
 * Created on: 2008-10-17
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
 * Last Modified on: 2010-05-08
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#ifndef LOGINBASE_H_
#define LOGINBASE_H_

#include <QObject>


//#include "../../core/user.h"
//#include "../../core/global_core.h"
#include "HHSharedCore/huser.h"
#include "HHSharedCore/hglobal_core.h"

#include "../guilib.h"

namespace HEHUI {

class GUI_LIB_API LoginBase: public QObject {
    Q_OBJECT

public:
    LoginBase(User *user, QObject *parent = 0);
    LoginBase(User *user, const QString &windowTitle = "", QObject *parent = 0);
    virtual ~LoginBase();

    virtual bool isVerified();

    QString getUserID() const;
    //        QString userName() const;
    QString passWord() const;

    QWidget* getParentWidget();
    //	User* getUser();


    void setDatabaseOptions(const QString &connectionName,
                             const QString &driver, const QString &host, int port,
                             const QString &user, const QString &passwd,
                             const QString &databaseName, HEHUI::DatabaseType databaseType);


    QString connectionName() const;
    QString driverName() const;
    QString databaseName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    quint16 port() const;
    HEHUI::DatabaseType databaseType() const;
    bool isSettingsModified();


private slots:
    virtual void modifySettings();

private:
    void initUI(QObject *parent);

    virtual bool verifyUser();
    virtual bool getUserInfo();
    virtual bool canLogin();

    void setUser(User *u);

private:
    QWidget *parentWidget;
    User *user;
    bool isSuccesseful;
    QString m_windowTitle;

    QString m_connectionName;
    QString m_driver;
    QString m_host;
    quint16 m_port;
    QString m_user;
    QString m_passwd;
    QString m_databaseName;
    HEHUI::DatabaseType m_databaseType;

    bool m_settingsModified;

};

} //namespace HEHUI

#endif /* LOGINBASE_H_ */
