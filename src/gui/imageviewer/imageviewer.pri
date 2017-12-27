
QT += core widgets printsupport
#qtHaveModule(printsupport): QT += printsupport



INCLUDEPATH += $$PWD


HEADERS     = \
    $$PWD/imageviewercontroler.h \
    $$PWD/animationcontroler.h \
    $$PWD/imageviewer.h \
<<<<<<< HEAD
    $$PWD/renderwidget.h \
    $$PWD/rawimagereader.h

    
=======
<<<<<<< HEAD
    $$PWD/renderwidget.h \
    $$PWD/rawimagereader.h \


SOURCES +=  \
=======
    $$PWD/renderwidget.h
>>>>>>> 9b2ff2e01338d8c256693cbb39f374ef688b6fd0
SOURCES     =  \
>>>>>>> f14699b2a2d2e863fb74fb1d2ec520b1fe626d7e
    $$PWD/imageviewercontroler.cpp \
    $$PWD/animationcontroler.cpp \
    $$PWD/imageviewer.cpp \
    $$PWD/renderwidget.cpp \
<<<<<<< HEAD
    $$PWD/rawimagereader.cpp
=======
    $$PWD/rawimagereader.cpp \
>>>>>>> 9b2ff2e01338d8c256693cbb39f374ef688b6fd0


FORMS += \
    $$PWD/imageviewercontroler.ui \
    $$PWD/animationcontroler.ui

RESOURCES += $$PWD/imageviewer.qrc

DEFINES += IMAGEVIEWER_LIBRARY_EXPORT

