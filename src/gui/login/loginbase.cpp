/*
 ****************************************************************************
 * loginbase.cpp
 *
 * Created on: 2008-10-17
 *     Author: 贺辉
 *    License: LGPL
 *    Comment:
 *
 *
 *    =============================  Usage  =============================
 *|
        User *user = new User();
        LoginBase *login = new LoginBase(user);
        if (!login->isVerified()) {
                return ;
        }
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



#include <QtSql>
#include <QtGui>
#include <QString>
#include <QMessageBox>


#include "loginbase.h"
#include "logindlg.h"
#include "../databaseconnecter/databaseconnecter.h"



//SQLite
#ifndef LOCAL_CONFIG_DB_CONNECTION_NAME
#define LOCAL_CONFIG_DB_CONNECTION_NAME	"LOCAL_CONFIG_DB"
#endif

#ifndef LOCAL_CONFIG_DB_NAME
#define LOCAL_CONFIG_DB_NAME	".config.db"
#endif

#ifndef LOCAL_CONFIG_DB_DRIVER
#define LOCAL_CONFIG_DB_DRIVER	"QSQLITE"
#endif






namespace HEHUI {

LoginBase::LoginBase(User *user, QObject *parent)
    :QObject(parent), user(user)
{
    m_windowTitle = tr("Login");

    qDebug("----LoginBase::LoginBase(User *user, QObject *parent)");
    Q_ASSERT_X(user != NULL, "LoginBase::LoginBase(User *user, QObject *parent)", " 'user' is NULL!");


//    if(parent && parent->isWidgetType()){
//        this->parentWidget = qobject_cast<QWidget *>(parent);
//    }else{
//        this->parentWidget = 0;
//    }
//    isSuccesseful = false;

//    m_connectionName = LOCAL_CONFIG_DB_CONNECTION_NAME;
//    m_driver = LOCAL_CONFIG_DB_DRIVER;
//    m_host = "";
//    m_port = 0;
//    m_user = "";
//    m_passwd = "";
//    m_databaseName = LOCAL_CONFIG_DB_NAME;
//    m_databaseType = HEHUI::SQLITE;

    initUI(parent);
}

LoginBase::LoginBase(User *user, const QString &windowTitle, QObject *parent)
    :QObject(parent), user(user), m_windowTitle(windowTitle)
{
    qDebug("----LoginBase::LoginBase(User *user, QObject *parent)");
    Q_ASSERT_X(user != NULL, "LoginBase::LoginBase(User *user, QObject *parent)", " 'user' is NULL!");

//    isSuccesseful = false;

//    if(parent && parent->isWidgetType()){
//        this->parentWidget = qobject_cast<QWidget *>(parent);
//    }else{
//        this->parentWidget = 0;
//    }

//    m_connectionName = LOCAL_CONFIG_DB_CONNECTION_NAME;
//    m_driver = LOCAL_CONFIG_DB_DRIVER;
//    m_host = "";
//    m_port = 0;
//    m_user = "";
//    m_passwd = "";
//    m_databaseName = LOCAL_CONFIG_DB_NAME;
//    m_databaseType = HEHUI::SQLITE;

    initUI(parent);

}

LoginBase::~LoginBase() {

    //LoginDlg::instance()->deleteLater();

    //    delete loginDlg;
    //    loginDlg = 0;

}

inline QString LoginBase::getUserID() const {
    return user->getUserID();
}

//inline QString LoginBase::userName() const{
//    return user->getUserName();
//}

inline QString LoginBase::passWord() const {
    return user->getPassword();
}

QWidget* LoginBase::getParentWidget()
{
    return this->parentWidget;
}

void LoginBase::setDatabaseOptions(const QString &connectionName,
                         const QString &driver, const QString &host, int port,
                         const QString &user, const QString &passwd,
                         const QString &databaseName, HEHUI::DatabaseType databaseType){



    m_connectionName = connectionName;
    m_driver = driver;
    m_host = host;
    m_port = port;
    m_user = user;
    m_passwd = passwd;
    m_databaseName = databaseName;
    m_databaseType = databaseType;

}


QString LoginBase::connectionName() const{
    return m_connectionName;
}

QString LoginBase::driverName() const{
    return m_driver;
}

QString LoginBase::databaseName() const{
    return m_databaseName;
}

QString LoginBase::userName() const{
    return m_user;
}

QString LoginBase::password() const{
    return m_passwd;
}

QString LoginBase::hostName() const{
    return m_host;
}

quint16 LoginBase::port() const{
    return m_port;
}

HEHUI::DatabaseType LoginBase::databaseType() const{
    return m_databaseType;
}

bool LoginBase::isSettingsModified(){
    return m_settingsModified;
}

bool LoginBase::saveSettings(){
    return m_saveSettings;
}

inline void LoginBase::setUser(User *u){
    user = u;
}

//User* LoginBase::getUser(){
//	return this->user;
//}

void LoginBase::modifySettings(){

    DatabaseConnecterDialog dbConnecterDlg(
                m_connectionName,
                m_host,
                m_port,
                "",
                "",
                m_databaseName,
                m_databaseType,
                parentWidget
                );
    dbConnecterDlg.showSaveSettingsOption(true);
    QStringList parameters = dbConnecterDlg.getParameters();
    if (parameters.size() <= 0) {
        return;
    }

    m_connectionName = parameters.at(0);
    m_driver = parameters.at(1);
    m_host = parameters.at(2);
    m_port = parameters.at(3).toUShort();
    m_user = parameters.at(4);
    m_passwd = parameters.at(5);
    m_databaseName = parameters.at(6);
    m_databaseType = (HEHUI::DatabaseType) parameters.at(7).toUInt();

    m_settingsModified = true;
    m_saveSettings = dbConnecterDlg.saveSettings();

}

void LoginBase::initUI(QObject *parent){

    if(parent && parent->isWidgetType()){
        this->parentWidget = qobject_cast<QWidget *>(parent);
    }else{
        this->parentWidget = 0;
    }

    isSuccesseful = false;

    m_connectionName = LOCAL_CONFIG_DB_CONNECTION_NAME;
    m_driver = LOCAL_CONFIG_DB_DRIVER;
    m_host = "";
    m_port = 0;
    m_user = "";
    m_passwd = "";
    m_databaseName = LOCAL_CONFIG_DB_NAME;
    m_databaseType = HEHUI::SQLITE;

    m_settingsModified = false;
    m_saveSettings = false;

}

bool LoginBase::verifyUser() {
    qDebug("----LoginBase::verifyUser()");

    DatabaseConnecter dc(parentWidget);
    if(!dc.isDatabaseOpened(
                m_connectionName,
                m_driver,
                m_host,
                m_port,
                m_user,
                m_passwd,
                m_databaseName,
                m_databaseType
                )){

        qCritical() << QString("Error! Database Connection Failed! Authentication Failed!");
        return false;
    }

    QSqlDatabase db;
    db = QSqlDatabase::database(m_connectionName);
    QSqlQueryModel model(this);

    int latestVersion = 141225;

    //初始化数据库
    //Init the database
    if(!db.tables().contains("version", Qt::CaseInsensitive)){
        QString createTableStatement = QString("CREATE TABLE `versioninfo` ( `LatestVersion` int NOT NULL PRIMARY KEY, `Remark` varchar(255) )");
        //QString insertStatement = QString("INSERT INTO `versioninfo` VALUES (%1,'')").arg(latestVersion);
        //QString dropString = "DROP TABLE IF EXISTS `systemadministrators`;";
        model.setQuery(QSqlQuery(createTableStatement, db));
        //model.setQuery(QSqlQuery(insertStatement, db));
        //model.setQuery(QSqlQuery(dropString, db));

        model.clear();
    }

    QString string = QString("select LatestVersion from versioninfo where LatestVersion = %1").arg(latestVersion);
    QSqlQuery query(string, db);
    if(query.exec() && !query.first()){
        string = "DROP TABLE IF EXISTS `systemadministrators`;";
        QString insertStatement = QString("INSERT INTO `versioninfo` VALUES (%1,'')").arg(latestVersion);
        if(query.exec(string)){
            query.exec(insertStatement);
        }
    }

    if(!db.tables().contains("systemadministrators", Qt::CaseInsensitive)){
        QString createTableStatement = QString("CREATE TABLE `systemadministrators` ( `UserID` varchar(24) NOT NULL PRIMARY KEY,  `UserName` varchar(24) NOT NULL,  `BusinessAddress` varchar(16) NOT NULL,  `PassWD` varchar(56) NOT NULL ,  `LastLoginTime` timestamp NOT NULL ,  `Remark` varchar(255) )");
        QString insertStatement = QString("INSERT INTO `systemadministrators` VALUES ('hehui','','DG','KlcsSsfmfp6B3ya+LliE2bHO2uc=','','')");
        QString insertStatement2 = QString("INSERT INTO `systemadministrators` VALUES ('admindg','','DG','c8HdVaYFOAXZ2oYjM4L6gqTIiFk=','','')"); //admin.dg.2015
        QString insertStatement3 = QString("INSERT INTO `systemadministrators` VALUES ('adminyd','','YD','MXuRPpxRE1xTPP/X4zO1bNkgy/0=','','')"); //admin.yd.2015
        QString insertStatement4 = QString("INSERT INTO `systemadministrators` VALUES ('king','','DG','apJt8QFtRPAZaUawJkZZTVFEiOo=','','')"); //king.2014
        QString insertStatement5 = QString("INSERT INTO `systemadministrators` VALUES ('lhc','','DG','es9NRQZZf2kbdGvMp/GrkV0+OV0=','','')"); //cheng.2014
        QString insertStatement6 = QString("INSERT INTO `systemadministrators` VALUES ('lj','','DG','v+7twKPCWLNfk9pWrsDVB/CEQ1c=','','')"); //jian.2014
        QString insertStatement7 = QString("INSERT INTO `systemadministrators` VALUES ('ljf','','DG','RvhSIcEL1/AeICOvyeoCNMvXh3g=','','')"); //fu.2014
        QString insertStatement8 = QString("INSERT INTO `systemadministrators` VALUES ('zk','','DG','usJrQRUF6lLB0kUaoWJco3bFht8=','','')"); //kui.2014
        QString insertStatement9 = QString("INSERT INTO `systemadministrators` VALUES ('adminhk','','HK','ImosDlUaBd8YyNUrX7pfHTuuI3A=','','')"); //admin.hk.2015

        model.setQuery(QSqlQuery(createTableStatement, db));
        model.setQuery(QSqlQuery(insertStatement, db));
        model.setQuery(QSqlQuery(insertStatement2, db));
        model.setQuery(QSqlQuery(insertStatement3, db));
        model.setQuery(QSqlQuery(insertStatement4, db));
        model.setQuery(QSqlQuery(insertStatement5, db));
        model.setQuery(QSqlQuery(insertStatement6, db));
        model.setQuery(QSqlQuery(insertStatement7, db));
        model.setQuery(QSqlQuery(insertStatement8, db));
        model.setQuery(QSqlQuery(insertStatement9, db));

    }

    QString queryString = QString("select * from systemadministrators where UserID = '%1'") .arg(getUserID());

    model.setQuery(QSqlQuery(queryString, db));

    if (model.lastError().type() != QSqlError::NoError) {
        qCritical() << QString("An error occurred when querying the database: %1").arg(model.lastError().text());
        QMessageBox::critical(parentWidget, tr("Fatal Error"), tr("An error occurred when querying the database!<br> %1") .arg(model.lastError().text()));
        return false;
    }

    QString userID = QVariant(model.record(0).value("UserID")).toString();
    qDebug()<<"rowCount:"<<model.rowCount();
    qDebug()<<"userID:"<<userID;

    //从数据库取回经Base64编码后的SHA-1加密过的密码,将其还原
    //Fetch the Base64 encoded password which is already encrypted by SHA-1, then decode it and convert it to QCryptographicHash
    QByteArray passWD = QVariant(model.record(0).value("PassWD")).toByteArray();
    passWD = QByteArray::fromBase64(passWD);

    QString businessAddress = QVariant(model.record(0).value("BusinessAddress")).toString();

    if (userID.isEmpty()) {
        QMessageBox::critical(parentWidget, tr("Authentication Failed"), tr(
                                  "<font color=red>Sorry,I can't find your ID!<br>  "
                                  "Please check out your ID.</font>"));
        return false;

    } else if (passWD != passWord()) {
        QMessageBox::critical(parentWidget, tr("Authentication Failed"), tr(
                                  "<font color=red>Sorry,Password incorrect!<br>  "
                                  "Please check out your password.</font>"));
        return false;
    } else if((businessAddress != "DG") && (businessAddress != "YD") && (businessAddress != "HK")){
        QMessageBox::critical(parentWidget, tr("Authentication Failed"), tr("Cannot access the specified server: <font color=red>Permission Denied!</font>"));
    }else{

        QString userName = QVariant(model.record(0).value("UserName")).toString();
        user->setUserName(userName);
        user->setBusinessAddress(businessAddress);
        user->setVerified(true);

        //int id = QVariant(model.record(0).value("ID")).toInt();
        //model.setQuery(QSqlQuery(QString("update systemadministrators set LastLoginTime =(now()) where id=%1").arg(id), db));
        //QString currentDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        QString currentDateTime = "NULL" ;

        model.setQuery(QSqlQuery(QString("update systemadministrators set LastLoginTime =%1 where UserID='%2' ").arg(currentDateTime).arg(userID), db));

        return true;
    }

    return false;
}

bool LoginBase::getUserInfo(){

    LoginDlg dlg(user, m_windowTitle, parentWidget);
    //        if(!loginDlg){
    //            loginDlg = new LoginDlg(user, parentWidget);
    //            connect(loginDlg, SIGNAL(signalUserButtonClicked()), this, SLOT(slotProcessUserButtonClickedSignal()));
    //            connect(loginDlg, SIGNAL(signalKeyButtonClicked()), this, SLOT(slotProcessUserButtonClickedSignal()));
    //        }
    connect(&dlg, SIGNAL(signalModifySettings()), this, SLOT(modifySettings()));

    if (dlg.exec() != QDialog::Accepted) {
        return false;
    }
    return true;

}

bool LoginBase::canLogin(){
    return true;
}

/*
 * 检查用户是否验证成功
 * verify the user
 *
 */
bool LoginBase::isVerified() {

    if(!canLogin()){
        return false;
    }


    //检查用户是否验证成功
    //verify the user
    while (!isSuccesseful) {

        //		LoginDlg dlg(user, parentWidget);
        //		if (dlg.exec() != QDialog::Accepted) {
        //			return false;
        //		}
        if(!getUserInfo()){
            return false;
        }


        //RestoreMode 无需服务器密码验证,直接返回真
        //RestoreMode returns true, no need server authentication
        if(user->isRootMode()){
            isSuccesseful = true;
        }else{
            //服务器认证
            //Server Authentication
            isSuccesseful = verifyUser();
        }


        if (!isSuccesseful) {
            user->setVerified(false);
            qCritical() << QString("Error: Authentication Failed!");
        }
    }

    return isSuccesseful;
}





















} //namespace HEHUI


