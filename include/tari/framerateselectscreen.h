#ifndef TARI_FRAMERATESELECTSCREEN_H_
#define TARI_FRAMERATESELECTSCREEN_H_

#include "common/header.h"

typedef enum {
  FRAMERATE_SCREEN_RETURN_NORMAL,
  FRAMERATE_SCREEN_RETURN_ABORT
} FramerateSelectReturnType;

fup FramerateSelectReturnType selectFramerate();

#endif
