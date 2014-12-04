# Input
HEADERS += \
    windowsmanagement.h \
    wmlib.h \
    adsi.h
FORMS += 
SOURCES += windowsmanagement.cpp \
    adsi.cpp
RESOURCES += 

win32 { 
    INCLUDEPATH += resources/lib/AutoIt3 \
                    resources/lib/WindowsAPI
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
        -lWindowsAPI

    win32-g++{
        INCLUDEPATH += resources/lib/WinAPI_GCC
        LIBS += -Lresources/lib/WinAPI_GCC
    }

}
