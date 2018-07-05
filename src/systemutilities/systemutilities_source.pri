# Input

HEADERS += \
    systemutilities.h \
    systemutilitieslib.h \
    $$PWD/utilities_def.h

SOURCES += systemutilities.cpp

unix {
HEADERS += unixutilities.h

SOURCES += unixutilities.cpp
}

win32 { 

HEADERS += \
    windowsmanagement.h \
    adsi.h \
    hardwaremonitor/hardwaremonitor.h \
    activex/qaxtypefunctions.h \
    activex/qaxtypes.h \
    activex/wmiquery.h \
    winutilities.h

SOURCES += windowsmanagement.cpp \
    adsi.cpp \
    hardwaremonitor/hardwaremonitor.cpp \
    activex/qaxtypefunctions.cpp \
    activex/qaxtypes.cpp \
    activex/wmiquery.cpp \
    winutilities.cpp


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
