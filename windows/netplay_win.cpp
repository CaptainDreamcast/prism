#include "prism/netplay.h"

#include <enet/enet.h>

#include <assert.h>
#include <queue>
#include <thread>
#include <chrono>

#include <prism/thread.h>
#include <prism/input.h>
#include <prism/system.h>
#include <prism/math.h>
#include <prism/log.h>

#define NETPLAY_VERSION 2

static struct {
    FileHandler mLogFile;
    char mText[2048];
} gNetplayLogData;

static void netplayHardwareLogToFile(FileHandler& tFileHandler, const char* tText) {

    if (tFileHandler == FILEHND_INVALID) {
        createDirectory("$pc/debug");
        tFileHandler = fileOpen("$pc/debug/netplay_log.txt", O_WRONLY);
    }
    if (tFileHandler == FILEHND_INVALID) return;

    fileWrite(tFileHandler, tText, strlen(tText));
}

static void flushNetplayLog() {
    if (gNetplayLogData.mLogFile == FILEHND_INVALID) return;

    fileFlush(gNetplayLogData.mLogFile);
}

static void netplayLogprintf(const char* tFormatString, ...) {
    char* logEntry = gNetplayLogData.mText;
    char* writePoint = gNetplayLogData.mText + strlen(logEntry);
    va_list args;
    va_start(args, tFormatString);
    vsprintf(writePoint, tFormatString, args);
    va_end(args);
}

static void netplayLogCommit() {
    netplayHardwareLogToFile(gNetplayLogData.mLogFile, gNetplayLogData.mText);
    gNetplayLogData.mText[0] = '\0';
}

static void netplayLogFormatFunc(const char* tFormatString, ...) {
    char text[2048];
    va_list args;
    va_start(args, tFormatString);
    vsprintf(text, tFormatString, args);
    va_end(args);

    netplayLogprintf("%s\n", text);
}

#define netplayLogBegin() {netplayLogprintf("[%s::%s, line %d] ", __FILE__, __FUNCTION__, __LINE__);}	
#define netplayLogFormat(x, ...) {netplayLogBegin(); netplayLogFormatFunc(x,  __VA_ARGS__); netplayLogCommit();}
#define netplayLog(x)	{logBegin(); netplayLogprintf(x); netplayLogprintf("\n"); netplayLogCommit( );}

struct NetplaySingleFrameInput {
    uint8_t mKeys[SDL_NUM_SCANCODES];
    uint8_t mButtons[CONTROLLER_BUTTON_AMOUNT_PRISM];
};

#define NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE 5
struct StandardNetplayPackage {
    uint32_t mMagic;
    uint32_t mVersion;
    uint32_t mNegotiationIndex;
    uint64_t mUnixTimestamp;
    int64_t mSyncedFrameIndex;
    uint64_t mInputDelay;
    int64_t mConnectSyncFrame;

    NetplaySingleFrameInput mFrameInput[NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE];
};

typedef struct {
    int mNecessaryFrameDelay;

    bool mHasEstablishedInputDelay;
    int mReceivedFrameDelayEstablishmentFrames;

} FrameDelayInfo;

static struct 
{
    ENetHost* mServer;
    bool mIsHost;
    std::vector<ENetPeer*> mPeers;

    void(*mConnectCB)(void*);
    void* mConnectCBCaller;
    
    Buffer(*mSyncGatherCB)(void*);
    void* mSyncGatherCBCaller;
    int(*mSyncCheckCB)(void*, const Buffer&, const Buffer&);
    void* mSyncCheckCBCaller;

    void(*mDesyncCB)(void*);
    void* mDesyncCBCaller;
    void(*mDisconnectCB)(void*, const std::string&);
    void* mDisconnectCBCaller;

    std::deque<Buffer> mPreviousFrameSyncData;

    uint64_t mLastReceivedTimeStamp;
    int mSyncedFrameIndex;
    int64_t mLastReceivedFrameIndex;
    int mInputDelay;
    int mConnectSyncFrame;
    uint32_t mNegotiationIndex;

    FrameDelayInfo mFrameDelayInfo;

    bool mIsEnetInitialized;
} gNetplayData;

