DEPENDPATH += $$PWD/enet/include/enet
INCLUDEPATH += $$PWD/enet/include

DEFINES += HAS_GETHOSTBYNAME_R=1 \
           HAS_POLL=1 \
           HAS_FCNTL=1 \
           HAS_INET_PTON=1 \
           HAS_INET_NTOP=1 \
           HAS_MSGHDR_FLAGS=1 \
           HAS_SOCKLEN_T=1

# Bionic libc does not provide gethostbyaddr_r
!android-* {
    DEFINES += HAS_GETHOSTBYADDR_R=1
}

win32:LIBS += -lws2_32 -lwinmm

HEADERS += $$PWD/enet/include/enet/callbacks.h \
           $$PWD/enet/include/enet/enet.h \
           $$PWD/enet/include/enet/list.h \
           $$PWD/enet/include/enet/protocol.h \
           $$PWD/enet/include/enet/time.h \
           $$PWD/enet/include/enet/types.h \
           $$PWD/enet/include/enet/unix.h \
           $$PWD/enet/include/enet/utility.h \
           $$PWD/enet/include/enet/win32.h
SOURCES += $$PWD/enet/callbacks.c \
           $$PWD/enet/compress.c \
           $$PWD/enet/host.c \
           $$PWD/enet/list.c \
           $$PWD/enet/packet.c \
           $$PWD/enet/peer.c \
           $$PWD/enet/protocol.c \
           $$PWD/enet/unix.c \
           $$PWD/enet/win32.c
