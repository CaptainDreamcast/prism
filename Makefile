TARGET = libtari.a
OBJS = animation.o physics.o framerate.o drawing.o dc/drawing_dc.o texture.o dc/texture_dc.o \
collision.o input.o dc/input_dc.o dc/pvr_dc.o dc/framerateselect_dc.o file.o dc/file_dc.o math.o dc/math_dc.o geometry.o \
dc/system_dc.o dc/log_dc.o timer.o optionhandler.o datastructures.o memoryhandler.o \
wrapper.o physicshandler.o script.o stagehandler.o collisionhandler.o \
collisionanimation.o dc/soundeffect_dc.o dc/sound_dc.o animationtree.o texturepool.o \
storyboard.o screeneffect.o texthandler.o dc/memoryhandler_dc.o \
quicklz.o

defaultall: create_addons_link $(OBJS) subdirs linklib

KOS_CFLAGS += -W -Wall -Werror -DDREAMCAST -I ./include

include $(KOS_BASE)/addons/Makefile.prefab

create_addons_link:
	rm -f ../include/tari
	ln -s ../libtari/include/tari ../include/tari

clean:
	rm -f *.o
