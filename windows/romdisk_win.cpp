#include "prism/windows/romdisk_win.h"

#include <string.h>
#include <errno.h>

#include "prism/datastructures.h"
#include "prism/system.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"


static StringMap gRomdiskHandlers;

//TODO: REMOVE

#define O_MODE_MASK 0x0f        /**< \brief Mask for mode numbers */
//#define O_TRUNC       0x0100      /* Truncate */
#define O_ASYNC     0x0200      /**< \brief Open for asynchronous I/O */
//#define O_NONBLOCK    0x0400      /* Open for non-blocking I/O */
#define O_DIR       0x1000      /**< \brief Open as directory */
#define O_META 0x2000 /**< \brief Open as metadata */

#define MAX_RD_FILES 1024

typedef struct {
	char    magic[8];       /* Should be "-rom1fs-" */
	uint32_t  full_size;      /* Full size of the file system */
	uint32_t  checksum;       /* Checksum */
	char    volume_name[16];    /* Volume name (zero-terminated) */
} romdisk_hdr_t;

/* File header info; note that this header plus filename must be a multiple of
16 bytes, and the following file data must also be a multiple of 16 bytes. */
typedef struct {
	uint32_t  next_header;        /* Offset of next header */
	uint32_t  spec_info;      /* Spec info */
	uint32_t  size;           /* Data size */
	uint32_t  checksum;       /* File checksum */
	char    filename[16];       /* File name (zero-terminated) */
} romdisk_file_t;


/* Util function to reverse the byte order of a uint32 */
static uint32_t ntohl_32(const void *data) {
	const uint8_t *d = (const uint8_t*)data;
	return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | (d[3] << 0);
}

/********************************************************************************/

/* A single mounted romdisk image; a pointer to one of these will be in our
VFS struct for each mount. */
typedef struct rd_image {

	int         own_buffer; /* Do we own the memory? */
	const uint8_t* image;    /* The actual image */
	const romdisk_hdr_t * hdr;      /* Pointer to the header */
	uint32_t          files;      /* Offset in the image to the files area */
	char mountpath[1024];
} rd_image_t;

static StringMap gRomdiskMapping;

/********************************************************************************/
/* File primitives */

/* File handles.. I could probably do this with a linked list, but I'm just
too lazy right now. =) */
static struct {
	int32_t      index;      /* romfs image index */
	int     dir;        /* >0 if a directory */
	uint32_t      ptr;        /* Current read position in bytes */
	uint32_t      size;       /* Length of file in bytes */
	// dirent_t    dirent;     /* A static dirent to pass back to clients */
	rd_image_t  * mnt;      /* Which mount instance are we using? */
} gFileHandlers[MAX_RD_FILES];

/* Given a filename and a starting romdisk directory listing (byte offset),
search for the entry in the directory and return the byte offset to its
entry. */
static uint32_t romdisk_find_object(rd_image_t * mnt, const char *fn, size_t fnlen, int dir, uint32_t offset) {
	uint32_t          i, ni, type;
	const romdisk_file_t    *fhdr;

	i = offset;

	do {
		/* Locate the entry, next pointer, and type info */
		fhdr = (const romdisk_file_t *)(mnt->image + i);
		ni = ntohl_32(&fhdr->next_header);
		type = ni & 0x0f;
		ni = ni & 0xfffffff0;

		/* Check the type */
		if (!dir) {
			if ((type & 3) != 2) {
				i = ni;

				if (!i)
					break;
				else
					continue;
			}
		}
		else {
			if ((type & 3) != 1) {
				i = ni;

				if (!i)
					break;
				else
					continue;
			}
		}

		/* Check filename */
		// TODO: work around strncasecmp
		if ((strlen(fhdr->filename) == fnlen) && (!SDL_strncasecmp(fhdr->filename, fn, fnlen))) {
			/* Match: return this index */
			return i;
		}

		i = ni;
	} while (i != 0);

	/* Didn't find it */
	return 0;
}

/* Locate an object anywhere in the image, starting at the root, and
expecting a fully qualified path name. This is analogous to the
find_object_path in iso9660.
fn:      object filename (absolute path)
dir:     0 if looking for a file, 1 if looking for a dir
It will return an offset in the romdisk image for the object. */
static uint32_t romdisk_find(rd_image_t * mnt, const char *fn, int dir) {
	const char      *cur;
	uint32_t          i;
	const romdisk_file_t    *fhdr;

	/* If the object is in a sub-tree, traverse the trees looking
	for the right directory. */
	i = mnt->files;

	cur = strchr(fn, '/');
	while (cur) {
		if (cur != fn) {
			i = romdisk_find_object(mnt, fn, cur - fn, 1, i);

			if (i == 0) return 0;

			fhdr = (const romdisk_file_t *)(mnt->image + i);
			i = ntohl_32(&fhdr->spec_info);
		}

		fn = cur + 1;
		cur = strchr(fn, '/');
	}

	/* Locate the file in the resulting directory */
	if (*fn) {
		i = romdisk_find_object(mnt, fn, strlen(fn), dir, i);
		return i;
	}
	else {
		if (!dir)
			return 0;
		else
			return i;
	}
}

