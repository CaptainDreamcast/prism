#pragma once

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

void setupActorHandler();
void shutdownActorHandler();
void updateActorHandler();
void drawActorHandler();

int instantiateActor(ActorBlueprint tBP);
int instantiateActorWithData(ActorBlueprint tBP, void* tData, int tIsOwned);
void performOnActor(int tID, ActorInteractionFunction tFunc, void* tCaller);
void removeActor(int tID);
void* getActorData(int tID);