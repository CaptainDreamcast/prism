#pragma once


typedef enum {
  FIFTY_HERTZ = 50,
  SIXTY_HERTZ = 60
} Framerate;

#define FRAMERATE_AMOUNT 2

void setFramerate(Framerate tFramerate);
Framerate getFramerate();
double getFramerateFactor();
double getInverseFramerateFactor();
