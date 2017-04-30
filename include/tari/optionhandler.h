#ifndef TARI_OPTIONHANDLER
#define TARI_OPTIONHANDLER

#include "geometry.h"

typedef void (*OptionCB)(void* caller);

fup void setupOptionHandler();
fup void shutdownOptionHandler();
fup void disableOptionHandler();
fup int addOption(Position tPosition, char* tText, OptionCB tCB, void* tCaller);
fup void setOptionTextSize(int tSize);
fup void setOptionButtonA();
fup void setOptionButtonStart();
fup void updateOptionHandler();
fup void drawOptionHandler();

#endif