void initNetplay()
{
    if (!gNetplayData.mIsEnetInitialized)
    {
        if (enet_initialize() != 0)
        {
            logError("[Netplay] Unabled to load enet");
        }
        atexit(enet_deinitialize);
        gNetplayData.mIsEnetInitialized = true;
    }

    gNetplayData.mConnectCB = NULL;
    gNetplayData.mIsHost = false;
    gNetplayData.mSyncedFrameIndex = 0;
}

static void destroyClient();

static void resetNetplay() {
    if (gNetplayData.mServer) {
        destroyClient();
    }
    gNetplayData.mIsHost = false;
    gNetplayData.mPeers.clear();

    gNetplayData.mConnectCB = NULL;
    gNetplayData.mSyncGatherCB = NULL;
    gNetplayData.mSyncCheckCB = NULL;
    gNetplayData.mDesyncCB = NULL;
    gNetplayData.mDisconnectCB = NULL;

    gNetplayData.mPreviousFrameSyncData.clear();

    gNetplayData.mSyncedFrameIndex = 0;
    gNetplayData.mInputDelay = 0;
    gNetplayData.mConnectSyncFrame = 0;
    gNetplayData.mNegotiationIndex = 0;

    gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay = 0;
    gNetplayData.mFrameDelayInfo.mHasEstablishedInputDelay = 0;
    gNetplayData.mFrameDelayInfo.mReceivedFrameDelayEstablishmentFrames = 0;
}

void shutdownNetplay()
{
    resetNetplay();
}

static void createHostServer()
{
    ENetAddress address;
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    gNetplayData.mServer = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);
    if (gNetplayData.mServer == NULL)
    {
        logError("[Netplay] An error occurred while trying to create an ENet server host.");
    }

    gNetplayData.mFrameDelayInfo.mHasEstablishedInputDelay = false;
    gNetplayData.mFrameDelayInfo.mReceivedFrameDelayEstablishmentFrames = 0;
    gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay = 0;
    gNetplayData.mSyncedFrameIndex = 0;
    gNetplayData.mConnectSyncFrame = 0;
    gNetplayData.mNegotiationIndex = 0;
    gNetplayData.mInputDelay = 0;
    gNetplayData.mLastReceivedTimeStamp = 0;
    gNetplayData.mIsHost = true;
    setInputUsedKeyboardByPlayer(1, 1);
    setInputUsedKeyboardMappingByPlayer(1, 0);
    setInputBufferSize(NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE + 2);
    gNetplayData.mPeers.clear();
}

void startNetplayHosting()
{
    createHostServer();
}

static void destroyHost()
{
    enet_host_destroy(gNetplayData.mServer);
    gNetplayData.mServer = NULL;
}

void stopNetplayHosting()
{
    destroyHost();
}

bool isNetplayHost() {
    return gNetplayData.mServer && gNetplayData.mIsHost;
}

bool isNetplayActive()
{
    return gNetplayData.mServer;
}

bool isNetplayConnecting() {
    return isNetplayActive() && !gNetplayData.mPeers.empty() && !gNetplayData.mConnectSyncFrame;
}

static void createClient()
{
    gNetplayData.mServer = enet_host_create(NULL /* client host */,
        1      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);
    if (gNetplayData.mServer == NULL)
    {
        logError("[Netplay] An error occurred while trying to create an ENet client host.");
    }

    gNetplayData.mPeers.clear();
}

static void destroyClient()
{
    enet_host_destroy(gNetplayData.mServer);
    gNetplayData.mServer = NULL;
}

