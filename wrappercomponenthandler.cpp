#include "prism/wrappercomponenthandler.h"

#include <stdio.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"

static struct {
	List mComponents;

} gData;

void setupWrapperComponentHandler()
{
	gData.mComponents = new_list();
}

static void updateSingleWrapperComponent(void* tCaller, void* tData) {
	(void)tCaller;
	ActorBlueprint* e = (ActorBlueprint*)tData;
	if (e->mUpdate) e->mUpdate(NULL);
}


void updateWrapperComponentHandler()
{
	list_map(&gData.mComponents, updateSingleWrapperComponent, NULL);
}

static void drawSingleWrapperComponent(void* tCaller, void* tData) {
	(void)tCaller;
	ActorBlueprint* e = (ActorBlueprint*)tData;
	if (e->mDraw) e->mDraw(NULL);
}

void drawWrapperComponentHandler()
{
	list_map(&gData.mComponents, drawSingleWrapperComponent, NULL);
}

static int shutdownSingleWrapperComponent(void* tCaller, void* tData) {
	(void)tCaller;
	ActorBlueprint* e = (ActorBlueprint*)tData;
	if (e->mUnload) e->mUnload(NULL);

	return 1;
}

void shutdownWrapperComponentHandler()
{
	list_remove_predicate(&gData.mComponents, shutdownSingleWrapperComponent, NULL);
	delete_list(&gData.mComponents);
}

void addWrapperComponent(ActorBlueprint tComponentBlueprint)
{
	ActorBlueprint* e = (ActorBlueprint*)allocMemory(sizeof(ActorBlueprint));
	*e = tComponentBlueprint;
	if (e->mLoad) e->mLoad(NULL);

	list_push_back_owned(&gData.mComponents, e);
}
