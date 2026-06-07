#include "gamescreen.h"

#include <prism/numberpopuphandler.h>
#include "storyscreen.h"

/*
----- 16x16 TILE-BASED PLATFORMER WHERE PLAYER/ENEMIES CAN SHOOT

--- COMPONENTS

- FILES
- GENERAL
- BG
- PLATFORMS
- PLAYER
- ENEMY HANDLER
- SHOT HANDLER
- TUTORIAL
- VICTORY SCREEN
- LOSS SCREEN


--- Anims

- BG 1

- PLAYER 10
- Player walking 11
- Player dying 12
- Player winning 13
- Player shooting 14
- Player jumping 15

- ENEMY 20
- Enemy walking 21
- Enemy detection 22
- Enemy shooting 23
- Enemy dying 24

- TILES SOLID 30
- TILES LEFT 31
- TILES RIGHT 32
- TILES ALONE 33
- TILES Below 34
- TILES Below + Window 35
- Spikes 36

- Spawn 40
- Goal 41

- Enemy Bullet 60
- Enemy Bullet hit 61
- Player Bullet 62
- Player Bullet hit 63

- Start UI 70
- Loss UI 71
- Win UI 72
- Tutorial 73


--- Z

- BG 1
- Tiles 5
- Goal 5
- PLAYER 10
- Mask 11
- ENEMY 20
- Spawn 30
- Floating numbers 35
- UI 50


--- SFX
PLAYER JUMP 1 0
PLAYER DEATH 1 2
ENEMY SHOOTING 1 5
ENEMY HIT 1 6
PLAYER SHOOTING 1 7
LOSE JINGLE 100 0
VICTORY JINGLE 100 0

*/

static struct
{
    int mLevel = 0;
    int mGameTicks;
    int hasShownTutorial = 0;
} gGameScreenData;

class GameScreen
{
public:
    GameScreen() {
        instantiateActor(getPrismNumberPopupHandler());
        load();
    }

    double sfxVol = 0.2;
    double jingleVol = 1.0;

    MugenSpriteFile mSprites;
    MugenAnimations mAnimations;
    MugenSounds mSounds;

    void loadFiles() {
        mSprites = loadMugenSpriteFileWithoutPalette("game/GAME.sff");
        mAnimations = loadMugenAnimationFile("game/GAME.air");
        mSounds = loadMugenSoundFile("game/GAME.snd");
    }

    void load() {
        loadFiles();
        loadGeneral();
        loadBG();
        loadPlatforms();
        loadPlayer();
        loadEnemyHandler();
        loadPlayerShotHandler();
        loadTutorial();
        loadGetReady();
        loadVictory();
        loadLoss();

        streamMusicFile("game/GAME.ogg");
        //activateCollisionHandlerDebugMode();
    }

    void update() {
        updateBG();
        updatePlatforms();
        updatePlayer();
        updateEnemyHandler();
        updatePlayerShotHandler();
        updateTutorial();
        updateGetReady();
        updateVictory();
        updateLoss();
    }


    // GENERAL
    void loadGeneral() {
        loadCollisions();
    }
    CollisionListData* playerCollisionListShots;
    CollisionListData* playerCollisionListSpikes;
    CollisionListData* enemyShotCollisionList;
    CollisionListData* enemyCollisionList;
    CollisionListData* playerShotCollisionList;
    CollisionListData* spikeCollisionList;
    CollisionListData* tileCollisionList;
    void loadCollisions() {
        playerCollisionListShots = addCollisionListToHandler();
        playerCollisionListSpikes = addCollisionListToHandler();
        enemyShotCollisionList = addCollisionListToHandler();
        enemyCollisionList = addCollisionListToHandler();
        playerShotCollisionList = addCollisionListToHandler();
        spikeCollisionList = addCollisionListToHandler();
        tileCollisionList = addCollisionListToHandler();
        addCollisionHandlerCheck(playerCollisionListShots, enemyShotCollisionList);
        addCollisionHandlerCheck(playerCollisionListSpikes, spikeCollisionList);
        addCollisionHandlerCheck(enemyCollisionList, playerShotCollisionList);
        addCollisionHandlerCheck(tileCollisionList, enemyShotCollisionList);
        addCollisionHandlerCheck(tileCollisionList, playerShotCollisionList);
    }

