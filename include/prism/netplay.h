#pragma once

#include <string>
#include <prism/file.h>

namespace prism {

void initNetplay();
void shutdownNetplay();

void startNetplayHosting();
void stopNetplayHosting();
bool joinNetplayHost(const std::string& tIP, int tPort);
bool isNetplayHost();
bool isNetplayActive();
bool isNetplayConnecting();
bool isNetplaySyncing();
void shutdownScreenNetplay();

void updateNetplay();

void sendNetplayData(const Buffer& tData);
void setNetplayConnectCB(void(*tCB)(void*), void* tCaller);
void setNetplaySyncCBs(Buffer(*tGatherCB)(void*), void* tGatherCaller, int(*tCheckCB)(void*, const Buffer&, const Buffer&), void* tCheckCaller);
void setNetplayDesyncCB(void(*tCB)(void*), void* tCaller);
void setNetplayDisconnectCB(void(*tCB)(void*, const std::string&), void* tCaller);
void renegotiateNetplayConnection();

int getNetplaySyncFrame();
int getNetplayLastReceivedFrame();

}