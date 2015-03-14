
include(service_lib.pri)

isEmpty(LIB_NAME){
    LIB_NAME = HHSharedService
}

# ##
TEMPLATE = lib

# CONFIG += static
CONFIG += shared
TARGET = $${LIB_NAME}

QT += core

# include config file
include( ../buildlib_config.pri )

# include source 
include(service_source.pri)

# include the service
CONFIG( debug, debug|release ) {
        #Output Debugging Messages
        DEFINES	+= QTSERVICE_DEBUG
}
include(./3rdparty/qtservice/src/qtservice.pri)

DEFINES += SERVICE_LIBRARY_EXPORT QT_QTSERVICE_EXPORT



# #
#UI_DIR = ./ui

# OBJECTS_DIR = ./tmp
# MOC_DIR = ./tmp
# RCC_DIR = ./tmp









# ##
#DESTDIR = $${LIB_PATH}

# DLLDESTDIR = ../../bin
# define config DLLDESTDIR paths
#CONFIG( debug, debug|release ):DLLDESTDIR = $${BIN_PATH}/debug
#else:DLLDESTDIR = $${BIN_PATH}/release

# ##
#unix:target.path += /usr/lib
#win32:target.path += %windir%/system32
target.path += $$[QT_INSTALL_LIBS]
INSTALLS += target

#headers.files += $$HEADERS HHSharedService
#headers.path = $$HHSHAREDLIBS_HEADERS_TARGET_PATH
#INSTALLS += headers


# define some usefull values
#QMAKE_TARGET_COMPANY	= "He Hui Team"
QMAKE_TARGET_PRODUCT	= "Service Library"
QMAKE_TARGET_DESCRIPTION	= "Crossplatform Service Library Based On Qt"
#QMAKE_TARGET_COPYRIGHT	= "Copyright (C) 2007 - 2010 He Hui"