    // BG
    int bgEntityId;
    void loadBG() {
        bgEntityId = addBlitzEntity(Vector3D(0, 0, 1));
        addBlitzMugenAnimationComponent(bgEntityId, &mSprites, &mAnimations, 1);
    }
    void updateBG() {
    }

    bool isPaused()
    {
        return isShowingLoss || isShowingGetReady || isShowingVictory || isShowingTutorial;
    }

    Vector2DI pos2Tile(const Vector2D& pos)
    {
        return Vector2DI(int(pos.x / 16), int(pos.y / 16));
    }

    Vector2D tile2PlacementPos(const Vector2DI& tile)
    {
        return Vector2D(tile.x * 16 + 8, tile.y * 16 + 15);
    }
    Vector2D tile2TopLeftPos(const Vector2DI& tile)
    {
        return Vector2D(tile.x * 16, tile.y * 16);
    }

    int tileWidth = 20;
    int tileHeight = 15;

    enum class TileFlags : int
    {
        SOLID = 1,
        ENEMY_SPAWN = 2,
        ENEMY_MOVEMENT = 4,
        PLAYER_SPAWN = 8,
        PLAYER_GOAL = 16,
        SPIKE = 32,
    };
    std::vector<std::vector<int>> levelTileFlags;
    std::vector<std::vector<int>> levelTileEntities;

    // PLATFORMS
    Vector2DI playerStartTile = Vector2DI(0, 0);
    Vector2DI playerTargetTile = Vector2DI(0, 0);
    void loadLevelFromFile() {
        std::string path = std::string("game/LEVEL") + std::to_string(gGameScreenData.mLevel) + ".txt";
        Buffer b = fileToBuffer(path.c_str());
        auto p = getBufferPointer(b);
        levelTileFlags.resize(tileHeight);
        levelTileEntities.resize(tileHeight);
        for (int y = 0; y < tileHeight; y++)
        {
            levelTileFlags[y].resize(tileWidth, 0);
            levelTileEntities[y].resize(tileWidth, -1);
            for (int x = 0; x < tileWidth; x++)
            {
                levelTileFlags[y][x] = readIntegerFromTextStreamBufferPointer(&p);
                spawnBasedOnFlag(Vector2DI(x, y), levelTileFlags[y][x]);
            }
        }
    }

