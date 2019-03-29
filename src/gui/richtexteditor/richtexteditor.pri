

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

# Input
HEADERS += $$PWD/htmlhighlighter_p.h \
           $$PWD/richtexteditorwidget.h \
           $$PWD/richtexteditorlib.h
            
SOURCES += $$PWD/htmlhighlighter.cpp \
           $$PWD/richtexteditorwidget.cpp
           
RESOURCES += $$PWD/richtexteditor.qrc

DEFINES += RICHTEXTEDITOR_LIBRARY_EXPORT
