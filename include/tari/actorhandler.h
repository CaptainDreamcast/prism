#pragma once

#include "common/header.h"

typedef void(*LoadActorFunction)(void* tOptionalData);
typedef void(*UpdateActorFunction)(void* tOptionalData);
typedef void(*DrawActorFunction)(void* tOptionalData);
typedef void(*UnloadActorFunction)(void* tOptionalData);
typedef int(*IsActorActiveFunction)(void* tOptionalData);
typedef void(*ActorInteractionFunction)(void* tCaller, void* tActorData);

typedef struct {
	LoadActorFunction mLoad;
	UnloadActorFunction mUnload;
	UpdateActorFunction mUpdate;
	DrawActorFunction mDraw;

	IsActorActiveFunction mIsActive;
} ActorBlueprint;

fup void setupActorHandler();
fup void shutdownActorHandler();
fup void updateActorHandler();
fup void drawActorHandler();

fup int instantiateActor(ActorBlueprint tBP);
fup int instantiateActorWithData(ActorBlueprint tBP, void* tData, int tIsOwned);
fup void performOnActor(int tID, ActorInteractionFunction tFunc, void* tCaller);
fup void removeActor(int tID);
fup void* getActorData(int tID);