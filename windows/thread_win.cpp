#include "prism/thread.h"

#include <Windows.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

namespace prism {

	typedef struct {
		int mID;
		DWORD mThreadID;
		HANDLE mThreadHandle;

		void(*mFunc)(void*);
		void* mCaller;
	} ThreadData;

	static struct {
		IntMap mThreads;
		Semaphore mThreadMapAccessSemaphore;
	} gPrismWindowsThreadData;

#ifdef _WIN32
	static void imguiThreadEntry(void* tCaller, void* tData)
	{
		(void)tCaller;
		ThreadData* e = (ThreadData*)tData;
		ImGui::TableNextRow(); ImGui::TableNextColumn();
		ImGui::Text("%d", e->mID); ImGui::TableNextColumn();
		ImGui::Text("%lu", (unsigned long)e->mThreadID);
	}

	void imguiThread()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "Threading", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Threading", &isWindowShown);
		lockSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
		ImGui::Text("Active Threads = %d", int_map_size(&gPrismWindowsThreadData.mThreads));
		ImGui::Separator();
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Threads", 2, flags))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("OS Thread ID");
			ImGui::TableHeadersRow();
			int_map_map(&gPrismWindowsThreadData.mThreads, imguiThreadEntry, NULL);
			ImGui::EndTable();
		}
		releaseSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
		ImGui::End();
	}
#endif

	void initThreading() {
		gPrismWindowsThreadData.mThreadMapAccessSemaphore = createSemaphore(1);
		gPrismWindowsThreadData.mThreads = new_int_map();
	}

	static int forceShutdownSingleThread(void* tCaller, void* tData) {
		(void)tCaller;
		ThreadData* e = (ThreadData*)tData;

		TerminateThread(e->mThreadHandle, 0);

		return 1;
	}

	void shutdownThreading()
	{
		lockSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
		int_map_remove_predicate(&gPrismWindowsThreadData.mThreads, forceShutdownSingleThread, NULL);
		delete_int_map(&gPrismWindowsThreadData.mThreads);
		releaseSemaphore(&gPrismWindowsThreadData.mThreadMapAccessSemaphore);
	}

	DWORD WINAPI threadFunction(LPVOID lpParam) {
		ThreadData* e = (ThreadData*)lpParam;

		e->mFunc(e->mCaller);

		CloseHandle(e->mThreadHandle);
		lockSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
		int_map_remove(&gPrismWindowsThreadData.mThreads, e->mID);
		releaseSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);

		return 0;
	}

	int startThread(void(tFunc)(void*), void* tCaller)
	{
		ThreadData* e = (ThreadData*)allocMemory(sizeof(ThreadData));
		e->mFunc = tFunc;
		e->mCaller = tCaller;

		lockSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
		e->mID = int_map_push_back_owned(&gPrismWindowsThreadData.mThreads, e);
		releaseSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);

		e->mThreadHandle = CreateThread(NULL, 0, threadFunction, e, 0, &e->mThreadID);
		return e->mID;
	}

	Semaphore createSemaphore(int tInitialAccessesAllowed)
	{
		HANDLE ret = CreateSemaphore(NULL, tInitialAccessesAllowed, 1, NULL);
		return ret;
	}

	void destroySemaphore(Semaphore tSemaphore)
	{
		HANDLE sem = tSemaphore;
		CloseHandle(sem);
	}

	void lockSemaphore(Semaphore tSemaphore)
	{
		HANDLE sem = tSemaphore;
		WaitForSingleObject(sem, INFINITE);
	}

	void releaseSemaphore(Semaphore tSemaphore)
	{
		HANDLE sem = tSemaphore;
		ReleaseSemaphore(sem, 1, NULL);
	}

	void terminateSelfAsThread(int /*tReturnValue*/) {
		std::terminate();
	}
}