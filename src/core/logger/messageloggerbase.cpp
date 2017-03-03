#include "messageloggerbase.h"
#include <QDateTime>
#include <QFileInfo>
#include <QDir>


#if defined(Q_OS_WIN32)
    #include <qt_windows.h>
#else
    #include <unistd.h>
    #include <stdlib.h>
#endif



//////////////////////////////////////////////////////////////////////////
#if QT_VERSION >= 0x050000
void logQtDebug(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void logQtDebug(QtMsgType type, const char* msg)
#endif
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    QString fileName = "";
    QString funtionName = "";
    int lineNumber = 0;

#if QT_VERSION >= 0x050000
    fileName = context.file;
    funtionName = context.function;
    lineNumber = context.line;
#endif

    MsgType logType = MSG_DEBUG;

    switch (type) {
    case QtWarningMsg:
        logType = MSG_WARNING;
        break;

    case QtCriticalMsg:
        logType = MSG_CRITICAL;
        break;

    case QtFatalMsg:
        logType = MSG_FATAL;
        break;

    case QtDebugMsg:
        logType = MSG_DEBUG;
        break;

//    case QtInfoMsg:
//        logType = MSG_INFO;
//        break;

    default:
        logType = MSG_INFO;
        break;
    }

    switch (logType) {
    case MSG_WARNING:
        LogQtWarningMessage(fileName, funtionName, lineNumber, logType) << msg;
        break;

    case MSG_CRITICAL:
    case MSG_FATAL:
        LogMessage(fileName, funtionName, lineNumber, logType) << msg;
        break;

    case MSG_DEBUG:
        LogQtDebugMessage(fileName, funtionName, lineNumber, logType) << msg;
        break;

    default:
        LogQtInfoMessage(fileName, funtionName, lineNumber, logType) << msg;

        break;
    }

    //LogQtMessage(fileName, funtionName, lineNumber, logType) << msg;

//    if (type == QtFatalMsg) {
//        exit(1);
//    }
}

void installQtMessageLogger(){

#  if QT_VERSION >= 0x050000
    //reset the message handler
    qInstallMessageHandler(0);
    qInstallMessageHandler(logQtDebug);
#  else
    qInstallMsgHandler(0);
    qInstallMsgHandler(logQtDebug);
#  endif

    //qAddPostRoutine(closeDebugLog);

}

void uninstallQtMessageLogger(){
    //reset the message handler
#  if QT_VERSION >= 0x050000
    qInstallMessageHandler(0);
#  else
    qInstallMsgHandler(0);
#  endif

}
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
MessageLoggerBase * LogMessage::m_messageLoggerBase = 0;

LogMessage::LogMessage(const QString &fileName, const QString &funtionName, int lineNumber, MsgType type)
{

    m_messageStruct = new MessageStruct();
    quint64 tid = *(quint64*)QThread::currentThreadId();
    m_messageStruct->threadId = "0x" + QString::number(tid, 16);
    m_messageStruct->time = QDateTime::currentDateTime();
    m_messageStruct->logType = type;

    m_messageStruct->fileName = fileName;
    m_messageStruct->fileName = m_messageStruct->fileName.replace("\\", "/");
    m_messageStruct->fileName = m_messageStruct->fileName.remove(0, m_messageStruct->fileName.lastIndexOf("/") + 1);

    m_messageStruct->funtionName = funtionName;
    m_messageStruct->lineNumber = lineNumber;

}

LogMessage::LogMessage(const LogMessage &other)
{

    if (this != &other) {


//        this->m_messageStruct->threadId = other.m_messageStruct->threadId;
//        this->m_messageStruct->time = other.m_messageStruct->time;
//        this->m_messageStruct->logType = other.m_messageStruct->logType;
//        this->m_messageStruct->messages = other.m_messageStruct->messages;
//        this->m_messageStruct->fileName = other.m_messageStruct->fileName;
//        this->m_messageStruct->funtionName = other.m_messageStruct->funtionName;
//        this->m_messageStruct->lineNumber = other.m_messageStruct->lineNumber;
//        this->m_messageStruct->ref = other.m_messageStruct->ref;

        m_messageStruct = other.m_messageStruct;
        m_messageStruct->ref++;
    }
}

