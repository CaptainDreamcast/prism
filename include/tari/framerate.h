#ifndef TARI_FRAMERATE
#define TARI_FRAMERATE

#include "common/header.h"

typedef enum {
  FIFTY_HERTZ = 50,
  SIXTY_HERTZ = 60
} Framerate;

#define FRAMERATE_AMOUNT 2

fup void setFramerate(Framerate tFramerate);
fup Framerate getFramerate();
fup double getFramerateFactor();
fup double getInverseFramerateFactor();

#endif
