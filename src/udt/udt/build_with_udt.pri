
DEPENDPATH += $$PWD $$PWD/src
INCLUDEPATH += $$PWD $$PWD/src

# Input
HEADERS += src/api.h \
           src/buffer.h \
           src/cache.h \
           src/ccc.h \
           src/channel.h \
           src/common.h \
           src/core.h \
           src/epoll.h \
           src/list.h \
           src/md5.h \
           src/packet.h \
           src/queue.h \
           src/udt.h \
           src/window.h
SOURCES += src/api.cpp \
           src/buffer.cpp \
           src/cache.cpp \
           src/ccc.cpp \
           src/channel.cpp \
           src/common.cpp \
           src/core.cpp \
           src/epoll.cpp \
           src/list.cpp \
           src/md5.cpp \
           src/packet.cpp \
           src/queue.cpp \
           src/window.cpp
