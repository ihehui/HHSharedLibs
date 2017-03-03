//////////////////////////////////////////////////////////////
///     Author:HeHui
///     Update:2017.2.9
///
///
///                         Usage:
///     //INIT
///     MessageLogger *logger = new MessageLogger();
///     logger->setLogQtMessagesEnabled(true);
///     logger->start();
///     logger->setOutputTargets(MessageLoggerBase::TARGET_FILE | MessageLoggerBase::TARGET_CONSOLE);
///     logger->setOutputTypes(MSG_ANY, MSG_IMPORTANT, MSG_ANY);
///     FileConfigStruct config;
///     config.baseDir = QDir::homePath() + "/log";
///     config.dirTimeTemplate = "/yyyy";
///     config.fileBaseName = "log";
///     config.fileNameTimeTemplate = "yyyyMMdd";
///     logger->setFileConfig(config);
///
///     //Start logging
///     LogDebug()<<"This is a debug message.";
///     ...
///     ...
///     ...
///     //Close before App quit
///     logger->quit();
///     delete logger;
///
//////////////////////////////////////////////////////////////


#ifndef MESSAGELOGGERBASE_H
#define MESSAGELOGGERBASE_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QFile>
#include <QStringList>

#define NO_DEBUG_OUTPUT




enum MsgType {
    MSG_WARNING = 0x01, //1
    MSG_CRITICAL = 0x02, //2
    MSG_FATAL = 0x04, //4
    MSG_IMPORTANT = MSG_WARNING | MSG_CRITICAL | MSG_FATAL, //7
    MSG_INFO = 0x08, //8
    MSG_DEBUG = 0x10, //16

    MSG_ANY = 0xFF //255
};

struct MessageStruct{
    MessageStruct(){
        threadId = "0x00";
        time = QDateTime();
        logType = MSG_DEBUG;
        messages = "";
        fileName = "";
        funtionName = "";
        lineNumber = 0;

        ref = 1;
    }

    QString threadId;
    QDateTime time;
    MsgType logType;
    QString messages;
    QString fileName;
    QString funtionName;
    int lineNumber;

    int ref;

};

struct FileConfigStruct{
    FileConfigStruct(){
        baseDir = "./log";
        dirTimeTemplate = "/yyyy";
        fileBaseName = "log";
        fileNameTimeTemplate = "yyyyMMdd";
        fileSuffix = "log";

        maxFileSize = 10000000;
        truncateFile = true;
        file = 0;
    }

    QString baseDir;
    QString dirTimeTemplate;
    QString fileBaseName;
    QString fileNameTimeTemplate;
    QString fileSuffix;
    qint64 maxFileSize;
    bool truncateFile;
    QFile *file;

};


//////////////////////////////////////////////////////////////////////////

class NoLogMessage
{
public:

    template <typename T>
    inline NoLogMessage &operator<< (const T &t) { return *this; }
};

class MessageLoggerBase;

class LogMessage
{

public:
    LogMessage(const QString &fileName, const QString &funtionName, int lineNumber, MsgType type);
    LogMessage(const LogMessage &other);
    LogMessage &operator=(const LogMessage &other);
    virtual ~LogMessage();


    LogMessage & operator<< (const long &value);
    LogMessage & operator<< (const QVariant &variant);

    static void setMessageLoggerBase(MessageLoggerBase *messageLoggerBase);

private:
    MessageStruct *m_messageStruct;

    static MessageLoggerBase *m_messageLoggerBase;

};

namespace QtPrivate {

template <typename SequentialContainer>
inline LogMessage printSequentialContainer(LogMessage debug, const char *which, const SequentialContainer &c)
{
    debug << which << "(";
    typename SequentialContainer::const_iterator it = c.begin(), end = c.end();
    if (it != end) {
        debug << *it;
        ++it;
    }
    while (it != end) {
        debug << ", " << *it;
        ++it;
    }
    debug << ")";
    return debug;
}

} // namespace QtPrivate

