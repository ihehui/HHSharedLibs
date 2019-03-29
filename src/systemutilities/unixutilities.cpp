#include "unixutilities.h"


#include <sys/sysinfo.h>

#include <sys/time.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
//#include <ctype.h>

#include <errno.h>
#include <mntent.h>

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QElapsedTimer>
#include <QCoreApplication>



double cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n)
{
    double od, nd;
    double id, sd;
    double cpu_use ;

    od = (double) (o->user + o->nice + o->system +o->idle+o->softirq+o->iowait+o->irq);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (double) (n->user + n->nice + n->system +n->idle+n->softirq+n->iowait+n->irq);//第二次(用户+优先级+系统+空闲)的时间再赋给od

    id = (double) (n->idle);    //用户第一次和第二次的时间之差再赋给id
    sd = (double) (o->idle) ;    //系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
        cpu_use =100.0- ((id-sd))/(nd-od)*100.00; //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else cpu_use = 0;
    return cpu_use;
}

void get_cpuoccupy (CPU_OCCUPY *cpust)
{
    FILE *fd;
    int n;
    char buff[256];
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy=cpust;

    fd = fopen ("/proc/stat", "r");
    fgets (buff, sizeof(buff), fd);

    sscanf (buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle ,&cpu_occupy->iowait,&cpu_occupy->irq,&cpu_occupy->softirq);

    fclose(fd);
}

double getCpuRate()
{
    CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;
    double cpu;
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
    sleep(1);

    //第二次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);

    //计算cpu使用率
    cpu = cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);

    return cpu;
}


static const char *ignore_fs[] = {
    "none", "proc", "sysfs", "devpts", "usbfs", "usbdevfs", NULL };
void getSysDrivesStatus(int *count, char info[][512])
{

    struct mntent   *mnt;
    struct statfs   fsu;
    FILE            *fp;
    char            *table = MOUNTED;
    long            total, used, available, available_to_root;
    double          pct;
    char            **p;
    int             scale, flag = 0;
    char            buf[BUFSIZ];
    memset (buf, 0, BUFSIZ);
    fp = setmntent (table, "r");
    if (fp == NULL)
    {
        return;
    }

    int index = 0;
    while ((mnt = getmntent (fp)))
    {

        for (p = (char **)ignore_fs; *p; p++)
        {
            if (strcmp (mnt->mnt_fsname, *p) == 0)
            {
                flag = 1;
                break;;
            }
        }
        if (flag)
        {
            flag = 0;
            continue;
        }
        if (statfs (mnt->mnt_dir, &fsu) < 0)
        {
            endmntent (fp);
            *count = index;
            return;
        }
        else
        {
            total = fsu.f_blocks;
            available = fsu.f_bavail;
            available_to_root = fsu.f_bfree;
            used  = total - available_to_root;
            scale = fsu.f_bsize / 1024;
            pct = 0;
            if (total != 0 )
                pct = (used * 100) / (used + available) + ((used *100) % (used
                                                                          + available) != 0);
//            fprintf (stdout, "%-15s %15s %10ld %10ld %10ld %9.0f%% %-s\n",
//                     mnt->mnt_fsname, mnt->mnt_type, total*scale, used*scale,
//                     available*scale, pct, mnt->mnt_dir);

            snprintf (info[index], 512, "%s %s %ld %ld %ld %.0f%% %s",
                     mnt->mnt_fsname, mnt->mnt_type, total*scale, used*scale,
                     available*scale, pct, mnt->mnt_dir);

        }

        index++;
        if((*count) <= index){
            endmntent (fp);
            return;
        }
    }
    endmntent (fp);

    *count = index;

}

bool getHardDriveSerialNumber(char *drive, char *sn, int snMaxLength)
{
    // drive: /dev/sda, Root NOLY!!!

    struct hd_driveid hid;

    int fd = open(drive, O_RDONLY);
    if(fd < 0 ){return false;}

    if(ioctl(fd, HDIO_GET_IDENTITY, &hid) < 0){return false;}

    snprintf(sn, snMaxLength, "%s", hid.serial_no);

    close(fd);
    return true;
}

//////////////////////////////////////////////////////////////////////////

UnixUtilities::UnixUtilities(QObject *parent) : QObject(parent)
{

}

