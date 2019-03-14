# Input

HEADERS += \
    $$PWD/systemutilities.h \
    $$PWD/systemutilitieslib.h \
    $$PWD/utilities_def.h \
    $$PWD/hardwareinfo/hardwareinfo.h

SOURCES += $$PWD/systemutilities.cpp \
    $$PWD/hardwareinfo/hardwareinfo.cpp


unix {
HEADERS += $$PWD/unixutilities.h \
    $$PWD/hardwareinfo/hardwareinfo_unix.h

SOURCES += $$PWD/unixutilities.cpp \
    $$PWD/hardwareinfo/hardwareinfo_unix.cpp
}

win32 { 

HEADERS += \
    $$PWD/windowsmanagement.h \
    $$PWD/adsi.h \
    $$PWD/activex/qaxtypefunctions.h \
    $$PWD/activex/qaxtypes.h \
    $$PWD/activex/wmiquery.h \
    $$PWD/winutilities.h \
    $$PWD/ctrlaltdelsimulator.h \
    $$PWD/hardwareinfo/hardwareinfo_win.h


SOURCES += $$PWD/windowsmanagement.cpp \
    $$PWD/adsi.cpp \
    $$PWD/activex/qaxtypefunctions.cpp \
    $$PWD/activex/qaxtypes.cpp \
    $$PWD/activex/wmiquery.cpp \
    $$PWD/winutilities.cpp \
    $$PWD/ctrlaltdelsimulator.cpp \
    $$PWD/hardwareinfo/hardwareinfo_win.cpp


    INCLUDEPATH += $$PWD \
                   $$PWD/resources/lib/WindowsAPI \
                   $$PWD/resources/lib/WinRing0

    LIBS += -L$$PWD/resources/lib/WindowsAPI \ #-lWindowsAPI \
            -L$$PWD/resources/lib/WinRing0

    equals(QT_ARCH, "x86_64"){
        LIBS += -lWindowsAPI64
    }else{
        LIBS += -lWindowsAPI
    }


    LIBS += -luser32 \
        -lNetAPI32 \
        -lAdvapi32 \
        -lgdi32 \
        -lgdiplus \
        -lMpr \
        -lWinspool \
        -lVersion \
        -lUserenv \
        -lOleAut32 \
        -lOle32 \
        -lwbemuuid


    #win32-g++{
    #    INCLUDEPATH += resources/lib/WinAPI_GCC
    #    LIBS += -Lresources/lib/WinAPI_GCC
    #}

    win32-msvc:QMAKE_CFLAGS -= -Zc:strictStrings
    win32-msvc:QMAKE_CXXFLAGS -= -Zc:strictStrings

}