static bool connectToHost(const std::string& tIP, int tPort)
{
    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;
    enet_address_set_host(&address, tIP.c_str());
    address.port = (enet_uint16)tPort;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(gNetplayData.mServer, &address, 2, 0);
    if (peer == NULL)
    {
        logError("[Netplay] No available peers for initiating an ENet connection.");
        return false;
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(gNetplayData.mServer, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        logFormat("Connection to host succeeded.");
        gNetplayData.mPeers.push_back(peer);
        return true;
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
        logFormat("[Netplay] Connection to host failed.");
        return false;
    }
}

bool joinNetplayHost(const std::string& tIP, int tPort)
{
    createClient();
    if (!connectToHost(tIP, tPort))
    {
        destroyClient();
        return false;
    }

    gNetplayData.mFrameDelayInfo.mHasEstablishedInputDelay = false;
    gNetplayData.mFrameDelayInfo.mReceivedFrameDelayEstablishmentFrames = 0;
    gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay = 0;
    gNetplayData.mSyncedFrameIndex = -1;
    gNetplayData.mConnectSyncFrame = 0;
    gNetplayData.mNegotiationIndex = 0;
    gNetplayData.mInputDelay = 0;
    gNetplayData.mLastReceivedTimeStamp = 0;
    gNetplayData.mIsHost = false;
    setInputUsedKeyboardByPlayer(1, 1);
    setInputUsedKeyboardMappingByPlayer(1, 0);
    setInputBufferSize(NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE + 2);
    return true;
}

extern void setInputForFrameDelta(int i, int tNegativeFrameDelta, const std::vector<std::vector<uint8_t>>& tKeyStates, const std::vector<std::vector<uint8_t>>& tButtonStates);

static void receivePeerInput(const StandardNetplayPackage* package) {
    std::vector<std::vector<uint8_t>> keyStates = std::vector<std::vector<uint8_t>>(NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE, std::vector<uint8_t>(SDL_NUM_SCANCODES));
    std::vector<std::vector<uint8_t>> buttonStates = std::vector<std::vector<uint8_t>>(NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE, std::vector<uint8_t>(CONTROLLER_BUTTON_AMOUNT_PRISM));
    for (size_t i = 0; i < NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE; i++)
    {
        memcpy(keyStates[i].data(), package->mFrameInput[i].mKeys, SDL_NUM_SCANCODES);
        memcpy(buttonStates[i].data(), package->mFrameInput[i].mButtons, CONTROLLER_BUTTON_AMOUNT_PRISM);
    }

    const int frameDelta = int(gNetplayData.mSyncedFrameIndex) - int(package->mSyncedFrameIndex);
    netplayLogFormat("[Netplay] Set input on frame %d for frames starting between %d and %d", gNetplayData.mSyncedFrameIndex, gNetplayData.mSyncedFrameIndex - frameDelta - NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE + 1, gNetplayData.mSyncedFrameIndex - frameDelta);
    setInputForFrameDelta(1, -frameDelta, keyStates, buttonStates);
}

static void establishFrameDelayWithPeer(const StandardNetplayPackage* package)
{
    static constexpr auto INPUT_DELAY_SAFETY_BUFFER = 10;

    if (gNetplayData.mIsHost) {
        if (!gNetplayData.mInputDelay && package->mUnixTimestamp)
        {
            const auto nowMs = int64_t(getUnixTimestampMilliseconds());
            const auto timeDelayMs = (nowMs -package->mUnixTimestamp) / 2;

            const auto frameTimeMs = (1.0 / double(getFramerate())) * 1000;
            auto frameDelay = int((timeDelayMs / frameTimeMs)) + 1;

            if (frameDelay <= 20)
            {
                netplayLogFormat("[Netplay] adding frame delay for consideration %d", frameDelay);
                gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay = max(gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay, frameDelay);
                gNetplayData.mFrameDelayInfo.mReceivedFrameDelayEstablishmentFrames++;
            }
            else
            {
                netplayLogFormat("[Netplay] discarding potential frame delay %d, substitute with max", frameDelay);
                gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay = max(gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay, 20);
                gNetplayData.mFrameDelayInfo.mReceivedFrameDelayEstablishmentFrames++;
            }

            if (gNetplayData.mFrameDelayInfo.mReceivedFrameDelayEstablishmentFrames >= 10)
            {
                gNetplayData.mInputDelay = gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay + INPUT_DELAY_SAFETY_BUFFER;
                setInputDelay(gNetplayData.mInputDelay);
                netplayLogFormat("[Netplay] Peer input delay set to %d", gNetplayData.mInputDelay);
                gNetplayData.mFrameDelayInfo.mHasEstablishedInputDelay = true;
                gNetplayData.mConnectSyncFrame = gNetplayData.mSyncedFrameIndex + gNetplayData.mInputDelay * 2;
                netplayLogFormat("[Netplay] Connect sync frame set to %d", gNetplayData.mConnectSyncFrame);
            }
        }
    }
    else {
        if (!gNetplayData.mInputDelay) 
        {
            if (!package->mInputDelay)
            {
                gNetplayData.mLastReceivedTimeStamp = package->mUnixTimestamp;
                netplayLogFormat("[Netplay] Received ping package with timestamp %llu", package->mUnixTimestamp);
            }
            else
            {
                gNetplayData.mInputDelay = int(package->mInputDelay);
                setInputDelay(gNetplayData.mInputDelay);
                netplayLogFormat("[Netplay] Input delay set to %d", gNetplayData.mInputDelay);
                gNetplayData.mFrameDelayInfo.mHasEstablishedInputDelay = true;
                gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay = gNetplayData.mInputDelay - INPUT_DELAY_SAFETY_BUFFER;
                gNetplayData.mSyncedFrameIndex = int(package->mSyncedFrameIndex) + gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay;
                netplayLogFormat("[Netplay] Synched frame established at %d", gNetplayData.mSyncedFrameIndex);
                gNetplayData.mConnectSyncFrame = int(package->mConnectSyncFrame);
                netplayLogFormat("[Netplay] Connect sync frame set to %d", gNetplayData.mConnectSyncFrame);
            }
        }
    }
}

static void handlePeerSyncCheck(StandardNetplayPackage* package, const Buffer& b) {
    if (!b.mLength || !gNetplayData.mSyncCheckCB) return;

    const int frameDelta = int(gNetplayData.mSyncedFrameIndex) - int(package->mSyncedFrameIndex);
    if (frameDelta < 0 || frameDelta >= gNetplayData.mPreviousFrameSyncData.size()) return;
    const auto& correspondingFrameData = *(gNetplayData.mPreviousFrameSyncData.rbegin() + frameDelta);
    const auto isSame = gNetplayData.mSyncCheckCB(gNetplayData.mSyncCheckCB, correspondingFrameData, b);
    if (!isSame)
    {
        logWarning("[Netplay] desync detected!");
        netplayLog("[Netplay] desync detected");
        if (gNetplayData.mDesyncCB) {
            gNetplayData.mDesyncCB(gNetplayData.mDesyncCBCaller);
        }
    }
}

static void updateNetplayEvents()
{
    ENetEvent event;
    while (gNetplayData.mServer && enet_host_service(gNetplayData.mServer, &event, 0) > 0)
    {
        StandardNetplayPackage* package = nullptr;
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            logFormat("A new client connected from %x:%u.\n",
                event.peer->address.host,
                event.peer->address.port);
            /* Store any relevant client information here. */
            event.peer->data = "Client information";
            gNetplayData.mPeers.push_back(event.peer);
            if (isNetplayHost()) {
                gNetplayData.mSyncedFrameIndex = 0;
            }
            logFormat("Connection received from peer");
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            assert(event.packet->dataLength >= sizeof(StandardNetplayPackage));
            package = (StandardNetplayPackage*)event.packet->data;
            
            if (package->mVersion != NETPLAY_VERSION) {
                gNetplayData.mDisconnectCB(gNetplayData.mDisconnectCBCaller, "version");
                break;
            }
            
            gNetplayData.mLastReceivedFrameIndex = package->mSyncedFrameIndex;
            if (package->mNegotiationIndex == gNetplayData.mNegotiationIndex) {
                if (!gNetplayData.mConnectSyncFrame)
                {
                    establishFrameDelayWithPeer(package);
                }

                if (gNetplayData.mFrameDelayInfo.mHasEstablishedInputDelay)
                {
                    receivePeerInput(package);
                    handlePeerSyncCheck(package, makeBuffer(event.packet->data + sizeof(StandardNetplayPackage) + sizeof(int32_t), *((int32_t*)(event.packet->data + sizeof(StandardNetplayPackage)))));
                }
            }

            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            logFormat ("%s disconnected.\n", event.peer->data);
            /* Reset the peer's client information. */
            event.peer->data = NULL;
            if (gNetplayData.mDisconnectCB) {
                gNetplayData.mDisconnectCB(gNetplayData.mDisconnectCBCaller, "");
            }
        }
    }
}

