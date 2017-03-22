#include "include/framerateselectscreen.h"

#include <kos.h>

#include "include/framerate.h"
#include "include/drawing.h"
#include "include/physics.h"
#include "include/input.h"
#include "include/system.h"

typedef struct {
  char ArrayToBePrinted[17];
  int PositionX;
  int PositionY;
  int FontSize;
} FramerateSelectMenuPointStruct;

typedef struct {
  char ArrayToBePrinted[100];
  int PositionX;
  int PositionY;
  int FontSize;
} FramerateSelectLargerArrayStruct;

#define FRAMERATE_WARNING_ARRAY_AMOUNT 2

void setFramerateSixty() {

  setFramerate(SIXTY_HERTZ);
  setScreenFramerate(60);
}

void setFramerateVGA() {
  setFramerate(SIXTY_HERTZ);
  setScreenFramerate(60);
  setVGA();
}

void setFramerateFifty() {
  setFramerate(FIFTY_HERTZ);
  setScreenFramerate(50);
}

int hasToSetFramerate() {
  int whichVideoCable;

  whichVideoCable = vid_check_cable();

  if (whichVideoCable == CT_VGA) {
    setFramerateVGA();
  }
  else {
    if (flashrom_get_region() == FLASHROM_REGION_EUROPE) {

      return 1;

    }
    else {
      setFramerateSixty();
    }
  }
  return 0;
}

static double getScreenFactor() {
	ScreenSize sz = getScreenSize();
	return sz.x / 640.0;
}

#define PAL_INDEX 0
#define NTSC_INDEX 1

FramerateSelectReturnType selectFramerate() {

  if (!hasToSetFramerate()) {
    return FRAMERATE_SCREEN_RETURN_NORMAL;
  }

  FramerateSelectMenuPointStruct selectableArray[FRAMERATE_AMOUNT];
  FramerateSelectLargerArrayStruct currentOptionArray;
  FramerateSelectLargerArrayStruct warningArray[FRAMERATE_AMOUNT];

  uint8 i;
  uint8 whichArraySelected = NTSC_INDEX;
  uint8 isRunning;

  sprintf(selectableArray[0].ArrayToBePrinted, "50HZ");
  sprintf(selectableArray[1].ArrayToBePrinted, "60HZ");
  sprintf(currentOptionArray.ArrayToBePrinted, "CURRENT SELECTED OPTION: %s", selectableArray[whichArraySelected].ArrayToBePrinted);
  sprintf(warningArray[0].ArrayToBePrinted, "WARNING: 50HZ MODE WILL RESULT IN SLOWER GAMEPLAY!!");
  sprintf(warningArray[1].ArrayToBePrinted, "USE 60HZ IF POSSIBLE!");

  for (i = 0; i < FRAMERATE_AMOUNT; i++) {
    selectableArray[i].FontSize = 50*getScreenFactor();
    selectableArray[i].PositionY = 150*getScreenFactor();
  }

  selectableArray[0].PositionX = 80*getScreenFactor();
  selectableArray[1].PositionX = 360*getScreenFactor();

  currentOptionArray.PositionX = 40*getScreenFactor();
  currentOptionArray.PositionY = 335*getScreenFactor();
  currentOptionArray.FontSize = 20*getScreenFactor();

  for (i = 0; i < FRAMERATE_WARNING_ARRAY_AMOUNT; i++) {
    warningArray[i].FontSize = 11*getScreenFactor();
  }

  warningArray[0].PositionX = 40*getScreenFactor();
  warningArray[0].PositionY = 375*getScreenFactor();

  warningArray[1].PositionX = 205*getScreenFactor();
  warningArray[1].PositionY = 390*getScreenFactor();

  setFramerateFifty();
  isRunning = 1;
  while (isRunning) {

    waitForScreen();
    startDrawing();

    for (i = 0; i < FRAMERATE_AMOUNT; i++) {
      drawText(selectableArray[i].ArrayToBePrinted, makePosition(selectableArray[i].PositionX, selectableArray[i].PositionY, 1), selectableArray[i].FontSize, COLOR_WHITE);
    }
    drawText(selectableArray[whichArraySelected].ArrayToBePrinted, makePosition(selectableArray[whichArraySelected].PositionX, selectableArray[whichArraySelected].PositionY, 2), selectableArray[whichArraySelected].FontSize, COLOR_GREEN);
    drawText(currentOptionArray.ArrayToBePrinted, makePosition(currentOptionArray.PositionX, currentOptionArray.PositionY, 1), currentOptionArray.FontSize, COLOR_WHITE);

    if (whichArraySelected == PAL_INDEX) {
      for (i = 0; i < FRAMERATE_WARNING_ARRAY_AMOUNT; i++)
        drawText(warningArray[i].ArrayToBePrinted, makePosition(warningArray[i].PositionX, warningArray[i].PositionY, 1), warningArray[i].FontSize, COLOR_RED);
    }

    stopDrawing();

    updateInput();

    if (hasPressedLeftFlank() && whichArraySelected > 0) {
      whichArraySelected--;
      sprintf(currentOptionArray.ArrayToBePrinted, "CURRENT SELECTED OPTION: %s", selectableArray[whichArraySelected].ArrayToBePrinted);
    }

    if (hasPressedRightFlank() && whichArraySelected < FRAMERATE_AMOUNT - 1) {
      whichArraySelected++;
      sprintf(currentOptionArray.ArrayToBePrinted, "CURRENT SELECTED OPTION: %s", selectableArray[whichArraySelected].ArrayToBePrinted);
    }

    if (hasPressedStartFlank() || hasPressedAFlank()) {
      if (whichArraySelected == PAL_INDEX) {
        setFramerateFifty();
      }
      else {
        setFramerateSixty();
      }
      isRunning = 0;
    }

    if (hasPressedAbortFlank()) {
      return FRAMERATE_SCREEN_RETURN_ABORT;
    }
  }

  return FRAMERATE_SCREEN_RETURN_NORMAL;
}

