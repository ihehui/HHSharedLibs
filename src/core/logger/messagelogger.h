/* *****************************************************************

     Author:HeHui
     Update:2017.2.10

                            ------------- Usage: -------------
//////////////////////////////////// INIT ////////////////////////////////////  
    MessageLogger::instance()->setOutputTargets(MessageLoggerBase::TARGET_FILE | MessageLoggerBase::TARGET_DATABASE | MessageLoggerBase::TARGET_CONSOLE);
    MessageLogger::instance()->setOutputTypes(MSG_ANY, MSG_IMPORTANT, MSG_ANY);
    MessageLogger::instance()->setLogQtMessagesEnabled(true);

    FileConfigStruct fileConfig;
    fileConfig.baseDir = QDir::homePath() + "/log";
    fileConfig.dirTimeTemplate = "/yyyy";
    fileConfig.fileBaseName = "log";
    fileConfig.fileNameTimeTemplate = "yyyyMMdd";
    MessageLogger::instance()->setFileConfig(fileConfig);

    ////SQLITE
    //DBConfigStruct dbconfig;
    //dbconfig.useFileDB = true;
    //dbconfig.dbName = QDir::homePath() + "/log/log.db";
    //dbconfig.dbConnectionName = dbconfig.dbName;
    //MessageLogger::instance()->setDBConfig(dbconfig);

    //MySQL
    DBConfigStruct mysqlDBconfig;
    mysqlDBconfig.useFileDB = false;
    //dbconfig.dbConnectionName;
    mysqlDBconfig.dbDriver = "QMYSQL";
    mysqlDBconfig.dbServerHost = "127.0.0.1";
    mysqlDBconfig.dbServerPort = 3306;
    mysqlDBconfig.dbServerUserName = "user";
    mysqlDBconfig.dbServerUserPassword = "pswd";
    mysqlDBconfig.dbName = "debuglog";
    mysqlDBconfig.dbType = HEHUI::MYSQL;
    MessageLogger::instance()->setDBConfig(mysqlDBconfig);

    if(MessageLogger::instance()->isOutputToDatabase() && (!MessageLogger::instance()->openDatabase())){
        QString err = QObject::tr("Failed to open debug log database!");
        qCritical()<<err;
        QMessageBox::critical(0, QObject::tr("Error"), err);
    }
    MessageLogger::instance()->start();
////////////////////////////////////////////////////////////////////////////

    ...
    ...
    ...

//////////////////////////////////// Close ////////////////////////////////////
    //Close
    MessageLogger::instance()->close();
//////////////////////////////////////////////////////////////////////////////



/////////////////////////////// Crash Handler ///////////////////////////////
     //Add the following lines to main() function:
      #if defined(Q_OS_LINUX)
        enableCrashHandler(afterCrashDump);
      #endif
//////////////////////////////////////////////////////////////////////////////

***************************************************************** */



#ifndef MESSAGELOGGER_H
#define MESSAGELOGGER_H

#include "messageloggerbase.h"
#include <QSqlDatabase>

#include "../database/databaseutility.h"


#include "../core_lib.h"

///////////////////////////////////////////////////////////////////////////////
#if defined(Q_OS_LINUX)
void afterCrashDump(int signalNO, const QStringList &dump);
#endif
///////////////////////////////////////////////////////////////////////////////


namespace HEHUI {



struct DBConfigStruct{
    DBConfigStruct(){
        useFileDB = true;
        dbConnectionName = "@DEBUGLOG@";
        dbDriver = "QSQLITE";
        dbServerHost = "";
        dbServerPort = 0;
        dbServerUserName = "";
        dbServerUserPassword = "";
        dbName = "./log.db";
        dbType = HEHUI::SQLITE;

        tableName = "DebugLogs";
        database = QSqlDatabase();
    }

    bool useFileDB;

    QString dbConnectionName;
    QString dbDriver;
    QString dbServerHost;
    quint16 dbServerPort;
    QString dbServerUserName;
    QString dbServerUserPassword;
    QString dbName;
    HEHUI::DatabaseType dbType;

    QString tableName;
    QSqlDatabase database;

};


class CORE_LIB_API MessageLogger : public MessageLoggerBase
{
    Q_OBJECT
public:
    static MessageLogger *instance();
    static void close();

    void setDBConfig(DBConfigStruct dbConfig);
    bool openDatabase(QString *errorMessage = 0, bool reopen = false);

protected:
    MessageLogger(QObject *parent = 0);
    virtual ~MessageLogger();

private:
    bool beginOutputMessage();
    bool endOutputMessage();

    bool saveToDatabase(MessageStruct *messageStruct);
    void closeDatabase();

    bool initDatabase(QString *errorMessage = 0);

    QSqlError execSQL(const QString & sqlString, QSqlQuery *sqlQuery = 0);

    QSqlQuery queryDatabase(const QString & queryString);
    QSqlQuery queryDatabase(const QString & queryString,
                            const QString &connectionName, const QString &driver,
                            const QString &host, int port, const QString &user,
                            const QString &passwd, const QString &databaseName,
                            HEHUI::DatabaseType databaseType);


private:
    static MessageLogger *m_instance;

    DBConfigStruct m_dbConfig;



};

} //namespace HEHUI

#endif // MESSAGELOGGER_H
