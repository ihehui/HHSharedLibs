# Input

HEADERS += \
    $$PWD/systemutilities.h \
    $$PWD/systemutilitieslib.h \
    $$PWD/utilities_def.h

SOURCES += $$PWD/systemutilities.cpp

unix {
HEADERS += $$PWD/unixutilities.h

SOURCES += $$PWD/unixutilities.cpp
}

win32 { 

HEADERS += \
    $$PWD/windowsmanagement.h \
    $$PWD/adsi.h \
    $$PWD/hardwaremonitor/hardwaremonitor.h \
    $$PWD/activex/qaxtypefunctions.h \
    $$PWD/activex/qaxtypes.h \
    $$PWD/activex/wmiquery.h \
    $$PWD/winutilities.h \
    $$PWD/ctrlaltdelsimulator.h


SOURCES += $$PWD/windowsmanagement.cpp \
    $$PWD/adsi.cpp \
    $$PWD/hardwaremonitor/hardwaremonitor.cpp \
    $$PWD/activex/qaxtypefunctions.cpp \
    $$PWD/activex/qaxtypes.cpp \
    $$PWD/activex/wmiquery.cpp \
    $$PWD/winutilities.cpp \
    $$PWD/ctrlaltdelsimulator.cpp


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
