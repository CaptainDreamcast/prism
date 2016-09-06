#ifndef TARI_FRAMERATE
#define TARI_FRAMERATE

typedef enum {
  FIFTY_HERTZ = 50,
  SIXTY_HERTZ = 60
} Framerate;

#define FRAMERATE_AMOUNT 2

void setFramerate(Framerate tFramerate);
double getFramerateFactor();
double getInverseFramerateFactor();

#endif
