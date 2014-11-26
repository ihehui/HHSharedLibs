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




namespace HEHUI {

LoginBase::LoginBase(User *user, QObject *parent)
    :QObject(parent), user(user)
{
    m_windowTitle = tr("Login");

    qDebug("----LoginBase::LoginBase(User *user, QObject *parent)");
    Q_ASSERT_X(user != NULL, "LoginBase::LoginBase(User *user, QObject *parent)", " 'user' is NULL!");

    isSuccesseful = false;

    if(parent && parent->isWidgetType()){
        this->parentWidget = qobject_cast<QWidget *>(parent);
    }else{
        this->parentWidget = 0;
    }

}

LoginBase::LoginBase(User *user, const QString &windowTitle, QObject *parent)
    :QObject(parent), user(user), m_windowTitle(windowTitle)
{
    qDebug("----LoginBase::LoginBase(User *user, QObject *parent)");
    Q_ASSERT_X(user != NULL, "LoginBase::LoginBase(User *user, QObject *parent)", " 'user' is NULL!");

    isSuccesseful = false;

    if(parent && parent->isWidgetType()){
        this->parentWidget = qobject_cast<QWidget *>(parent);
    }else{
        this->parentWidget = 0;
    }


    //        loginDlg = new LoginDlg(user, parentWidget);
    //        connect(loginDlg, SIGNAL(signalUserButtonClicked()), this, SLOT(slotProcessUserButtonClickedSignal()));
    //        connect(loginDlg, SIGNAL(signalKeyButtonClicked()), this, SLOT(slotProcessUserButtonClickedSignal()));



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


inline void LoginBase::setUser(User *u){
    user = u;
}

//User* LoginBase::getUser(){
//	return this->user;
//}


bool LoginBase::getUserInfo(){

    LoginDlg dlg(user, m_windowTitle, parentWidget);
    //        if(!loginDlg){
    //            loginDlg = new LoginDlg(user, parentWidget);
    //            connect(loginDlg, SIGNAL(signalUserButtonClicked()), this, SLOT(slotProcessUserButtonClickedSignal()));
    //            connect(loginDlg, SIGNAL(signalKeyButtonClicked()), this, SLOT(slotProcessUserButtonClickedSignal()));
    //        }
    if (dlg.exec() != QDialog::Accepted) {
        return false;
    }
    return true;

}

bool LoginBase::canLogin(){

    return true;

}

bool LoginBase::verifyUser() {
    qDebug("----LoginBase::verifyUser()");


    DatabaseConnecter dc(parentWidget);
    if(!dc.isDatabaseOpened(LOCAL_CONFIG_DB_CONNECTION_NAME,
                            LOCAL_CONFIG_DB_DRIVER,
                            "",
                            0,
                            "",
                            "",
                            LOCAL_CONFIG_DB_NAME,
                            HEHUI::SQLITE
                            )){

        qCritical() << QString("Error! Database Connection Failed! Authentication Failed!");
        return false;
    }


    QSqlDatabase db;
    db = QSqlDatabase::database(LOCAL_CONFIG_DB_CONNECTION_NAME);
    QSqlQueryModel model(this);

    int latestVersion = 121031;

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
        QString insertStatement2 = QString("INSERT INTO `systemadministrators` VALUES ('admindg','','DG','4r0+BGnQSAKdmIXHyGI8vVoKjsk=','','')"); //admin.dg.2014
        QString insertStatement3 = QString("INSERT INTO `systemadministrators` VALUES ('adminyd','','YD','vWdLKOBN+2qAcbjw7QrQr3nRLBQ=','','')"); //admin.yd.2014
        QString insertStatement4 = QString("INSERT INTO `systemadministrators` VALUES ('king','','DG','apJt8QFtRPAZaUawJkZZTVFEiOo=','','')"); //king.2014
        QString insertStatement5 = QString("INSERT INTO `systemadministrators` VALUES ('lhc','','DG','es9NRQZZf2kbdGvMp/GrkV0+OV0=','','')"); //cheng.2014
        QString insertStatement6 = QString("INSERT INTO `systemadministrators` VALUES ('lj','','DG','v+7twKPCWLNfk9pWrsDVB/CEQ1c=','','')"); //jian.2014
        QString insertStatement7 = QString("INSERT INTO `systemadministrators` VALUES ('ljf','','DG','RvhSIcEL1/AeICOvyeoCNMvXh3g=','','')"); //fu.2014
        QString insertStatement8 = QString("INSERT INTO `systemadministrators` VALUES ('zk','','DG','usJrQRUF6lLB0kUaoWJco3bFht8=','','')"); //kui.2014

        model.setQuery(QSqlQuery(createTableStatement, db));
        model.setQuery(QSqlQuery(insertStatement, db));
        model.setQuery(QSqlQuery(insertStatement2, db));
        model.setQuery(QSqlQuery(insertStatement3, db));
        model.setQuery(QSqlQuery(insertStatement4, db));
        model.setQuery(QSqlQuery(insertStatement5, db));
        model.setQuery(QSqlQuery(insertStatement6, db));
        model.setQuery(QSqlQuery(insertStatement7, db));
        model.setQuery(QSqlQuery(insertStatement8, db));

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
    } else if((businessAddress != "DG") && (businessAddress != "YD")){
        QMessageBox::critical(parentWidget, tr("Authentication Failed"), tr("Cannot access the specified server: <font color=red>Permission Denied!</font>"));
    }else{

        QString userName = QVariant(model.record(0).value("UserName")).toString();
        user->setUserName(userName);
        user->setBusinessAddress(businessAddress);
        user->setVerified(true);

        //int id = QVariant(model.record(0).value("ID")).toInt();
        //model.setQuery(QSqlQuery(QString("update systemadministrators set LastLoginTime =(now()) where id=%1").arg(id), db));
        QString currentDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        model.setQuery(QSqlQuery(QString("update systemadministrators set LastLoginTime =%1 where UserID='%2' ").arg(currentDateTime).arg(userID), db));

        return true;

    }

    return false;
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


