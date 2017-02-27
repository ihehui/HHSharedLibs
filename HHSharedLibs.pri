

### Config file for using the libs

HHSHAREDLIBS_ROOT_PATH = $$PWD

isEmpty(HHSharedLibs){

    contains(QT, core){
        HHSharedLibs += HHSharedCore
    }
    contains(QT, gui){
        HHSharedLibs += HHSharedGUI
    }
    contains(QT, network){
        HHSharedLibs += HHSharedNetwork
    }


    #HHSharedLibs += HHSharedCore HHSharedGUI HHSharedNetwork HHSharedService
    #win32{
    #    HHSharedLibs += HHSharedSystemUtilities
    #}

}

contains(HHSharedLibs, HHSharedCore){
    include(src/core/core_lib.pri)
#    INCLUDEPATH += src/core
#    DEPENDPATH += src/core
    #HEADERS += $${HHSHAREDLIBS_ROOT_PATH}/include/HHSharedCore
    #HEADERS += ./include/HHSharedCore

}

contains(HHSharedLibs, HHSharedGUI){
    include(src/gui/gui_lib.pri)
#    INCLUDEPATH += src/gui
#    DEPENDPATH += src/gui
    #HEADERS += ./include/HHSharedGUI
}

contains(HHSharedLibs, HHSharedNetwork){
    include(src/network/network_lib.pri)
#    INCLUDEPATH += src/network
#    DEPENDPATH += src/network
}

contains(HHSharedLibs, HHSharedService){
    include(src/service/service_lib.pri)
#    INCLUDEPATH += src/service
#    DEPENDPATH += src/service
}

contains(HHSharedLibs, HHSharedUDT){
    include(src/udt/udt_lib.pri)
#    INCLUDEPATH += src/udt
#    DEPENDPATH += src/udt

    win32:win32-g++:DEFINES += __MINGW__
}

contains(HHSharedLibs, HHSharedENET){
    include(src/enet/enet_lib.pri)
#    INCLUDEPATH += src/enet
#    DEPENDPATH += src/enet
}

#win32{
    contains(HHSharedLibs, HHSharedSysUtilities){
        include(src/systemutilities/systemutilities_lib.pri)
#        INCLUDEPATH += src/windowsmanagement
#        DEPENDPATH += src/windowsmanagement
    }
#}

#INCLUDEPATH += $${HHSHAREDLIBS_ROOT_PATH}/include

