include(gui_lib.pri)
isEmpty(LIB_NAME):LIB_NAME = HHSharedGUI

# ##
TEMPLATE = lib

# CONFIG += static
CONFIG += shared
TARGET = $${LIB_NAME}

# VERSION = 2009.09.08.1055
# ##
QT += core \
    gui \
    widgets \
    printsupport \
    sql \
    xml

# include config file
include( ../buildlib_config.pri )


##################### HHSharedCore #####################
#### 1.Link With HHSharedCore DLL ####
HHSharedLibs += HHSharedCore
include(../core/core_lib.pri)

#### 2.Build With HHSharedCore Source ####
#DEFINES += CORE_LIB_LIBRARY_EXPORT
#include(../core/core_source.pri)
##################### HHSharedCore #####################


# include source 
include(gui_source.pri)
#include (./widgetbase/itembox/itembox.pri)
#include (./imageviewer/imageviewer.pri)


DEFINES += GUI_LIBRARY_EXPORT
DEFINES += ITEMBOX_LIBRARY_EXPORT
DEFINES += IMAGEVIEWER_LIBRARY_EXPORT

# DEPENDPATH += . \
# UI_DIR \
# ../core
# INCLUDEPATH += . \
# UI_DIR \
# ../core
# ########## Link shared libs #############
# SHARED_LIB_FOLDER = ../lib
# SHARED_CORE_FOLDER = ../core
# include($$SHARED_CORE_FOLDER/core.pri)
# !include($$SHARED_CORE_FOLDER/core.pri):error( "Can not include $$SHARED_CORE_FOLDER/core.pri !" )
# LIBS += -L$$SHARED_LIB_FOLDER -lHHSharedCore
# define link libs
# LIBS += -L$$SHARED_LIB_FOLDER

#CONFIG( debug, debug|release ):

# Debug
# unix:LIBS += -lHHSharedCore_debug
# win32:LIBS += -lHHSharedCored$${VER_MAJ}
# mac:LIBS += -lHHSharedCored
#else:
# Release
# win32:LIBS += -lHHSharedCore$${VER_MAJ}
# else:LIBS += -lHHSharedCore
# ## Export the library
# DEFINES += MYSHAREDLIB_LIBRARY_EXPORT
# #
# build_all:!build_pass {
# CONFIG -= build_all
# CONFIG += release
# }

# ##
UI_DIR = ./ui

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
QMAKE_TARGET_PRODUCT = "GUI Library"
QMAKE_TARGET_DESCRIPTION = "Crossplatform GUI Library Based On Qt"