static void getPotentialMountFromPath(char* tPotentialMount, const char* tPath) {
	tPotentialMount[0] = '\0';
	if (tPath[0] == '$') tPath++;
	if (tPath[0] != '/') return;

	strcpy(tPotentialMount, tPath + 1);
	char* endPos = strchr(tPotentialMount, '/');
	if (endPos != NULL) *endPos = '\0';
}



static rd_image_t* getRomdiskImageFromPath(char* tPath) {
	char potentialMount[1024];
	getPotentialMountFromPath(potentialMount, tPath);
	return (rd_image_t*)string_map_get(&gRomdiskMapping, potentialMount);
}

/* Open a file or directory */

FileHandler fileOpenRomdisk(char* tPath, int tFlags) {
	int fd;
	uint32_t          filehdr;
	const romdisk_file_t    *fhdr;
	rd_image_t      *mnt = getRomdiskImageFromPath(tPath);

	/* Make sure they don't want to open things as writeable */
	if (tFlags != O_RDONLY) { // TODO: check better
		errno = EPERM;
		return NULL;
	}

	const char* romdiskPath = strchr(tPath+1, '/');
	if (romdiskPath == NULL) romdiskPath = tPath;

	/* No blank filenames */
	if (romdiskPath[0] == 0)
		romdiskPath = "";

	/* Look for the file */
	filehdr = romdisk_find(mnt, romdiskPath + 1, tFlags & O_DIR);

	if (filehdr == 0) {
		errno = ENOENT;
		return NULL;
	}

	/* Find a free file handle */
	// mutex_lock(&fh_mutex);

	for (fd = 0; fd < MAX_RD_FILES; fd++)
		if (gFileHandlers[fd].index == 0) {
			gFileHandlers[fd].index = -1;
			break;
		}

	// mutex_unlock(&fh_mutex);

	if (fd >= MAX_RD_FILES) {
		errno = ENFILE;
		return NULL;
	}

	/* Fill the fd structure */
	fhdr = (const romdisk_file_t *)(mnt->image + filehdr);
	gFileHandlers[fd].index = filehdr + sizeof(romdisk_file_t) + (strlen(fhdr->filename) / 16) * 16;
	gFileHandlers[fd].dir = (tFlags & O_DIR) ? 1 : 0;
	gFileHandlers[fd].ptr = 0;
	gFileHandlers[fd].size = ntohl_32(&fhdr->size);
	gFileHandlers[fd].mnt = mnt;

	char handlerString[100];
	sprintf(handlerString, "%d", fd);
	string_map_push(&gRomdiskHandlers, handlerString, NULL);

	return (FileHandler)fd;
}

/* Close a file or directory */
int fileCloseRomdisk(FileHandler tHandler) {
	int fd = (int)tHandler;

	/* Check that the fd is valid */
	if (fd < MAX_RD_FILES) {
		/* No need to lock the mutex: this is an atomic op */
		gFileHandlers[fd].index = 0;
	}

	char handlerString[100];
	sprintf(handlerString, "%d", fd);
	string_map_remove(&gRomdiskHandlers, handlerString);

	return 0;
}

/* Read from a file */
size_t fileReadRomdisk(FileHandler tHandler, void* tBuffer, size_t tCount) {
	int fd = (int)tHandler;

	/* Check that the fd is valid */
	if (fd >= MAX_RD_FILES || gFileHandlers[fd].index == 0 || gFileHandlers[fd].dir) {
		logError("Invalid file handler");
		logErrorInteger(fd);
		recoverFromError();
	}

	/* Is there enough left? */
	if ((gFileHandlers[fd].ptr + tCount) > gFileHandlers[fd].size)
		tCount = gFileHandlers[fd].size - gFileHandlers[fd].ptr;

	/* Copy out the requested amount */
	memcpy(tBuffer, gFileHandlers[fd].mnt->image + gFileHandlers[fd].index + gFileHandlers[fd].ptr, tCount);
	gFileHandlers[fd].ptr += tCount;

	return tCount;
}

/* Seek elsewhere in a file */
size_t fileSeekRomdisk(FileHandler tHandler, size_t tOffset, int tWhence) {
	int fd = (int)tHandler;

	/* Check that the fd is valid */
	if (fd >= MAX_RD_FILES || gFileHandlers[fd].index == 0 || gFileHandlers[fd].dir) {
		logError("Invalid file handler");
		logErrorInteger(fd);
		recoverFromError();
	}

	int32_t offset = (int32_t)tOffset;
	/* Update current position according to arguments */
	switch (tWhence) {
	case SEEK_SET:
		if (offset < 0) {
			logError("Invalid offset");
			logErrorInteger(tOffset);
			recoverFromError();
		}

		gFileHandlers[fd].ptr = tOffset;
		break;

	case SEEK_CUR:
		if (offset < 0 && (-offset) > (int32_t)gFileHandlers[fd].ptr) {
			logError("Invalid offset");
			logErrorInteger(tOffset);
			recoverFromError();
		}

		gFileHandlers[fd].ptr += tOffset;
		break;

	case SEEK_END:
		if (offset < 0 && (-offset) >(int32_t)gFileHandlers[fd].size) {
			logError("Invalid offset");
			logErrorInteger(tOffset);
			recoverFromError();
		}

		gFileHandlers[fd].ptr = gFileHandlers[fd].size + tOffset;
		break;

	default:
		logError("Invalid whence");
		logErrorInteger(tWhence);
		recoverFromError();
	}

	/* Check bounds */
	if (gFileHandlers[fd].ptr > gFileHandlers[fd].size) gFileHandlers[fd].ptr = gFileHandlers[fd].size;

	return gFileHandlers[fd].ptr;
}

