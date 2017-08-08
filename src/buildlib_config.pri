
### Config file for building the lib

isEmpty(LIB_NAME){
    error(Empty LIB NAME! PWD:$$PWD OUT_PWD:$$OUT_PWD)
}

isEmpty(HHSHAREDLIBS_ROOT_PATH){
    #HHSHAREDLIBS_ROOT_PATH = $$PWD/../
    HHSHAREDLIBS_ROOT_PATH = $$PWD/../../
}

isEmpty(HHSHAREDLIBS_OUT_ROOT_PATH){
    HHSHAREDLIBS_OUT_ROOT_PATH = $$OUT_PWD/../../
}

isEmpty(HHSHAREDLIBS_BIN_TARGET_PATH){
    CONFIG( debug, debug|release ) {
        HHSHAREDLIBS_BIN_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/bin/debug
    } else {
        HHSHAREDLIBS_BIN_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/bin/release
    }

    unix:HHSHAREDLIBS_BIN_TARGET_PATH = $$system(echo $HOME/bin)
}
message($$LIB_NAME BIN DSTDIR: $$HHSHAREDLIBS_BIN_TARGET_PATH)

isEmpty(HHSHAREDLIBS_LIB_TARGET_PATH){
    HHSHAREDLIBS_LIB_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/lib

    unix:HHSHAREDLIBS_LIB_TARGET_PATH = $$system(echo $HOME/lib)
}
message($$LIB_NAME LIB DSTDIR: $$HHSHAREDLIBS_LIB_TARGET_PATH)

isEmpty(HHSHAREDLIBS_DLL_TARGET_PATH){
    HHSHAREDLIBS_DLL_TARGET_PATH = $${HHSHAREDLIBS_BIN_TARGET_PATH}
}

isEmpty(HHSHAREDLIBS_HEADERS_TARGET_PATH){
    HHSHAREDLIBS_HEADERS_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/include/$${LIB_NAME}
    #HHSHAREDLIBS_HEADERS_TARGET_PATH = $$[QT_INSTALL_HEADERS]/$${LIB_NAME}
}




#DEFINES += MYSHAREDLIB_LIBRARY_EXPORT

# #
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# package name
PACKAGE_TARGET	= HeHui
mac:PACKAGE_TARGET	= $$quote(HeHui)

# package destdir
PACKAGE_DESTDIR	= $${PACKAGE_PWD}/bin

# temporary path for building
PACKAGE_BUILD_PATH	= $${HHSHAREDLIBS_OUT_ROOT_PATH}/temp/$${TARGET}

# build mode
#CONFIG	+= qt warn_on thread x11 windows release

# define config mode paths
CONFIG( debug, debug|release ) {
        #message( DEBUG Build )
        #Output Debugging Messages, For Windows
        CONFIG	+= console

	unix:PACKAGE_TARGET	= $$quote($$join(PACKAGE_TARGET,,,_debug))
	else:PACKAGE_TARGET	= $$quote($$join(PACKAGE_TARGET,,,d))
	unix:TARGET	= $$quote($$join(TARGET,,,_debug))
	else:TARGET	= $$quote($$join(TARGET,,,d))
	unix:OBJECTS_DIR	= $${PACKAGE_BUILD_PATH}/debug/.obj/unix
	win32:OBJECTS_DIR	= $${PACKAGE_BUILD_PATH}/debug/.obj/win32
	mac:OBJECTS_DIR	= $${PACKAGE_BUILD_PATH}/debug/.obj/mac
        #UI_DIR	= $${PACKAGE_BUILD_PATH}/debug/.ui
        #UI_DIR	= $${PACKAGE_PWD}/.ui
	MOC_DIR	= $${PACKAGE_BUILD_PATH}/debug/.moc
	RCC_DIR	= $${PACKAGE_BUILD_PATH}/debug/.rcc
} else {
        #message( RELEASE Build )
        #Disable Debug Output
        DEFINES += QT_NO_DEBUG_OUTPUT

	mac:TARGET	= $$quote($$TARGET)
	unix:OBJECTS_DIR	= $${PACKAGE_BUILD_PATH}/release/.obj/unix
	win32:OBJECTS_DIR	= $${PACKAGE_BUILD_PATH}/release/.obj/win32
	mac:OBJECTS_DIR	= $${PACKAGE_BUILD_PATH}/release/.obj/mac
        #UI_DIR	= $${PACKAGE_BUILD_PATH}/release/.ui
        #UI_DIR	= $${PACKAGE_PWD}/.ui
	MOC_DIR	= $${PACKAGE_BUILD_PATH}/release/.moc
	RCC_DIR	= $${PACKAGE_BUILD_PATH}/release/.rcc
}


