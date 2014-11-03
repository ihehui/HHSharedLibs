include(enet_lib.pri)
isEmpty(LIB_NAME):LIB_NAME = HHSharedENET

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
include(enet_source.pri)
#ENET Source
include( ./build_with_enet.pri )

DEFINES += ENET_LIBRARY_EXPORT

INCLUDEPATH += . \
    ./enet \
    ./enet/include
DEPENDPATH += $${INCLUDEPATH}

# #
# UI_DIR = ./ui
# OBJECTS_DIR = ./tmp
# MOC_DIR = ./tmp
# RCC_DIR = ./tmp




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
QMAKE_TARGET_COMPANY = "ENET"
QMAKE_TARGET_PRODUCT = "ENET 1.3"
QMAKE_TARGET_DESCRIPTION = "UDP-based data transfer protocol"
QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2002-2014 Lee Salzman"
PACKAGE_DOMAIN = "http://enet.bespin.org"




