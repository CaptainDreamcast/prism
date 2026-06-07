#include "prism/blitzcollision.h"

#include <unordered_set>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/blitzentity.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/collisionhandler.h"
#include "prism/blitzmugenanimation.h"
#include "prism/blitzphysics.h"
#include "prism/stlutil.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

using namespace std;
namespace prism {

	typedef struct {
		void(*mFunc)(void*, void*);
		void* mCaller;
	} BlitzCollisionCallbackData;

	typedef struct {
		int mEntityID;
		CollisionListData* mCollisionList;

		int mOwnsCollisionHandlerObject;
		CollisionListElement* mCollisionHandlerElement;

		void* mCollisionData;

		int mIsSolid;
		int mIsMovable;

		int mHasCollidedThisFrame;

		List mCollisionCallbacks;
	} BlitzCollisionObject;

	typedef struct {
		BlitzCollisionObject* a;
		BlitzCollisionObject* b;
	} ActiveSolidCollision;

	using CollidedEntity = std::pair<int, CollisionListData*>;
	struct CollidedEntityHash {
		std::size_t operator()(const CollidedEntity& k) const {
			return std::hash<int>()(k.first) ^ std::hash<CollisionListData*>()(k.second);
		}
	};

	typedef struct {
		int mEntityID;

		int mIsTopCollided;
		int mIsBottomCollided;
		int mIsLeftCollided;
		int mIsRightCollided;

		unordered_map<int, BlitzCollisionObject> mCollisionObjects;
		std::unordered_set<CollidedEntity, CollidedEntityHash> mCollidedEntities[2]; // [0] is current frame for use in CBs, [1] is last frame for access
	} CollisionEntry;

