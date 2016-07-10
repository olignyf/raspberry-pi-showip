//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-filesystem.h
//
// Copyright (c) 2105 Francois Oligny-Lemieux
// All rights reserved
//
//        Author : Francois Oligny-Lemieux
//       Created : 10.Apr.2015
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <stdio.h>
#include <errno.h>
#define _GNU_SOURCE // for basename not to modify source
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if ( defined(_MSC_VER) )
#	include "dirent.h" // local implementation by Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
#else
#	include <dirent.h>
#endif

#include "toolbox.h"
#include "toolbox-basic-types.h"
#include "toolbox-filesystem.h"


#if defined(_MSC_VER)

const char * GetBaseName(const char * fullpath)
{
	char * strRet = strrchr(fullpath, '\\');
	if (strRet == NULL) return fullpath;

	return strRet+1;
}
#else
const char * GetBaseName(char const *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return strdup(path);
	else
		return strdup(s + 1);
}
#endif

int traverseDir(const char* directory, fileEntryCallback_func f_callback, void * opaque1, void * opaque2)
{
	DIR *pRecordDir;
	struct dirent *pEntry;
	struct stat FStat;
	const char *pszBaseName;
	char szFullPathName[512];

	fileEntry_T curEntry;

	if (directory == NULL) return -1;
	if (f_callback == NULL) return -2;

	pRecordDir = opendir(directory);
	if (pRecordDir == NULL)
	{
		return -10;
	}

	while ((pEntry = readdir(pRecordDir)) != NULL)
	{
		pszBaseName = GetBaseName(pEntry->d_name);
		if (pszBaseName != NULL && pszBaseName[0] != '.')
		{
			memset(&curEntry, 0, sizeof(curEntry));

			snprintf(szFullPathName, sizeof(szFullPathName)-1, "%s/%s", directory, pszBaseName);
			if (pEntry->d_type == DT_REG)
			{
//				C_GetFileSize(szFullPathName, &curEntry.size);
			}
			else if (pEntry->d_type == DT_DIR)
			{
				curEntry.isDirectory = 1;
			}
			else if (pEntry->d_type == DT_UNKNOWN)
			{
				if (lstat(szFullPathName, &FStat) == 0)
				{
					if (S_ISDIR(FStat.st_mode))
					{
						curEntry.isDirectory = 1;
					}
				}
				else
				{
					printf("lstat failed for \"%s\", errno=%d.\n", szFullPathName);
				}

			}

			curEntry.name = pEntry->d_name;

			f_callback(szFullPathName, &curEntry, opaque1, opaque2);
		}
	}

	closedir (pRecordDir);
	return 1;
}
