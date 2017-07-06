#include "messagelogger.h"

#include <QDir>
#include <QFileInfo>


///////////////////////////////////////////////////////////////////////////////
#if defined(Q_OS_LINUX)
#include <unistd.h>
#include <stdlib.h>
#include <QProcess>

void afterCrashDump(int signalNO, const QStringList &dump)
{
    MessageLogger::instance()->stopLogger();

    QString title = QObject::tr("Exception");
    QString text = QObject::tr("An unhandled exception has occurred, the application has been terminated!\nSignal:%1").arg(signalNO);
    QString detail = dump.join("\n");

    bool isKDE = qgetenv("XDG_CURRENT_DESKTOP").toLower() == "kde";
    QString process;
    QStringList args;
    if(isKDE){
        process = "kdialog";
        args<< "--detailederror" << text << detail << "--title" << title;
    }else{
        process = "zenity";
        args << "--error" << "--text=" + text + "\n\n" + detail;
    }
    QProcess::startDetached(process,  args);

    exit(signalNO);
    return;

//    QMessageBox msgBox;
//    msgBox.setWindowTitle(QObject::tr("Exception"));
//    msgBox.setText("The application has been terminated! An unhandled exception has occurred!");
//    msgBox.setInformativeText(QString("SIGNAL:%1").arg(signalNO));
//    msgBox.setDetailedText(dump);
//    msgBox.setIcon(QMessageBox::Critical);
//    msgBox.exec();
//    exit(signalNO);
}

#endif

///////////////////////////////////////////////////////////////////////////////







MessageLogger* MessageLogger::m_instance = 0;

MessageLogger* MessageLogger::instance()
{
    if(!m_instance){
        m_instance = new MessageLogger();
        LogMessage::setMessageLoggerBase(m_instance);
    }

    return m_instance;
}

void MessageLogger::close()
{
    if(m_instance){
        m_instance->stopLogger();
        m_instance->quit();
        delete m_instance;
        m_instance = 0;
        LogMessage::setMessageLoggerBase(m_instance);
    }
}

MessageLogger::MessageLogger(QObject *parent)
    :MessageLoggerBase(parent)
{

}

MessageLogger::~MessageLogger()
{

}

void MessageLogger::setDBConfig(DBConfigStruct dbConfig)
{
    m_dbConfig = dbConfig;
}

bool MessageLogger::beginOutputMessage()
{
    if(isOutputToDatabase()){
            //QString statement = QString("BEGIN TRANSACTION;");
//            QString statement = QString("START TRANSACTION;");
//            QSqlError error = execSQL(statement);
//            if(error.type() != QSqlError::NoError){
//                return false;
//            }

        m_dbConfig.database.transaction();

    }

    return true;
}

bool MessageLogger::endOutputMessage()
{
    if(isOutputToDatabase()){
//        QString statement = QString("COMMIT;");
//        QSqlError error = execSQL(statement);
//        if(error.type() != QSqlError::NoError){
//            return false;
//        }

        m_dbConfig.database.commit();
    }

    return true;
}


