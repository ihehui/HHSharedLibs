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

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "../servicelib.h"

#include "../3rdparty/qtservice/src/qtservice.h"


class SERVICE_LIB_API Service : public QObject, public QtService<QCoreApplication>
{
    Q_OBJECT

public:
    Service(int argc, char **argv, const QString &serviceName = "Service", const QString &description = "A Qt service.", bool interactive = false);
    ~Service();

#ifdef Q_OS_WIN
    static QString WinSysErrorMsg(DWORD winErrorCode, DWORD dwLanguageId = 0);

    /// Run application in active console session with current console process token information.
    /// Service application(usually in session 0) can start another process in active console session(usually session 1) which has the same token information,
    /// the child process can interact with the input desktop(usually session 1) if setDesktopToCurrentThread(OpenInputDesktop()) is called.
    static bool runASCurrentConsoleProcessInActiveConsoleSession(const QString &exeFilePath, const QString &parameters = "", bool show = true,
                                                                 const QString &workingDir = "", bool wait = false, DWORD milliseconds = 6000);

    /// For Windows, the service process(usually in session 0) can interact with the input desktop(usually session 1)
    /// if setDesktopToCurrentThread(OpenInputDesktop()) is called in class constructor.
    static bool enableInteractWithInputDesktop();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    static DWORD getActiveConsoleSessionId();

    ////Desktop
    // This funtion gets a handle to a desktop that receive user inputs.
    // If success the function returns a handle to the desktop. On fail the function returns zero.
    // After use the returned handle must be called the closeDesktop() function.
    static HDESK getInputDesktop();

    // This funtion gets a handle to a named desktop by the name.
    // Param: name - a QString object that contain a valid desktop name such as "Winlogon".
    // if success the function returns a handle to the desktop that receives user inputs. On fail the function returns zero.
    // After use the returned handle must be called the closeDesktop() function.
    static HDESK getDesktop(const QString &name);

    // This function closes a handle to a desktop.
    // Param hdesk is a handle to a desktop that will be closed.
    // If success the function return true else false.
    static bool closeDesktop(HDESK hdesk);

    // This function sets a desktop to a thread from that it was called.
    // Param: newDesktop - is a handle to a desktop.
    // If success the function return true else false.
    static bool setDesktopToCurrentThread(HDESK newDesktop);

    // This function select a desktop that assigned by name or not to a current thread from that it was called.
    // Param: name - a QString object that contain a valid desktop name such as "Winlogon" that will be assigned to the thread. If name is empty the input desktop will be assigned.
    // If success the function return true else false.
    static bool selectDesktop(const QString &name);

    // Param: desktopName - is a pointer to a QString object that will be used to store the desktop name. If function has failed then the desktopName object will not change. If the function succeeds, the name of a current input desktop stores in the desktopName object.
    // If success the function return true else false.
    static bool getCurrentDesktopName(QString *desktopName);

    // Param: desktopName - is a pointer to a QString object that will be used to store the desktop name. If function has failed then the desktopName object will not change. If the function succeeds, the desktop name of the current thread stores in the desktopName object.
    // If success the function return true else false.
    static bool getThreadDesktopName(QString *desktopName);

    // This function gets desktop name by a handle to a desktop.
    // Param: desktopName - is a pointer to a QString object that will be used to store the desktop name. If function has failed then the desktopName object will not change. If the function succeeds, the desktop name stores in the desktopName object.
    // If success the function return true else false.
    static bool getDesktopName(HDESK desktop, QString *desktopName);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

    static void enableLog(bool enable, const QString &fileBaseName = "log", const QString &baseDir = "./");

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
