
INCLUDEPATH += $$PWD

HEADERS += screenshot/screenshot.h \
    screenshot/selecttargetimagewidget.h \
    dataexport/dataoutputdialog.h \
    preference/preferenceswindow.h \
    plugin/pluginmanager/pluginmanagerwindow.h \
    plugin/pluginmanager/plugininfomodel.h \
    plugin/guiinterface.h \
    progress/progressdlg.h \
    widgetbase/systemtrayiconbase.h \
    imageresourcebase.h \
    plugin/guipluginbase.h \
    login/loginbase.h \
    settingsbase.h \
    databaseconnecter/databaseconnecter.h \
    databaseconnecter/databaseconnecterdialog.h \
    login/logindlg.h \
    dataprint.h \
    widgetbase/mainwindowbase.h \
    widgetbase/widgetbase.h \
    guilib.h \
    imageviewer/imageviewercontroler.h \
    imageviewer/animationcontroler.h \
    imageviewer/imageviewer.h

SOURCES += screenshot/screenshot.cpp \
    screenshot/selecttargetimagewidget.cpp \
    dataexport/dataoutputdialog.cpp \
    preference/preferenceswindow.cpp \
    plugin/pluginmanager/pluginmanagerwindow.cpp \
    plugin/pluginmanager/plugininfomodel.cpp \
    progress/progressdlg.cpp \
    widgetbase/systemtrayiconbase.cpp \
    imageresourcebase.cpp \
    plugin/guipluginbase.cpp \
    login/loginbase.cpp \
    settingsbase.cpp \
    databaseconnecter/databaseconnecter.cpp \
    databaseconnecter/databaseconnecterdialog.cpp \
    login/logindlg.cpp \
    dataprint.cpp \
    widgetbase/mainwindowbase.cpp \
    widgetbase/widgetbase.cpp \
    imageviewer/imageviewercontroler.cpp \
    imageviewer/animationcontroler.cpp \
    imageviewer/imageviewer.cpp

FORMS += screenshot/screenshot.ui \
    screenshot/selecttargetimagewidget.ui \
    dataexport/dataoutputdialog.ui \
    preference/preferenceswindow.ui \
    plugin/pluginmanager/pluginmanagerwindow.ui \
    progress/progressdlg.ui \
    databaseconnecter/databaseconnecterdialog.ui \
    login/logindlg.ui \
    imageviewer/imageviewercontroler.ui \
    imageviewer/animationcontroler.ui


RESOURCES += gui.qrc \
    imageviewer/resource.qrc

include (./widgetbase/itembox/itembox.pri)


#DEFINES += GUI_LIBRARY_EXPORT
