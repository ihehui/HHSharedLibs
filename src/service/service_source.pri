# Input
HEADERS += service/service.h \
    service/logdebug.h
    servicelib.h
SOURCES += service/service.cpp
FORMS += 


RESOURCES += 
win32 { 
    HEADERS += 
    SOURCES += 
    INCLUDEPATH +=
    LIBS += -luser32 -lAdvapi32

}
unix { 
    HEADERS += 
    SOURCES += 
}
