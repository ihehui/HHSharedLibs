/*
 ****************************************************************************
 * service.h
 *
 * Created on: 2009-5-1
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
 * Last Modified on: 2010-05-15
 * Last Modified by: 贺辉
 ***************************************************************************
 */


#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
//#include <QApplication>
#include <QString>

//#ifdef Q_OS_WIN
// #include <windows.h>
//#endif

#include "../servicelib.h"

#include "../3rdparty/qtservice/src/qtservice.h"


class SERVICE_LIB_API Service : public QObject, public QtService<QCoreApplication>
{
    Q_OBJECT

public:
    Service(int argc, char **argv, const QString &serviceName = "Service", const QString &description = "A Qt service.");
    ~Service();

//#ifdef Q_OS_WIN
//    virtual bool runAs(const QString &userName, const QString &password, const QString &exeFilePath, const QString &parameters = "", bool show = true, const QString &workingDir = QCoreApplication::applicationDirPath(), bool wait = false);

//    int changeToDefaultDesktop();
//    void resetDesktop();
//#endif

protected:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void processCommand(int code) = 0;


public slots:

private:





};

#endif // SERVICE_H