#set INCLUDEPATH
INCLUDEPATH += $${OUT_PWD} \
               $$UI_DIR \
               $$MOC_DIR \
               $$RCC_DIR  \
               $$OBJECTS_DIR

DEPENDPATH += $$INCLUDEPATH



DESTDIR = $${HHSHAREDLIBS_LIB_TARGET_PATH}
win32 {
    #DLLDESTDIR = $${HHSHAREDLIBS_BIN_TARGET_PATH}
    DLLDESTDIR = $$[QT_INSTALL_BINS]
    QMAKE_DISTCLEAN += $$[QT_INSTALL_BINS]\\$${TARGET}.dll
}



#define version info
VERSION = $${LIB_VER_MAJ}.$${LIB_VER_MIN}.$${LIB_VER_PAT}.$${LIB_VER_CMP}


# define some usefull values
QMAKE_TARGET_COMPANY = "He Hui"
QMAKE_TARGET_PRODUCT = "He Hui"
QMAKE_TARGET_DESCRIPTION = "Crossplatform Libraries Based On Qt"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2007 - 2016 He Hui"
PACKAGE_DOMAIN = "www.hehui.org"


APP_ORG = "He Hui"
APP_AUTHOR = "He Hui"
APP_AUTHOR_EMAIL = "hehui@hehui.org"
APP_LICENSE = "LGPL"

# define variable for source code
#DEFINES *= "APP_NAME=\"\\\"$${QMAKE_TARGET_PRODUCT}\\\"\"" \
DEFINES	*= "APP_VERSION=\"\\\"$${VERSION}\\\"\"" \
        "APP_ORG=\"\\\"$${APP_ORG}\\\"\"" \
        "APP_ORG_DOMAIN=\"\\\"$${PACKAGE_DOMAIN}\\\"\"" \
        "APP_AUTHOR=\"\\\"$${APP_AUTHOR}\\\"\"" \
        "APP_AUTHOR_EMAIL=\"\\\"$${APP_AUTHOR_EMAIL}\\\"\"" \
        "APP_LICENSE=\"\\\"$${APP_LICENSE}\\\"\"" \
        "APP_COPYRIGHTS=\"\\\"$${QMAKE_TARGET_COPYRIGHT}\\\"\""



win32-g++:QMAKE_CXXFLAGS_RELEASE = -o1
#win32-g++:QMAKE_LFLAGS = -enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc
win32-g++:QMAKE_CXXFLAGS_EXCEPTIONS_OFF =

#####Release Builed With Debug Info####
#win32-g++:QMAKE_CXXFLAGS_RELEASE += -g
#win32-g++:QMAKE_CFLAGS_RELEASE += -g
#win32-g++:QMAKE_LFLAGS_RELEASE =
#win32-msvc2010:QMAKE_CXXFLAGS_RELEASE += /Zi /Od /DEBUG
#win32-msvc2010:QMAKE_CFLAGS_RELEASE += /Zi /Od /DEBUG
#win32-msvc2010:QMAKE_LFLAGS_RELEASE = /DEBUG

