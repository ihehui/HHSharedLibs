

include (imageviewer/imageviewer.pri)
include (widgetbase/itembox/itembox.pri)
include(screencapture/screencapture.pri)
include(paletteeditor/paletteeditor.pri)
include(richtexteditor/richtexteditor.pri)


DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS += $$PWD/screenshot/screenshot.h \
    $$PWD/screenshot/selecttargetimagewidget.h \
    $$PWD/dataexport/dataoutputdialog.h \
    $$PWD/preference/preferenceswindow.h \
    $$PWD/plugin/pluginmanager/pluginmanagerwindow.h \
    $$PWD/plugin/pluginmanager/plugininfomodel.h \
    $$PWD/plugin/guiinterface.h \
    $$PWD/progress/progressdlg.h \
    $$PWD/widgetbase/systemtrayiconbase.h \
    $$PWD/imageresourcebase.h \
    $$PWD/plugin/guipluginbase.h \
    $$PWD/login/loginbase.h \
    $$PWD/settingsbase.h \
    $$PWD/databaseconnecter/databaseconnecter.h \
    $$PWD/databaseconnecter/databaseconnecterdialog.h \
    $$PWD/login/logindlg.h \
    $$PWD/dataprint.h \
    $$PWD/widgetbase/mainwindowbase.h \
    $$PWD/widgetbase/widgetbase.h \
    $$PWD/guilib.h \
    $$PWD/guiutilities.h

SOURCES += $$PWD/screenshot/screenshot.cpp \
    $$PWD/screenshot/selecttargetimagewidget.cpp \
    $$PWD/dataexport/dataoutputdialog.cpp \
    $$PWD/preference/preferenceswindow.cpp \
    $$PWD/plugin/pluginmanager/pluginmanagerwindow.cpp \
    $$PWD/plugin/pluginmanager/plugininfomodel.cpp \
    $$PWD/progress/progressdlg.cpp \
    $$PWD/widgetbase/systemtrayiconbase.cpp \
    $$PWD/imageresourcebase.cpp \
    $$PWD/plugin/guipluginbase.cpp \
    $$PWD/login/loginbase.cpp \
    $$PWD/settingsbase.cpp \
    $$PWD/databaseconnecter/databaseconnecter.cpp \
    $$PWD/databaseconnecter/databaseconnecterdialog.cpp \
    $$PWD/login/logindlg.cpp \
    $$PWD/dataprint.cpp \
    $$PWD/widgetbase/mainwindowbase.cpp \
    $$PWD/widgetbase/widgetbase.cpp \
    $$PWD/guiutilities.cpp

FORMS += $$PWD/screenshot/screenshot.ui \
    $$PWD/screenshot/selecttargetimagewidget.ui \
    $$PWD/dataexport/dataoutputdialog.ui \
    $$PWD/preference/preferenceswindow.ui \
    $$PWD/plugin/pluginmanager/pluginmanagerwindow.ui \
    $$PWD/progress/progressdlg.ui \
    $$PWD/databaseconnecter/databaseconnecterdialog.ui \
    $$PWD/login/logindlg.ui


RESOURCES += $$PWD/resources/gui.qrc

