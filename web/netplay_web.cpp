#include "prism/netplay.h"

namespace prism {

	void initNetplay() {}
	void shutdownNetplay() {}

	void startNetplayHosting() {}
	void stopNetplayHosting() {}
	bool joinNetplayHost(const std::string&, int) { return false; }
	bool isNetplayHost() { return false; }
	bool isNetplayActive() { return false; }
	bool isNetplayConnecting() { return false; }
	bool isNetplaySyncing() { return false; }
	void shutdownScreenNetplay() {}

	void updateNetplay() {}

	void sendNetplayData(const Buffer& tData) {}
	void setNetplayConnectCB(void(*tCB)(void*), void*) {}
	void setNetplaySyncCBs(Buffer(*tGatherCB)(void*), void*, int(*tCheckCB)(void*, const Buffer&, const Buffer&), void*) {}
	void setNetplayDesyncCB(void(*tCB)(void*), void* tCaller) {}
	void setNetplayDisconnectCB(void(*tCB)(void*, const std::string&), void*) {}
	void renegotiateNetplayConnection() {}

}