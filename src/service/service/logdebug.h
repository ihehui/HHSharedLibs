#ifndef LOGDEBUG_H
#define LOGDEBUG_H


#include <QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTime>
#include <QtCore/QMutex>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

#if defined(Q_OS_WIN32)
    #include <qt_windows.h>
#else
    #include <unistd.h>
    #include <stdlib.h>
#endif


//#ifndef LOGCRITICAL
//#define LOGCRITICAL qCritical() << QString("%1:%2, %3").arg(__FILE__).arg(__LINE__).arg(Q_FUNC_INFO)
//#endif

//#ifndef LOGWARNING
//#define LOGWARNING qWarning() << QString("%1:%2, %3").arg(__FILE__).arg(__LINE__).arg(Q_FUNC_INFO)
//#endif

//#ifndef LOGDEBUG
//#define LOGDEBUG qDebug() << QString("%1:%2, %3").arg(__FILE__).arg(__LINE__).arg(Q_FUNC_INFO)
//#endif


//#if defined(Q_OS_WIN32)
//static QString logFileName = "c:/macs_debuglog.txt";
//#else
//static QString logFileName = "/tmp/macs_debuglog.txt";
//#endif

//static QString m_baseDir = QCoreApplication::applicationDirPath();
//static QString m_fileBaseName = QCoreApplication::applicationName();
static QString m_baseDir = "./";
static QString m_fileBaseName = "log";


static QFile* file = 0;
static qulonglong processId = 0;
static bool printTostderr = false;

static void closeDebugLog()
{
    if (!file){return;}

    QDateTime curTime = QDateTime::currentDateTime();

    QByteArray s(curTime.toString("HH:mm:ss.zzz").toLatin1());
    s += " [";
    s += QByteArray::number(processId);
    s += "] ";
    QByteArray ps(s + "------ DEBUG LOG CLOSED " + curTime.toString("yyyy.MM.dd").toLatin1() + " ------\n");
    file->write(ps);
    file->flush();
    file->close();
    delete file;
    file = 0;
}


#if QT_VERSION >= 0x050000
static void logDebug(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
static void logDebug(QtMsgType type, const char* msg)
#endif
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if(!processId){
#if defined(Q_OS_WIN32)
    processId = GetCurrentProcessId();
#else
    processId = getpid();
#endif
    }

    QDateTime curTime = QDateTime::currentDateTime();

    QByteArray ba(curTime.toString("HH:mm:ss.zzz").toLatin1());
    ba += " [";
    ba += QByteArray::number(processId);
    ba += "] ";


    QString logFileName = m_baseDir + "/" + curTime.toString("yyyyMM") + "/" + m_fileBaseName + curTime.toString("yyyyMMdd") + ".log";
    if(file && file->fileName() != logFileName){
        closeDebugLog();
    }

    if (!file) {
        QFileInfo info(logFileName);
        QDir dir;
        dir.mkpath(info.path());
        file = new QFile(logFileName);
        if (!file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            delete file;
            file = 0;
            return;
        }
        QByteArray ps('\n' + ba + "------ DEBUG LOG OPENED " + "[" + APP_VERSION + "] " + curTime.toString("yyyy.MM.dd").toLatin1() + " ------\n");
        file->write(ps);
    }

    switch (type) {
    case QtWarningMsg:
        ba += " WARNING: ";
        break;
    case QtCriticalMsg:
        ba += "CRITICAL: ";
        break;
    case QtFatalMsg:
        ba+= "    FATAL: ";
        break;
    case QtDebugMsg:
        ba += "   DEBUG: ";
        break;
//    case QtInfoMsg:
//        s += "    INFO: ";
//        break;

    default:
        ba += "    INFO: ";
        break;
    }




#if QT_VERSION >= 0x050000
    QString str = QString("%1 (%2:%3, %4)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
    ba += str.toUtf8();
#else
    ba += msg;
#endif


//#if QT_VERSION >= 0x050400
//    s += qFormatLogMessage(type, context, msg).toLocal8Bit();
//#elif QT_VERSION >= 0x050000
//    s += msg.toLocal8Bit();
//    Q_UNUSED(context)
//#else
//    s += msg;
//#endif
    ba += '\n';

    file->write(ba);
    file->flush();

    if(printTostderr){
        fprintf(stderr, "%s\n", ba.data());
    }

    if (type == QtFatalMsg) {
        closeDebugLog();
        exit(1);
    }
}



static void installMessageLogger(const QString &baseDir, const QString &fileBaseName, bool printMsgTostderr = false){
    if(!baseDir.trimmed().isEmpty()){
        m_baseDir = baseDir;
    }
    QDir dir(baseDir);
    if(!dir.exists()){
        dir.mkpath(baseDir);
    }
    if(!fileBaseName.trimmed().isEmpty()){
        m_fileBaseName = fileBaseName;
    }

#  if QT_VERSION >= 0x050000
    //reset the message handler
    qInstallMessageHandler(0);
    qInstallMessageHandler(logDebug);
#  else
    qInstallMsgHandler(0);
    qInstallMsgHandler(logDebug);
#  endif
    qAddPostRoutine(closeDebugLog);

    printTostderr = printMsgTostderr;

}

static void uninstallMessageLogger(){
    //reset the message handler
#  if QT_VERSION >= 0x050000
    qInstallMessageHandler(0);
#  else
    qInstallMsgHandler(0);
#  endif

}
























#endif // LOGDEBUG_H
