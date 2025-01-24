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

	void sendNetplayData(const Buffer&) {}
	void setNetplayConnectCB(void(*)(void*), void*) {}
	void setNetplaySyncCBs(Buffer(*)(void*), void*, int(*)(void*, const Buffer&, const Buffer&), void*) {}
	void setNetplayDesyncCB(void(*)(void*), void*) {}
	void setNetplayDisconnectCB(void(*)(void*, const std::string&), void*) {}
	void renegotiateNetplayConnection() {}

}