#ifndef LOGDEBUG_H
#define LOGDEBUG_H


//#if defined(LOG_DEBUG)
#include <QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTime>
#include <QtCore/QMutex>
#if defined(Q_OS_WIN32)
    #include <qt_windows.h>
#else
    #include <unistd.h>
    #include <stdlib.h>
#endif

static QFile *f = 0;
static qulonglong processId = 0;

static void closeDebugLog()
{
    if (!f) {
        return;
    }

    QByteArray s(QTime::currentTime().toString("HH:mm:ss.zzz").toLatin1());
    s += " [";
    s += QByteArray::number(processId);
    s += "] ";
    QByteArray ps(s + "------ DEBUG LOG CLOSED ------\n");
    f->write(ps);
    f->flush();
    f->close();
    delete f;
    f = 0;
}


#if QT_VERSION >= 0x050000
    static void logDebug(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
    static void logDebug(QtMsgType type, const char *msg)
#endif
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if(!processId) {
#if defined(Q_OS_WIN32)
        processId = GetCurrentProcessId();
#else
        processId = getpid();
#endif
    }

    QByteArray s(QTime::currentTime().toString("HH:mm:ss.zzz").toLatin1());
    s += " [";
    s += QByteArray::number(processId);
    s += "] ";

    if (!f) {
#if defined(Q_OS_WIN32)
        f = new QFile("./" + logFilename);
#else
        f = new QFile("/tmp/" + logFilename);
#endif
        if (!f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            delete f;
            f = 0;
            return;
        }
        QByteArray ps('\n' + s + "------ DEBUG LOG OPENED ------\n");
        f->write(ps);
    }

    switch (type) {
    case QtWarningMsg:
        s += "WARNING: ";
        break;
    case QtCriticalMsg:
        s += "CRITICAL: ";
        break;
    case QtFatalMsg:
        s += "FATAL: ";
        break;
    case QtDebugMsg:
        s += "DEBUG: ";
        break;
    default:
        // Nothing
        break;
    }




#if QT_VERSION >= 0x050000
    QString str = QString("%1 (%2:%3, %4)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
    s += str.toUtf8();
#else
    s += msg;
#endif


//#if QT_VERSION >= 0x050400
//    s += qFormatLogMessage(type, context, msg).toLocal8Bit();
//#elif QT_VERSION >= 0x050000
//    s += msg.toLocal8Bit();
//    Q_UNUSED(context)
//#else
//    s += msg;
//#endif
    s += '\n';

    f->write(s);
    f->flush();

    if (type == QtFatalMsg) {
        closeDebugLog();
        exit(1);
    }
}



static void installMessageLogger()
{

#  if QT_VERSION >= 0x050000
    //reset the message handler
    qInstallMessageHandler(0);
    qInstallMessageHandler(logDebug);
#  else
    qInstallMsgHandler(0);
    qInstallMsgHandler(logDebug);
#  endif
    qAddPostRoutine(closeDebugLog);

}

static void uninstallMessageLogger()
{
    //reset the message handler
#  if QT_VERSION >= 0x050000
    qInstallMessageHandler(0);
#  else
    qInstallMsgHandler(0);
#  endif

}
























#endif // LOGDEBUG_H
