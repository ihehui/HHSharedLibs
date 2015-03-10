# Input
HEADERS += \
    windowsmanagement.h \
    wmlib.h \
    adsi.h \
    hardwaremonitor/hardwaremonitor.h

SOURCES += windowsmanagement.cpp \
    adsi.cpp \
    hardwaremonitor/hardwaremonitor.cpp

FORMS +=
RESOURCES += 

win32 { 
    INCLUDEPATH += resources/lib/AutoIt3 \
                    resources/lib/WindowsAPI \
                    resources/lib/hardwaremonitor

    LIBS += -luser32 \
        -lNetAPI32 \
        -lAdvapi32 \
        -lMpr \
        -lWinspool \
        -lVersion \
        -lUserenv \
        #-Lresources/lib/AutoIt3 \
        #-lAutoItX3 \
        -Lresources/lib/WindowsAPI \
        -lWindowsAPI \
        -Lresources/lib/hardwaremonitor

    win32-g++{
        INCLUDEPATH += resources/lib/WinAPI_GCC
        LIBS += -Lresources/lib/WinAPI_GCC
    }

}

DISTFILES +=