extern void gatherWindowsInputStateForPastFrames(int i, size_t pastFrames, std::vector<std::vector<uint8_t>>& tKeyStates, std::vector<std::vector<uint8_t>>& tButtonStates);

static void updateNetplaySendingFrame() {
    StandardNetplayPackage netplayPackage;
    netplayPackage.mVersion = NETPLAY_VERSION;
    netplayPackage.mMagic = 503;
    netplayPackage.mUnixTimestamp = gNetplayData.mIsHost ? getUnixTimestampMilliseconds() : gNetplayData.mLastReceivedTimeStamp;
    netplayPackage.mSyncedFrameIndex = gNetplayData.mSyncedFrameIndex;
    netplayPackage.mConnectSyncFrame = gNetplayData.mConnectSyncFrame;
    netplayPackage.mNegotiationIndex = gNetplayData.mNegotiationIndex;
    netplayPackage.mInputDelay = gNetplayData.mInputDelay;

    std::vector<std::vector<uint8_t>> keyStates;
    std::vector<std::vector<uint8_t>> buttonStates;
    gatherWindowsInputStateForPastFrames(0, NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE, keyStates, buttonStates);

    assert(keyStates.size() == NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE);
    assert(buttonStates.size() == NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE);

    for (size_t i = 0; i < keyStates.size(); i++)
    {
        memcpy(netplayPackage.mFrameInput[i].mKeys, keyStates[i].data(), keyStates[i].size());
        memcpy(netplayPackage.mFrameInput[i].mButtons, buttonStates[i].data(), buttonStates[i].size());
    }
    Buffer packageBuffer = makeBufferEmptyOwned();
    appendBufferBuffer(&packageBuffer, makeBuffer(&netplayPackage, sizeof(StandardNetplayPackage)));

    if (gNetplayData.mSyncGatherCB)
    {
        Buffer newSyncData = gNetplayData.mSyncGatherCB(gNetplayData.mSyncGatherCBCaller);
        appendBufferInt32(&packageBuffer, newSyncData.mLength);
        appendBufferBuffer(&packageBuffer, newSyncData);
        gNetplayData.mPreviousFrameSyncData.push_back(newSyncData);
        if (gNetplayData.mPreviousFrameSyncData.size() > NETPLAY_SENT_FRAME_COUNT_PER_PACKAGE) 
        {
            auto& buffer = gNetplayData.mPreviousFrameSyncData.front();
            freeBuffer(buffer);
            gNetplayData.mPreviousFrameSyncData.pop_front();
        }
    }
    else
    {
        appendBufferInt32(&packageBuffer, 0);
    }

    sendNetplayData(packageBuffer);
    freeBuffer(packageBuffer);
}

