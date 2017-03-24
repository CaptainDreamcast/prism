#include "../include/texture.h"

#include <SDL.h>
#include <SDL_image.h>

#include "../include/file.h"
#include "../include/log.h"
#include "../include/memoryhandler.h"
#include "../include/system.h"
#include "../include/math.h"



extern SDL_Renderer* gRenderer;

TextureData loadTexturePKG(char* tFileDir) {
	char fullFileName[1024];

	getFullPath(fullFileName, tFileDir);

	// TODO: Properly decode PKG files; currently assume we have PNG instead
	int len = strlen(fullFileName);
	fullFileName[len - 2] = 'n';

	SDL_Texture* newTexture;

	SDL_Surface* loadedSurface = IMG_Load(fullFileName);
	if (loadedSurface == NULL)
	{
		logError("Unable to load file:");
		logErrorString(fullFileName);
		logErrorString(IMG_GetError());
		abortSystem();
	}

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if (newTexture == NULL)
	{
		logError("Unable to load file:");
		logErrorString(fullFileName);
		logErrorString(SDL_GetError());
		abortSystem();
	}

	SDL_FreeSurface(loadedSurface);
	
	TextureData returnData;
	returnData.mTexture = newTexture;
	int access;
	Uint32 format;
	SDL_QueryTexture(newTexture, &format, &access, &returnData.mTextureSize.x, &returnData.mTextureSize.y);

	return returnData;
}

TextureData loadTexture(char* tFileDir) {
	char* fileExt = getFileExtension(tFileDir);

	if(!strcmp("pkg", fileExt)) {
		return loadTexturePKG(tFileDir);
	} else {
		logError("Unable to identify texture file type.");
		logErrorString(fileExt);
		abortSystem();
		TextureData errData;
		return errData;
	}
}

void unloadTexture(TextureData tTexture) {
  freeTextureMemory(tTexture.mTexture);
}

int getAvailableTextureMemory() {
	return INF;
}