int UnixUtilities::getCPULoad()
{
    return getCpuRate();

    //    QProcess process;
    //    //QString cmdString = QString("top -n 1 |grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
    //    QString cmdString = QString("top -n 1 |grep Cpu | cut -d \",\" -f 4");
    //    process.start(cmdString);
    //    if(!process.waitForFinished()) {
    //        return 0;
    //    }
    //    if(!process.waitForReadyRead()) {
    //        return 0;
    //    }
    //    QString idle = QString::fromLocal8Bit(process.readAllStandardOutput());
    //    if(!idle.endsWith("%id")) {
    //        return 0;
    //    }
    //    idle = idle.replace("%id", "");
    //    return idle.toUInt();

}
bool UnixUtilities::getMemoryStatus(quint64 *totalBytes, int *loadPercentage)
{

    {
        struct sysinfo tmp;
        int ret = 0;
        ret = sysinfo(&tmp);
        if(0 == ret){
            if(totalBytes) {
                *totalBytes = tmp.totalram;
            }

            if(loadPercentage) {
                *loadPercentage = (int)((1 - ((double)tmp.freeram) / ((double)tmp.totalram) ) * 100);
            }
        }
        return true;
    }



    //    QProcess process;
    //    //QString cmdString = QString("top -n 1 | grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
    //    //QString cmdString = QString("cat /proc/meminfo | grep Mem");
    //    QString cmdString = QString("top -n 1 | grep Mem");

    //    process.start(cmdString);
    //    if(!process.waitForFinished()) {
    //        return false;
    //    }
    //    if(!process.waitForReadyRead()) {
    //        return false;
    //    }
    //    QString memString = QString::fromLocal8Bit(process.readAllStandardOutput());
    //    if(!memString.startsWith("Mem:")) {
    //        return false;
    //    }
    //    memString = memString.replace("Mem:", "").simplified();
    //    QStringList list = memString.split(",");
    //    if(list.size() != 4) {
    //        return false;
    //    }
    //    QString str = list.at(0);
    //    quint64 totalMem = str.replace("total", "").simplified().toULongLong();
    //    str = list.at(0);
    //    quint64 usedMem = str.replace("used", "").simplified().toULongLong();
    //    if(!totalMem || !usedMem) {
    //        return false;
    //    }

    //    if(totalBytes) {
    //        *totalBytes = totalMem * 1024;
    //    }

    //    if(loadPercentage) {
    //        *loadPercentage = (int)(((float)usedMem / (*totalBytes)) * 100);
    //    }


    return true;
}

QStringList UnixUtilities::getDrivesStatus()
{

    int count = 512;
    char info[count][512];
    memset(info, 0, sizeof(info));

    getSysDrivesStatus(&count, info);

    QStringList infoList;
    QString infoStr;
    for(int i=0; i<count; i++){
        infoStr = QString::fromLocal8Bit(&info[i][0]);
        if(infoStr.isEmpty() || (!infoStr.startsWith("/dev/"))){continue;}
        infoList.append(infoStr);
        qDebug()<<i<<": "<<infoStr;
    }

    return infoList;
}

QString UnixUtilities::getDriveSN(const QString &drive){
    // drive: /dev/sda, Root NOLY!!!

    char sn[256];
    memset(sn, 0, sizeof(sn));
    if(getHardDriveSerialNumber(drive.toLocal8Bit().data(), sn, 256)){
        return QString::fromLocal8Bit(sn).simplified();
    }

    return "";
}

bool UnixUtilities::getAllUsersLoggedOn(QStringList *users)
{
    if(!users){return false;}

    QProcess process;
    QString cmdString = QString("who -m");
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return false;
    }
    if(!process.waitForReadyRead()) {
        return false;
    }

    QByteArray output = process.readAllStandardOutput();
    QString line = "";
    QTextStream in(&output);
    QString name = "";
    while (in.readLineInto(&line)) {
        name = line.split(" ").first();
        if(name.trimmed().isEmpty()){continue;}
        users->append(name);
    }

    return true;
}