static void updateNetplayStart()
{
    if (gNetplayData.mConnectSyncFrame && gNetplayData.mSyncedFrameIndex == gNetplayData.mConnectSyncFrame)
    {
        setRandomSeed(gNetplayData.mConnectSyncFrame);
        if (gNetplayData.mConnectCB) {
            gNetplayData.mConnectCB(gNetplayData.mConnectCBCaller);
        }
    }
}

extern void gatherWindowsInputStateForLogging(std::vector<uint8_t>& tConfirmationStates, std::vector<std::vector<uint8_t>>& tKeyStates, std::vector<std::vector<uint8_t>>& tButtonStates);

static void updateLoggedInput() {
    if (!gNetplayData.mConnectSyncFrame || (gNetplayData.mSyncedFrameIndex < gNetplayData.mConnectSyncFrame)) return;

    std::vector<uint8_t> confirmationStates; 
    std::vector<std::vector<uint8_t>> keyStates;
    std::vector<std::vector<uint8_t>> buttonStates;
    gatherWindowsInputStateForLogging(confirmationStates, keyStates, buttonStates);

    for (int i = 0; i < 2; i++) {
        netplayLogFormat("[Netplay] Frame %d start input logging for player %d ", gNetplayData.mSyncedFrameIndex, i + 1);
        netplayLogFormat("[Netplay] Frame %d Player %d confirmation state: %d", gNetplayData.mSyncedFrameIndex, i + 1, confirmationStates[i]);
        
        std::string keyStateString = "";
        for (int j = 0; j < keyStates[i].size(); j++)
        {
            keyStateString += std::to_string(keyStates[i][j]);
            keyStateString += " ";
        }
        netplayLogFormat("[Netplay] Frame %d Player %d key states: %s", gNetplayData.mSyncedFrameIndex, i + 1, keyStateString.c_str());

        std::string buttonInputString = "";
        for (int j = 0; j < buttonStates[i].size(); j++)
        {
            buttonInputString += std::to_string(buttonStates[i][j]);
            buttonInputString += " ";
        }
        netplayLogFormat("[Netplay] Frame %d Player %d button states: %s", gNetplayData.mSyncedFrameIndex, i + 1, buttonInputString.c_str());
    }

    flushNetplayLog();
}

