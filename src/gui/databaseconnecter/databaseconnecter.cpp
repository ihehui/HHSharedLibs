/*
 ****************************************************************************
 * databaseconnecter.h
 *
 * Created on: 2009-8-5
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


#include <QApplication>
#include <QMessageBox>
//#include <QProgressDialog>

#include "databaseconnecter.h"
#include "databaseconnecterdialog.h"
#include "../../core/database/databaseutility.h"

DatabaseConnecter::DatabaseConnecter(QObject *parent) :
    QObject(parent) {

    qDebug() << "----DatabaseConnecter::DatabaseConnecter(QObject *parent)";

    if (parent && parent->isWidgetType()) {
        this->parentWidget = qobject_cast<QWidget *> (parent);
        qDebug() << "~~ DatabaseConnecter: parent is Widget Type";
    } else {
        this->parentWidget = 0;
        qDebug() << "~~ DatabaseConnecter: parent is not Widget Type";
    }

}

DatabaseConnecter::~DatabaseConnecter() {

}

bool DatabaseConnecter::isDatabaseOpened(const QString &connectionName,
                                         const QString &driver, const QString &host, int port,
                                         const QString &user, const QString &passwd,
                                         const QString &databaseName, HEHUI::DatabaseType databaseType)

{

    qDebug() << "----DatabaseConnecter::isDatabaseOpened(...)";
    Q_ASSERT_X(!connectionName.isEmpty(), "DatabaseConnecter::isDatabaseOpened(...)", "'connectionName' is empty!");
    Q_ASSERT_X(!driver.isEmpty(), "DatabaseConnecter::isDatabaseOpened(...)", "'driver' is empty!");
    Q_ASSERT_X(!databaseName.isEmpty(), "DatabaseConnecter::isDatabaseOpened(...)", "'databaseName' is empty!");

    QSqlDatabase db;
    db = QSqlDatabase::database(connectionName);

    if (!db.isValid()) {
        db = getDatabase(connectionName, driver, host, port, user, passwd,
                         databaseName, databaseType);

        //        db = QSqlDatabase::database(connectionName);

    }

    if(db.isValid() && db.isOpen()){
        return true;
    }else{
        return false;
    }


}

QSqlDatabase DatabaseConnecter::getDatabase(const QString &connectionName,
                                            const QString &driver, const QString &host, int port,
                                            const QString &user, const QString &passwd,
                                            const QString &databaseName, HEHUI::DatabaseType databaseType)

{
    qDebug()<<"----DatabaseConnecter::getDatabase(...)";

    qApp->processEvents();
    //QApplication::setOverrideCursor(Qt::WaitCursor);

    QSqlDatabase db;
    db = QSqlDatabase::database(connectionName);

    if (!db.isValid()) {

        QSqlError err;
        DatabaseUtility databaseUtility;

        //TODO:
        //        QProgressDialog progressDialog(parentWidget);
        //        progressDialog.setRange(0,0);
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &progressDialog, SLOT(close()));
        //        progressDialog.show();
        //        //progressDialog.raise();
        //        //progressDialog.exec();

        //        QEventLoop eventLoop;
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &eventLoop, SLOT(quit()));
        //        eventLoop.exec();
        //        qCritical() << QString("eventLoop.exec();");
        //
        //        QTimer timer;
        //        timer.setSingleShot(false);
        //        timer.setInterval(100);
        //        connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &eventLoop, SLOT(quit()));
        //
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &timer, SLOT(stop()));
        //        //eventLoop.exec();
        //        //timer.setInterval(500);
        //        timer.start();

        //TODO
        //可能会引起界面冻结
        //Maybe freeze the GUI
        err = databaseUtility.openDatabase(connectionName, driver, host, port,
                                           user, passwd, databaseName, databaseType);

        if (err.type() != QSqlError::NoError) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(parentWidget, tr("Fatal Error"), tr("An error occurred when opening the database!<br> %1").arg(err.text()));
            qCritical() << QString("ERROR! An error occurred when opening the database: %1").arg(err.text());

            DatabaseConnecterDialog dbConnecterDlg(connectionName, host, port, "", "", databaseName, databaseType, parentWidget);
            QStringList parameters = dbConnecterDlg.getParameters();
            if (parameters.size() <= 0) {
                qCritical() << QString("ERROR!") << QString("Can not connect to database server!");
                return QSqlDatabase();
            }

            getDatabase(parameters.at(0), parameters.at(1), parameters.at(2),
                        parameters.at(3).toInt(), parameters.at(4),
                        parameters.at(5), parameters.at(6),
                        (HEHUI::DatabaseType) parameters.at(7).toUInt());

        }

        db = QSqlDatabase::database(connectionName);
        emit signalNewDatabaseConnected(connectionName);

    }

    QApplication::restoreOverrideCursor();

    return db;

}

bool DatabaseConnecter::connectToNewDatabase(QString *connectionName) {
    qDebug() << "----DatabaseConnecter::connectToNewDatabase()";

    DatabaseConnecterDialog dbConnecterDlg(parentWidget);

    QStringList parameters = dbConnecterDlg.getParameters();
    if (parameters.size() <= 0) {
        QMessageBox::critical(parentWidget, tr("Fatal Error"), tr(
                                  "Can not connect to database server!"));
        qCritical() << QString("XX Fatal Error!") << QString(
                           "Can not connect to database server!");
        return false;
    }

    getDatabase(parameters.at(0), parameters.at(1), parameters.at(2),
                parameters.at(3).toInt(), parameters.at(4), parameters.at(5),
                parameters.at(6), (HEHUI::DatabaseType) parameters.at(7).toUInt());

    QSqlDatabase db;
    db = QSqlDatabase::database(parameters.at(0));

    if(connectionName){
        *connectionName = parameters.at(0);
    }

    return db.isValid();
}