/* Tell where in the file we are */
size_t fileTellRomdisk(FileHandler tHandler) {
	int fd = (int)tHandler;

	if (fd >= MAX_RD_FILES || gFileHandlers[fd].index == 0 || gFileHandlers[fd].dir) {
		logError("Invalid file handler");
		logErrorInteger(fd);
		recoverFromError();
	}

	return gFileHandlers[fd].ptr;
}

/* Tell how big the file is */
size_t fileTotalRomdisk(FileHandler tHandler) {
	int fd = (int)tHandler;

	if (fd >= MAX_RD_FILES || gFileHandlers[fd].index == 0 || gFileHandlers[fd].dir) {
		logError("Invalid file handler");
		logErrorInteger(fd);
		recoverFromError();
	}

	return gFileHandlers[fd].size;
}


/* Are we initialized? */
static int gInitted = 0;

/* Initialize the file system */
void initRomdisks() {
	if (gInitted) return;

	gRomdiskHandlers = new_string_map();
	gRomdiskMapping = new_string_map();

	/* Reset fd's */
	memset(gFileHandlers, 0, sizeof gFileHandlers);

	/* Mark the first as active so we can have an error FD of zero */
	gFileHandlers[0].index = -1;

	gInitted = 1;
}

void mountRomdiskWindowsFromBuffer(Buffer b, const char * tMountPath)
{
	int isAlreadyMounted = string_map_contains(&gRomdiskMapping, tMountPath);
	if (isAlreadyMounted) {
		logError("Unable to mount. Already mounted.");
		logErrorString(tMountPath);
		recoverFromError();
	}


	char* img = (char*)b.mData;

	const romdisk_hdr_t * hdr;
	rd_image_t      * mnt;
	int own_buffer = 1;

	/* Are we initted? */
	if (!gInitted) {
		logError("Trying to mount romdisk before init");
		logErrorString(tMountPath);
		recoverFromError();
	}

	/* Check the image and print some info about it */
	hdr = (const romdisk_hdr_t *)img;

	if (strncmp((char *)img, "-rom1fs-", 8)) {
		logError("Rom disk image is not a ROMFS image\n");
		logErrorPointer(img);
		recoverFromError();
	}

	/* Create a mount struct */
	mnt = (rd_image_t *)allocMemory(sizeof(rd_image_t));
	mnt->own_buffer = own_buffer;
	mnt->image = (const uint8_t*)img;
	mnt->hdr = hdr;
	mnt->files = sizeof(romdisk_hdr_t)
		+ (strlen(hdr->volume_name) / 16) * 16;
	strcpy(mnt->mountpath, tMountPath);

	/* Add it to our mount list */
	// mutex_lock(&fh_mutex);
	string_map_push_owned(&gRomdiskMapping, tMountPath, mnt);
	// mutex_unlock(&fh_mutex);
}

/* Mount a romdisk image; must have called fs_romdisk_init() earlier.
Also note that we do _not_ take ownership of the image data if
own_buffer is 0, so if you alloc'd that buffer, you must
also free it after the unmount. If own_buffer is non-zero, then
we free the buffer when it is unmounted. */
void mountRomdiskWindows(const char* tFilePath, const char* tMountPath) {
	Buffer b = fileToBuffer(tFilePath);
	mountRomdiskWindowsFromBuffer(b, tMountPath);
}



/* Unmount a romdisk image */
void unmountRomdiskWindows(const char* tMountPath) {
	rd_image_t  * n;

	// mutex_lock(&fh_mutex);

	n = (rd_image_t*)string_map_get(&gRomdiskMapping, tMountPath);

	/* If we own the buffer, free it */
	if (n->own_buffer) freeMemory((void *)n->image);

	string_map_remove(&gRomdiskMapping, tMountPath);
}

int isRomdiskPath(const char * tPath)
{
	char mount[1024];
	getPotentialMountFromPath(mount, tPath);

	return string_map_contains(&gRomdiskMapping, mount);
}

int isRomdiskFileHandler(FileHandler tHandler)
{
	char handlerString[100];
	sprintf(handlerString, "%d", (int)tHandler);
	return string_map_contains(&gRomdiskHandlers, handlerString);
}