template <class T>
inline LogMessage operator<<(LogMessage debug, const QList<T> &list)
{
    return QtPrivate::printSequentialContainer(debug, "List", list);
}

template <typename T>
inline LogMessage operator<<(LogMessage debug, const QVector<T> &vec)
{
    return QtPrivate::printSequentialContainer(debug, "QVector", vec);
}

template <typename T, typename Alloc>
inline LogMessage operator<<(LogMessage debug, const std::vector<T, Alloc> &vec)
{
    return QtPrivate::printSequentialContainer(debug, "std::vector", vec);
}

template <typename T, typename Alloc>
inline LogMessage operator<<(LogMessage debug, const std::list<T, Alloc> &vec)
{
    return QtPrivate::printSequentialContainer(debug, "std::list", vec);
}

template <typename Key, typename T, typename Compare, typename Alloc>
inline LogMessage operator<<(LogMessage debug, const std::map<Key, T, Compare, Alloc> &map)
{
    return QtPrivate::printSequentialContainer(debug, "std::map", map); // yes, sequential: *it is std::pair
}

template <typename Key, typename T, typename Compare, typename Alloc>
inline LogMessage operator<<(LogMessage debug, const std::multimap<Key, T, Compare, Alloc> &map)
{
    return QtPrivate::printSequentialContainer(debug, "std::multimap", map); // yes, sequential: *it is std::pair
}

template <class Key, class T>
inline LogMessage operator<<(LogMessage debug, const QMap<Key, T> &map)
{
    debug << "QMap(";
    for (typename QMap<Key, T>::const_iterator it = map.constBegin();
         it != map.constEnd(); ++it) {
        debug << '(' << it.key() << ", " << it.value() << ')';
    }
    debug << ')';
    return debug;
}

template <class Key, class T>
inline LogMessage operator<<(LogMessage debug, const QHash<Key, T> &hash)
{
    debug << "QHash(";
    for (typename QHash<Key, T>::const_iterator it = hash.constBegin();
            it != hash.constEnd(); ++it)
        debug << '(' << it.key() << ", " << it.value() << ')';
    debug << ')';
    return debug;
}

template <class T1, class T2>
inline LogMessage operator<<(LogMessage debug, const QPair<T1, T2> &pair)
{
    debug << "QPair(" << pair.first << ',' << pair.second << ')';
    return debug;
}

template <class T1, class T2>
inline LogMessage operator<<(LogMessage debug, const std::pair<T1, T2> &pair)
{
    debug << "std::pair(" << pair.first << ',' << pair.second << ')';
    return debug;
}

template <typename T>
inline LogMessage operator<<(LogMessage debug, const QSet<T> &set)
{
    return QtPrivate::printSequentialContainer(debug, "QSet", set);
}

template <class T>
inline LogMessage operator<<(LogMessage debug, const QContiguousCache<T> &cache)
{
    debug << "QContiguousCache(";
    for (int i = cache.firstIndex(); i <= cache.lastIndex(); ++i) {
        debug << cache[i];
        if (i != cache.lastIndex())
            debug << ", ";
    }
    debug << ')';
    return debug;
}

template <class T>
inline LogMessage operator<<(LogMessage debug, const QSharedPointer<T> &ptr)
{
    debug << "QSharedPointer(" << ptr.data() << ")";
    return debug;
}


template <typename Int>
void qt_QMetaEnum_flagDebugOperator(LogMessage &debug, size_t sizeofT, Int value)
{
    debug << "QFlags(" << hex << showbase;
    bool needSeparator = false;
    for (uint i = 0; i < sizeofT * 8; ++i) {
        if (value & (Int(1) << i)) {
            if (needSeparator)
                debug << '|';
            else
                needSeparator = true;
            debug << (Int(1) << i);
        }
    }
    debug << ')';
}

