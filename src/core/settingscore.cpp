/*
 ****************************************************************************
 * settingscore.cpp
 *
 * Created on: 2009-4-27
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
 * Last Modified on: 2010-05-07
 * Last Modified by: 贺辉
 ***************************************************************************
 */




#include <QDir>
#include <QFile>
#include <QLocale>

#include "settingscore.h"
#include "cryptography/cryptography.h"
#include "logger/messagelogger.h"



namespace HEHUI
{



SettingsCore::SettingsCore(const QString fileBaseName, const QString fileDirPath, QObject *parent )
    : QSettings( QDir::toNativeSeparators( QString( "%1/%2.ini" ).arg( fileDirPath, fileBaseName) ), QSettings::IniFormat, parent )

{

    m_enableLog = false;
}

SettingsCore::SettingsCore(const QString fileName, Format format, QObject *parent )
    : QSettings( fileName, format, parent )
{

    m_enableLog = false;
}

SettingsCore::~SettingsCore()
{
    //endGroup();
}

void SettingsCore::setValueWithEncryption(const QString &settingsKey, const QVariant &value, const QByteArray &encryptionKey)
{
    QByteArray destination;
    Cryptography cryptography;
    cryptography.teaCrypto(&destination, value.toByteArray(), encryptionKey, true);
    setValue(settingsKey, QVariant(destination));
}

QVariant SettingsCore::getValueWithDecryption(const QString &settingsKey, const QByteArray &encryptionKey, const QVariant &defaultValue, bool *ok) const
{
    if(!contains(settingsKey)) {
        if(ok) {
            *ok = false;
        }
        return defaultValue;
    }

    QByteArray array = value(settingsKey).toByteArray();
    QByteArray destination;
    Cryptography cryptography;
    int ret = cryptography.teaCrypto(&destination, array, encryptionKey, false);
    if(ok) {
        *ok = ret;
    }
    if(!ret) {
        return defaultValue;
    }
    return QVariant(destination);
}

void SettingsCore::enableLog(bool enable, const QString &logFileBaaseName, bool logToConsole)
{
    m_enableLog = enable;

    if(enable){
        quint8 targets = MessageLoggerBase::TARGET_FILE;
        if(logToConsole){
            targets |= MessageLoggerBase::TARGET_CONSOLE;
        }
        bool saveDebuglogToDB = value("SaveDebuglogToDB", false).toBool();
        if(saveDebuglogToDB){
            targets |= MessageLoggerBase::TARGET_DATABASE;
        }

#ifdef _DEBUG_
    initLogger(targets | MessageLoggerBase::TARGET_CONSOLE, logFileBaaseName);
#else
    initLogger(targets, logFileBaaseName);
#endif

    }else{
        MessageLogger::instance()->close();
    }

}

void SettingsCore::initLogger(quint8 targets, const QString &logFileBaaseName)
{
    if(!targets){
        return;
    }

    m_enableLog = true;

    MessageLogger::instance()->setOutputTargets(targets);
    MessageLogger::instance()->setOutputTypes(MSG_ANY, MSG_ANY, MSG_ANY);
    MessageLogger::instance()->setLogQtMessagesEnabled(true);
    MessageLogger::instance()->setAppVersion(QString("%1").arg(APP_VERSION));

    FileConfigStruct fileConfig;
    fileConfig.baseDir = QDir::homePath() + "/log";
    fileConfig.dirTimeTemplate = "/yyyyMM";
    fileConfig.fileBaseName = logFileBaaseName;
    fileConfig.fileNameTimeTemplate = "yyyyMMdd";
    MessageLogger::instance()->setFileConfig(fileConfig);


    if(MessageLogger::instance()->isOutputToDatabase()){

        //      //SQLITE
        //    DBConfigStruct dbconfig;
        //    dbconfig.useFileDB = true;
        //    dbconfig.dbName = QDir::homePath() + "/log/log.db";
        //    dbconfig.dbConnectionName = dbconfig.dbName;
        //    MessageLogger::instance()->setDBConfig(dbconfig);

        beginGroup("debuglog");
        QString connectionName = value("ConnectionName", "DBCON@DEBUGLOG").toString();
        QString driver = value("Driver", "QMYSQL").toString();
        QString host = value("Host", "127.0.0.1").toString();
        quint16 port = value("Port", 3306).toUInt();
        QString user = value("Username", "").toString();
        QString passwd = value("Password", "").toString();
        QString databaseName = value("DB", "debuglog").toString();
        DatabaseType dbType = DatabaseType(value("DBType", quint8(HEHUI::MYSQL)).toUInt());

        endGroup();

        //MySQL
        DBConfigStruct mysqlDBconfig;
        mysqlDBconfig.useFileDB = false;
        mysqlDBconfig.dbConnectionName = connectionName;
        mysqlDBconfig.dbDriver = driver;
        mysqlDBconfig.dbServerHost = host;
        mysqlDBconfig.dbServerPort = port;
        mysqlDBconfig.dbServerUserName = user;
        mysqlDBconfig.dbServerUserPassword = passwd;
        mysqlDBconfig.dbName = databaseName;
        mysqlDBconfig.dbType = dbType;
        MessageLogger::instance()->setDBConfig(mysqlDBconfig);

        QString errorMessage = "";
        if(!MessageLogger::instance()->openDatabase(&errorMessage)){
            QString err = QObject::tr("Failed to open debug log database!");
            qCritical()<<err<<errorMessage;
            //QMessageBox::critical(0, QObject::tr("Error"), QString("%1<p>%2</p>").arg(err).arg(errorMessage));
        }

    }

    MessageLogger::instance()->start();

}

void SettingsCore::closeLogger()
{
    if(m_enableLog){
        m_enableLog = false;
        MessageLogger::instance()->close();
    }

    //fprintf(stderr, "Logger Closed!\n");
}

bool SettingsCore::isLogEnabled()
{
    return m_enableLog;
}






} //namespace HEHUI







