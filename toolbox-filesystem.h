#ifndef __C_TOOLBOX_FILESYSTEM_H__
#define __C_TOOLBOX_FILESYSTEM_H__

#include <time.h>

typedef struct
{
	int isDirectory;
	char *name;
	time_t ctime;
	time_t mtime;
	unsigned long long size;
} fileEntry_T;

typedef int (*fileEntryCallback_func) (const char *name, const fileEntry_T *entry, void * opaque1, void * opaque2);

// client can pass "opaque" pointer which will be feeded to the callback function
int traverseDir(const char* directory, fileEntryCallback_func f_callback, void * opaque1, void * opaque2);


#endif