#define LogDebug() LogMessage(__FILE__, Q_FUNC_INFO, __LINE__, MSG_DEBUG)
#define LOGDEBUG LogDebug()
#define LogInfo() LogMessage(__FILE__, Q_FUNC_INFO, __LINE__, MSG_INFO)
#define LOGINFO LogInfo()
#define LogWarning() LogMessage(__FILE__, Q_FUNC_INFO, __LINE__, MSG_WARNING)
#define LOGWARNING LogWarning()
#define LogCritical() LogMessage(__FILE__, Q_FUNC_INFO, __LINE__, MSG_CRITICAL)
#define LOGCRITICAL LogCritical()
#define LogFatal() LogMessage(__FILE__, Q_FUNC_INFO, __LINE__, MSG_FATAL)
#define LOGFATAL LogFatal()

#define LogQtDebugMessage(file, function, line, msgType) LogMessage(file, function, line, msgType)
#define LogQtInfoMessage(file, function, line, msgType) LogMessage(file, function, line, msgType)
#define LogQtWarningMessage(file, function, line, msgType) LogMessage(file, function, line, msgType)

#if defined(NO_DEBUG_LOG)
#  undef LogDebug
#  define LogDebug() NoLogMessage()

#  undef LogQtDebugMessage
#  define LogQtDebugMessage(file, function, line, msgType) NoLogMessage()
#endif

#if defined(NO_WARNING_LOG)
#  undef LogWarning
#  define LogWarning() NoLogMessage()

#  undef LogQtWarningMessage
#  define LogQtWarningMessage(file, function, line, msgType) NoLogMessage()
#endif

#if defined(NO_INFO_LOG)
#  undef LogInfo()
#  define LogInfo() NoLogMessage()

#  undef LogQtInfoMessage()
#  define LogQtInfoMessage(file, function, line, msgType) NoLogMessage()
#endif

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
class MessageLoggerBase : public QThread
{
    Q_OBJECT
public:
    enum OutputTarget {TARGET_CONSOLE = 0x01, TARGET_FILE = 0x02, TARGET_DATABASE = 0x04};

    MessageLoggerBase(QObject *parent = 0);
    virtual ~MessageLoggerBase();

    void stopLogger();


    void setOutputTargets(quint8 targets); //The targets is a OR'ed combination of OutputTarget
    void setOutputTypes(quint8 typesForFile = MSG_IMPORTANT, quint8 typesForDatabase = MSG_IMPORTANT, quint8 typesForConsole = MSG_ANY); //The targets is a OR'ed combination of MsgType

    void setFileConfig(FileConfigStruct fileConfig);

    void appendMessage(MessageStruct *messageStruct);

    quint8 getOutputTarget();
    bool isOutputToDatabase();

    void setLogQtMessagesEnabled(bool enabled);
    bool isloggingQtMessages() const;

    void setAppVersion(const QString &appVersion);

protected:
    void run();

private slots:
    void wakeUp();

private:
    virtual bool beginOutputMessage();
    void outputMessage(MessageStruct *messageStruct);
    virtual bool endOutputMessage();

    virtual bool saveToFile(MessageStruct *messageStruct);
    virtual bool closeFile();

    virtual bool saveToDatabase(MessageStruct *messageStruct);
    virtual void closeDatabase();

    QString messageTypeToString();

private:
    quint8 m_outputTarget;
    quint8 m_logTypesForFile;
    quint8 m_logTypesForDatabase;
    quint8 m_logTypesForConsole;

    FileConfigStruct m_fileConfigStruct;

    bool m_quit;
    bool m_wokeUp;
    bool m_logQtMessages;

    QHash<MsgType, QString> m_messageTypeToString;

    QMutex mutex;
    QList<MessageStruct *> m_messageList;

    QWaitCondition cond;

protected:
    QString m_appVersion;
    static qulonglong m_processId;



};
//////////////////////////////////////////////////////////////////////////


#endif // MESSAGELOGGERBASE_H
