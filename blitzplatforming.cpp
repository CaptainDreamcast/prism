#include "prism/blitzplatforming.h"

#include <algorithm>
#include <unordered_map>
#include <prism/log.h>
#include <prism/blitzcomponent.h>
#include <prism/blitzentity.h>

namespace prism {

	typedef struct
	{
		CollisionRect mCollisionRect;
		Vector2D mVelocity;
		Vector2D mAcceleration;
		bool mIsJumping;
        int mFallTicks;
	} BlitzPlatformingPlayerData;

	typedef struct
	{
		CollisionRect mCollisionRect;
	} BlitzPlatformingTileData;

	static struct
	{
        bool mIsPaused;
        bool mIsClampedToScreen = false;
        double mGravity = 3.0;

		std::unordered_map<int, BlitzPlatformingPlayerData> mPlayerEntries;
		std::unordered_map<int, BlitzPlatformingTileData> mTileEntries;
	} gBlitzPlatformingData;

	static void loadBlitzPlatformingHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
        gBlitzPlatformingData.mIsPaused = false;
		gBlitzPlatformingData.mPlayerEntries.clear();
		gBlitzPlatformingData.mTileEntries.clear();
	}

	static void unloadBlitzPlatformingHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
		gBlitzPlatformingData.mPlayerEntries.clear();
		gBlitzPlatformingData.mTileEntries.clear();
	}


	static BlitzPlatformingPlayerData* getBlitzPlatformingPlayerData(int tEntityID) {
		if (!stl_map_contains(gBlitzPlatformingData.mPlayerEntries, tEntityID))
		{
			logErrorFormat("Entity %d has no blitz platforming player data", tEntityID);
			recoverFromError();
		}

		return &gBlitzPlatformingData.mPlayerEntries[tEntityID];
	}

	static BlitzPlatformingTileData* getBlitzPlatformingTileData(int tEntityID) {
		if (!stl_map_contains(gBlitzPlatformingData.mTileEntries, tEntityID))
		{
			logErrorFormat("Entity %d has no blitz platforming tile data", tEntityID);
			recoverFromError();
		}

		return &gBlitzPlatformingData.mTileEntries[tEntityID];
	}


    static bool isCollidingWithTilesGeneral(const Vector2D& tOldPos, const Vector2D& tNewPos, const CollisionRect& tCollisionRect, bool(tSpecialCheckFunc)(const CollisionRect& tileRect, const CollisionRect& oldPlayerRect), int* outputTile = nullptr)
    {
        const auto oldPlayerCollisionRect = tCollisionRect + tOldPos;
        const auto newPlayerCollisionRect = tCollisionRect + tNewPos;
        for (const auto& t : gBlitzPlatformingData.mTileEntries)
        {
            const auto pos = getBlitzEntityPosition(t.first);
            const auto collisionRect = pos + t.second.mCollisionRect;
            if (tSpecialCheckFunc(collisionRect, oldPlayerCollisionRect) && checkCollision(newPlayerCollisionRect, collisionRect))
            {
                if (outputTile)
                {
                    *outputTile = t.first;
                }
                return true;
            }
        }
        return false;
    }

    static bool isCollidingWithTiles(const Vector2D& tNewPos, const CollisionRect& tCollisionRect)
    {
        return isCollidingWithTilesGeneral(Vector2D(0, 0), tNewPos, tCollisionRect, [](const CollisionRect&, const CollisionRect&) { return true; });
    }


    static bool isCollidingWithFloorTiles(const Vector2D& tOldPos, const Vector2D& tNewPos, const CollisionRect& tCollisionRect, int* outputTile = nullptr)
    {
        return isCollidingWithTilesGeneral(tOldPos, tNewPos, tCollisionRect, [](const CollisionRect& tileRect, const CollisionRect& oldPlayerRect) 
            { 
                return tileRect.mTopLeft.y > oldPlayerRect.mBottomRight.y;
            }, outputTile);
    }

    static bool isCollidingWithCeilingTiles(const Vector2D& tOldPos, const Vector2D& tNewPos, const CollisionRect& tCollisionRect, int* outputTile = nullptr)
    {
        return isCollidingWithTilesGeneral(tOldPos, tNewPos, tCollisionRect, [](const CollisionRect& tileRect, const CollisionRect& oldPlayerRect)
            {
                return tileRect.mBottomRight.y < oldPlayerRect.mTopLeft.y;
            }, outputTile);
    }

    static bool isCollidingWithWallLeftTiles(const Vector2D& tOldPos, const Vector2D& tNewPos, const CollisionRect& tCollisionRect, int* outputTile = nullptr)
    {
        return isCollidingWithTilesGeneral(tOldPos, tNewPos, tCollisionRect, [](const CollisionRect& tileRect, const CollisionRect& oldPlayerRect)
            {
                return tileRect.mBottomRight.x < oldPlayerRect.mTopLeft.x;
            }, outputTile);
    }

    static bool isCollidingWithWallRightTiles(const Vector2D& tOldPos, const Vector2D& tNewPos, const CollisionRect& tCollisionRect, int* outputTile = nullptr)
    {
        return isCollidingWithTilesGeneral(tOldPos, tNewPos, tCollisionRect, [](const CollisionRect& tileRect, const CollisionRect& oldPlayerRect)
            {
                return tileRect.mTopLeft.x > oldPlayerRect.mBottomRight.x;
            }, outputTile);
    }

    static int blitzPlatformingCheckIterations = 4;
    static bool snapToFloor(int tPlayerEntity, BlitzPlatformingPlayerData& tPlayerData)
    {
        auto pos = getBlitzEntityPositionReference(tPlayerEntity);
        if (isCollidingWithTiles(pos->xy(), tPlayerData.mCollisionRect)) return false;
        if (tPlayerData.mVelocity.y < 0 || tPlayerData.mAcceleration.y < 0) return false;
        const auto delta = (tPlayerData.mIsJumping && tPlayerData.mVelocity.y > 1) ? Vector2D(0, int(tPlayerData.mVelocity.y) + 1) : Vector2D(0, 1);
        for (int i = 1; i <= blitzPlatformingCheckIterations; i++)
        {
            const auto newPos = pos->xy() + delta * (i / double(blitzPlatformingCheckIterations));
            int targetTile = -1;
            if (isCollidingWithFloorTiles(pos->xy(), newPos, tPlayerData.mCollisionRect, &targetTile))
            {
                const auto tilePos = getBlitzEntityPosition(targetTile).xy();
                const auto tileData = getBlitzPlatformingTileData(targetTile);
                const auto tileCollision = tileData->mCollisionRect + tilePos;
                pos->y = tileCollision.mTopLeft.y - tPlayerData.mCollisionRect.mBottomRight.y - 0.5;
                return true;
            }
        }
        return false;
    }

    static bool bumpHeadOnCeiling(int tPlayerEntity, BlitzPlatformingPlayerData& tPlayerData)
    {
        auto pos = getBlitzEntityPositionReference(tPlayerEntity);
        if (isCollidingWithTiles(pos->xy(), tPlayerData.mCollisionRect)) return false;
        if (tPlayerData.mVelocity.y > 0) return false;
        const auto delta = (tPlayerData.mIsJumping  && tPlayerData.mVelocity.y < -1) ? Vector2D(0, int(tPlayerData.mVelocity.y) - 1) : Vector2D(0, -1);
        for (int i = 1; i <= blitzPlatformingCheckIterations; i++)
        {
            const auto newPos = pos->xy() + delta * (i / double(blitzPlatformingCheckIterations));
            int targetTile = -1;
            if (isCollidingWithCeilingTiles(pos->xy(), newPos, tPlayerData.mCollisionRect, &targetTile))
            {
                const auto tilePos = getBlitzEntityPosition(targetTile).xy();
                const auto tileData = getBlitzPlatformingTileData(targetTile);
                const auto tileCollision = tileData->mCollisionRect + tilePos;
                pos->y = tileCollision.mBottomRight.y - tPlayerData.mCollisionRect.mTopLeft.y + 0.5;

                return true;
            }
        }
        return false;
    }

    static bool crashIntoWallLeft(int tPlayerEntity, BlitzPlatformingPlayerData& tPlayerData)
    {
        auto pos = getBlitzEntityPositionReference(tPlayerEntity);
        if (isCollidingWithTiles(pos->xy(), tPlayerData.mCollisionRect)) return false;
        if (tPlayerData.mVelocity.x > 0) return false;
        const auto delta = (tPlayerData.mVelocity.x < -1) ? Vector2D(int(tPlayerData.mVelocity.x) - 1, 0) : Vector2D(-1, 0);
        for (int i = 1; i <= blitzPlatformingCheckIterations; i++)
        {
            const auto newPos = pos->xy() + delta * (i / double(blitzPlatformingCheckIterations));
            int targetTile = -1;
            if (isCollidingWithWallLeftTiles(pos->xy(), newPos, tPlayerData.mCollisionRect, &targetTile))
            {
                const auto tilePos = getBlitzEntityPosition(targetTile).xy();
                const auto tileData = getBlitzPlatformingTileData(targetTile);
                const auto tileCollision = tileData->mCollisionRect + tilePos;
                pos->x = tileCollision.mBottomRight.x - tPlayerData.mCollisionRect.mTopLeft.x + 0.5;

                return true;
            }
        }
        return false;
    }

    static bool crashIntoWallRight(int tPlayerEntity, BlitzPlatformingPlayerData& tPlayerData)
    {
        auto pos = getBlitzEntityPositionReference(tPlayerEntity);
        if (isCollidingWithTiles(pos->xy(), tPlayerData.mCollisionRect)) return false;
        if (tPlayerData.mVelocity.x < 0) return false;
        const auto delta = (tPlayerData.mVelocity.x > 1) ? Vector2D(int(tPlayerData.mVelocity.x) + 1, 0) : Vector2D(1, 0);
        for (int i = 1; i <= blitzPlatformingCheckIterations; i++)
        {
            const auto newPos = pos->xy() + delta * (i / double(blitzPlatformingCheckIterations));
            int targetTile = -1;
            if (isCollidingWithWallRightTiles(pos->xy(), newPos, tPlayerData.mCollisionRect, &targetTile))
            {
                const auto tilePos = getBlitzEntityPosition(targetTile).xy();
                const auto tileData = getBlitzPlatformingTileData(targetTile);
                const auto tileCollision = tileData->mCollisionRect + tilePos;
                pos->x = tileCollision.mTopLeft.x - tPlayerData.mCollisionRect.mBottomRight.x - 0.5;

                return true;
            }
        }
        return false;
    }

    static void updatePlayerPositionUpdate(int tPlayerEntity, BlitzPlatformingPlayerData& e)
    {
        auto pos = getBlitzEntityPositionReference(tPlayerEntity);
        e.mVelocity += e.mAcceleration;
        const auto crashedIntoLeft = crashIntoWallLeft(tPlayerEntity, e);
        const auto crashedIntoRight = crashIntoWallRight(tPlayerEntity, e);
        if (crashedIntoLeft || crashedIntoRight)
        {
            e.mVelocity.x = 0;
            e.mAcceleration.x = 0;
        }
        const auto newPos = *pos + e.mVelocity;
        if (!isCollidingWithTiles(newPos.xy(), e.mCollisionRect))
        {
            pos->x += e.mVelocity.x;
        }
        pos->y += e.mVelocity.y;
        if (gBlitzPlatformingData.mIsClampedToScreen)
        {
            pos->x = std::clamp(pos->x, 0.0, 320.0);
        }
        e.mVelocity.x *= 0.85;
        e.mAcceleration = Vector2D(0, 0);
    }

    static void updatePlayerGravity(BlitzPlatformingPlayerData& e) {
        if (e.mIsJumping)
        {
            if (e.mVelocity.y < -0.5) e.mAcceleration.y += 0.1;
            else if (e.mVelocity.y > 1) e.mAcceleration.y += 0.1;
            else e.mAcceleration.y += 0.2;
        }
    }

    static void updatePlayerLanding(int tEntityID, BlitzPlatformingPlayerData& e) {
        if (!e.mIsJumping) return;

        if (bumpHeadOnCeiling(tEntityID, e))
        {
            e.mVelocity.y = 0;
        }

        if (snapToFloor(tEntityID, e))
        {
            e.mIsJumping = false;
            e.mAcceleration.y = 0;
            e.mVelocity.y = 0;
        }
    }

    static void updatePlayerFalling(int tEntityID, BlitzPlatformingPlayerData& e) {
        if (e.mIsJumping)
        {
            e.mFallTicks = 0;
            return;
        }

        if (!snapToFloor(tEntityID, e))
        {
            e.mFallTicks++;
        }
        else
        {
            e.mFallTicks = 0;
        }

        if (e.mFallTicks >= 5)
        {
            e.mFallTicks = 0;
            e.mIsJumping = true;
        }
    }

	static void updateBlitzPlatformingSinglePlayer(int tEntityID, BlitzPlatformingPlayerData& e)
	{
        updatePlayerGravity(e);
        updatePlayerFalling(tEntityID, e);
        updatePlayerLanding(tEntityID, e);
        updatePlayerPositionUpdate(tEntityID, e);
	}

	static void updateBlitzPlatformingPlayers()
	{
		for (auto& p : gBlitzPlatformingData.mPlayerEntries)
		{
			updateBlitzPlatformingSinglePlayer(p.first, p.second);
		}
	}

	static void updateBlitzPlatformingHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
        if (gBlitzPlatformingData.mIsPaused) return;

		updateBlitzPlatformingPlayers();
	}

	static void unregisterEntity(int tEntityID);

	static BlitzComponent getBlitzPlatformingComponent() {
		return makeBlitzComponent(unregisterEntity);
	}

	ActorBlueprint getBlitzPlatformingHandler() {
		return makeActorBlueprint(loadBlitzPlatformingHandler, unloadBlitzPlatformingHandler, updateBlitzPlatformingHandler);
	}

	void removeBlitzPlatformingComponent(int tEntityID)
	{
		unregisterEntity(tEntityID);
	}

	static void addBlitzPlatformingComponentGeneral(int tEntityID)
	{
		registerBlitzComponent(tEntityID, getBlitzPlatformingComponent());
	}

	void addBlitzPlatformingPlayerComponent(int tEntityID, const CollisionRect& tCollisionRect)
	{
        if (stl_map_contains(gBlitzPlatformingData.mPlayerEntries, tEntityID))
        {
            logErrorFormat("Entity ID %d already has player entry, removing previous one.", tEntityID);
        }
        BlitzPlatformingPlayerData& e = gBlitzPlatformingData.mPlayerEntries[tEntityID];
        e.mCollisionRect = tCollisionRect;
        e.mVelocity = Vector2D(0, 0);
        e.mAcceleration = Vector2D(0, 0);
        e.mIsJumping = false;
        e.mFallTicks = 0;

		addBlitzPlatformingComponentGeneral(tEntityID);
	}

	void addBlitzPlatformingPlayerJump(int tEntityID)
	{
        auto e = getBlitzPlatformingPlayerData(tEntityID);
        e->mAcceleration.y = -gBlitzPlatformingData.mGravity;
        e->mIsJumping = true;
	}

	void addBlitzPlatformingPlayerMovement(int tEntityID, double t)
	{
        auto e = getBlitzPlatformingPlayerData(tEntityID);
        static constexpr auto ACCELERATION_SPEED = 0.4;
        e->mAcceleration.x = t * ACCELERATION_SPEED;
	}

    bool isBlitzPlatformingPlayerJumping(int tEntityID)
    {
        auto e = getBlitzPlatformingPlayerData(tEntityID);
        return e->mIsJumping;
    }

    void setBlitzPlatformingPlayerMovementXStopped(int tEntityID)
    {
        auto e = getBlitzPlatformingPlayerData(tEntityID);
        e->mAcceleration.x = 0.f;
        e->mVelocity.x = 0.f;
    }

    void setBlitzPlatformClampedToScreen(bool tIsClamped)
    {
        gBlitzPlatformingData.mIsClampedToScreen = tIsClamped;
    }
	void setBlitzPlatformGravity(double tGravity)
    {
        gBlitzPlatformingData.mGravity = tGravity;
    }

	void addBlitzPlatformingSolidTileComponent(int tEntityID, const CollisionRect& tCollisionRect)
	{
		if (stl_map_contains(gBlitzPlatformingData.mTileEntries, tEntityID))
		{
			logErrorFormat("Entity ID %d already has tile entry, removing previous one.", tEntityID);
		}
		BlitzPlatformingTileData& e = gBlitzPlatformingData.mTileEntries[tEntityID];
		e.mCollisionRect = tCollisionRect;

		addBlitzPlatformingComponentGeneral(tEntityID);
	}

	static void unregisterEntity(int tEntityID) {
		gBlitzPlatformingData.mPlayerEntries.erase(tEntityID);
		gBlitzPlatformingData.mTileEntries.erase(tEntityID);
	}

    void setBlitzPlatformingPausing(bool tIsPaused)
    {
        gBlitzPlatformingData.mIsPaused = tIsPaused;
    }
}