LogMessage &LogMessage::operator=(const LogMessage &other)
{

    if (this != &other) {
//        this->m_messageStruct->threadId = other.m_messageStruct->threadId;
//        this->m_messageStruct->time = other.m_messageStruct->time;
//        this->m_messageStruct->logType = other.m_messageStruct->logType;
//        this->m_messageStruct->messages = other.m_messageStruct->messages;
//        this->m_messageStruct->fileName = other.m_messageStruct->fileName;
//        this->m_messageStruct->funtionName = other.m_messageStruct->funtionName;
//        this->m_messageStruct->lineNumber = other.m_messageStruct->lineNumber;
//        this->m_messageStruct->ref = other.m_messageStruct->ref;

        m_messageStruct = other.m_messageStruct;
        m_messageStruct->ref++;

    }

    return *this;
}

LogMessage::~LogMessage()
{
    (m_messageStruct->ref)--;
    if(!(m_messageStruct->ref)){
        if(m_messageLoggerBase){
            m_messageLoggerBase->appendMessage(m_messageStruct);
        }
    }

}

LogMessage & LogMessage::operator<< (const long &value)
{
    m_messageStruct->messages += (QString::number(value));
    return *this;
}

LogMessage & LogMessage::operator<< (const QVariant &variant){
    if(variant.canConvert(QVariant::String)){
        m_messageStruct->messages += (variant.toString());
    }
    return *this;
}

void LogMessage::setMessageLoggerBase(MessageLoggerBase *messageLoggerBase)
{
    m_messageLoggerBase = messageLoggerBase;
}
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

#if defined(Q_OS_WIN32)
qulonglong MessageLoggerBase::m_processId = GetCurrentProcessId();
#else
qulonglong MessageLoggerBase::m_processId = getpid();
#endif

MessageLoggerBase::MessageLoggerBase(QObject *parent)
    :QThread(parent)
{
    m_outputTarget = MessageLoggerBase::TARGET_FILE;
    m_logTypesForFile = MSG_IMPORTANT;
    m_logTypesForDatabase = MSG_IMPORTANT;
    m_logTypesForConsole = MSG_ANY;

    m_quit = false;
    m_wokeUp = false;
    m_logQtMessages = false;

    m_messageTypeToString.insert(MSG_WARNING, " WARNING");
    m_messageTypeToString.insert(MSG_CRITICAL, "CRITICAL");
    m_messageTypeToString.insert(MSG_FATAL, "   FATAL");
    m_messageTypeToString.insert(MSG_INFO, "    INFO");
    m_messageTypeToString.insert(MSG_DEBUG, "   DEBUG");

    m_appVersion = "0.0.1";

}

MessageLoggerBase::~MessageLoggerBase()
{
//    if(m_logQtMessages){
//        uninstallQtMessageLogger();
//    }

//    m_quit = true;
//    cond.wakeOne();
//    wait();

    if(!m_quit){
        stopLogger();
    }

}

void MessageLoggerBase::stopLogger()
{
    if(m_logQtMessages){
        uninstallQtMessageLogger();
    }

    m_quit = true;
    cond.wakeOne();
    wait();

    quit();
}

//The targets is a OR'ed combination of OutputTarget
void MessageLoggerBase::setOutputTargets(quint8 targets)
{
    m_outputTarget = targets;
}

void MessageLoggerBase::setOutputTypes(quint8 typesForFile, quint8 typesForDatabase, quint8 typesForConsole)
{
    m_logTypesForFile = typesForFile;
    m_logTypesForDatabase = typesForDatabase;
    m_logTypesForConsole = typesForConsole;
}

void MessageLoggerBase::setFileConfig(FileConfigStruct fileConfig)
{
    m_fileConfigStruct = fileConfig;
}

void MessageLoggerBase::appendMessage(MessageStruct *messageStruct)
{
    QMutexLocker locker(&mutex);
    m_messageList.append(messageStruct);
    cond.wakeOne();

//    if (!m_wokeUp) {
//        m_wokeUp = true;
//        QMetaObject::invokeMethod(this, "wakeUp", Qt::QueuedConnection);
//    }

}

