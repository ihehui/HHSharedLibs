
HEADERS += $$PWD/logdebug.h \
    $$PWD/cryptography/tea/teacrypt.h \
    $$PWD/cryptography/cryptography.h \
    $$PWD/plugin/abstractplugininterface.h \
    $$PWD/plugin/coreinterface.h \
    $$PWD/plugin/pluginmanager.h \
    $$PWD/plugin/corepluginbase.h \
    $$PWD/userbase.h \
    $$PWD/settingscore.h \
    $$PWD/global_core.h \
    $$PWD/database/databaseutility.h \
    $$PWD/singleton.h \
    $$PWD/user.h \
    $$PWD/error.h \
    $$PWD/core_lib.h \
    $$PWD/jobmonitor.h \
    $$PWD/logger/messagelogger.h \
    $$PWD/logger/messageloggerbase.h \
    $$PWD/crashhandler.h \
    $$PWD/coreutilities.h
FORMS += 
SOURCES += $$PWD/cryptography/tea/teacrypt.cpp \
    $$PWD/cryptography/cryptography.cpp \
    $$PWD/plugin/pluginmanager.cpp \
    $$PWD/plugin/corepluginbase.cpp \
    $$PWD/userbase.cpp \
    $$PWD/settingscore.cpp \
    $$PWD/database/databaseutility.cpp \
    $$PWD/singleton.cpp \
    $$PWD/user.cpp \
    $$PWD/error.cpp \
    $$PWD/jobmonitor.cpp \
    $$PWD/logger/messagelogger.cpp \
    $$PWD/logger/messageloggerbase.cpp \
    $$PWD/coreutilities.cpp
RESOURCES += 
win32:LIBS += -Lresources/lib \
    -lwsock32 \
    -lws2_32

#INCLUDEPATH += $$PWD


