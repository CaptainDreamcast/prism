#ifndef TARI_OPTIONHANDLER
#define TARI_OPTIONHANDLER

#include "geometry.h"

typedef void (*OptionCB)(void* caller);

void setupOptionHandler();
void shutdownOptionHandler();
void disableOptionHandler();
int addOption(Position tPosition, char* tText, OptionCB tCB, void* tCaller);
void setOptionTextSize(int tSize);
void setOptionButtonA();
void setOptionButtonStart();
void updateOptionHandler();
void drawOptionHandler();

#endif