quint8 MessageLoggerBase::getOutputTarget()
{
    return m_outputTarget;
}

bool MessageLoggerBase::isOutputToDatabase()
{
    return (m_outputTarget & TARGET_DATABASE);
}

void MessageLoggerBase::setLogQtMessagesEnabled(bool enabled)
{
    m_logQtMessages = enabled;
    if(enabled){
        installQtMessageLogger();
    }else{
        uninstallQtMessageLogger();
    }
}

bool MessageLoggerBase::isloggingQtMessages() const
{
    return m_logQtMessages;
}

void MessageLoggerBase::setAppVersion(const QString &appVersion)
{
    m_appVersion = appVersion;
}

void MessageLoggerBase::run()
{

    do {

        {
            // Go to sleep if there's nothing to do.
            QMutexLocker locker(&mutex);
            if (!m_quit && m_messageList.isEmpty())
            {
                cond.wait(&mutex);
            }
        }

        // Write pending messages
        mutex.lock();
        QList<MessageStruct *> messageList = m_messageList;
        m_messageList.clear();
        mutex.unlock();

        beginOutputMessage();
        while (!messageList.isEmpty()) {
            MessageStruct *msgStruct = messageList.takeFirst();
            outputMessage(msgStruct);
            delete msgStruct;
        }
        endOutputMessage();


    } while (!m_quit);

    // Write pending messages
    mutex.lock();
    QList<MessageStruct *> messageList = m_messageList;
    m_messageList.clear();
    mutex.unlock();

    beginOutputMessage();
    while (!messageList.isEmpty()) {
        MessageStruct *msgStruct = messageList.takeFirst();
        outputMessage(msgStruct);
        delete msgStruct;
    }
    endOutputMessage();

    //Close File
    if(m_outputTarget & TARGET_FILE){
        closeFile();
    }

    if(m_outputTarget & TARGET_DATABASE){
        closeDatabase();
    }

}

void MessageLoggerBase::wakeUp()
{
    QMutexLocker locker(&mutex);
    m_wokeUp = false;
    cond.wakeOne();
}

bool MessageLoggerBase::beginOutputMessage()
{
    return true;
}

void MessageLoggerBase::outputMessage(MessageStruct *messageStruct)
{

    if( (m_outputTarget & TARGET_CONSOLE) && (m_logTypesForConsole & messageStruct->logType) ){
        QString typeString = m_messageTypeToString[messageStruct->logType];
        QString functionInfo = "";
        if(!messageStruct->fileName.trimmed().isEmpty()){
            functionInfo = QString("%1:%2, %3.").arg(messageStruct->fileName).arg(messageStruct->lineNumber).arg(messageStruct->funtionName);
            functionInfo = functionInfo.replace("\\", "/");
            functionInfo = functionInfo.remove(0, functionInfo.lastIndexOf("/") + 1);
        }

        //QString str = QString("%1[%2] %3 %4 (%5)").arg(messageStruct->time.toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(messageStruct->threadId).arg(typeString).arg(messageStruct->messages.join("")).arg(functionInfo);
        QString str = QString("[%1] [%2] [%3] [%4] [%5]").arg(messageStruct->time.toString("hh:mm:ss:zzz")).arg(messageStruct->threadId).arg(typeString).arg(messageStruct->messages).arg(functionInfo);
        fprintf(stderr, "%s\n", str.toUtf8().data());
    }

    if( (m_outputTarget & TARGET_FILE) && (m_logTypesForFile & messageStruct->logType) ){
        saveToFile(messageStruct);
    }

    if( (m_outputTarget & TARGET_DATABASE) && (m_logTypesForDatabase & messageStruct->logType)){
        saveToDatabase(messageStruct);
    }

}

bool MessageLoggerBase::endOutputMessage()
{
    return true;
}

