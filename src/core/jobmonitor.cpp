#include "jobmonitor.h"

#include <QDebug>

namespace HEHUI {


Job::Job(quint32 jobType, const QString &jobTitle, QObject *parent) : QObject(parent)
{

    ID = 0;
    Type = jobType;
    Title = jobTitle;
    CurrentProgress = 0;
    CurrentProgressMessage = "";
    Result = Working;
    ExtraData = QVariant("");

    m_timeout = false;


    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SIGNAL(timeout()));

}

Job::~Job(){
    qDebug()<<"--Job::~Job()";
    m_timer.stop();
}

void Job::startTimer(int msecTimeout){
    m_timer.start(msecTimeout);
}

void Job::stopTimer(){
    m_timer.stop();
}

//////////////////////////////////////////////////////////////////



JobMonitor *JobMonitor::m_instance = 0;

JobMonitor * JobMonitor::instance(){
    if(!m_instance){
        m_instance = new JobMonitor();
    }
    return m_instance;
}

void JobMonitor::destoryInstance(){
    if(m_instance){
        delete m_instance;
        m_instance = 0;
    }
}


JobMonitor::JobMonitor(QObject *parent) : QObject(parent)
{

}

JobMonitor::~JobMonitor(){
    qDeleteAll(m_jobs);
}

quint32 JobMonitor::newJob(quint32 jobType, const QString &jobTitle){
    static quint32 jobID = 1;
    while (m_jobs.contains(jobID)) {
        jobID = jobID%0x7FFFFFFF + 1;
    }

    Job *job = new Job(jobType, jobTitle, this);
    connect(job, SIGNAL(timeout()), this, SLOT(jobTimeout()));

    m_jobs.insert(jobID, job);

    return jobID;
}

Job * JobMonitor::getJob(quint32 jobID) const{
    return m_jobs.value(jobID);
}

void JobMonitor::deleteJob(quint32 jobID){
    Job *job = m_jobs.take(jobID);
    if(!job){return;}
    delete job;
}

bool JobMonitor::startJobMonitoring(quint32 jobID, int msecTimeout){
    Job *job = m_jobs.value(jobID);
    if(!job){return false;}

    job->startTimer(msecTimeout);
    return true;
}

void JobMonitor::finishJob(quint32 jobID, quint8 result, const QVariant &extraData){
    Job *job = m_jobs.value(jobID);
    if(!job){return;}

    job->stopTimer();
    job->Result = Job::JobResult(result);
    job->ExtraData = extraData;

    emit jobFinished(jobID);
}

void JobMonitor::updateJobProgress(quint32 jobID, int progress, const QString &progressMessage){
    Job *job = m_jobs.value(jobID);
    if(!job){return;}

    job->CurrentProgress = progress;
    job->CurrentProgressMessage = progressMessage;
}

void JobMonitor::jobTimeout(){
    Job *job = qobject_cast<Job *>(sender());
    if(!job){return;}

    job->Result = Job::Timeout;

    emit jobFinished(job->ID);
}





}
