
QT += core widgets printsupport
#qtHaveModule(printsupport): QT += printsupport



INCLUDEPATH += $$PWD


HEADERS     += \
    $$PWD/imageviewercontroler.h \
    $$PWD/animationcontroler.h \
    $$PWD/imageviewer.h \
    $$PWD/renderwidget.h \
    $$PWD/rawimagereader.h


SOURCES +=  \
    $$PWD/imageviewercontroler.cpp \
    $$PWD/animationcontroler.cpp \
    $$PWD/imageviewer.cpp \
    $$PWD/renderwidget.cpp \
    $$PWD/rawimagereader.cpp


FORMS += \
    $$PWD/imageviewercontroler.ui \
    $$PWD/animationcontroler.ui

RESOURCES += $$PWD/imageviewer.qrc

DEFINES += IMAGEVIEWER_LIBRARY_EXPORT

