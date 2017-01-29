TARGET = libtari.a
OBJS = animation.o physics.o framerate.o drawing.o texture.o \
collision.o input.o pvr.o framerateselect.o file.o math.o geometry.o \
system.o log.o timer.o optionhandler.o datastructures.o \
quicklz.o

defaultall: create_addons_link $(OBJS) subdirs linklib

KOS_CFLAGS += -W -Wall -Werror

include $(KOS_BASE)/addons/Makefile.prefab

create_addons_link:
	rm -f ../include/tari
	ln -s ../libtari/include ../include/tari

clean:
	rm -f *.o