QStringList UnixUtilities::localCreatedUsers()
{
    QStringList usersList;

    QFile file("/etc/passwd");
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly)){
        qCritical()<<QString("Failed to open file! %1").arg(file.errorString());
        return usersList;
    }

    QString line = "";
    QString name = "";
    while (!file.atEnd()) {
        line = QString::fromLocal8Bit(file.readLine());
        name = line.split(":").first();
        if(name.trimmed().isEmpty()){continue;}
        usersList.append(name);
    }

    return usersList;
}

bool UnixUtilities::getLocalGroupsTheUserBelongs(QStringList *groups, const QString &userName, unsigned long *errorCode)
{
    if(!groups || userName.trimmed().isEmpty()){
        return false;
    }

    QFile file("/etc/group");
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly)){
        qCritical()<<QString("Failed to open file! %1").arg(file.errorString());
        return false;
    }

    QString line = "";
    QString groupName = "";
    QStringList infoList;
    while (!file.atEnd()) {
        line = QString::fromLocal8Bit(file.readLine());
        infoList = line.split(":");
        if(infoList.size() < 4){continue;}

        groupName = infoList.first();
        if(groupName.trimmed().isEmpty()){continue;}

        infoList = infoList.at(3).split(",");
        if(infoList.contains(userName)){
            groups->append(groupName);
        }

    }

    return true;
}

bool UnixUtilities::getAllUsersInfo(QJsonArray *jsonArray,  unsigned long *errorCode)
{

    if(!jsonArray) {
        return false;
    }

    QFile file("/etc/passwd");
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly)){
        qCritical()<<QString("Failed to open file! %1").arg(file.errorString());
        return false;
    }

    QStringList loggedonUser;
    getAllUsersLoggedOn(&loggedonUser);
    loggedonUser.removeDuplicates();

    QString line = "";
    QString userName = "";
    QStringList infoList;
    while (!file.atEnd()) {
        line = QString::fromLocal8Bit(file.readLine());
        infoList = line.split(":");
        if(infoList.size() < 7){continue;}
        userName = infoList.first();

        QJsonArray array;
        array.append(userName); //User Name

        bool loggedon =  loggedonUser.contains(userName, Qt::CaseInsensitive);
        array.append(QString::number(loggedon)); //Loggedon

        array.append(infoList.at(5)); //Home Dir
        array.append(infoList.at(4)); //Comment

        array.append(QString::number(0)); //Account Disabled
        array.append(QString::number(0)); //Can not Change Password
        array.append(QString::number(0)); //Account Locked
        array.append(QString::number(1)); //Password Never Expires

        array.append(infoList.at(4)); //FullName

        array.append(QString::number(0)); //Last Logon Time
        array.append(QString::number(0)); //Last Logoff Time
        array.append(infoList.at(2)); //UID

        array.append(infoList.at(6)); //Profile

        array.append(QString::number(0)); //Must Change Password

        QStringList groups;
        getLocalGroupsTheUserBelongs(&groups, userName);
        groups.sort(Qt::CaseInsensitive);
        array.append(groups.join(";")); //Groups


        jsonArray->append(array);
    }

    return true;
}

bool UnixUtilities::serviceGetAllServicesInfo(QJsonArray *jsonArray, unsigned long *errorCode, unsigned long serviceType)
{

    if(!jsonArray) {
        return false;
    }

    QProcess process;
    QString cmdString = QString("who -m");
    process.start(cmdString);
    if(!process.waitForFinished()) {
        return false;
    }
    if(!process.waitForReadyRead()) {
        return false;
    }
    QByteArray output = process.readAllStandardOutput();
    if(output.trimmed().isEmpty()){
        return false;
    }

    char serviceName[1024];
    char loaded[32];
    char active[32];
    char running[32];
    char description[1024];
    int num = 0;

    QString line = "";
    QTextStream in(&output);
    while (in.readLineInto(&line)) {

        memset(serviceName, 0, 1024);
        memset(loaded, 0, 32);
        memset(active, 0, 32);
        memset(running, 0, 32);
        memset(description, 0, 1024);
        num = sscanf(line.toLocal8Bit().data(), "%s %s %s %s %[^\n]", serviceName, loaded, active, running, description);
        if(5 != num){
            QString msg = tr("Failed to parse data: %1").arg(line);
            qCritical()<<msg;
            continue;
        }

        QJsonArray array;
        array.append(serviceName); //Service Name
        array.append(QString(serviceName).replace(".service", "")); //Display Name
        array.append(QString::number(0)); //Process ID
        array.append(QString(description).trimmed()); //Description
        array.append(active); //Start Type
        array.append("Root"); //Account
        array.append(""); //Dependencies
        array.append(""); //Binary Path
        array.append(""); //Service Type

        jsonArray->append(array);
    }

    return true;

}

