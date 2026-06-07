#include "prism/texturepool.h"

#include "prism/datastructures.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

namespace prism {

	typedef struct {
		TextureData mTexture;
		char mPath[1024];
		int mCounter;
	} TexturePoolEntry;

	static struct {
		StringMap mPathToLoadedTexture;
		StringMap mTextureHashToLoadedTexture;

		int mIsLoaded;
	} gTexturePool;

#ifdef _WIN32
	static void imguiTexturePoolEntry(void* tCaller, char* tKey, void* tData)
	{
		(void)tCaller;
		TexturePoolEntry* e = (TexturePoolEntry*)tData;
		ImGui::TableNextRow(); ImGui::TableNextColumn();
		ImGui::Text("%s", tKey); ImGui::TableNextColumn();
		ImGui::Text("%s", e->mPath); ImGui::TableNextColumn();
		ImGui::Text("%d", e->mCounter); ImGui::TableNextColumn();
		ImGui::Text("%d", getTextureHash(e->mTexture));
	}

	void imguiTexturePool()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "Texture Pool", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Texture Pool", &isWindowShown);
		ImGui::Text("IsLoaded = %d", gTexturePool.mIsLoaded);
		ImGui::Text("Loaded Textures = %d", string_map_size(&gTexturePool.mPathToLoadedTexture));
		ImGui::Separator();

		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Textures", 4, flags))
		{
			ImGui::TableSetupColumn("Key");
			ImGui::TableSetupColumn("Path");
			ImGui::TableSetupColumn("RefCount");
			ImGui::TableSetupColumn("Texture Hash");
			ImGui::TableHeadersRow();
			string_map_map(&gTexturePool.mPathToLoadedTexture, imguiTexturePoolEntry, NULL);
			ImGui::EndTable();
		}
		ImGui::End();
	}
#endif

	void setupTexturePool() {
		if (gTexturePool.mIsLoaded) {
			logWarning("Attempting to use active texture pool. Resetting.");
			shutdownTexturePool();
		}

		gTexturePool.mPathToLoadedTexture = new_string_map();
		gTexturePool.mTextureHashToLoadedTexture = new_string_map();

		gTexturePool.mIsLoaded = 1;
	}

	static void cleanSingleTexturePoolEntry(void* tCaller, char* tKey, void* tData) {
		(void)tCaller;
		(void)tKey;
		TexturePoolEntry* e = (TexturePoolEntry*)tData;
		unloadTexture(e->mTexture);
	}

	void shutdownTexturePool() {
		string_map_map(&gTexturePool.mTextureHashToLoadedTexture, cleanSingleTexturePoolEntry, NULL);
		delete_string_map(&gTexturePool.mTextureHashToLoadedTexture);
		delete_string_map(&gTexturePool.mPathToLoadedTexture);
		gTexturePool.mIsLoaded = 0;
	}

	static TextureData increaseCounterAndFetchTexture(const char* tPath) {
		TexturePoolEntry* e = (TexturePoolEntry*)string_map_get(&gTexturePool.mPathToLoadedTexture, tPath);
		e->mCounter++;

		return e->mTexture;
	}

	TextureData loadTextureFromPool(const char* tPath) {
		int isLoadNotNecessary = string_map_contains(&gTexturePool.mPathToLoadedTexture, tPath);

		if (isLoadNotNecessary) {
			return increaseCounterAndFetchTexture(tPath);
		}

		TexturePoolEntry* e = (TexturePoolEntry*)allocMemory(sizeof(TexturePoolEntry));
		e->mCounter = 1;
		e->mTexture = loadTexture(tPath);
		strcpy(e->mPath, tPath);

		char hashString[100];
		sprintf(hashString, "%d", getTextureHash(e->mTexture));
		string_map_push_owned(&gTexturePool.mPathToLoadedTexture, tPath, e);
		string_map_push(&gTexturePool.mTextureHashToLoadedTexture, hashString, e);

		return e->mTexture;
	}

	void unloadTextureFromPool(TextureData& tTexture) {
		char hashString[100];
		sprintf(hashString, "%d", getTextureHash(tTexture));

		int hasBeenLoaded = string_map_contains(&gTexturePool.mTextureHashToLoadedTexture, hashString);
		if (!hasBeenLoaded) {
			logError("Unrecognized Texture.");
			logErrorString(hashString);
			recoverFromError();
		}

		TexturePoolEntry* e = (TexturePoolEntry*)string_map_get(&gTexturePool.mTextureHashToLoadedTexture, hashString);
		e->mCounter--;

		if (e->mCounter > 0) return;

		unloadTexture(e->mTexture);
		string_map_remove(&gTexturePool.mTextureHashToLoadedTexture, hashString);
		string_map_remove(&gTexturePool.mPathToLoadedTexture, e->mPath);
	}

}