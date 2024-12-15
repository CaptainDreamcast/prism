#pragma once

#include "wrapper.h"
#include "drawing.h"

namespace prism {

Screen* getLogoScreenFromWrapper();
void setScreenAfterWrapperLogoScreen(Screen* tScreen);
void setLogoScreenFadeOutColor(Color tColor);

}