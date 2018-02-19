#pragma once

typedef enum {
  FRAMERATE_SCREEN_RETURN_NORMAL,
  FRAMERATE_SCREEN_RETURN_ABORT
} FramerateSelectReturnType;

FramerateSelectReturnType selectFramerate();

