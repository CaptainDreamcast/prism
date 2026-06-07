#include "gamescreen.h"

#include <prism/numberpopuphandler.h>
#include "storyscreen.h"

/*

--- Anims

- BG 1

- Player 10
- Player banking left 11
- Player banking right 12
- Player dying 13

- Player bullet 20

- Enemy 30
- Enemy explosion 31

- Enemy bullet 42

- Boss 50
- Boss explosion 51

- UI bar 60
- Get ready UI 64
- Loss UI 65
- Win UI 66
- Tutorial UI 67

--- Z

- BG 1
- Bullets 9
- Player 10
- Enemy 15
- Boss 20
- Explosions 45
- Floating numbers 50
- UI 50/51

--- SFX

PLAYER SHOOTING 1 1
ENEMY EXPLODE 1 3
PLAYER DEATH 1 4
BOSS HIT 1 6
GET READY JINGLE 100 0
VICTORY JINGLE 100 1
LOSS JINGLE 100 2

*/

static struct
{
    int mLevel = 0;
    int mGameTicks;
    int mScore = 0;
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
        loadPlayer();
        loadEnemyHandler();
        loadBoss();
        loadShotHandler();
        loadScoreUI();
        loadLifeUI();
        loadGetReady();
        loadVictory();
        loadLoss();
        loadTutorial();
    }

    void update() {
        updateBG();
        updateTutorial();
        updateGetReady();

        if (!isPaused())
        {
            updateEnemyHandler();
            updateBoss();
            updateShotHandler();
        }

        updatePlayer();
        updateScoreUI();
        updateLifeUI();
        updateVictory();
        updateLoss();
    }

    // GENERAL
    void loadGeneral() {
        loadCollisions();
    }
    CollisionListData* playerCollisionListShots;
    CollisionListData* enemyShotCollisionList;
    CollisionListData* enemyCollisionListShots;
    CollisionListData* playerShotCollisionList;

    void loadCollisions() {
        playerCollisionListShots = addCollisionListToHandler();
        enemyShotCollisionList = addCollisionListToHandler();
        enemyCollisionListShots = addCollisionListToHandler();
        playerShotCollisionList = addCollisionListToHandler();
        addCollisionHandlerCheck(playerCollisionListShots, enemyShotCollisionList);
        addCollisionHandlerCheck(enemyCollisionListShots, playerShotCollisionList);
        addCollisionHandlerCheck(playerCollisionListShots, enemyCollisionListShots);
    }

    bool isPaused()
    {
        return isShowingLoss || isShowingGetReady || isShowingVictory || isShowingTutorial;
    }

    // BG
    std::vector<int> mBGEntities;
    void loadBG() {
        mBGEntities.resize(2);
        mBGEntities[0] = addBlitzEntity(Vector3D(0, 0, 1));
        addBlitzMugenAnimationComponent(mBGEntities[0], &mSprites, &mAnimations, 1);
        mBGEntities[1] = addBlitzEntity(Vector3D(0, -240, 1));
        addBlitzMugenAnimationComponent(mBGEntities[1], &mSprites, &mAnimations, 1);
    }
    void updateBG() {
        if (isPaused()) return;
        updateBGMovement();
    }

    void updateBGMovement() {
        auto pos1 = getBlitzEntityPositionReference(mBGEntities[0]);
        auto pos2 = getBlitzEntityPositionReference(mBGEntities[1]);
        double speed = 4;
        pos1->y += speed;
        pos2->y += speed;
        if (pos1->y >= 240)
        {
            pos1->y -= 480;
            std::swap(mBGEntities[0], mBGEntities[1]);
        }
    }

    // PLAYER
    int playerEntityId;
    int playerCollisionId;
    int shotCooldown = 0;
    int playerIsDying = 0;
    void loadPlayer() {
        playerEntityId = addBlitzEntity(Vector3D(160, 200, 10));
        addBlitzMugenAnimationComponent(playerEntityId, &mSprites, &mAnimations, 10);
        playerCollisionId = addBlitzCollisionRect(playerEntityId, playerCollisionListShots, CollisionRect(-3, -3, 6, 6));
    }
    void updatePlayer() {
        if (isPaused()) return;

        gGameScreenData.mGameTicks++;
        updatePlayerMovement();
        updatePlayerShoot();
        updatePlayerGetHit();
    }
    void updatePlayerGetHit() {
        if (playerIsDying) return;
        if (hasBlitzCollidedThisFrame(playerEntityId, playerCollisionId))
        {
            tryPlayMugenSoundAdvanced(&mSounds, 1, 4, sfxVol);
            changeBlitzMugenAnimationIfDifferent(playerEntityId, 13);
            playerIsDying = 1;
        }
    }

    void updatePlayerMovement() {
        if (playerIsDying) return;
        auto pos = getBlitzEntityPositionReference(playerEntityId);
        double speed = 2;
        Vector2D dir = Vector2D(0, 0);

        if (hasPressedLeft())
        {
            dir.x = -1;
        }
        if (hasPressedRight())
        {
            dir.x = 1;
        }
        if (hasPressedUp())
        {
            dir.y = -1;
        }
        if (hasPressedDown())
        {
            dir.y = 1;
        }

        if (dir.x == 0)
        {
            changeBlitzMugenAnimationIfDifferent(playerEntityId, 10);
        }
        else if (dir.x < 0)
        {
            changeBlitzMugenAnimationIfDifferent(playerEntityId, 11);
        }
        else
        {
            changeBlitzMugenAnimationIfDifferent(playerEntityId, 12);
        }

        if (dir.x == 0 && dir.y == 0)
        {
            return;
        }

        *pos += dir * speed;
        *pos = clampPositionToGeoRectangle(*pos, GeoRectangle2D(0, 0, 320, 240));
    }
    void updatePlayerShoot()
    {
        if (shotCooldown)
        {
            shotCooldown--;
            return;
        }

        if (hasPressedA())
        {
            addPlayerShot();
            tryPlayMugenSoundAdvanced(&mSounds, 1, 1, sfxVol);
            shotCooldown = 30;
        }
    }

    // ENEMY HANDLER
    struct Enemy
    {
        int entityId;
        int activeCollision;
        Vector2D direction;
        double speed = 1.0;
        int shootingTicks = 0;
        bool isToBeRemoved = false;
    };
    std::map<int, Enemy> mEnemies;

    int enemyCount = 10;
    int enemySpawnInterval = 45;
    int enemySpawnTimer = 30;
    int enemiesSpawned = 0;

    void loadEnemyHandler() {
    }
    void updateEnemyHandler() {
        updateEnemySpawning();
        updateEnemyHandlerActive();
    }
    void updateEnemySpawning() {
        if (bossStarted) return;
        if (enemiesSpawned >= enemyCount) return;

        enemySpawnTimer--;
        if (enemySpawnTimer <= 0)
        {
            double x = randfrom(30, 290);
            addEnemy(Vector2D(x, -10), Vector2D(0, 1), randfrom(1.0, 1.8));
            enemiesSpawned++;
            enemySpawnTimer = enemySpawnInterval;
        }
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

    void removeAllEnemies()
    {
        auto it = mEnemies.begin();
        while (it != mEnemies.end())
        {
            unloadEnemy(it->second);
            it = mEnemies.erase(it);
        }
    }

    void setPlayerIdle()
    {
        changeBlitzMugenAnimationIfDifferent(playerEntityId, 10);
    }

    // SINGLE ENEMY
    void addEnemy(const Vector2D& pos, const Vector2D& direction, double speed) {
        auto entityId = addBlitzEntity(pos.xyz(15));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, 30);
        auto activeCollision = addBlitzCollisionRect(entityId, enemyCollisionListShots, CollisionRect(-8, -8, 16, 16));
        Enemy e;
        e.entityId = entityId;
        e.activeCollision = activeCollision;
        e.direction = direction;
        e.speed = speed;
        e.shootingTicks = randfromInteger(60, 120);
        mEnemies[entityId] = e;
    }
    void unloadEnemy(Enemy& e) {
        removeBlitzEntity(e.entityId);
    }
    void updateSingleEnemy(Enemy& e) {
        updateEnemyMovement(e);
        updateEnemyShoot(e);
        updateEnemyRemove(e);
    }

    void updateEnemyMovement(Enemy& e) {
        auto pos = getBlitzEntityPositionReference(e.entityId);
        *pos += e.direction * e.speed;
    }

    void updateEnemyShoot(Enemy& e)
    {
        if (e.shootingTicks)
        {
            e.shootingTicks--;
            return;
        }

        auto pos = getBlitzEntityPositionReference(e.entityId);
        addEnemyShotAimed(pos->xy(), randfrom(1.5, 3));
        e.shootingTicks = randfromInteger(60, 120);
    }

    void updateEnemyRemove(Enemy& e)
    {
        auto pos = getBlitzEntityPositionReference(e.entityId);
        if (hasBlitzCollidedThisFrame(e.entityId, e.activeCollision))
        {
            tryPlayMugenSoundAdvanced(&mSounds, 1, 3, sfxVol);
            gGameScreenData.mScore += 300;
            addPrismNumberPopup(300, pos->xy().xyz(50), 1, Vector3D(0, -1, 0), 2.0, 2, 20);
            auto enemyExplodeEntity = addBlitzEntity(*pos);
            addBlitzMugenAnimationComponent(enemyExplodeEntity, &mSprites, &mAnimations, 31);
            setBlitzMugenAnimationNoLoop(enemyExplodeEntity);
            e.isToBeRemoved = true;
            return;
        }

        if (pos->y > 260)
        {
            e.isToBeRemoved = true;
        }
    }

    // BOSS
    int bossEntityId;
    int bossCollisionId;
    int bossHealth = 20;
    bool bossStarted = false;
    double bossTargetY = -3;
    double bossVelX = 1.2;
    int bossShootTimer = 90;
    int bossPatternIndex = 0;

    int explosionCount = 8;
    bool bossIsDying = false;
    bool bossIsDead = false;
    int bossDeathTimer = 0;
    int bossExplosionsDone = 0;
    void loadBoss() {
        bossEntityId = addBlitzEntity(Vector3D(160, -50, 20));
        addBlitzMugenAnimationComponent(bossEntityId, &mSprites, &mAnimations, 50);
        setBlitzMugenAnimationVisibility(bossEntityId, 0);
    }
    void updateBoss() {
        updateBossStart();
        updateBossMovement();
        updateBossGetHit();
        updateBossShooting();
        updateBossDeath();
    }

    void updateBossStart() {
        if (bossStarted) return;
        if (enemiesSpawned < enemyCount) return;
        if (!mEnemies.empty()) return;

        bossStarted = true;
        setBlitzMugenAnimationVisibility(bossEntityId, 1);
        bossCollisionId = addBlitzCollisionRect(bossEntityId, enemyCollisionListShots, CollisionRect(-24, -16, 48, 32));
    }
    void updateBossMovement() {
        if (!bossStarted) return;
        if (isBossDefeated()) return;

        auto pos = getBlitzEntityPositionReference(bossEntityId);
        if (pos->y < bossTargetY)
        {
            pos->y += 1.0;
            return;
        }

        pos->x += bossVelX;
        if (pos->x < 50) { pos->x = 50; bossVelX = fabs(bossVelX); }
        if (pos->x > 270) { pos->x = 270; bossVelX = -fabs(bossVelX); }
    }
    void updateBossGetHit() {
        if (!bossStarted) return;
        if (isBossDefeated()) return;

        if (hasBlitzCollidedThisFrame(bossEntityId, bossCollisionId))
        {
            bossHealth--;
            if (bossHealth <= 0)
            {
                bossHealth = 0;
                startBossDeath();
            }
            else
            {
                tryPlayMugenSoundAdvanced(&mSounds, 1, 6, sfxVol);
            }
            addPrismNumberPopup(1000, getBlitzEntityPosition(bossEntityId).xy().xyz(50) + Vector2D(0, 25), 1, Vector3D(0, -1, 0), 2.0, 1, 20);
        }
    }
    void updateBossShooting() {
        if (!bossStarted) return;
        if (isBossDefeated()) return;

        auto pos = getBlitzEntityPositionReference(bossEntityId);
        if (pos->y < bossTargetY) return;

        bossShootTimer--;
        if (bossShootTimer <= 0)
        {
            fireBossPattern(pos->xy());
            bossShootTimer = 75;
        }
    }
    void fireBossPattern(const Vector2D& origin) {
        if (bossPatternIndex == 0)
        {
            const int n = 16;
            for (int i = 0; i < n; i++)
            {
                double a = (2.0 * M_PI * i) / n;
                addEnemyShot(origin, Vector2D(cos(a), sin(a)), 1.6);
            }
        }
        else if (bossPatternIndex == 1)
        {
            auto pp = getBlitzEntityPositionReference(playerEntityId);
            Vector2D toPlayer = vecNormalize(Vector2D(pp->x - origin.x, pp->y - origin.y));
            for (int i = -2; i <= 2; i++)
            {
                addEnemyShot(origin, vecRotateZ2D(toPlayer, i * 0.18), 2.2);
            }
        }
        else
        {
            for (int i = 0; i < 7; i++)
            {
                double a = (M_PI / 2.0) - 0.6 + i * 0.2;
                addEnemyShot(origin, Vector2D(cos(a), sin(a)), 1.8);
            }
        }
        bossPatternIndex = (bossPatternIndex + 1) % 3;
    }

    void playExplosion(const Vector2D& pos, double scale) {
        auto e = addMugenAnimation(getMugenAnimation(&mAnimations, 51), &mSprites, pos.xyz(45));
        setMugenAnimationNoLoop(e);
        setMugenAnimationBaseDrawScale(e, scale);
    }
    void startBossDeath() {
        removeAllShots();
        bossIsDying = true;
        bossExplosionsDone = 0;
        bossDeathTimer = 0;
    }
    void updateBossDeath() {
        if (!bossIsDying) return;
        if (bossIsDead) return;

        if (bossDeathTimer > 0)
        {
            bossDeathTimer--;
            return;
        }

        Vector2D bossPos = getBlitzEntityPosition(bossEntityId).xy();
        if (bossExplosionsDone < explosionCount)
        {
            Vector2D offset = Vector2D(randfrom(-24, 24), randfrom(-16, 16));
            playExplosion(bossPos + offset, 1.0);
            tryPlayMugenSoundAdvanced(&mSounds, 1, 6, sfxVol);
            bossExplosionsDone++;
            int interval = 16 - bossExplosionsDone * 2;
            if (interval < 3) interval = 3;
            bossDeathTimer = interval;
        }
        else
        {
            playExplosion(bossPos, 10.0);
            tryPlayMugenSoundAdvanced(&mSounds, 1, 6, jingleVol);
            setBlitzMugenAnimationVisibility(bossEntityId, 0);
            bossIsDead = true;
        }
    }
    bool isBossDefeated() { return bossHealth == 0; }

    // SHOT HANDLER
    struct Shot
    {
        int entityId;
        Vector2D direction;
        double speed;
        int collisionAttack;
        bool isToBeRemoved = false;
    };
    std::map<int, Shot> mShots;
    void loadShotHandler() {}
    void updateShotHandler() {
        updateShotHandlerActive();
    }
    void updateShotHandlerActive() {
        auto it = mShots.begin();
        while (it != mShots.end())
        {
            updateSingleShot(it->second);
            if (it->second.isToBeRemoved)
            {
                unloadGeneralShot(it->second);
                it = mShots.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    void removeAllShots()
    {
        auto it = mShots.begin();
        while (it != mShots.end())
        {
            unloadGeneralShot(it->second);
            it = mShots.erase(it);
        }
    }

    // GENERAL SHOTS
    void addGeneralShot(const Vector2D& pos, CollisionListData* collisionList, int animationNo, double z, double speed, const Vector2D& direction)
    {
        auto entityId = addBlitzEntity(pos.xyz(z));
        addBlitzMugenAnimationComponent(entityId, &mSprites, &mAnimations, animationNo);
        setBlitzEntityRotationZ(entityId, getAngleFromDirection(direction) + M_PI / 2);

        auto collisionAttack = addBlitzCollisionRect(entityId, collisionList, CollisionRect(-3, -3, 6, 6));
        mShots[entityId] = Shot{ entityId, direction, speed, collisionAttack };
    }
    void unloadGeneralShot(Shot& e)
    {
        removeBlitzEntity(e.entityId);
    }
    void updateSingleShot(Shot& e) {
        updateSingleShotMovement(e);
        updateSingleShotGetHit(e);
        updateSingleShotOver(e);
    }
    void updateSingleShotMovement(Shot& e) {
        auto pos = getBlitzEntityPositionReference(e.entityId);
        *pos += e.direction * e.speed;
    }
    void updateSingleShotGetHit(Shot& e) {
        if (hasBlitzCollidedThisFrame(e.entityId, e.collisionAttack))
        {
            e.isToBeRemoved = true;
        }
    }
    void updateSingleShotOver(Shot& e) {
        auto pos = getBlitzEntityPositionReference(e.entityId);
        if (pos->x < -10 || pos->x > 330 || pos->y < -10 || pos->y > 250)
        {
            e.isToBeRemoved = true;
        }
    }

    // PLAYER SHOTS
    void addPlayerShot() {
        auto pos = getBlitzEntityPositionReference(playerEntityId);
        addGeneralShot(pos->xy() + Vector2D(-3, 0), playerShotCollisionList, 20, 9, 6.0, Vector2D(0, -1));
        addGeneralShot(pos->xy() + Vector2D(3, 0), playerShotCollisionList, 20, 9, 6.0, Vector2D(0, -1));
    }

    // ENEMY SHOTS
    void addEnemyShot(const Vector2D& pos, const Vector2D& direction, double speed) {
        addGeneralShot(pos, enemyShotCollisionList, 42, 9, speed, direction);
    }

    void addEnemyShotAimed(const Vector2D& pos, double speed) {
        auto playerPos = getBlitzEntityPositionReference(playerEntityId);
        Vector2D direction = vecNormalize(Vector2D(playerPos->x - pos.x, playerPos->y - pos.y));
        addGeneralShot(pos, enemyShotCollisionList, 42, 9, speed, direction);
    }

    // SCORE UI
    MugenAnimationHandlerElement* uiBgElement;
    int scoreTextId;
    int scorePerTick = 13;
    void loadScoreUI() {
        uiBgElement = addMugenAnimation(getMugenAnimation(&mAnimations, 60), &mSprites, Vector3D(160, 240, 50));
        scoreTextId = addMugenTextMugenStyle("SCORE: 0", Vector3D(20, 238, 51), Vector3DI(1, 0, 1));
        setMugenAnimationVisibility(uiBgElement, false);
        setMugenTextVisibility(scoreTextId, false);
    }
    void updateScoreUI() {
        if (isPaused()) return;
        gGameScreenData.mScore += scorePerTick;
        std::string newScoreText = std::string("SCORE: ") + std::to_string(gGameScreenData.mScore);
        changeMugenText(scoreTextId, newScoreText.c_str());
    }

    void setUIVisibility(bool isVisible)
    {
        setMugenAnimationVisibility(uiBgElement, isVisible);
        setMugenTextVisibility(scoreTextId, isVisible);
    }

    // LIFE UI
    void loadLifeUI() {}
    void updateLifeUI() {}

    // GET READY SCREEN
    MugenAnimationHandlerElement* getReadyElement;
    bool hasShownGetReady = false;
    bool isShowingGetReady = false;
    int uiAnimationTicks = 0;
    void loadGetReady() {
        getReadyElement = addMugenAnimation(getMugenAnimation(&mAnimations, 64), &mSprites, Vector3D(0, 0, 50));
        setMugenAnimationVisibility(getReadyElement, false);
    }
    void updateGetReady() {
        updateGetReadyStart();
        updateGetReadyOver();
    }
    void updateGetReadyStart() {
        if (isShowingTutorial) return;
        if (hasShownGetReady) return;

        setMugenAnimationVisibility(getReadyElement, 1);
        tryPlayMugenSoundAdvanced(&mSounds, 100, 0, jingleVol);

        uiAnimationTicks = 0;
        hasShownGetReady = true;
        isShowingGetReady = true;
    }
    void updateGetReadyOver() {
        if (!isShowingGetReady) return;

        uiAnimationTicks++;
        if (uiAnimationTicks > 150 || (uiAnimationTicks > 1 && hasPressedStartFlank()))
        {
            stopAllSoundEffects();
            setMugenAnimationVisibility(getReadyElement, 0);
            streamMusicFile("game/GAME.ogg");
            isShowingGetReady = false;
            setUIVisibility(true);
        }
    }

    // VICTORY SCREEN
    MugenAnimationHandlerElement* victoryElement;
    bool hasShownVictory = false;
    bool isShowingVictory = false;
    void loadVictory() {
        int animationNo = 66;
        victoryElement = addMugenAnimation(getMugenAnimation(&mAnimations, animationNo), &mSprites, Vector3D(0, 0, 50));
        setMugenAnimationVisibility(victoryElement, false);
    }
    void updateVictory() {
        updateVictoryStart();
        updateVictoryOver();
    }
    void updateVictoryStart() {
        if (hasShownVictory) return;
        if (!bossIsDead) return;

        setUIVisibility(false);
        tryPlayMugenSoundAdvanced(&mSounds, 100, 1, jingleVol);
        setMugenAnimationVisibility(victoryElement, 1);
        pauseMusic();
        removeAllShots();
        removeAllEnemies();
        setPlayerIdle();
        uiAnimationTicks = 0;
        hasShownVictory = true;
        isShowingVictory = true;
    }

    void screenWon() {
        setCurrentStoryDefinitionFile("game/OUTRO.def", 0);
        stopStreamingMusicFile();
        setNewScreen(getStoryScreen());
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

    // LOSS SCREEN
    MugenAnimationHandlerElement* lossElement;
    bool hasShownLoss = false;
    bool isShowingLoss = false;
    void loadLoss() {
        lossElement = addMugenAnimation(getMugenAnimation(&mAnimations, 65), &mSprites, Vector3D(0, 0, 50));
        setMugenAnimationVisibility(lossElement, false);
    }
    void updateLoss() {
        updateLossStart();
        updateLossOver();
    }
    void updateLossStart() {
        if (hasShownLoss) return;
        if (!playerIsDying) return;

        setUIVisibility(false);
        setMugenAnimationVisibility(lossElement, 1);
        pauseMusic();
        removeAllShots();
        removeAllEnemies();
        tryPlayMugenSoundAdvanced(&mSounds, 100, 2, jingleVol);
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

    // TUTORIAL
    MugenAnimationHandlerElement* tutorialElement;
    bool isShowingTutorial = false;
    void loadTutorial() {
        tutorialElement = addMugenAnimation(getMugenAnimation(&mAnimations, 67), &mSprites, Vector3D(0, 0, 50));
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
};

EXPORT_SCREEN_CLASS(GameScreen);

void resetGame()
{
    gGameScreenData.mLevel = 0;
    gGameScreenData.mGameTicks = 0;
    gGameScreenData.mScore = 0;
}

std::string getSpeedRunString() {
    int totalSeconds = gGameScreenData.mGameTicks / 60;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    int milliseconds = (gGameScreenData.mGameTicks % 60) * 1000 / 60;
    return std::to_string(minutes) + "m " + std::to_string(seconds) + "s " + std::to_string(milliseconds) + "ms. Your hyper score is " + std::to_string(gGameScreenData.mScore) + ".";
}
