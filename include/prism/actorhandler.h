#pragma once

#include <cstdio>

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

ActorBlueprint makeActorBlueprint(LoadActorFunction tLoad, UnloadActorFunction tUnload = NULL, UpdateActorFunction tUpdate = NULL, DrawActorFunction tDraw = NULL, IsActorActiveFunction tIsActive = NULL);
int instantiateActor(ActorBlueprint tBP);
int instantiateActorWithData(ActorBlueprint tBP, void* tData, int tIsOwned);
void performOnActor(int tID, ActorInteractionFunction tFunc, void* tCaller);
void setActorUnpausable(int tID);
void removeActor(int tID);
void* getActorData(int tID);

#define EXPORT_ACTOR_CLASS(tClassName) \
static std::unique_ptr<tClassName> g##tClassName; \
 \
static void loadActorInternal##tClassName(void* tData) { \
	(void)tData; \
	g##tClassName = std::make_unique<tClassName>(); \
} \
 \
static void updateActorInternal##tClassName(void* tData) { \
	(void)tData; \
	g##tClassName->update(); \
} \
 \
static void unloadActorInternal##tClassName(void* tData) { \
	(void)tData; \
	g##tClassName = nullptr; \
} \
 \
ActorBlueprint get##tClassName() \
{ \
	return makeActorBlueprint(loadActorInternal##tClassName, unloadActorInternal##tClassName, updateActorInternal##tClassName); \
} 
