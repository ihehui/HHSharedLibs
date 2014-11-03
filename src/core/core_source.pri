
HEADERS += $$PWD/logdebug.h \
    $$PWD/cryptography/tea/teacrypt.h \
    $$PWD/cryptography/cryptography.h \
    $$PWD/plugin/abstractplugininterface.h \
    $$PWD/plugin/coreinterface.h \
    $$PWD/plugin/pluginmanager.h \
    $$PWD/plugin/corepluginbase.h \
    $$PWD/userbase.h \
    $$PWD/utilities.h \
    $$PWD/settingscore.h \
    $$PWD/global_core.h \
    $$PWD/database/databaseutility.h \
    $$PWD/singleton.h \
    $$PWD/user.h \
    $$PWD/error.h \
    $$PWD/core_lib.h
FORMS += 
SOURCES += $$PWD/cryptography/tea/teacrypt.cpp \
    $$PWD/cryptography/cryptography.cpp \
    $$PWD/plugin/pluginmanager.cpp \
    $$PWD/plugin/corepluginbase.cpp \
    $$PWD/userbase.cpp \
    $$PWD/utilities.cpp \
    $$PWD/settingscore.cpp \
    $$PWD/database/databaseutility.cpp \
    $$PWD/singleton.cpp \
    $$PWD/user.cpp \
    $$PWD/error.cpp
RESOURCES += 
win32:LIBS += -Lresources/lib \
    -lwsock32 \
    -lws2_32

#INCLUDEPATH += $$PWD