bool MessageLogger::saveToDatabase(MessageStruct *messageStruct)
{

//    if(!m_dbConfig.database.isValid() || !m_dbConfig.database.isOpen()){
//        if(!openDatabase()){
//            return false;
//        }
//    }


    QString statement = QString("INSERT INTO %1(PID, ThreadId, LogTime, LogType, Message, FileName, LineNumber, FuntionName, Version) VALUES(%2, '%3', '%4', %5, '%6', '%7', %8, '%9', '%10');")
            .arg(m_dbConfig.tableName)
            .arg(m_processId)
            .arg(messageStruct->threadId)
            .arg(messageStruct->time.toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .arg(quint8(messageStruct->logType))
            .arg(messageStruct->messages.replace("'", "\\'"))
            .arg(messageStruct->fileName.replace("'", "\\'"))
            .arg(messageStruct->lineNumber)
            .arg(messageStruct->funtionName)
            .arg(m_appVersion)
            ;

    QSqlError error = execSQL(statement);
    if(error.type() != QSqlError::NoError){
        QString msg = QString("Can not save log to database! %1 Error Type:%2, NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        fprintf(stderr, "%s\n", msg.toLocal8Bit().data());

        return false;
    }

//    QSqlQuery query(m_dbConfig.database);
//    if(!query.exec(statement)){
//        QSqlError error = query.lastError();
//        QString msg = QString("Can not save log to database! %1 Error Type:%2, NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
//        qCritical()<<msg;
//        return false;
//    }


    return true;
}

bool MessageLogger::openDatabase(QString *errorMessage, bool reopen){

    //Check Local Database
    bool needInitUserDB = false;
    if( m_dbConfig.useFileDB && (!QFile(m_dbConfig.dbName).exists()) ){
        QFileInfo info(m_dbConfig.dbName);
        QDir dir;
        dir.mkpath(info.path());

        needInitUserDB = true;
    }

    if(reopen){
        DatabaseUtility::closeDBConnection(m_dbConfig.dbConnectionName);
    }

    QSqlDatabase db = QSqlDatabase::database(m_dbConfig.dbConnectionName);
    if(!db.isValid()){
        QSqlError err;
        DatabaseUtility databaseUtility;
        err = databaseUtility.openDatabase(m_dbConfig.dbConnectionName,
                                           m_dbConfig.dbDriver,
                                           m_dbConfig.dbServerHost,
                                           m_dbConfig.dbServerPort,
                                           m_dbConfig.dbServerUserName,
                                           m_dbConfig.dbServerUserPassword,
                                           m_dbConfig.dbName,
                                           m_dbConfig.dbType
                                           );
        if (err.type() != QSqlError::NoError) {
            QString msg = QString("An error occurred when opening the database on '%1'! %2").arg(m_dbConfig.dbServerHost).arg(err.text());
            //qCritical() << msg;
            fprintf(stderr, "%s\n", msg.toLocal8Bit().data());

            if(errorMessage){
                *errorMessage = err.text();
            }

            return false;
        }

    }

    db = QSqlDatabase::database(m_dbConfig.dbConnectionName);
    if(!db.isOpen()){
        QSqlError err = db.lastError();
        QString msg = QString("Database is not open! %1").arg(err.text());
        //qCritical()<< msg;
        fprintf(stderr, "%s\n", msg.toLocal8Bit().data());

        if(errorMessage){
            *errorMessage = err.text();
        }

        return false;
    }

    m_dbConfig.database = db;

    if(!db.tables().contains(m_dbConfig.tableName)){
        needInitUserDB = true;
    }

    if(needInitUserDB){
        if(!initDatabase(errorMessage)){
            return false;
        }
    }


    return true;
}

void MessageLogger::closeDatabase()
{
    if(m_dbConfig.database.isValid() && m_dbConfig.database.isOpen()){
//        MessageStruct messageStruct;
//        quint64 tid = *(quint64*)QThread::currentThreadId();
//        messageStruct.threadId = "0x" + QString::number(tid, 16);
//        messageStruct.time = QDateTime::currentDateTime();
//        messageStruct.logType = MSG_WARNING;
//        QStringList msgs;
//        msgs << "Logger Closed.";
//        messageStruct.messages = msgs;
//        saveToDatabase(&messageStruct);

        m_dbConfig.database.close();
    }

}

bool MessageLogger::initDatabase(QString *errorMessage){

    if(!m_dbConfig.database.isValid() || !m_dbConfig.database.isOpen()){
        if(errorMessage){
            *errorMessage = tr("Database Invalid Or Not Open!");
        }
        return false;
    }

//    QSqlQuery query;
//    QSqlError error = DatabaseUtility::excuteSQLScriptFromFile(m_dbConfig.database, ":/text/resources/text/userdata.sql", "UTF-8", &query, true);
//    if(error.type() != QSqlError::NoError){
//        QString msg = error.text();
//        if(errorMessage){
//            *errorMessage = msg;
//        }
//        return false;
//    }


    QString statement = QString("CREATE TABLE `%1` ("
                                "`ID` int(10) unsigned NOT NULL AUTO_INCREMENT,"
                                "`PID` int(10) unsigned DEFAULT NULL,"
                                "`ThreadId` varchar(32) DEFAULT NULL,"
                                "`LogTime` datetime DEFAULT NULL,"
                                "`LogType` tinyint(2) unsigned DEFAULT NULL,"
                                "`Message` text,"
                                "`FileName` varchar(64) DEFAULT NULL,"
                                "`FuntionName` varchar(128) DEFAULT NULL,"
                                "`LineNumber` int(10) unsigned DEFAULT NULL,"
                                "`Version` varchar(16) DEFAULT NULL,"
                                "PRIMARY KEY (`ID`)"
                              ");").arg(m_dbConfig.tableName);

    if(m_dbConfig.dbType == HEHUI::SQLITE){
        statement = QString("CREATE TABLE '%1'("
                    "'ID' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",'PID' INTEGER"
                    ",'ThreadId' varchar(32)"
                    ",'LogTime' datetime"
                    ",'LogType' TINYINT(2)"
                    ",'Message' TEXT(512)"
                    ",'FileName' varchar(64)"
                    ",'LineNumber' INTEGER"
                    ",'FuntionName' varchar(128)"
                    ",'Version' varchar(16)"
                    ");").arg(m_dbConfig.tableName);
    }

    QSqlError error = execSQL(statement);
    if(error.type() != QSqlError::NoError){
        QString msg = QString("Can not initialize log table! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        //qCritical()<<msg;
        fprintf(stderr, "%s\n", msg.toLocal8Bit().data());

        if(errorMessage){
            *errorMessage = msg;
        }
        return false;
    }

//    QSqlQuery query(m_dbConfig.database);
//    if(!query.exec(statement)){
//        QSqlError error = query.lastError();
//        QString msg = QString("Can not initialize database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
//        qCritical()<<msg;
//        if(errorMessage){
//            *errorMessage = msg;
//        }
//        return false;
//    }

    return true;

}

QSqlError MessageLogger::execSQL(const QString & sqlString, QSqlQuery *sqlQuery)
{
    QSqlError error;
    if(!m_dbConfig.database.isValid() || !m_dbConfig.database.isOpen()){
        if(!openDatabase()){
            error.setType(QSqlError::UnknownError);
            error.setDatabaseText(tr("Failed to open database."));
            return error;
        }
    }

    QSqlQuery query(m_dbConfig.database);
    bool ok = query.exec(sqlString);
    error = query.lastError();
    if(!ok){
        QString msg = QString("Can not exec SQL! %1 Error Type:%2, NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        //qCritical()<<msg;
        fprintf(stderr, "%s\n", msg.toLocal8Bit().data());
        fprintf(stderr, "SQL:%s\n", sqlString.toLocal8Bit().data());
    }

    if(sqlQuery){
        *sqlQuery = query;
    }

    return error;
}

QSqlQuery MessageLogger::queryDatabase(const QString & queryString) {

    DatabaseUtility du;
    QSqlQuery query;
    query = du.queryDatabase(queryString,
                             m_dbConfig.dbConnectionName,
                             m_dbConfig.dbDriver,
                             m_dbConfig.dbServerHost,
                             m_dbConfig.dbServerPort,
                             m_dbConfig.dbServerUserName,
                             m_dbConfig.dbServerUserPassword,
                             m_dbConfig.dbName,
                             m_dbConfig.dbType
                             );

    return query;
}

QSqlQuery MessageLogger::queryDatabase(const QString & queryString, const QString &connectionName, const QString &driver,
                                      const QString &host, int port, const QString &user, const QString &passwd,
                                      const QString &databaseName, HEHUI::DatabaseType databaseType) {

    DatabaseUtility du;
    QSqlQuery query;
    query = du.queryDatabase(queryString, connectionName, driver, host, port, user, passwd, databaseName, databaseType);

    return query;
}



