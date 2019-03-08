#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QCoreApplication>

#include "logger/messagelogger.h"

#ifdef Q_OS_WIN32
    #include "crashdump/wincrashdump.h"
#endif

void setupCrashHandler()
{
#ifdef Q_OS_WIN32
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
#elif defined(Q_OS_LINUX)
    enableCrashHandler(afterCrashDump);
    qAddPostRoutine(cleanup_ptr);
#endif
}




#endif // CRASHHANDLER_H