QString UnixUtilities::getOSVersionInfo()
{
    QString unameInfo = QSysInfo::prettyProductName();
    QProcess process;
    process.start("uname -srm");
    if(process.waitForStarted() && process.waitForFinished()) {
        unameInfo = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        if(unameInfo.trimmed().isEmpty()){
            unameInfo = QSysInfo::prettyProductName();
        }
    }


    QString cmd = QString("sh -c \"lsb_release  -a | grep Description | cut -d \":\" -f 2\"");
    process.start(cmd);
    if(!process.waitForStarted()) {
        return unameInfo;
    }
    if(!process.waitForFinished()) {
        return unameInfo;
    }

    QString osInfo = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    if(osInfo.trimmed().isEmpty()){
        return unameInfo;
    }

    if(!unameInfo.contains(osInfo, Qt::CaseInsensitive)){
        osInfo += QString("(%1)").arg(unameInfo);
    }

    return osInfo;
}

bool UnixUtilities::getOSInfo(QJsonObject *object){
    if(!object){return false;}

    QString osInfo = getOSVersionInfo();

    object->insert("OS", osInfo);
    object->insert("InstallDate", "");
    object->insert("Key", "");

    return true;
}

bool UnixUtilities::shutdown(const QString &machineName, const QString &message, unsigned long timeout, bool forceAppsClosed, bool rebootAfterShutdown, QString *errorMessage)
{
    QProcess process;
    QString cmdString = QString("shutdown");
    QStringList args;
    if(rebootAfterShutdown){
        args.append("-r");
    }else{
        args.append("-h");
    }
    args.append(QString("+%1").arg(timeout/60));
    if(!message.trimmed().isEmpty()){
        args.append(message);
    }

    process.start(cmdString, args);
    if(!process.waitForStarted()) {
        QString msg = process.errorString();
        if(errorMessage){
            *errorMessage = msg;
        }
        qCritical()<<msg;
        return false;
    }
    if(!process.waitForFinished()) {
        QString msg = process.errorString();
        if(errorMessage){
            *errorMessage = msg;
        }
        qCritical()<<msg;
        return false;
    }

    return true;
}

bool UnixUtilities::setComputerName(const QString &newComputerName, const QString &rootPassword, unsigned long *errorCode, QString *errorMessage)
{

    QProcess process;
    QString cmdString = QString("sh");
    process.start(cmdString);
    if(!process.waitForStarted()) {
        QString msg = process.errorString();
        if(errorMessage){
            *errorMessage = msg;
        }
        qCritical()<<msg;
        return false;
    }

    cmdString = QString("echo %1 | sudo -S sh -c \"echo %2 > /etc/hostname && hostname %2\"").arg(rootPassword).arg(newComputerName);
    process.write(cmdString.toLocal8Bit());
    process.closeWriteChannel();

    if(!process.waitForFinished()) {
        QString msg = process.errorString();
        if(errorMessage){
            *errorMessage = msg;
        }
        qCritical()<<msg;
        return false;
    }

    //Verify
    cmdString = QString("sh -c \"cat /etc/hostname && hostname\"").arg(rootPassword).arg(newComputerName);
    process.start(cmdString);
    if(!process.waitForStarted()) {
        QString msg = process.errorString();
        if(errorMessage){
            *errorMessage = msg;
        }
        qCritical()<<msg;
        return false;
    }
    if(!process.waitForFinished()) {
        QString msg = process.errorString();
        if(errorMessage){
            *errorMessage = msg;
        }
        qCritical()<<msg;
        return false;
    }

    QString output = process.readAllStandardOutput();
    if(!output.contains(newComputerName+"\n"+newComputerName)){
        QString msg = QString("Error occurred while setting up computer name!");
        if(errorMessage){
            *errorMessage = msg;
        }
        qCritical()<<msg;
        return false;
    }

    return true;
}
