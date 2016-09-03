#ifndef TARI_FRAMERATE
#define TARI_FRAMERATE

typedef enum {
	FIFTY_HERTZ = 50,
	SIXTY_HERTZ = 60
} Framerate;

void setFramerate(Framerate tFramerate);

#endif
