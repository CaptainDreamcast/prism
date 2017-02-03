#include "include/wrapper.h"

#include "include/pvr.h"
#include "include/physics.h"
#include "include/file.h"
#include "include/drawing.h"
#include "include/log.h"
#include "include/memoryhandler.h"

void initTariWrapperWithDefaultFlags() {
	initiatePVR();
	initMemoryHandler();
	initPhysics();
	initFileSystem();
	initDrawing();
	setFont("$/rd/fonts/dolmexica.hdr", "$/rd/fonts/dolmexica.pkg");
}
void shutdownTariWrapper() {
	shutdownMemoryHandler();
}
