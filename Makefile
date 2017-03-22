TARGET = libtari.a
OBJS = animation.o physics.o framerate.o dc/drawing.o dc/texture.o \
collision.o dc/input.o dc/pvr.o dc/framerateselect.o dc/file.o math.o geometry.o \
dc/system.o dc/log.o timer.o optionhandler.o datastructures.o dc/memoryhandler.o \
wrapper.o physicshandler.o script.o stagehandler.o collisionhandler.o \
collisionanimation.o dc/soundeffect.o dc/sound.o \
quicklz.o

defaultall: create_addons_link $(OBJS) subdirs linklib

KOS_CFLAGS += -W -Wall -Werror

include $(KOS_BASE)/addons/Makefile.prefab

create_addons_link:
	rm -f ../include/tari
	ln -s ../libtari/include ../include/tari

clean:
	rm -f *.o
