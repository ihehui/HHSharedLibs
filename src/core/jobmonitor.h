#ifndef JOBMONITOR_H
#define JOBMONITOR_H

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QVariant>

#include "core_lib.h"


namespace HEHUI
{

class CORE_LIB_API Job : public QObject
{
    Q_OBJECT
public:
    //enum JobState{NotRunning, Starting, Running};
    enum JobResult {Finished = 0, FinishedWithError, Failed, Timeout, Working};
    explicit Job(quint32 jobType, const QString &jobTitle, QObject *parent = 0);
    ~Job();
    void startTimer(int msecTimeout = 30000);
    void stopTimer();

public:
    quint32 ID;
    quint32 Type;
    QString Title;
    int CurrentProgress;
    QString CurrentProgressMessage;
    JobResult Result;
    QVariant ExtraData;

signals:
    void timeout();

private:
    QTimer m_timer;
    bool m_timeout;

};
///////////////////////////////////////////////



class CORE_LIB_API JobMonitor : public QObject
{
    Q_OBJECT
public:
    static JobMonitor *instance();
    static void destoryInstance();

private:
    explicit JobMonitor(QObject *parent = 0);
    ~JobMonitor();

public:
    quint32 newJob(quint32 jobType, const QString &jobTitle);
    Job *getJob(quint32 jobID) const;
    void deleteJob(quint32 jobID);

signals:
    void jobFinished(quint32 jobID);

public slots:
    bool startJobMonitoring(quint32 jobID, int msecTimeout = 30000);
    void finishJob(quint32 jobID, quint8 result, const QVariant &extraData);
    void updateJobProgress(quint32 jobID, int progress, const QString &progressMessage);

private slots:
    void jobTimeout();

private:
    static JobMonitor *m_instance;
    QHash<quint32 /*Job ID*/, Job *> m_jobs;



};

} //namespace HEHUI

#endif // JOBMONITOR_H
