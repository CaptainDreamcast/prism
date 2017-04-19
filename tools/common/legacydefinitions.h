#pragma once

// TODO: remove all this

#define NormalNameSize 9
#define ActualNameSize 8

#define WhiteColor 					0
#define BlackColor 					1
#define RedColor 					2
#define GreenColor 					3
#define BlueColor 					4
#define YellowColor 				5

#define ScreenSizeY					480

#define ReturnOffset 0
#define ReturnExistence 1

#define AnimationIDOffsetSameStrengthButtons	 	2
#define AnimationIDOffsetDownDownForwardForward 	3
#define AnimationIDOffsetDownDownBackBack	 	4
#define AnimationIDOffsetForwardDownDownForward		5
#define AnimationIDOffsetHoldingForward			6
#define AnimationIDOffsetHoldingBack	7


#ifdef DREAMCAST

#define FOLDER_SLASH '/'

#elif defined _WIN32
#define	FOLDER_SLASH '\\' 
#endif
