
#DEPENDPATH += $$PWD $$PWD/udt/src
INCLUDEPATH += $$PWD #$$PWD/udt/src

# Input
HEADERS += udt/src/api.h \
           udt/src/buffer.h \
           udt/src/cache.h \
           udt/src/ccc.h \
           udt/src/channel.h \
           udt/src/common.h \
           udt/src/core.h \
           udt/src/epoll.h \
           udt/src/list.h \
           udt/src/md5.h \
           udt/src/packet.h \
           udt/src/queue.h \
           udt/src/udt.h \
           udt/src/window.h

SOURCES += udt/src/api.cpp \
           udt/src/buffer.cpp \
           udt/src/cache.cpp \
           udt/src/ccc.cpp \
           udt/src/channel.cpp \
           udt/src/common.cpp \
           udt/src/core.cpp \
           udt/src/epoll.cpp \
           udt/src/list.cpp \
           udt/src/md5.cpp \
           udt/src/packet.cpp \
           udt/src/queue.cpp \
           udt/src/window.cpp