    void spawnBasedOnFlag(const Vector2DI& tile, int flags) {
        if (flags & int(TileFlags::SOLID))
        {
            spawnSolid(tile);
        }
        if (flags & int(TileFlags::SPIKE))
        {
            spawnSpike(tile);
        }
        if (flags & int(TileFlags::ENEMY_SPAWN))
        {
            spawnEnemy(tile);
        }
        if (flags & int(TileFlags::PLAYER_SPAWN))
        {
            spawnPlayerSpawn(tile);
        }
        if (flags & int(TileFlags::PLAYER_GOAL))
        {
            spawnPlayerGoal(tile);
        }

    }
    void spawnSolid(const Vector2DI& tile) {
        auto pos = tile2TopLeftPos(tile);
        auto entityId = addBlitzEntity(pos.xyz(5));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, 30);
        addBlitzPlatformingSolidTileComponent(entityId, CollisionRect(0, 0.5, 16, 15.5));
        addBlitzCollisionComponent(entityId);
        addBlitzCollisionRect(entityId, tileCollisionList, CollisionRect(0, 0, 16, 16));
        levelTileEntities[tile.y][tile.x] = entityId;
    }

    void spawnSpike(const Vector2DI& tile) {
        auto pos = tile2PlacementPos(tile);
        auto entityId = addBlitzEntity(pos.xyz(5));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, 36);
        addBlitzCollisionComponent(entityId);
        addBlitzCollisionRect(entityId, spikeCollisionList, CollisionRect(-7, -1, 14, 1));
        levelTileEntities[tile.y][tile.x] = entityId;
    }

    void spawnEnemy(const Vector2DI& tile) {
        addEnemy(tile);
    }
    void spawnPlayerSpawn(const Vector2DI& tile) {
        auto pos = tile2PlacementPos(tile);
        auto entityId = addBlitzEntity(pos.xyz(30));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, 40);
        playerStartTile = tile;
        levelTileEntities[tile.y][tile.x] = entityId;
    }
    void spawnPlayerGoal(const Vector2DI& tile) {
        auto pos = tile2PlacementPos(tile);
        auto entityId = addBlitzEntity(pos.xyz(5));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, 41);
        playerTargetTile = tile;
        levelTileEntities[tile.y][tile.x] = entityId;
    }

    void loadPlatforms() {
        loadLevelFromFile();
    }
    void updatePlatforms() {}

    // PLAYER
    int playerEntity;
    int playerCollisionIdShots;
    int playerCollisionIdSpikes;
    bool playerIsDying = false;
    void loadPlayer() {
        auto pos = tile2PlacementPos(playerStartTile);
        playerEntity = addBlitzEntity(pos.xyz(10));
        addBlitzMugenAnimationComponent(playerEntity, &mSprites, &mAnimations, 10);
        playerCollisionIdSpikes = addBlitzCollisionRect(playerEntity, playerCollisionListSpikes, CollisionRect(-4, -16, 8, 16));
        playerCollisionIdShots = addBlitzCollisionRect(playerEntity, playerCollisionListShots, CollisionRect(-1, -6, 3, 2));
        addBlitzPlatformingPlayerComponent(playerEntity, CollisionRect(-4, -16, 8, 16));
    }
    void updatePlayer() {
        if (!isPaused())
        {
            gGameScreenData.mGameTicks++;
        }
        updatePlayerMovement();
        if (!isPaused())
        {
            updatePlayerHit();
        }
    }

    void updatePlayerMovement() {
        if (!isPaused() && !playerIsDying)
        {
            updatePlayerWalking();
            updatePlayerJumping();
            updatePlayerShooting();
            updatePlayerDying();
        }
    }

    int playerShotCooldown = 0;
    void updatePlayerShooting() {
        if (playerShotCooldown)
        {
            playerShotCooldown--;
            return;
        }

        if (hasPressedB())
        {
            addPlayerShot();
            tryPlayMugenSoundAdvanced(&mSounds, 1, 7, sfxVol);
            playerShotCooldown = 30;
        }
    }

    void addPlayerShot()
    {
        const auto dx = getBlitzMugenAnimationIsFacingRight(playerEntity) ? 1.0 : -1.0;
        auto shotOffset = Vector2D(-2, -9);
        if (!getBlitzMugenAnimationIsFacingRight(playerEntity)) shotOffset.x *= -1;
        addGeneralShot(getBlitzEntityPosition(playerEntity).xy() + shotOffset, playerShotCollisionList, 62, 33.0, 4, Vector2D(dx, 0.0));
        changeBlitzMugenAnimationIfDifferent(playerEntity, 14);
    }

    CollisionRect playerCollisionRect = CollisionRect(-4, -16, 8, 16);
    CollisionRect tileCollisionRect = CollisionRect(0, 0, 16, 16);

    void updatePlayerDying()
    {
        auto pos = getBlitzEntityPosition(playerEntity);
        if (pos.y > 270)
        {
            changeBlitzMugenAnimation(playerEntity, 11);
            playerIsDying = true;
        }
    }

    bool canPlayerAnimationIdle()
    {
        auto anim = getBlitzMugenAnimationAnimationNumber(playerEntity);
        auto step = getBlitzMugenAnimationAnimationStep(playerEntity);
        auto stepCount = getBlitzMugenAnimationAnimationStepAmount(playerEntity);
        if (anim == 14 && step < stepCount - 1) return false;
        return true;
    }

    void updatePlayerWalking() {
        double delta = 0;
        if (hasPressedLeft())
        {
            delta -= 1;
            setBlitzMugenAnimationFaceDirection(playerEntity, 0);
        }
        if (hasPressedRight())
        {
            delta += 1;
            setBlitzMugenAnimationFaceDirection(playerEntity, 1);
        }

        if (canPlayerAnimationIdle())
        {
            if (delta && !isBlitzPlatformingPlayerJumping(playerEntity))
            {
                changeBlitzMugenAnimationIfDifferent(playerEntity, 11);
            }
            else
            {
                if (isBlitzPlatformingPlayerJumping(playerEntity))
                {
                    changeBlitzMugenAnimationIfDifferent(playerEntity, 15);
                }
                else
                {
                    changeBlitzMugenAnimationIfDifferent(playerEntity, 10);
                }
            }
        }

        addBlitzPlatformingPlayerMovement(playerEntity, delta);
    }
    void updatePlayerJumping() {
        if (isBlitzPlatformingPlayerJumping(playerEntity))
        {
            return;
        }

        if (hasPressedAFlank())
        {
            tryPlayMugenSoundAdvanced(&mSounds, 1, 0, sfxVol);
            addBlitzPlatformingPlayerJump(playerEntity);
        }
    }

    void updatePlayerHit() {
        if (!playerIsDying && (hasBlitzCollidedThisFrame(playerEntity, playerCollisionIdShots) || hasBlitzCollidedThisFrame(playerEntity, playerCollisionIdSpikes)))
        {
            auto collidedEntities = getBlitzCollidedEntitiesThisFrame(playerEntity);
            bool hasCollidedWithBullet = false;
            for (auto& e : collidedEntities)
            {
                if (e.second == enemyShotCollisionList || e.second == spikeCollisionList)
                {
                    hasCollidedWithBullet = true;
                    break;
                }
            }
            if (hasCollidedWithBullet)
            {
                tryPlayMugenSoundAdvanced(&mSounds, 1, 2, sfxVol);
                changeBlitzMugenAnimation(playerEntity, 12);
                playerIsDying = true;
            }
        }
    }

    // ENEMYHANDLER
    struct Enemy
    {
        int entityId;
        int activeCollision;
        int playerShotCollision;
        Vector2DI currentTile;
        int isFacingRight;
        int detectionTicks = 0;
        int shootingTicks = 0;
        bool isDying = false;
        int stunningTicks = 0;
        bool isToBeRemoved = false;
    };
    std::map<int, Enemy> mEnemies;
    void loadEnemyHandler() {
    }
    void updateEnemyHandler() {
        updateEnemyHandlerActive();
    }
    void updateEnemyHandlerActive() {
        if (isPaused()) return;

        auto it = mEnemies.begin();
        while (it != mEnemies.end())
        {
            updateSingleEnemy(it->second);
            if (it->second.isToBeRemoved)
            {
                unloadEnemy(it->second);
                it = mEnemies.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    // ENEMY
    void addEnemy(const Vector2DI& tile) {
        auto entityId = addBlitzEntity(tile2PlacementPos(tile).xyz(20));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, 20);
        auto activeCollision = addBlitzCollisionRect(entityId, enemyShotCollisionList, CollisionRect(-4, -16, 8, 16)); 
        auto playerShotCollision = addBlitzCollisionAttackMugen(entityId, enemyCollisionList);
        int faceDirection = randfromInteger(0, 4) % 2;
        setBlitzMugenAnimationFaceDirection(entityId, faceDirection);
        mEnemies[entityId] = Enemy{ entityId, activeCollision, playerShotCollision, tile, faceDirection };
    }
    void unloadEnemy(Enemy& e) {
        removeBlitzEntity(e.entityId);
    }

    void updateSingleEnemy(Enemy& e) {
        updateSingleEnemyMovement(e);
        updateSingleEnemyDetection(e);
        updateSingleEnemyShoot(e);
        updateSingleEnemyDying(e);
        updateSingleEnemyStunning(e);
        updateSingleEnemyOver(e);
    }

    bool isEnemyStunned(Enemy& e)
    {
        return e.stunningTicks;
    }

    void updateSingleEnemyStunning(Enemy& e)
    {
        updateSingleEnemyStunningStart(e);
        updateSingleEnemyStunningEnd(e);
    }

    void updateSingleEnemyStunningStart(Enemy& e)
    {
        if (isEnemyStunned(e)) return;

        if (hasBlitzCollidedThisFrame(e.entityId, e.playerShotCollision))
        {
            changeBlitzMugenAnimationIfDifferent(e.entityId, 24);
            tryPlayMugenSoundAdvanced(&mSounds, 1, 6, sfxVol);
            e.stunningTicks = 360;
        }
    }

    void updateSingleEnemyStunningEnd(Enemy& e)
    {
        if (!isEnemyStunned(e)) return;
        e.stunningTicks--;
    }

    bool isSolid(const Vector2DI& tile)
    {
        if (tile.x < 0) return false;
        if (tile.x > tileWidth - 1) return false;
        if (tile.y < 0) return false;
        if (tile.y > tileHeight - 1) return false;
        return levelTileFlags[tile.y][tile.x] & int(TileFlags::SOLID);
    }

    bool canEnemyWalkThere(const Vector2DI& tile)
    {
        if (tile.x < 0) return false;
        if (tile.x > tileWidth - 1) return false;
        if (tile.y < 0) return false;
        if (tile.y > tileHeight - 1) return false;
        return levelTileFlags[tile.y][tile.x] & int(TileFlags::ENEMY_MOVEMENT);
    }

    bool canEnemyMove(Enemy& e)
    {
        auto floorTile = e.currentTile + Vector2DI(0, 1);
        return (isSolid(floorTile - Vector2DI(1, 0)) && canEnemyWalkThere(e.currentTile - Vector2DI(1, 0))) || (isSolid(floorTile + Vector2DI(1, 0)) && canEnemyWalkThere(e.currentTile + Vector2DI(1, 0)));
    }

    bool canEnemyMoveForward(Enemy& e)
    {
        auto pos = getBlitzEntityPosition(e.entityId);
        auto posInTile = int(pos.x) % 16;
        auto floorTile = e.currentTile + Vector2DI(0, 1);
        if (e.isFacingRight)
        {
            return (posInTile < 8 || (isSolid(floorTile + Vector2DI(1, 0)) && canEnemyWalkThere(e.currentTile + Vector2DI(1, 0))));
        }
        else
        {
            return (posInTile > 8 || (isSolid(floorTile - Vector2DI(1, 0)) && canEnemyWalkThere(e.currentTile - Vector2DI(1, 0))));
        }
    }

    void turnEnemyAround(Enemy& e)
    {
        e.isFacingRight ^= 1;
        setBlitzMugenAnimationFaceDirection(e.entityId, e.isFacingRight);
    }

    void moveEnemyForward(Enemy& e)
    {
        auto pos = getBlitzEntityPositionReference(e.entityId);
        auto delta = e.isFacingRight ? 1 : -1;
        double speed = 1.0;
        pos->x += speed * delta;
        e.currentTile.x = pos->x / 16.0;
    }

    bool isEnemyDetecting(Enemy& e)
    {
        return e.detectionTicks > 0;
    }
    bool isEnemyShooting(Enemy& e)
    {
        return e.shootingTicks > 0;
    }
    bool canEnemySeePlayer(Enemy& e)
    {
        if (getPlayerTile() == playerStartTile) return false;
        auto pos = getBlitzEntityPosition(e.entityId) - Vector2D(0, 15);
        auto playerPos = getBlitzEntityPositionReference(playerEntity)->xy() - Vector2D(0, 8);
        if (playerPos.x < pos.x && e.isFacingRight) return false;
        if (playerPos.x > pos.x && !e.isFacingRight) return false;

        auto direction = playerPos - pos.xy();
        if (vecLength(direction) < 1)
        {
            return true;
        }
        int steps = 100;
        for (int i = 0; i < 100; i++)
        {
            double t = i / double(steps);
            auto testPos = pos.xy() + (t * direction);
            if (isSolid(pos2Tile(testPos))) return false;
        }
        return true;
    }

    void updateEnemyTurning(Enemy& e)
    {
        if (randfromInteger(0, 1000) < 25)
        {
            turnEnemyAround(e);
        }
    }

    void updateSingleEnemyMovement(Enemy& e) {
        if (isEnemyStunned(e)) return;
        if (isEnemyDetecting(e)) return;
        if (isEnemyShooting(e)) return;
        if (!canEnemyMove(e))
        {
            changeBlitzMugenAnimationIfDifferent(e.entityId, 20);
            updateEnemyTurning(e);
            return;
        }

        if (!canEnemyMoveForward(e))
        {
            turnEnemyAround(e);
            changeBlitzMugenAnimationIfDifferent(e.entityId, 20);
        }
        else
        {
            moveEnemyForward(e);
            changeBlitzMugenAnimationIfDifferent(e.entityId, 21);
        }
    }
    void updateSingleEnemyDetection(Enemy& e) {
        if (isEnemyStunned(e)) return;
        if (isEnemyShooting(e)) return;
        if (canEnemySeePlayer(e))
        {
            changeBlitzMugenAnimationIfDifferent(e.entityId, 22);
            e.detectionTicks++;
            if (e.detectionTicks >= 30)
            {
                e.shootingTicks = 1;
                e.detectionTicks = 0;
            }
        }
        else
        {
            e.detectionTicks = 0;
        }
    }
    void updateSingleEnemyShoot(Enemy& e) {
        if (isEnemyStunned(e)) return;
        if (!isEnemyShooting(e)) return;

        if (e.shootingTicks % 30 == 1)
        {
            changeBlitzMugenAnimationIfDifferent(e.entityId, 23);
            auto pos = getBlitzEntityPosition(e.entityId);
            auto shotPos = pos.xy() - Vector2D(4, 11);
            if (getBlitzMugenAnimationIsFacingRight(e.entityId)) pos.x *= -1;

            tryPlayMugenSoundAdvanced(&mSounds, 1, 5, sfxVol);
            addEnemyShot(shotPos, 1);
        }

        if (canEnemySeePlayer(e))
        {
            e.shootingTicks++;
        }
        else
        {
            changeBlitzMugenAnimationIfDifferent(e.entityId, 20);
            e.shootingTicks = 0;
        }
    }

    Vector2DI getPlayerTile()
    {
        return pos2Tile(getBlitzEntityPosition(playerEntity).xy() - Vector2D(0, 8));
    }

    bool hasPlayerReachedTarget()
    {
        return getPlayerTile() == playerTargetTile;
    }

    void updateSingleEnemyDying(Enemy& e) {
        if (e.isDying) return;

        if (hasPlayerReachedTarget())
        {
            changeBlitzMugenAnimation(e.entityId, 24);
            auto pos = getBlitzEntityPosition(e.entityId).xy() - Vector2D(0, 16);
            addPrismNumberPopup(-1, pos.xyz(35), 1, Vector3D(0, -1, 0), 2.0, 0, 20);
            e.isDying = true;
        }
    }
    void updateSingleEnemyOver(Enemy& e) {}


    // PLAYERSHOTHANDLER
    struct PlayerShot
    {
        int entityId;
        Vector2D direction;
        double speed;
        int collisionAttack;
        bool isToBeRemoved = false;
    };
    std::map<int, PlayerShot> mPlayerShots;
    void loadPlayerShotHandler() {}
    void updatePlayerShotHandler() {
        updatePlayerShotHandlerActive();
    }
    void updatePlayerShotHandlerActive() {
        auto it = mPlayerShots.begin();
        while (it != mPlayerShots.end())
        {
            updateSinglePlayerShot(it->second);
            if (it->second.isToBeRemoved)
            {
                unloadShot(it->second);
                it = mPlayerShots.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    void removeAllShots()
    {
        auto it = mPlayerShots.begin();
        while (it != mPlayerShots.end())
        {
            unloadShot(it->second);
            it = mPlayerShots.erase(it);
        }
    }

    // SINGLE SHOT
    void unloadShot(PlayerShot& e)
    {
        removeBlitzEntity(e.entityId);
    }

    void addGeneralShot(const Vector2D& pos, CollisionListData* collisionList, int animationNo, double z, double speed, const Vector2D& direction)
    {
        auto entityId = addBlitzEntity(pos.xyz(z));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, animationNo);
        setBlitzEntityRotationZ(entityId, getAngleFromDirection(direction) + M_PI);
        auto collisionAttack = addBlitzCollisionAttackMugen(entityId, collisionList);
        mPlayerShots[entityId] = PlayerShot{ entityId, direction, speed, collisionAttack };
    }

    void updateSinglePlayerShot(PlayerShot& e) {
        updateSinglePlayerShotMovement(e);
        updateSinglePlayerShotGetHit(e);
        updateSinglePlayerShotOver(e);
    }
    void updateSinglePlayerShotMovement(PlayerShot& e) {
        auto pos = getBlitzEntityPositionReference(e.entityId);
        *pos += e.direction * e.speed;
    }
    void updateSinglePlayerShotGetHit(PlayerShot& e) {
        if (hasBlitzCollidedThisFrame(e.entityId, e.collisionAttack))
        {
            e.isToBeRemoved = true;
        }
    }
    void updateSinglePlayerShotOver(PlayerShot& e) {
        auto pos = getBlitzEntityPositionReference(e.entityId);
        if (pos->x < -10 || pos->x > 330 || pos->y < -10 || pos->y > 250)
        {
            e.isToBeRemoved = true;
        }
    }

    void addEnemyShot(const Vector2D& pos, int shotType) {
        auto playerPos = getBlitzEntityPositionReference(playerEntity);
        Vector2D direction = Vector2D(0, 1);
        if (shotType == 1)
        {
            auto playerPos = getBlitzEntityPositionReference(playerEntity)->xy() - Vector2D(0, 8);
            if (vecLength(pos - playerPos) >= 3)
            {
                direction = vecNormalize(playerPos - pos);
            }
        }
        addGeneralShot(pos, enemyShotCollisionList, 60, 33.0, 2.0, direction);
    }
    void addEnemyShot(const Vector2D& pos, const Vector2D& direction) {
        addGeneralShot(pos, enemyShotCollisionList, 60, 33.0, 2.0, direction);
    }

    // TUTORIAL
    MugenAnimationHandlerElement* tutorialElement;
    bool isShowingTutorial = false;
    void loadTutorial() {
        tutorialElement = addMugenAnimation(getMugenAnimation(&mAnimations, 73), &mSprites, Vector3D(0, 0, 50));
        setMugenAnimationVisibility(tutorialElement, false);
    }
    void updateTutorial() {
        updateTutorialStart();
        updateTutorialOver();
    }
    void updateTutorialStart() {
        if (gGameScreenData.hasShownTutorial) return;

        setMugenAnimationVisibility(tutorialElement, 1);

        gGameScreenData.hasShownTutorial = true;
        isShowingTutorial = true;
    }
    void updateTutorialOver() {
        if (!isShowingTutorial) return;

        if (hasPressedStartFlank())
        {
            setMugenAnimationVisibility(tutorialElement, 0);
            isShowingTutorial = false;
        }
    }

    // STARTING
    MugenAnimationHandlerElement* getReadyElement;
    bool hasShownGetReady = false;
    bool isShowingGetReady = false;
    int waveText;
    int uiAnimationTicks = 0;
    void loadGetReady() {
        getReadyElement = addMugenAnimation(getMugenAnimation(&mAnimations, 70), &mSprites, Vector3D(0, 0, 50));
        setMugenAnimationVisibility(getReadyElement, false);
        waveText = addMugenTextMugenStyle((std::string("LEVEL ") + ((gGameScreenData.mLevel < 10) ? "0" : "") + std::to_string(gGameScreenData.mLevel)).c_str(), Vector3D(165, 180, 60), Vector3DI(1, 0, 0));
        setMugenTextScale(waveText, 2.0);
        setMugenTextVisibility(waveText, 0);
    }
    void updateGetReady() {
        updateGetReadyStart();
        updateGetReadyOver();
    }
    void updateGetReadyStart() {
        if (isShowingTutorial) return;
        if (hasShownGetReady) return;

        setMugenAnimationVisibility(getReadyElement, 1);
        setMugenTextVisibility(waveText, 1);

        uiAnimationTicks = 0;
        hasShownGetReady = true;
        isShowingGetReady = true;
    }
    void updateGetReadyOver() {
        if (!isShowingGetReady) return;

        uiAnimationTicks++;
        if (uiAnimationTicks > 120 || (uiAnimationTicks > 1 && hasPressedStartFlank()))
        {
            setMugenAnimationVisibility(getReadyElement, 0);
            setMugenTextVisibility(waveText, 0);
            isShowingGetReady = false;
        }
    }

    bool isFinalLevel()
    {
        std::string path = std::string("game/LEVEL") + std::to_string(gGameScreenData.mLevel + 1) + ".txt";
        return !isFile(path);
    }

    void setPlayerIdle()
    {
        setBlitzPlatformingPlayerMovementXStopped(playerEntity);
    }

    // WINNING
    MugenAnimationHandlerElement* victoryElement;
    bool hasShownVictory = false;
    bool isShowingVictory = false;
    void loadVictory() {
        int animationNo = 72;
        victoryElement = addMugenAnimation(getMugenAnimation(&mAnimations, animationNo), &mSprites, Vector3D(0, 0, 50));
        setMugenAnimationVisibility(victoryElement, false);
    }
    void updateVictory() {
        updateVictoryStart();
        updateVictoryOver();
    }
    void updateVictoryStart() {
        if (hasShownVictory) return;
        if (!hasPlayerReachedTarget()) return;

        tryPlayMugenSoundAdvanced(&mSounds, 100, 1, jingleVol);
        setMugenAnimationVisibility(victoryElement, 1);
        pauseMusic();
        removeAllShots();
        setPlayerIdle();
        changeBlitzMugenAnimation(playerEntity, 13);
        uiAnimationTicks = 0;
        hasShownVictory = true;
        isShowingVictory = true;
    }

    void screenWon() {
        if (isFinalLevel())
        {
            setCurrentStoryDefinitionFile("game/OUTRO.def", 0);
            stopStreamingMusicFile();
            setNewScreen(getStoryScreen());
        }
        else
        {
            gGameScreenData.mLevel++;
            setNewScreen(getGameScreen());
        }
    }
    void resetScreen()
    {
        setNewScreen(getGameScreen());
    }

    void updateVictoryOver() {
        if (!isShowingVictory) return;

        uiAnimationTicks++;
        if (hasPressedStartFlank())
        {
            setMugenAnimationVisibility(victoryElement, 0);
            isShowingVictory = false;
            screenWon();
        }
    }

    // LOSING
    MugenAnimationHandlerElement* lossElement;
    bool hasShownLoss = false;
    bool isShowingLoss = false;
    void loadLoss() {
        lossElement = addMugenAnimation(getMugenAnimation(&mAnimations, 71), &mSprites, Vector3D(0, 0, 50));
        setMugenAnimationVisibility(lossElement, false);
    }
    void updateLoss() {
        updateLossStart();
        updateLossOver();
    }
    void updateLossStart() {
        if (hasShownLoss) return;
        if (!playerIsDying) return;

        setMugenAnimationVisibility(lossElement, 1);
        pauseMusic();
        removeAllShots();
        tryPlayMugenSoundAdvanced(&mSounds, 100, 0, jingleVol);
        setPlayerIdle();
        uiAnimationTicks = 0;
        hasShownLoss = true;
        isShowingLoss = true;
    }
    void updateLossOver() {
        if (!isShowingLoss) return;

        uiAnimationTicks++;
        if (hasPressedStartFlank())
        {
            isShowingLoss = false;
            resetScreen();
        }
    }
};

EXPORT_SCREEN_CLASS(GameScreen);

void resetGame()
{
    gGameScreenData.mLevel = 1;
    gGameScreenData.mGameTicks = 0;
}

std::string getSpeedRunString() {
    int totalSeconds = gGameScreenData.mGameTicks / 60;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    int milliseconds = (gGameScreenData.mGameTicks % 60) * 1000 / 60;
    return std::to_string(minutes) + "m " + std::to_string(seconds) + "s " + std::to_string(milliseconds) + "ms.";
}