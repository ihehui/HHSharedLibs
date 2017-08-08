include(core_lib.pri)
isEmpty(LIB_NAME):LIB_NAME = HHSharedCore

# ##
TEMPLATE = lib

# CONFIG += static
CONFIG += shared c++11
TARGET = $${LIB_NAME}

# VERSION = 2009.09.08.1055
QT += core \
    sql
QT -= gui

# include config file
include(../buildlib_config.pri)

# include source 
include(core_source.pri)

DEFINES += CORE_LIB_LIBRARY_EXPORT


win32 {
    LIBS += -lwinmm
}

# #
# DEFINES += MYSHAREDLIB_LIBRARY_EXPORT
# #
# build_all:!build_pass {
# CONFIG -= build_all
# CONFIG += release
# }
# ##
# UI_DIR = ./ui
# OBJECTS_DIR = ./tmp
# MOC_DIR = ./tmp
# RCC_DIR = ./tmp
# Input


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
# QMAKE_TARGET_COMPANY	= "He Hui Team"
QMAKE_TARGET_PRODUCT = "Core Library"
QMAKE_TARGET_DESCRIPTION = "Crossplatform NON-GUI Library Based On Qt"
