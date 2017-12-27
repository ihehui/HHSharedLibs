
TEMPLATE = subdirs

CONFIG += ordered \
    debug_and_release

SUBDIRS += core \
           network \
           #udt \
            enet \
           gui \
           service \
           systemutilities

#win32{
#	SUBDIRS += systemutilities
#}