	static struct {
		unordered_map<int, CollisionEntry> mEntries;
		list<ActiveSolidCollision> mActiveSolidCollisions;
	} gBlitzCollisionData;

#ifdef _WIN32
	void imguiBlitzCollisionHandler()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Blitz", "Collision Handler", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Blitz Collision Handler", &isWindowShown);
		ImGui::Text("Entries = %d", (int)gBlitzCollisionData.mEntries.size());
		ImGui::Text("Active Solid Collisions = %d", (int)gBlitzCollisionData.mActiveSolidCollisions.size());
		ImGui::TextDisabled("T/B/L/R = Top/Bottom/Left/Right collided");
		ImGui::Separator();

		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
		if (ImGui::BeginTable("BlitzCollisions", 4, flags, ImVec2(0, 300)))
		{
			ImGui::TableSetupColumn("Entity");
			ImGui::TableSetupColumn("T/B/L/R");
			ImGui::TableSetupColumn("Objects");
			ImGui::TableSetupColumn("Collided");
			ImGui::TableHeadersRow();

			for (auto& entryPair : gBlitzCollisionData.mEntries)
			{
				CollisionEntry& e = entryPair.second;
				ImGui::TableNextRow(); ImGui::TableNextColumn();
				ImGui::Text("%d", e.mEntityID); ImGui::TableNextColumn();
				ImGui::Text("%d%d%d%d", e.mIsTopCollided, e.mIsBottomCollided, e.mIsLeftCollided, e.mIsRightCollided); ImGui::TableNextColumn();
				ImGui::Text("%d", (int)e.mCollisionObjects.size()); ImGui::TableNextColumn();
				ImGui::Text("%d", (int)e.mCollidedEntities[1].size());
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}
#endif

	static void loadBlitzCollisionHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
		gBlitzCollisionData.mEntries.clear();
		gBlitzCollisionData.mActiveSolidCollisions.clear();
	}

	static void unloadBlitzCollisionHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
		gBlitzCollisionData.mEntries.clear();
		gBlitzCollisionData.mActiveSolidCollisions.clear();
	}

	static void updateSingleBlitzCollidedValue(int* tValue) {
		*tValue = 0;
	}

	static void resetSingleBlitzCollisionEntryCollisionObjectCollisionStates(CollisionEntry& tData)
	{
		for (auto& e : tData.mCollisionObjects) {
			// keep it around for a frame, this is executed after the base collision handler sets it in the callback
			if (e.second.mHasCollidedThisFrame == 1) e.second.mHasCollidedThisFrame = 2;
			else if (e.second.mHasCollidedThisFrame == 2) e.second.mHasCollidedThisFrame = 0;
		}
	}

	static void updateSingleBlitzCollisionEntry(void*, CollisionEntry& tData) {
		CollisionEntry* e = &tData;
		resetSingleBlitzCollisionEntryCollisionObjectCollisionStates(tData);
		updateSingleBlitzCollidedValue(&e->mIsTopCollided);
		updateSingleBlitzCollidedValue(&e->mIsBottomCollided);
		updateSingleBlitzCollidedValue(&e->mIsLeftCollided);
		updateSingleBlitzCollidedValue(&e->mIsRightCollided);
		e->mCollidedEntities[1] = std::move(e->mCollidedEntities[0]);
	}

	static CollisionEntry* getBlitzCollisionEntry(int tEntityID) {
		if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) {
			addBlitzCollisionComponent(tEntityID);
		}

		return &gBlitzCollisionData.mEntries[tEntityID];
	}

	static BlitzCollisionObject* getBlitzCollisionObject(int tEntityID, int tCollisionID) {
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);

		if (!stl_map_contains(e->mCollisionObjects, tCollisionID)) {
			logErrorFormat("Entity with ID %d does not have collision with id %d.", tEntityID, tCollisionID);
			recoverFromError();
		}

		return &e->mCollisionObjects[tCollisionID];
	}

	static void setBlitzCollisionTopCollided(int tEntityID, int tIsCollided) {
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		e->mIsTopCollided |= tIsCollided;
	}

	static void setBlitzCollisionBottomCollided(int tEntityID, int tIsCollided) {
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		e->mIsBottomCollided |= tIsCollided;
	}

	static void setBlitzCollisionLeftCollided(int tEntityID, int tIsCollided) {
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		e->mIsLeftCollided |= tIsCollided;
	}

	static void setBlitzCollisionRightCollided(int tEntityID, int tIsCollided) {
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		e->mIsRightCollided |= tIsCollided;
	}


	static int updateSingleSolidCollision(void*, ActiveSolidCollision& tData) {
		ActiveSolidCollision* e = &tData;
		BlitzCollisionObject* selfObject = e->a;
		BlitzCollisionObject* otherObject = e->b;
		if (!isHandledCollisionValid(selfObject->mCollisionHandlerElement)) return 1;
		if (!isHandledCollisionValid(otherObject->mCollisionHandlerElement)) return 1;

		if (selfObject->mIsSolid && otherObject->mIsSolid) {
			if (selfObject->mIsMovable && !otherObject->mIsMovable) {
				resolveHandledCollisionMovableStatic(selfObject->mCollisionHandlerElement, otherObject->mCollisionHandlerElement, getBlitzEntityPositionReference(selfObject->mEntityID), getBlitzPhysicsVelocity(selfObject->mEntityID));
				setBlitzCollisionTopCollided(selfObject->mEntityID, isHandledCollisionBelowOtherCollision(selfObject->mCollisionHandlerElement, otherObject->mCollisionHandlerElement));
				setBlitzCollisionBottomCollided(selfObject->mEntityID, isHandledCollisionAboveOtherCollision(selfObject->mCollisionHandlerElement, otherObject->mCollisionHandlerElement));
				setBlitzCollisionLeftCollided(selfObject->mEntityID, isHandledCollisionRightOfOtherCollision(selfObject->mCollisionHandlerElement, otherObject->mCollisionHandlerElement));
				setBlitzCollisionRightCollided(selfObject->mEntityID, isHandledCollisionLeftOfOtherCollision(selfObject->mCollisionHandlerElement, otherObject->mCollisionHandlerElement));
			}
		}
		return 1;
	}

	static void updateBlitzCollisionHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();

		stl_int_map_map(gBlitzCollisionData.mEntries, updateSingleBlitzCollisionEntry);
		stl_list_remove_predicate(gBlitzCollisionData.mActiveSolidCollisions, updateSingleSolidCollision);
	}

	static void unregisterEntity(int tEntityID);

	static BlitzComponent getBlitzCollisionComponent() {
		return makeBlitzComponent(unregisterEntity);
	}

	ActorBlueprint getBlitzCollisionHandler() {
		return makeActorBlueprint(loadBlitzCollisionHandler, unloadBlitzCollisionHandler, updateBlitzCollisionHandler);
	}

	static void internalCollisionHandleSingleCB(void* tCaller, void* tData) {
		BlitzCollisionObject* otherObject = (BlitzCollisionObject*)tCaller;
		BlitzCollisionCallbackData* callbackData = (BlitzCollisionCallbackData*)tData;

		if (!otherObject) {
			callbackData->mFunc(callbackData->mCaller, NULL);
		}
		else {
			callbackData->mFunc(callbackData->mCaller, otherObject->mCollisionData);
		}
	}

	static void internalCollisionCB(void* tCaller, void* tCollisionData, int /*tOtherCollisionList*/) {
		BlitzCollisionObject* selfObject = (BlitzCollisionObject*)tCaller;
		BlitzCollisionObject* otherObject = (BlitzCollisionObject*)tCollisionData;

		auto isCollidingWithBlitz = stl_map_contains(gBlitzCollisionData.mEntries, otherObject->mEntityID);
		if(isCollidingWithBlitz) {
			if (selfObject->mIsSolid && otherObject->mIsSolid) {
				if (selfObject->mIsMovable && !otherObject->mIsMovable) {
					ActiveSolidCollision solidCollision;
					solidCollision.a = selfObject;
					solidCollision.b = otherObject;
					gBlitzCollisionData.mActiveSolidCollisions.push_back(solidCollision);
				}
			}
		}

		list_map(&selfObject->mCollisionCallbacks, internalCollisionHandleSingleCB, otherObject);

		selfObject->mHasCollidedThisFrame = 1;
		if(isCollidingWithBlitz) {
			otherObject->mHasCollidedThisFrame = 1;

			auto ownCollisionEntry = getBlitzCollisionEntry(selfObject->mEntityID);
			ownCollisionEntry->mCollidedEntities[0].insert(std::make_pair(otherObject->mEntityID, otherObject->mCollisionList));
			auto otherCollisionEntry = getBlitzCollisionEntry(otherObject->mEntityID);
			otherCollisionEntry->mCollidedEntities[0].insert(std::make_pair(selfObject->mEntityID, selfObject->mCollisionList));
		}
	}

	void addBlitzCollisionComponent(int tEntityID)
	{
		CollisionEntry e{ tEntityID, 0, 0, 0, 0, {} };
		registerBlitzComponent(tEntityID, getBlitzCollisionComponent());
		gBlitzCollisionData.mEntries[tEntityID] = e;
	}

	void removeBlitzCollisionComponent(int tEntityID)
	{
		unregisterEntity(tEntityID);
	}

	static int addEmptyCollisionObject(CollisionEntry* tEntry, CollisionListData* tList, int tOwnsCollisionHandlerObject) {
		int id = stl_int_map_push_back(tEntry->mCollisionObjects, BlitzCollisionObject());
		BlitzCollisionObject* e = &tEntry->mCollisionObjects[id];
		e->mEntityID = tEntry->mEntityID;
		e->mCollisionList = tList;
		e->mOwnsCollisionHandlerObject = tOwnsCollisionHandlerObject;
		e->mCollisionHandlerElement = NULL;
		e->mCollisionData = NULL;
		e->mIsSolid = 0;
		e->mIsMovable = 0;
		e->mHasCollidedThisFrame = 0;
		e->mCollisionCallbacks = new_list();
		return id;
	}

	int addBlitzCollisionPassiveMugen(int tEntityID, CollisionListData* tList)
	{
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		auto mugenAnimationElement = getBlitzMugenAnimationElement(tEntityID);
		int collisionID = addEmptyCollisionObject(e, tList, 0);
		BlitzCollisionObject* object = &e->mCollisionObjects[collisionID];
		setMugenAnimationPassiveCollisionActive(mugenAnimationElement, tList, internalCollisionCB, object, object);

		return collisionID;
	}

	int addBlitzCollisionAttackMugen(int tEntityID, CollisionListData* tList) {
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		auto mugenAnimationElement = getBlitzMugenAnimationElement(tEntityID);
		int collisionID = addEmptyCollisionObject(e, tList, 0);
		BlitzCollisionObject* object = &e->mCollisionObjects[collisionID];
		setMugenAnimationAttackCollisionActive(mugenAnimationElement, tList, internalCollisionCB, object, object);

		return collisionID;
	}

	int addBlitzCollisionRect(int tEntityID, CollisionListData* tList, const CollisionRect& tRectangle)
	{
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		int collisionObjectID = addEmptyCollisionObject(e, tList, 1);
		BlitzCollisionObject* object = &e->mCollisionObjects[collisionObjectID];
		object->mCollisionHandlerElement = addCollisionRectangleToCollisionHandler(tList, getBlitzEntityPositionReference(tEntityID), tRectangle, internalCollisionCB, object, object);

		return collisionObjectID;
	}

	void changeBlitzCollisionRect(int tEntityID, int tCollisionID, const CollisionRect& tRectangle) {
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		BlitzCollisionObject* object = &e->mCollisionObjects[tCollisionID];
		changeCollisionRectangleInCollisionHandler(object->mCollisionHandlerElement, tRectangle);
	}

	int addBlitzCollisionCirc(int tEntityID, CollisionListData* tList, const CollisionCirc& tCircle)
	{
		CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
		int collisionObjectID = addEmptyCollisionObject(e, tList, 1);
		BlitzCollisionObject* object = &e->mCollisionObjects[collisionObjectID];
		object->mCollisionHandlerElement = addCollisionCircleToCollisionHandler(tList, getBlitzEntityPositionReference(tEntityID), tCircle, internalCollisionCB, object, object);

		return collisionObjectID;
	}

	void addBlitzCollisionCB(int tEntityID, int tCollisionID, void(*tCB)(void*, void*), void* tCaller)
	{
		BlitzCollisionObject* object = getBlitzCollisionObject(tEntityID, tCollisionID);
		BlitzCollisionCallbackData* e = (BlitzCollisionCallbackData*)allocMemory(sizeof(BlitzCollisionCallbackData));
		e->mFunc = tCB;
		e->mCaller = tCaller;
		list_push_back_owned(&object->mCollisionCallbacks, e);
	}

	void setBlitzCollisionCollisionData(int tEntityID, int tCollisionID, void* tCollisionData) {
		BlitzCollisionObject* e = getBlitzCollisionObject(tEntityID, tCollisionID);
		e->mCollisionData = tCollisionData;
	}

	void setBlitzCollisionSolid(int tEntityID, int tCollisionID, int tIsMovable)
	{
		BlitzCollisionObject* e = getBlitzCollisionObject(tEntityID, tCollisionID);
		if (!e->mOwnsCollisionHandlerObject) {
			logErrorFormat("Unable to set entity %d collision id %d solid, does not own collision handler entry.", tEntityID, tCollisionID);
			recoverFromError();
		}

		e->mIsMovable = tIsMovable;
		e->mIsSolid = 1;
	}

	void setBlitzCollisionUnsolid(int tEntityID, int tCollisionID)
	{
		BlitzCollisionObject* e = getBlitzCollisionObject(tEntityID, tCollisionID);
		if (!e->mOwnsCollisionHandlerObject) {
			logErrorFormat("Unable to set entity %d collision id %d solid, does not own collision handler entry.", tEntityID, tCollisionID);
			recoverFromError();
		}

		e->mIsSolid = 0;
	}

	void removeBlitzCollision(int tEntityID, int tCollisionID)
	{
		BlitzCollisionObject* e = getBlitzCollisionObject(tEntityID, tCollisionID);
		if (e->mOwnsCollisionHandlerObject) {
			removeFromCollisionHandler(e->mCollisionHandlerElement);
		}
		delete_list(&e->mCollisionCallbacks);
		gBlitzCollisionData.mEntries[tEntityID].mCollisionObjects.erase(tCollisionID);
	}

	int hasBlitzCollidedTop(int tEntityID)
	{
		if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

		CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
		return e->mIsTopCollided;
	}

	int hasBlitzCollidedBottom(int tEntityID)
	{
		if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

		CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
		return e->mIsBottomCollided;
	}

	int hasBlitzCollidedLeft(int tEntityID)
	{
		if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

		CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
		return e->mIsLeftCollided;
	}

	int hasBlitzCollidedRight(int tEntityID)
	{
		if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

		CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
		return e->mIsRightCollided;
	}

	int hasBlitzCollidedThisFrame(int tEntityID, int tCollisionID)
	{
		if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;
		BlitzCollisionObject* object = getBlitzCollisionObject(tEntityID, tCollisionID);
		return object->mHasCollidedThisFrame;
	}

	std::vector<std::pair<int, CollisionListData*>> getBlitzCollidedEntitiesThisFrame(int tEntityID)
	{
		if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) {
			return std::vector<std::pair<int, CollisionListData*>>();
		}

		CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
		return std::vector<std::pair<int, CollisionListData*>>(e->mCollidedEntities[1].begin(), e->mCollidedEntities[1].end());
	}

	static int removeSingleCollisionObject(void*, BlitzCollisionObject& tData) {
		BlitzCollisionObject* e = &tData;
		if (e->mOwnsCollisionHandlerObject) {
			removeFromCollisionHandler(e->mCollisionHandlerElement);
		}
		delete_list(&e->mCollisionCallbacks);

		return 1;
	}

	static int checkSingleActiveSolidCollisionForRemoval(CollisionEntry* tCaller, ActiveSolidCollision& tData) {
		return tData.a->mEntityID == tCaller->mEntityID || tData.b->mEntityID == tCaller->mEntityID;
	}



	static void unregisterEntity(int tEntityID) {
		CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
		stl_list_remove_predicate(gBlitzCollisionData.mActiveSolidCollisions, checkSingleActiveSolidCollisionForRemoval, e);
		stl_int_map_remove_predicate(e->mCollisionObjects, removeSingleCollisionObject);
		gBlitzCollisionData.mEntries.erase(tEntityID);
	}

	void removeAllBlitzCollisions(int tEntityID)
	{
		CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
		stl_int_map_remove_predicate(e->mCollisionObjects, removeSingleCollisionObject);
	}

}