static int checkConfirmedInputAndWaitIfNecessary() {
    if (!gNetplayData.mConnectSyncFrame || (gNetplayData.mSyncedFrameIndex < gNetplayData.mConnectSyncFrame)) return 1;

    int ret = isNetplayInputConfirmed();

    if (!ret)
    {
        netplayLogFormat("[Netplay] Using unconfirmed input for frame %d.", gNetplayData.mSyncedFrameIndex - getInputDelay());
    }
    return ret;
}

void updateNetplay() {
    if (!gNetplayData.mServer) return;
    updateNetplaySendingFrame();  
    updateNetplayEvents();
    if (!gNetplayData.mServer) return;

    updateLoggedInput();
    checkConfirmedInputAndWaitIfNecessary();
    updateNetplayStart();
    if (gNetplayData.mSyncedFrameIndex >= 0)
    {
        gNetplayData.mSyncedFrameIndex++;
    }
}

void sendNetplayData(const Buffer& tData) {
    ENetPacket* packet = enet_packet_create(tData.mData,
       tData.mLength,
        ENET_PACKET_FLAG_RELIABLE);

    for (auto peer : gNetplayData.mPeers)
    {
        enet_peer_send(peer, 0, packet);
    }

    enet_host_flush(gNetplayData.mServer);
}

void setNetplayConnectCB(void(*tCB)(void*), void* tCaller)
{
    gNetplayData.mConnectCB = tCB;
    gNetplayData.mConnectCBCaller = tCaller;
}

void setNetplaySyncCBs(Buffer(*tGatherCB)(void*), void* tGatherCaller, int(*tCheckCB)(void*, const Buffer&, const Buffer&), void* tCheckCaller)
{
    gNetplayData.mSyncGatherCB = tGatherCB;
    gNetplayData.mSyncGatherCBCaller = tGatherCaller;
    gNetplayData.mSyncCheckCB = tCheckCB;
    gNetplayData.mSyncCheckCBCaller = tCheckCaller;
}

void setNetplayDesyncCB(void(*tCB)(void*), void* tCaller) {
    gNetplayData.mDesyncCB = tCB;
    gNetplayData.mDesyncCBCaller = tCaller;
}

void setNetplayDisconnectCB(void(*tCB)(void*, const std::string&), void* tCaller) {
    gNetplayData.mDisconnectCB = tCB;
    gNetplayData.mDisconnectCBCaller = tCaller;
}

void renegotiateNetplayConnection() {
    updateNetplayEvents(); // flush packages so far

    gNetplayData.mConnectCB = NULL;
    gNetplayData.mConnectCBCaller = NULL;

    gNetplayData.mFrameDelayInfo.mHasEstablishedInputDelay = false;
    if (!isNetplayHost())
    {
        gNetplayData.mSyncedFrameIndex = -1;
    }
    else
    {
        gNetplayData.mSyncedFrameIndex = 0;
    }
    gNetplayData.mFrameDelayInfo.mNecessaryFrameDelay = 0;
    gNetplayData.mFrameDelayInfo.mReceivedFrameDelayEstablishmentFrames = 0;
    gNetplayData.mLastReceivedTimeStamp = 0;
    gNetplayData.mConnectSyncFrame = 0;
    gNetplayData.mNegotiationIndex++;
    gNetplayData.mInputDelay = 0;
}

int getNetplaySyncFrame() {
    return gNetplayData.mSyncedFrameIndex;
}
int getNetplayLastReceivedFrame()
{
    return int(gNetplayData.mLastReceivedFrameIndex);
}