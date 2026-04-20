#pragma once 

#include "prism/wrapper.h"

namespace prism {
    Screen* getWindowFocusScreen();
    void setWindowFocusScreenNextScreen(Screen* tNext);
}