bool MessageLoggerBase::saveToFile(MessageStruct *messageStruct)
{
    QDateTime time = QDateTime::currentDateTime();
    QString logFileName = m_fileConfigStruct.baseDir + time.toString(m_fileConfigStruct.dirTimeTemplate) + "/" + m_fileConfigStruct.fileBaseName + time.toString(m_fileConfigStruct.fileNameTimeTemplate) + "." + m_fileConfigStruct.fileSuffix;

    QFile *file = m_fileConfigStruct.file;
    if(file && file->fileName() != logFileName){
        closeFile();
        file = 0;
    }

    if(!file){
        QFileInfo info(logFileName);
        QDir dir;
        dir.mkpath(info.path());

        file = new QFile(logFileName);
        if (!file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)) {
            qCritical()<<QString("ERROR! Failed to open log file '%1'. %2").arg(logFileName).arg(file->errorString());
            delete file;
            file = 0;
            return false;
        }
        m_fileConfigStruct.file = file;

//        qulonglong processId = 0;
//    #if defined(Q_OS_WIN32)
//        processId = GetCurrentProcessId();
//    #else
//        processId = getpid();
//    #endif

        QByteArray header;
        header += QString(" [V%1] ").arg(m_appVersion).toLocal8Bit();
        header += time.toString("[yyyy-MM-dd hh:mm:ss.zzz]").toLatin1();
        header += " [PID:";
        header += QByteArray::number(m_processId);
        header += "]";
        QByteArray ba("\n-------------------- DEBUG LOG OPENED "+ header + " --------------------\n");
        file->write(ba);
    }

    if(m_fileConfigStruct.truncateFile){
        if(file->size() > m_fileConfigStruct.maxFileSize){
            file->seek(0);
            qint64 sizeRead = 0;
            const qint64 bufSize = (qint64)(file->size() - m_fileConfigStruct.maxFileSize * 0.95);
            char buf[bufSize];
            while (sizeRead < bufSize) {
                sizeRead += file->readLine(buf, sizeof(buf));
            }

            QByteArray oldData = file->readAll();
            file->resize(0);
            file->write(oldData);
            file->flush();
        }
    }

    QString typeString = m_messageTypeToString[messageStruct->logType];
    QString functionInfo = "";
    if(!messageStruct->fileName.trimmed().isEmpty()){
        functionInfo = QString("%1:%2, %3").arg(messageStruct->fileName).arg(messageStruct->lineNumber).arg(messageStruct->funtionName);
    }

    //QString functionInfo = QString("%1:%2, %3").arg(messageStruct->fileName).arg(messageStruct->lineNumber).arg(messageStruct->funtionName);
    //functionInfo = functionInfo.replace("\\", "/");
    //functionInfo = functionInfo.remove(0, functionInfo.lastIndexOf("/") + 1);
    QString str = QString("[%1] [%2] [%3] [%4] [%5]").arg(messageStruct->time.toString("hh:mm:ss.zzz")).arg(messageStruct->threadId).arg(typeString).arg(messageStruct->messages).arg(functionInfo);

    QByteArray ba;
    ba += str.toUtf8();
    ba += '\n';
    file->write(ba);
    file->flush();

    return true;
}

bool MessageLoggerBase::closeFile()
{
    QFile *file = m_fileConfigStruct.file;
    if (!file){return false;}

//    qulonglong processId = 0;
//#if defined(Q_OS_WIN32)
//    processId = GetCurrentProcessId();
//#else
//    processId = getpid();
//#endif

    QByteArray end;
    end += QString(" [V%1] ").arg(m_appVersion).toLocal8Bit();
    end += QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]").toLatin1();
    end += " [PID:";
    end += QByteArray::number(m_processId);
    end += "] ";
    QByteArray ba("-------------------- DEBUG LOG CLOSED "+ end + " --------------------\n\n");
    file->write(ba);

    file->flush();
    file->close();
    delete file;
    file = 0;

    m_fileConfigStruct.file = 0;

    return true;
}

bool MessageLoggerBase::saveToDatabase(MessageStruct *messageStruct)
{
    Q_UNUSED(messageStruct);

    return true;
}

void MessageLoggerBase::closeDatabase()
{

}

//////////////////////////////////////////////////////////////////////////

