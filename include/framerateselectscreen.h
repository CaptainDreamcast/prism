#ifndef TARI_FRAMERATESELECTSCREEN_H_
#define TARI_FRAMERATESELECTSCREEN_H_

typedef enum {
  FRAMERATE_SCREEN_RETURN_NORMAL,
  FRAMERATE_SCREEN_RETURN_ABORT
} FramerateSelectReturnType;

FramerateSelectReturnType selectFramerate();

#endif
