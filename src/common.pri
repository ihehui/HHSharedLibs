

isEmpty(LIB_NAME){
    error(Empty LIB NAME! PWD:$$PWD OUT_PWD:$$OUT_PWD)
}

isEmpty(HHSHAREDLIBS_ROOT_PATH){
    HHSHAREDLIBS_ROOT_PATH = $$PWD/../
}

#isEmpty(HHSHAREDLIBS_BIN_TARGET_PATH){
#    CONFIG( debug, debug|release ) {
#        HHSHAREDLIBS_BIN_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/bin/debug
#    } else {
#        HHSHAREDLIBS_BIN_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/bin/release
#    }
#}

isEmpty(HHSHAREDLIBS_LIB_TARGET_PATH){
    HHSHAREDLIBS_LIB_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/lib
}

#isEmpty(HHSHAREDLIBS_DLL_TARGET_PATH){
#    HHSHAREDLIBS_DLL_TARGET_PATH = $${HHSHAREDLIBS_BIN_TARGET_PATH}
#}

isEmpty(HHSHAREDLIBS_HEADERS_TARGET_PATH){
    HHSHAREDLIBS_HEADERS_TARGET_PATH = $${HHSHAREDLIBS_ROOT_PATH}/include/$${LIB_NAME}
#    HHSHAREDLIBS_HEADERS_TARGET_PATH = $$[QT_INSTALL_HEADERS]/$${LIB_NAME}
}
#UI_DIR = $${HHSHAREDLIBS_ROOT_PATH}/include/$${LIB_NAME}/ui

#define lib  version info
LIB_VER_MAJ = 2014
LIB_VER_MIN = 12
LIB_VER_PAT = 23
LIB_VER_CMP = 1
#VERSION = $${LIB_VER_MAJ}.$${LIB_VER_MIN}.$${LIB_VER_PAT}.$${LIB_VER_CMP}


win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += HHSHAREDLIBS_EXPORT
    else:USELIB:DEFINES += HHSHAREDLIBS_IMPORT
}

DEFINES += QT_NO_CAST_TO_ASCII


INCLUDEPATH += $$OUT_PWD
INCLUDEPATH += $${HHSHAREDLIBS_ROOT_PATH}/include
INCLUDEPATH += $${HHSHAREDLIBS_HEADERS_TARGET_PATH}
INCLUDEPATH += $$UI_DIR

DEPENDPATH += $$INCLUDEPATH


##Link with $${LIB_NAME}
contains(HHSharedLibs, $${LIB_NAME}){
    LIBS += -L$${HHSHAREDLIBS_LIB_TARGET_PATH}

    CONFIG( debug, debug|release ) {
        # Debug
        message(Debug Build)
        unix:TARGET_LIB_NAME = $$quote($$join(LIB_NAME,,,_debug))
        else:TARGET_LIB_NAME = $$quote($$join(LIB_NAME,,,d))
    }else {
        # Release
        message(Release Build)
        TARGET_LIB_NAME = $$quote($$LIB_NAME)
    }

    win32:LIBS += -l$${TARGET_LIB_NAME}$${LIB_VER_MAJ}
    else:LIBS += -l$${TARGET_LIB_NAME}

    message(Link with library: $${TARGET_LIB_NAME} $$OUT_PWD)

}








