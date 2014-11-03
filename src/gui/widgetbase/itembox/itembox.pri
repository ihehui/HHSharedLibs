
INCLUDEPATH += $$PWD

SOURCES +=  $$PWD/itemboxeventhandler.cpp \
            $$PWD/itemboxcategorylistview.cpp \
            $$PWD/itemboxtreewidget.cpp \
            $$PWD/itemboxwidget.cpp \
            $$PWD/sheet_delegate.cpp \
            $$PWD/itemboxwidgetbase.cpp \
            $$PWD/abstractitemboxwidget.cpp \
            $$PWD/filterwidget.cpp \
            $$PWD/iconloader.cpp

HEADERS +=  $$PWD/itemboxlib.h \
            $$PWD/itemboxeventhandler.h \
            $$PWD/itemboxcategorylistview.h \
            $$PWD/itemboxtreewidget.h \
            $$PWD/itemboxwidget.h \
            $$PWD/sheet_delegate_p.h \
            $$PWD/itemboxwidgetbase_p.h \
            $$PWD/abstractitemboxwidget.h \
            $$PWD/filterwidget_p.h \
            $$PWD/iconloader_p.h

#RESOURCES += $$PWD/itembox.qrc

DEFINES += ITEMBOX_LIBRARY_EXPORT
