include(udt_lib.pri)
isEmpty(LIB_NAME):LIB_NAME = HHSharedUDT

# ##
TEMPLATE = lib

# CONFIG += static
CONFIG += shared
TARGET = $${LIB_NAME}

# VERSION = 2009.09.08.1055
QT += core \
    network \
    concurrent
QT -= gui

# include config file
include( ../buildlib_config.pri )

# include source 
include(udt_source.pri)
#include UDT Source
include( ./build_with_udt.pri )

DEFINES += UDT_LIBRARY_EXPORT

INCLUDEPATH += . \
    ./udt \
    ./udt/src
DEPENDPATH += $${INCLUDEPATH}

# #
# UI_DIR = ./ui
# OBJECTS_DIR = ./tmp
# MOC_DIR = ./tmp
# RCC_DIR = ./tmp


####Add the following code to "udt.h" before "#define NO_BUSY_WAITING" ####
#ifdef WIN32
    #ifdef __WINPTHREADS_VERSION
        #define USE_PTHREADS
    #endif
#else
    #define USE_PTHREADS
#endif

#ifdef NO_UDT_DLL
#undef UDT_API
#define UDT_API
#endif
###########################################################################

win32 { 

    DEFINES += WIN32  _WIN32_WINNT=0x0501 NO_UDT_DLL
    #DEFINES += WIN32 LEGACY_WIN32 _WIN32_WINNT=0x0500 NO_UDT_DLL
    #win32-g++:DEFINES += __MINGW__
    win32-g++{
        DEFINES += __MINGW__
        #INCLUDEPATH += resources/lib/WinAPI_GCC
        #LIBS += -Lresources/lib/WinAPI_GCC
    }

    HEADERS +=
    SOURCES += 

    LIBS += -lws2_32

    #LIBS += -Lresources/lib/win \
    #    #-lwsock32 \
    #    -lws2_32 \
    #    -ludt



}
unix { 
    HEADERS += 
    SOURCES += 
    #LIBS += -Lresources/lib/linux \
    #    -ludt
}

# ##
# DESTDIR = $${LIB_PATH}
# DLLDESTDIR = ../../bin
# define config DLLDESTDIR paths
# CONFIG( debug, debug|release ):DLLDESTDIR = $${BIN_PATH}/debug
# else:DLLDESTDIR = $${BIN_PATH}/release
# ##
# unix:target.path += /usr/lib
# win32:target.path += %windir%/system32
target.path += $$[QT_INSTALL_LIBS]
INSTALLS += target

# headers.files += $$HEADERS
# headers.path = $$HHSHAREDLIBS_HEADERS_TARGET_PATH
# INSTALLS += headers
# define some usefull values
QMAKE_TARGET_COMPANY = "UDT"
QMAKE_TARGET_PRODUCT = "UDT4"
QMAKE_TARGET_DESCRIPTION = "UDP-based data transfer protocol"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2001 - 2014 Yunhong Gu. All rights reserved."
PACKAGE_DOMAIN = "http://udt.sf.net"




