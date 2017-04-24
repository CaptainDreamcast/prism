TARGET = libtari.a
OBJS = animation.o physics.o framerate.o drawing.o dc/drawing.o texture.o dc/texture.o \
collision.o input.o dc/input.o dc/pvr.o dc/framerateselect.o file.o dc/file.o math.o dc/math.o geometry.o \
dc/system.o dc/log.o timer.o optionhandler.o datastructures.o memoryhandler.o \
wrapper.o physicshandler.o script.o stagehandler.o collisionhandler.o \
collisionanimation.o dc/soundeffect.o dc/sound.o animationtree.o texturepool.o \
storyboard.o screeneffect.o texthandler.o \
quicklz.o

defaultall: create_addons_link $(OBJS) subdirs linklib

KOS_CFLAGS += -W -Wall -Werror -DDREAMCAST

include $(KOS_BASE)/addons/Makefile.prefab

create_addons_link:
	rm -f ../include/tari
	ln -s ../libtari/include ../include/tari

clean:
	rm -f *.o
