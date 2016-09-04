#include "include/framerate.h"

#include "include/animation.h"
#include "include/physics.h"

void setFramerate(Framerate tFramerate) {
  setAnimationFramerate(tFramerate);
  setPhysicsFramerate(tFramerate);
}
