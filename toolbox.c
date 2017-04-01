//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox.c
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
//				
//        Author : Francois Oligny-Lemieux
//       Created : 15.May.2006
//      Modified : 26.Jun.2016
//
//  Description: 
//      Common functions
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#include "toolbox.h"
#include "toolbox-basic-types.h"

#if C_TOOLBOX_TIMING == 1 
#	if ( defined(_MSC_VER) )
#		if ! defined(_WINDOWS_)
#			include <windows.h>
#		endif
#		include <mmsystem.h>
#	else
#		include "sys/time.h" 
#		include <dirent.h>
#	endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined(_MSC_VER) && !defined(__TOOLBOX_NETWORK_H__)
#	include <windows.h>
#endif

#if C_TOOLBOX_LOG_COMMAND == 1
FILE * g_toolbox_toolbox_log_file = NULL;
#endif

/* global variable configs */
/* your app can modify those */
int g_verbose = 0;

int char_extract_path(const char * source, char * destination, int destination_size)
{
	const char * strRet;
	unsigned int length = 0;

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

	strRet = strrchr(source, '\\');
	if ( strRet == NULL )
	{	strRet = strrchr(source, '/');
	}
	if ( strRet )
	{
		length = strRet - source;
	}

	if ( destination_size <= length )
	{
		return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
	}

	strncpy(destination, source, length);
	destination[length] = '\0';

	return 1;
}


int char_extract_filename(const char * source, char * destination, int destination_size)
{
	const char * strRet;
	unsigned int length = 0;

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

	destination[0] = '\0';

	strRet = strrchr(source, '\\');
	if ( strRet == NULL )
	{	strRet = strrchr(source, '/');
	}
	if ( strRet )
	{
		length = strlen(strRet+1);
		if ( length > 0 )
		{
			if ( destination_size <= length )
			{
				return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
			}
			strncpy(destination, strRet+1, length);
			destination[length] = '\0';
			
			return 1;
		}
	}

	return 0;
}


// utf8 string MUST be terminated.
char * C_utf8EndOfString(const char * utf8, int utf8_buffer_size)
{
	const char * zero = "\0";
	if ( utf8 == NULL )
	{
		return NULL;
	}

	if ( utf8_buffer_size == -1 )
	{
		utf8_buffer_size = 0x7FFFFFFF;
	}

	return C_memfind((unsigned char *)utf8, utf8_buffer_size, (unsigned char *)zero, 1); 
}

// written 25.Jan.2008
// will realloc the string if too small, or malloc it if NULL
// separator is optional and can be NULL
// other parameters are mandatory
// max_length is either -1 or to the maximum length that can be copied from append.
int C_Append(char ** string, unsigned int * buffersize, const char * append, int max_length, const char * separator)
{
	//unsigned int strLen;
	unsigned int newSize = 0;
	int mallocop = 0;
	char * new_string = NULL;

	if ( max_length == 0 )
	{
		return 0;
	}

	if ( *string )
	{	newSize = strlen(*string);
	}
	if ( max_length < 0 )
	{	newSize += strlen(append); // append without restriction
	}
	else
	{	newSize += max_length; // append up to max_length
	}
	if ( separator )
	{	newSize += strlen(separator);
	}

	newSize += 2;
	if ( *string == NULL )
	{
		new_string = (char*)malloc(newSize);
		mallocop = 1;
		*buffersize = 0;
	}
	else if ( newSize > *buffersize )
	{
		new_string = (char*)realloc(*string, newSize);
		mallocop = 1;
	}
	
	if ( mallocop )
	{
		if ( new_string == NULL )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		*string = new_string;
		memset(new_string+*buffersize, 0, newSize - *buffersize); // frank fixme optimize by erasing only end char and first chars
		*buffersize = newSize;
	}

	if ( separator )
	{	strcat(*string, separator);
	}
	if ( max_length < 0 )
	{
		strcat(*string, append);
	}
	else
	{
		strncat(*string, append, max_length);
	}
	 
	return 1;
}


// written 01.May.2008
int buffer_to_file(const char * buffer, const char * filename)
{
	FILE * file;
	int iret;

	if ( buffer == NULL )
	{
		return -1;
	}
	if ( filename == NULL )
	{
		return -2;
	}

	file = fopen(filename, "wb");

	if ( file == NULL )
	{
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	iret = fwrite((char*)buffer, 1, strlen(buffer), file);
	
	fclose(file);

	if ( iret <= 0 )
	{		
		return -10;
	}

	return 1;
}

int C_FileExists(const char * filename)
{
	FILE * test;
	int ret = 0;

	if ( filename == NULL || filename[0] == '\0' )
	{
		return -1;
	}

	test = fopen(filename, "r");
	if (test == NULL)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
		fclose(test);
	}
	return ret;
}
const char * C_GetFileExtension(const char * filename){	int count = 5;	const char * strLook;		int strLength = strlen(filename);	if (strLength == 0) return NULL;		strLook = &filename[strLength-1];	while (count > 0 && strLook > &filename[0])	{		if (*strLook == '.') return strLook+1;		strLook--;		count--;	}		return NULL;}

int C_DirectoryExists(const char * directory)
{
	//FILE * test;
	//int ret = 0;
#if ( defined(_MSC_VER) )
	DWORD attributes = GetFileAttributes(directory);
	if ( attributes != INVALID_FILE_ATTRIBUTES
	  && attributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		return 1;
	}
#	if _DEBUG
	attributes = GetLastError();
	attributes = attributes;
#	endif
	return 0;
#else
	DIR * dirPtr;
	dirPtr = opendir(directory);
	if ( dirPtr == NULL )
	{
		return 0;
	}
	closedir(dirPtr);
	return 1;
#endif
}


int C_CreateDirectory(const char * directory)
{
	//FILE * test;
	//int ret = 0;
#if ( defined(_MSC_VER) )
	BOOL bret = CreateDirectory(directory, NULL);
	if ( bret == FALSE )
	{	
		// failure
		return -10;
	}
	return 1;
#else
	int iret;
	iret = mkdir(directory, 0755);
	if ( iret < 0 )
	{
		// failure
		return -10;
	}
	return 1;
#endif
}



// by default will overwrite file
int C_CopyFile(const char * source, const char * destination)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	bret = CopyFile(source,destination,FALSE/*bFailIfExists*/);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		printf("WIN32 CopyFile failed with error(%d)\r\n", (int)GetLastError());
#endif
		return -10;
	}
#else
	snprintf(command,255,"cp -f \"%s\" \"%s\"", source,destination);
	system(command);
#endif

	return 1;
}

// written 08.Jan.2008
int C_CopyFileEx(const char * source, const char * destination, int flags)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
	DWORD win32_flags = 0x00000008/*COPY_FILE_ALLOW_DECRYPTED_DESTINATION*/;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )

#	if(_WIN32_WINNT >= 0x0400)
	if ( (flags&TOOLBOX_OVERWRITE_DESTINATION)==0 )
	{	win32_flags |= 0x00000001/*COPY_FILE_FAIL_IF_EXISTS*/;
	}
	bret = CopyFileEx(source, destination, NULL/*progress*/, NULL/*progress data*/, FALSE, win32_flags);
	if ( bret == FALSE )
	{
		// error
#	if _DEBUG
		printf("WIN32 CopyFileEx failed with error(%d)\r\n", (int)GetLastError());
#	endif
		return -10;
	}

#	else

	bret = FALSE;
	if ( (flags&TOOLBOX_OVERWRITE_DESTINATION)==0 )
	{	bret = TRUE;
	}
	bret = CopyFile(source,destination,bret/*bFailIfExists*/);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		printf("WIN32 CopyFile failed with error(%d)\r\n", (int)GetLastError());
#endif
		return -10;
	}
#	endif

#else
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{
		snprintf(command,255,"cp -f \"%s\" \"%s\"", source,destination);
	}
	else
	{
		snprintf(command,255,"cp \"%s\" \"%s\"", source,destination);
	}
	system(command);
#endif

	return 1;
}


// by default will overwrite file
int C_MoveFile(const char * source, const char * destination)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	bret = MoveFileEx(source,destination, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH|MOVEFILE_COPY_ALLOWED);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		int win32_error = GetLastError();
		printf("WIN32 MoveFileEx failed with error(%d)\r\n", win32_error);
#endif
		return -10;
	}
#else
	snprintf(command,255,"mv -f \"%s\" \"%s\"", source,destination);
	system(command);
#endif

	return 1;
}

// flags are agregates of gnucFlags_E
// written 08.Jan.2008
int C_MoveFileEx(const char * source, const char * destination, int flags)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
	DWORD win32_flags = MOVEFILE_WRITE_THROUGH;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

#if ( defined(_MSC_VER) )
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{	win32_flags |= MOVEFILE_REPLACE_EXISTING;
	}
	win32_flags |= MOVEFILE_COPY_ALLOWED;
	bret = MoveFileEx(source, destination, win32_flags);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		int win32_error = GetLastError();
		printf("WIN32 MoveFile failed with error(%d)\r\n", win32_error);
#endif
		return -10;
	}
#else
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{
		snprintf(command,255,"mv -f \"%s\" \"%s\"", source,destination);
	}
	else
	{
		snprintf(command,255,"mv \"%s\" \"%s\"", source,destination);
	}
	system(command);
#endif

	return 1;
}



// written 19.Feb.2011
int C_endSlashDirectory(char * directory_inout)
{
	int backslashes = 0;
	char * strRet;
	char * strRet2;
		
	strRet = strstr(directory_inout, "\\");
	if ( strRet ) 
	{
		backslashes = 1;
	}

	strRet = C_strendstr(directory_inout, "\\");
	strRet2 = C_strendstr(directory_inout, "/");
	if ( strRet == NULL && strRet2 == NULL )
	{
		if ( backslashes ) 
		{	strcat(directory_inout, "\\");
		}
		else
		{	strcat(directory_inout, "/");
		}
	}

	return 1;
}




int C_GetTempFilename(const char * path, char * out_filename)
{
#if defined(_MSC_VER)
	char temp[256]="";
#endif


	if ( out_filename == NULL )
	{
		return -1;
	}

#if defined(_MSC_VER)
	// windows
	if ( tmpnam(temp)==NULL )
	{
		return -12;
	}
	if ( temp[0] == '\\' )
	{
		int length = strlen(temp);
		memmove(temp, temp+1, length); // copy the last byte '\0'
		temp[length-1] = '\0'; // redundant
	}
	out_filename[0] = '\0';
	if ( path )
	{
		strcpy(out_filename, path);
		if ( path[strlen(path)-1] != '/'
			&& path[strlen(path)-1] != '\\'
			)
		{
			// append slash
			strcat(out_filename, "/");
		}
	}
	strcat(out_filename, temp);
#else
	// linux
	int outfd; 
	if ( path )
	{
		strcpy(out_filename, path);
		if ( path[strlen(path)-1] != '/'
			&& path[strlen(path)-1] != '\\'
			)
		{
			// append slash
			strcat(out_filename, "/");
		}
	}
	strcat(out_filename, "XXXXXX");
	outfd = mkstemp(out_filename);
	if (outfd == -1) 
	{
		printf("<!-- mkstemp on filename(%s) failed -->\n", out_filename);
		return -10;
	}
	close(outfd);
	/* Fix the permissions */
	if (chmod(out_filename, 0600) != 0)
	{
		unlink(out_filename);
		return -11;
	}
#endif
	return 1;
}



int C_itoa( unsigned int i, char * toLoad, unsigned int toLoad_size )
{
	char temp[22];
	char *p = &temp[21];

	*p-- = '\0';

	do {
		*p-- = '0' + i % 10;
		i /= 10;
	} while (i > 0);

	strncpy(toLoad, p+1, toLoad_size-1);
	toLoad[toLoad_size-1] = '\n';

	return 1;
}

// this function has been obtained somewhere on the web.
int C_axtoi(const char * hex)
{
	int n = 0; // position in string
	int m = 0; // position in digit[] to shift
	int count; // loop index
	int intValue = 0; // integer value of hex string
	int digit[32]; // hold values to convert
	if ( strstr(hex, "0x")==hex )
	{	hex+=2;
	}
	while (n < 32)
	{
		if (hex[n] == '\0')
		{	break; }
		if (hex[n] >= 0x30 && hex[n] <= 0x39 )	//if 0 to 9
		{	digit[n] = hex[n] & 0x0f; }	//convert to int
		else if (hex[n] >= 'a' && hex[n] <= 'f') //if a to f
		{	digit[n] = (hex[n] & 0x0f) + 9; } //convert to int
		else if (hex[n] >= 'A' && hex[n] <= 'F') //if A to F
		{	digit[n] = (hex[n] & 0x0f) + 9; } //convert to int
		else
		{	break; }
		n++;
	}
	count = n;
	m = n - 1;
	n = 0;
	while (n < count)
	{
		// digit[n] is value of hex digit at position n
		// (m << 2) is the number of positions to shift
		// OR the bits into return value
		intValue = intValue | (digit[n] << (m << 2));
		m--; // adjust the position to set
		n++; // next digit to process
	}
	return (intValue);
}

// will replace in place
char* C_strreplace(char * source, const char token, const char replacement)
{
	char * strRet;
	
	if (source == NULL) return NULL;
	if (token == replacement) return source;
	
	strRet = strchr(source, token);
	while (strRet)
	{	*strRet = replacement;
		strRet = strchr(source, token);
	}
	return source;
}

// written 04.Feb.2009
// will write at most dst_max_length and always NULL-terminate the string.
int C_strncpy(char * destination, const char * source, int dst_max_length)
{
	if ( destination == NULL ) 
	{	return -1;
	}
	if ( source == NULL ) 
	{	return -2;
	}
	if ( dst_max_length <= 0 ) 
	{	return -3;
	}

	while ( *source != '\0' )
	{
		*destination = *source++;
		dst_max_length--;
		if ( dst_max_length == 0 )
		{
			*destination = '\0';
			break;
		}		
		destination++;
	}

	return 1;
}

// returns NULL on error or not found
// returns pointer from inside string when found, at beginning of needle in string.
char * C_strcasestr(const char * string, const char * needle)
{
	int i;
	int j;
	int stringLen;
	int needleLen;
	int searchLen;
    
	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLen = strlen(string);
	needleLen = strlen(needle);
	searchLen = stringLen - needleLen + 1;

	for (i=0; i<searchLen; i++)
	{
		for (j=0; ; j++)
		{
			if ( j == needleLen )
			{
				return (char*) string+i;
			}
			if ( tolower(string[i+j]) != tolower(needle[j]) )
			{
				// we are not interested in this i
				break;
			}
		}
	}
	
	return NULL; // not found
}

char * C_strncasestr(const char * string, int string_length, const char * needle)
{
	int i;
	int j;
	int stringLen;
	int needleLen;
	int searchLen;
    
	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLen = string_length;
	needleLen = strlen(needle);
	searchLen = stringLen - needleLen + 1;

	for (i=0; i<searchLen; i++)
	{
		for (j=0; ; j++)
		{
			if ( j == needleLen )
			{
				return (char*) string+i;
			}
			if ( string[i+j] == '\0' 
				|| ( tolower(string[i+j]) != tolower(needle[j]) )
				)
			{
				// string is over.
				// we are not interested in this i
				break;
			}
		}
	}
	
	return NULL; // not found
}

// Author: Francois Oligny-Lemieux
// Created: 22.Mar.2007
// if string ends with needle, return pointer to first char of needle in string
// else return NULL
char * C_strendstr(const char * string, const char * needle)
{ 	
	unsigned int stringLength;
	unsigned int needleLength;
	char * strRet;

	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLength = strlen(string);
	needleLength = strlen(needle);

	if ( needleLength > stringLength )
	{
		return NULL;
	}

	// string[stringLength] represents '\0'
	// string[stringLength-needleLength] represents theorical start of needle in string
	strRet = strstr(&string[stringLength-needleLength], needle);
	if ( strRet )
	{
		return strRet;
	}

	return NULL;
}


// Author: Francois Oligny-Lemieux
// Created: 22.Mar.2007
// if string ends with needle (case insensitive), return pointer to first char of needle in string
// else return NULL
char * C_striendstr(const char * string, const char * needle)
{
	unsigned int stringLength;
	unsigned int needleLength;
	char * strRet;

	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLength = strlen(string);
	needleLength = strlen(needle);

	if ( needleLength > stringLength )
	{
		return NULL;
	}

	// string[stringLength] represents '\0'
	// string[stringLength-needleLength] represents theorical start of needle in string
	strRet = C_strcasestr(&string[stringLength-needleLength], needle);
	if ( strRet )
	{
		return strRet;
	}

	return NULL;
}

// Author: Francois Oligny-Lemieux
// Created: 18.May.2007
// Binary-safe strlen
int C_strlen(const char * string, int string_buffer_size)
{
	uint8_t success = 0;
	int i = 0;
	if ( string == NULL )
	{
		return -1;
	}

	while ( i<string_buffer_size )
	{
		if ( string[i] == '\0' )
		{
			success = 1;
			break;
		}
		i++;
	}

	if ( success == 0 )
	{
		return -1;
	}
	return i;
}


// written 04.Feb.2009, returns end of string
const char * C_eos(const char * string)
{
	if ( string == NULL )
	{
		return NULL;
	}

	while ( *string != '\0' )
	{
		string++;
	}

	return string;
}

// written 20.Aug.2007
void * C_memfind(const unsigned char * buffer, int buffer_length, const unsigned char * needle, int needle_length)
{
	unsigned int i;
	unsigned int j; 
	//unsigned int found;
	void * out_value = NULL;

	if ( buffer == NULL )
	{
		return NULL;
	}
	if ( buffer_length <= 0 )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}
	if ( needle_length <= 0 )
	{
		return NULL;
	}

	for (i=0; i<(unsigned int)buffer_length; i++)
	{
		for (j=0; j<(unsigned int)needle_length; j++)
		{
			if ( buffer[i+j] != needle[j] )
			{
				break;
			}
		}
		if ( j == needle_length )
		{
			out_value = (void*)&buffer[i];
			return out_value;
		}
	}

	return NULL;
}


#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
int putTextBufferReader_into_charArray(textBufferReader * reader, charArray * array)
{
	int iret;
	char buffer[TEXT_BUFFER_READER_MAX_LINE_SIZE+1];

	if ( reader == 0 )
	{
		return -1;
	}
	if ( array == 0 )
	{
		return -1;
	}

	iret = TextBufferReader_GetLine(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);

	while ( iret > 0 || iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL || iret == TOOLBOX_WARNING_CONTINUE_READING )
	{	
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] ERROR_BUFFER_TOO_SMALL\n");
		}
#if TOOLBOX_DEBUG_LEVEL >= 1
		else if ( g_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n",buffer);
		}
#endif
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n",buffer);
		}
		if ( iret != TOOLBOX_WARNING_CONTINUE_READING )
		{
			// frank fixme, in case TOOLBOX_WARNING_CONTINUE_READING I trim line... so it's not 100% success
			// frank fixme, test with sample cases
			charArray_add(array, buffer);
		}
		iret = TextBufferReader_GetLine(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);
	}

#if TOOLBOX_DEBUG_LEVEL >= 1
	if ( g_verbose >= 1 )
	{
		printf("[putTextBufferReader_into_charArray] End of function\n");
	}
#endif

	return 1;
}


// will append to array.
int buffer_to_array(const char * buffer, charArray * array_ptr)
{	
	int iret;
	textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}

	//charArray_Destructor(array_ptr);
	//charArray_Constructor(array_ptr, 1024);

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] going to TextBufferReader_Constructor with strlen(buffer)=(%u)\n",strlen(buffer));
	}
#endif

	iret = TextBufferReader_Constructor(&reader, buffer, strlen(buffer)+1);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] TextBufferReader_Constructor returned iret(%d)\n",iret);
	}
#endif

	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] going to putTextBufferReader_into_charArray\n");
	}
#endif

	putTextBufferReader_into_charArray(&reader, array_ptr);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] putTextBufferReader_into_charArray returned iret(%d)\n",iret);
	}
#endif

	iret = TextBufferReader_Destructor(&reader);

	return 1;
}

// written 11.Jun.2008
int buffer_to_arrayEx(const char * buffer, charArray * array_ptr, int chop_empty_lines)
{
	int iret;
	textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}

	//charArray_Destructor(array_ptr);
	//charArray_Constructor(array_ptr, 1024);

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] going to TextBufferReader_Constructor with strlen(buffer)=(%u)\n",strlen(buffer));
	}
#endif

	iret = TextBufferReader_Constructor(&reader, buffer, strlen(buffer)+1);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] TextBufferReader_Constructor returned iret(%d)\n",iret);
	}
#endif

	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	reader.chop_empty_lines = chop_empty_lines;

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] going to putTextBufferReader_into_charArray\n");
	}
#endif

	putTextBufferReader_into_charArray(&reader, array_ptr);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[buffer_to_array] putTextBufferReader_into_charArray returned iret(%d)\n",iret);
	}
#endif

	iret = TextBufferReader_Destructor(&reader);

	return 1;
}
#endif


// Modified 27.Jun.2007 for multi-thread safety
#define RESULT_SIZE 1024*512
int C_System2(const char * command, char * loadme, unsigned int loadme_size, int * out_status)
{
#if !defined(_MSC_VER)
	char temp[256];
#endif
	FILE * file;
	FILE * fileErr;
	char tmpFilename[256] = "";
	char tmpFilenameErr[256] = "";
	char full_command[256];
	int status;
	int iret;
	int fret = 1;
	unsigned int result_written = 0;
	unsigned int read_size = 24;
	int size_read = 0;

#if C_TOOLBOX_LOG_COMMAND == 1
	static unsigned int log_fileSize = 0;
	uint64_t fileSize = 0;
#endif

	if ( command == NULL )
	{
		return -1;
	}

	if ( loadme == NULL )
	{
		return -2;
	}

	if ( loadme_size == 0 )
	{
		return -3;
	}

	loadme[loadme_size-1] = '\0';

#if ( defined(_MSC_VER) )
	iret = C_GetTempFilename(tmpPath,tmpFilename);
	if ( iret <= 0 )
	{
		//printf("<!-- C_GetTempFilename returned iret(%d) -->\n",iret);
		return -10;
	}	
#else
	snprintf(temp, 255, "%s/tempio/", tmpPath);
	iret = C_DirectoryExists(temp);
	if ( iret <= 0 )
	{
		snprintf(temp, 255, "mkdir %s/tempio/", tmpPath);
		system(temp);
		snprintf(temp, 255, "%s/tempio/", tmpPath);
	}

	iret = C_GetTempFilename(temp,&tmpFilename[0]);
	if ( iret <= 0 )
	{
		//printf("<!-- C_GetTempFilename returned iret(%d) -->\n",iret);
		return -11;
	}
	iret = C_GetTempFilename(temp,&tmpFilenameErr[0]);
	if ( iret <= 0 )
	{
		//printf("<!-- C_GetTempFilename returned iret(%d) -->\n",iret);
		return -12;
	}
#endif

	strcpy(full_command,command);
	/* pipe output in file */
	strcat(full_command," > ");
	strcat(full_command,tmpFilename);
#if ( !defined(_MSC_VER) )
	strcat(full_command," 2> ");
	strcat(full_command, tmpFilenameErr);
#endif

#if C_TOOLBOX_LOG_COMMAND == 1
	if ( g_toolbox_toolbox_log_file == NULL )
	{
		iret = C_GetFileSize(C_TOOLBOX_LOG_FILENAME, &fileSize);
		if ( iret > 0 && fileSize > 1024*1024 )
		{
			g_toolbox_toolbox_log_file = fopen(C_TOOLBOX_LOG_FILENAME, "w");
		}
		else
		{
			g_toolbox_toolbox_log_file = fopen(C_TOOLBOX_LOG_FILENAME, "aw");
		}
	}

	if ( g_toolbox_toolbox_log_file )
	{
		if ( log_fileSize > 1024*1024 )
		{
			fclose(g_toolbox_toolbox_log_file);
			g_toolbox_toolbox_log_file = fopen(C_TOOLBOX_LOG_FILENAME, "w");
		}
		fwrite(full_command, 1, strlen(full_command), g_toolbox_toolbox_log_file);
		fwrite("\n", 1, 1, g_toolbox_toolbox_log_file);
		fflush(g_toolbox_toolbox_log_file);
		log_fileSize += strlen(full_command)+1;
	}
#endif

	status = system(full_command);
	if (out_status) *out_status = status;

#if defined(_MSC_VER)
	file = fopen(tmpFilename, "r");
	if ( file == NULL )
	{
		return -12;
	}
#else
	fileErr = fopen(tmpFilenameErr, "r");
	file = fopen(tmpFilename, "r");
	if ( file == NULL && fileErr == NULL )
	{
		return -12;
	}
#endif

	if (file)
	{
		if ( loadme_size-1-result_written < read_size )
		{	read_size = loadme_size-1-result_written;
		}
		size_read = fread(loadme+result_written, 1, read_size, file);
		//printf("after initial fgets, charRet(0x%X) and feof=%d\n", (unsigned int)charRet, feof(outf));
		while ( size_read > 0 )
		{
			result_written += size_read;
			*(loadme+result_written) = '\0';
	
			if ( loadme_size-1-result_written < read_size )
			{	read_size = loadme_size-1-result_written;
			}
	
			if ( read_size == 0 )
			{
				fret=-13; // buffer is too small
				break;
			}
			
			size_read = fread(loadme+result_written, 1, read_size, file);
		}
		fclose(file);
		
		if (fileErr)
		{
			if ( loadme_size-1-result_written < read_size )
			{	read_size = loadme_size-1-result_written;
			}
			size_read = fread(loadme+result_written, 1, read_size, fileErr);
			//printf("after initial fgets, charRet(0x%X) and feof=%d\n", (unsigned int)charRet, feof(outf));
			while ( size_read > 0 )
			{
				result_written += size_read;
				*(loadme+result_written) = '\0';
		
				if ( loadme_size-1-result_written < read_size )
				{	read_size = loadme_size-1-result_written;
				}
		
				if ( read_size == 0 )
				{
					fret=-13; // buffer is too small
					break;
				}
				
				size_read = fread(loadme+result_written, 1, read_size, fileErr);
			}
			fclose(fileErr);
		}
	}
	
	// delete temporary file
#if ( defined(_MSC_VER) )
	TOOLBOX_DeleteFile(tmpFilename);
#else
	unlink(tmpFilename);
	unlink(tmpFilenameErr);
#endif

#if C_TOOLBOX_LOG_COMMAND == 1
	if ( g_toolbox_toolbox_log_file )
	{
		snprintf(temp, 255, " ... command passed through ... ");
		fwrite(temp, 1, strlen(temp), g_toolbox_toolbox_log_file);
		fwrite("\n", 1, 1, g_toolbox_toolbox_log_file);
		fflush(g_toolbox_toolbox_log_file);
	}
#endif

	return fret;
}

// we supply the char buffer from static memory (not thread safe)
int C_System(const char * command, char ** insider, int * status)
{
	//char temp[256];
	int fret = 1;
	static char result[RESULT_SIZE];
	unsigned int result_written = 0;

	result[0] = '\0';
	result_written = 0;

	result[RESULT_SIZE-1] =- '\0';

	if ( insider )
	{
		*insider = &result[0];
	}
	else
	{
		return -2;
	}

	if ( command == NULL )
	{
		return -1;
	}

	fret = C_System2(command, result, RESULT_SIZE, status);

	return fret;
}


int C_GetFileSize(const char * filename, uint64_t * loadme)
{
	int iret;
#if defined(_MSC_VER)
	struct _stati64 statObject;
#else
	struct stat statObject;
#endif

	if (loadme==NULL)
	{
		return -1;
	}

#if defined(_MSC_VER)
	iret = _stati64(filename, &statObject);
#else
	iret = stat(filename, &statObject);
#endif

	if ( iret < 0 )
	{
		return -10;
	}
	
	*loadme = statObject.st_size;
	//printf("fileSize("I64u")\n",statObject.st_size);
	return 1;
}

int C_GetFileCreationTime(const char * filename, uint64_t * loadme)
{
	int iret;
#if defined(_MSC_VER)
	struct _stati64 statObject;
#else
	struct stat statObject;
#endif

	if (loadme==NULL)
	{
		return -1;
	}

#if defined(_MSC_VER)
	iret = _stati64(filename, &statObject);
#else
	iret = stat(filename, &statObject);
#endif

	if ( iret < 0 )
	{
		return -10;
	}
	
	*loadme = statObject.st_ctime;
	//printf("fileSize("I64u")\n",statObject.st_size);
	return 1;
}


#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
int C_GetNetworkInformation(char * machine_ip,
							   char * subnet,
							   char * broadcast,
							   char * mac,
							   char * RX_packets,
							   char * RX_errors,
							   char * RX_bytes,
							   char * TX_packets,
							   char * TX_errors,
							   char * TX_bytes,
							   char * collisions,
							   char * ifconfig, /*INOUT*/
							   int ifconfig_size,
							   const char * which_interface)
{
	char temp[256];
	char command[256];
	char _line[256];
	char * line = &_line[0];
	charArray array;
	lineParser parser;
	char * insider = NULL;
	char * borrowed = NULL;
	char * position = NULL;
	int iret;
	int length;
	int status;

	temp[255] = '\0';

#if 0
	if ( machine_ip == NULL )
	{
		return -1;
	}
	if ( subnet == NULL )
	{
		return -2;
	}
	if ( broadcast == NULL )
	{
		return -3;
	}
	if ( mac == NULL )
	{
		return -4;
	}
	if ( RX_packets == NULL )
	{
		return -5;
	}
	if ( RX_errors == NULL )
	{
		return -6;
	}
	if ( RX_bytes == NULL )
	{
		return -6;
	}
	if ( TX_packets == NULL )
	{
		return -7;
	}
	if ( TX_errors == NULL )
	{
		return -8;
	}
	if ( TX_bytes == NULL )
	{
		return -8;
	}
	if ( collisions == NULL )
	{
		return -8;
	}
#endif
	
	if (ifconfig && ifconfig[0] != '\0')
	{
		insider = ifconfig;
		iret = 1;
	}
	else
	{
#if ( defined(_MSC_VER) )
		strcpy(command,"ipconfig /all");
		iret = C_System(command, &insider, &status);
#else
		strcpy(command,"ifconfig");
		iret = C_System(command, &insider, &status);
#endif
	}

	if ( iret <= 0 )
	{
		return -10;
		// C_System error
	}

	if ( insider == NULL )
	{
		return -11;
		// C_System error
	}
	
	if (ifconfig && ifconfig[0] == '\0')
	{
		strncpy(ifconfig, insider, ifconfig_size-1);
		ifconfig[ifconfig_size-1] = '\0';
	}

	if ( machine_ip ) machine_ip[0] = '\0';
	if ( subnet ) subnet[0] = '\0';
	if ( broadcast ) broadcast[0] = '\0';
	if ( mac ) mac[0] = '\0';
	if ( RX_packets ) RX_packets[0] = '\0';
	if ( RX_errors ) RX_errors[0] = '\0';
	if ( RX_bytes ) RX_bytes[0] = '\0';
	if ( TX_packets ) TX_packets[0] = '\0';
	if ( TX_errors ) TX_errors[0] = '\0';
	if ( TX_bytes ) TX_bytes[0] = '\0';
	if ( collisions ) collisions[0] = '\0';

	if ( strlen(insider)==0 )
	{
		return -12;
		// probably the command was good or there is no network adaptor.
	}
	
	if (which_interface != NULL && which_interface[0] != '\0')
	{
		printf("will seek to %s\n", which_interface);
		strcpy(line, "\n");
		strcat(line, which_interface);
		position = strstr(insider, line);
		if (position)
		{
			insider = position;
		}
		else
		{
			return 0; // not found
		}
	}

	charArray_Constructor(&array,0);
	lineParser_Constructor(&parser);
	
#if ( defined(_MSC_VER) )
	buffer_to_array(insider, &array);
	//file_to_array("g:\\tmp\\ifconfig.txt", &array); 

	iret = charArray_getLineThatMatches(&array, "IP Address", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, ": ");
		length = position - line + 2;
		if ( position != NULL )
		{
			strncpy(temp, line+length, 255);
			position = strstr(temp, " ");
			if ( position )
			{
				position[0]='\0';
			}
			if ( machine_ip ) strcpy(machine_ip, temp);
		}
	}

	iret = charArray_getLineThatMatches(&array, "Physical Address", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, ": ");
		length = position - line + 2;
		if ( position != NULL )
		{
			strncpy(temp, line+length, 255);
			position = strstr(temp, " ");
			if ( position )
			{
				position[0]='\0';
			}
			if ( mac ) strcpy(mac, temp);
		}
	}
#else
	buffer_to_array(insider, &array);
	
	iret = charArray_getLineThatMatches(&array, "HWaddr", &borrowed);
	if ( iret && borrowed )
	{
		position = strstr(borrowed, "HWaddr");
		if ( position != NULL )
		{
			strcpy(temp, position+6);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( mac ) strcpy(mac, temp);
		}
	}
    
	iret = charArray_getLineThatMatches(&array, "inet addr:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "inet addr:");
		if ( position )
		{
			length = position - line + 10;

			position = strstr(line, "Bcast");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( machine_ip ) strcpy(machine_ip, temp);
			}
		}
	}
    
	iret = charArray_getLineThatMatches(&array, "Bcast:", &borrowed);
	if ( iret && borrowed )
	{	
		strcpy(line,borrowed);
		position = strstr(line, "Bcast:");
		if ( position )
		{
			length = position - line + 6;

			position = strstr(line, "Mask");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( broadcast ) strcpy(broadcast, temp);
			}
		}
	}
	
	iret = charArray_getLineThatMatches(&array, "Mask:", &borrowed);
	if ( iret && borrowed )
	{
		position = strstr(borrowed, "Mask:");
		if ( position != NULL )
		{
			strcpy(temp, position+5);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( subnet ) strcpy(subnet, temp);
		}
	}

	iret = charArray_getLineThatMatches(&array, "RX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "RX packets:");
		if ( position )
		{
			length = position - line + 11;
			position = strstr(line, "errors:");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( RX_packets ) strcpy(RX_packets, temp);
			}
		}
	}

	iret = charArray_getLineThatMatches(&array, "RX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "errors:");
		if ( position )
		{
			length = position - line + 7;
			position = strstr(line, "dropped:");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( RX_errors ) strcpy(RX_errors, temp);
			}
		}
	}

	iret = charArray_getLineThatMatches(&array, "RX bytes:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "RX bytes:");
		if ( position )
		{
			length = position - line + 9;
			position = strstr(line, "(");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( RX_bytes ) strcpy(RX_bytes, temp);
			}
		}
	}

	iret = charArray_getLineThatMatches(&array, "TX bytes:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "TX bytes:");
		length = position - line + 9;

		position = strstr(position, "(");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( TX_bytes ) strcpy(TX_bytes, temp);
		}
	}

	iret = charArray_getLineThatMatches(&array, "TX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "TX packets:");
		length = position - line + 11;

		position = strstr(line, "errors:");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( TX_packets ) strcpy(TX_packets, temp);
		}
	}
    	
	iret = charArray_getLineThatMatches(&array, "TX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "errors:");
		length = position - line + 7;

		position = strstr(line, "dropped:");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( TX_errors ) strcpy(TX_errors, temp);
		}
	}
	
	iret = charArray_getLineThatMatches(&array, "collisions:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "collisions:");
		length = position - line + 11;

		position = strstr(line, "txqueuelen:");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( collisions ) strcpy(collisions, temp);
		}
	}
#endif
    
	charArray_Destructor(&array);
	lineParser_Destructor(&parser);

	return 1;
}
#endif

#if defined(__TOOLBOX_NETWORK_H__)
int C_GetHostname(char * hostname, int hostname_bufsize)
{
	int iret;
	int status;
	char * insider = NULL;

	if ( hostname == NULL )
	{
		return -1;
	}

	if ( hostname_bufsize <= 0 )
	{
		return -2;
	}

	hostname[0] = '\0';

#if defined(_MSC_VER)
	iret = gethostname(hostname, hostname_bufsize-1);
	if ( iret == SOCKET_ERROR )
	{
		iret = WSAGetLastError();
		if ( iret == WSANOTINITIALISED )
		{
			Network_Initialize();
			iret = gethostname(hostname, hostname_bufsize-1);
		}
	}
	if ( iret == 0 )
	{
		return 1;
	}
	return -10; // failed ?? :(
#else
	iret = C_System("hostname", &insider);
	if ( iret > 0 && insider )
	{
		char * strRet;
		strncpy(hostname, insider, hostname_bufsize-1);
		hostname[hostname_bufsize-1] = '\0';
		strRet = strchr(hostname, '\r');
		if ( strRet )
		{
			*strRet = '\0';
		}
		strRet = strchr(hostname, '\n');
		if ( strRet )
		{
			*strRet = '\0';
		}
	}
#endif
	return 1;
}
#endif

#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
// written 17.May.2007
int C_GetDefaultGateway(char * gateway, int gateway_bufsize)
{
	int iret;
	int status;
	int fret = -20; // general not succeeded
	char * line;
	char * insider = NULL;
	char * strRet;
	int row;
	int count = 0;
	charArray array;
	lineParser parser;

	if ( gateway == NULL )
	{
		return -1;
	}

	if ( gateway_bufsize <= 0 )
	{
		return -2;
	}

	gateway[0] = '\0';
	charArray_Constructor(&array, 20);
	lineParser_Constructor(&parser);

#if defined(_MSC_VER)
	iret = C_System("route PRINT", &insider, &status);
#else
	iret = C_System("route -n", &insider, &status);
#endif

	if ( iret > 0 && insider )
	{
		buffer_to_array(insider, &array);
		row = -1;
		iret = charArray_getLineThatMatchesEx(&array, "0.0.0.0", 0, -1, &row, &insider);
		while ( iret > 0 && insider && count < 100 )
		{
			line = insider;
			insider = NULL;
			iret = lineParser_stripSpacing(&parser, line, 1, 0, &insider);
			if ( iret <= 0 || insider == NULL )
			{
				insider = line;
			}
			if ( strstr(insider, "0.0.0.0")==insider ) // matches begining
			{
				// there must be two 0.0.0.0 on the line to be the wanted line
				strRet = strstr(insider+7, "0.0.0.0");
				if ( strRet )
				{
					// it's my line
					insider = NULL;
					iret = lineParser_replaceAll(&parser, line, "0.0.0.0", "", &insider);
					if ( insider )
					{
						iret = lineParser_stripSpacing(&parser, insider, 1, 0, &insider);
						if ( iret > 0 && insider )
						{
							strRet = strchr(insider, ' ');
							if ( strRet )
							{
								*strRet = '\0';
								if ( strlen(insider) >= (unsigned int)gateway_bufsize )
								{
									printf("ERROR - Found gateway is bigger than supplied buffer size(%d)\n", gateway_bufsize);
								}
								else
								{
									strcpy(gateway, insider);
									fret = 1;
									break;
								}
							}
						}
					}
					
				}
				else
				{
					// not default gateway line
				}
			}
			charArray_removeLineWithIndexAndAllBefore(&array, row);
			count++;
			row = -1;
			iret = charArray_getLineThatMatchesEx(&array, "0.0.0.0", 0, -1, &row, &insider);
		}

	}
	else
	{
		fret = -10; // executing command failed.
	}
	
	lineParser_Destructor(&parser);
	charArray_Destructor(&array);
	return fret;
}
#endif

int C_Sleep(int milliseconds)
{
#if ( defined(_MSC_VER) )
	Sleep((DWORD)milliseconds);
#else
	usleep(1000*milliseconds);
#endif
	return 1;
}

static const char * const g_rand_source_alphanum = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char * const g_rand_source_alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char * const g_rand_source_num = "0123456789";
static const char * const g_rand_source_num_nozero = "123456789123456789";
static int g_srand_done = 0;
// will write length char in toload + '\0', thus toload must be at least length+1 big
int C_Random_alpha(int length, char * toload, int buffer_size)
{
	int random_value = 0;
	unsigned int srandom_value = 0;
	char temp[256]="";
	int count=0;
	//int iret;
	int i;
	int number;
	char letter[3]="";

	if ( toload == NULL )
	{
		return -1;
	}
	
	if ( length <= 0 )
	{
		return -2;
	}

	if ( length >= buffer_size || length >= 127 )
	{
		return -3;
	}

	temp[254]='\0';
	temp[255]='\0';
    
	if ( g_srand_done == 0 )
	{
		srandom_value = (unsigned int) time(NULL);
        srand(srandom_value);
		g_srand_done = 1;
	}
	random_value = rand();

	//printf("<!-- rand(%d) -->\n",random_value);

	snprintf(temp, 255, "%d",random_value);

	while ( count < 200 )
	{
		random_value = rand();
		snprintf(temp+strlen(temp),255-strlen(temp),"%d",random_value);
		if ( temp[254] != '\0' )
		{
			// done
			break;
		}
		count++;
	}

	for (i=0; i<length; i++)
	{
		//number = atoi(&temp[i]);
		letter[0] = temp[i];
		letter[1] = temp[i+1];
		letter[2] = '\0';
		number = atoi(letter);
		if ( strlen(g_rand_source_alpha) > (unsigned int) number )
		{
			toload[i] = g_rand_source_alpha[number];
		}
		else
		{
			letter[1] = '\0';
			number = atoi(letter);
			toload[i] = g_rand_source_alpha[number];
		}
	}
	if ( length < buffer_size )
	{
		toload[length]='\0';
	}
	else
	{
		toload[buffer_size-1]='\0';
	}

	return 1;
}

int C_Random_numeric(int length, char * toload, int buffer_size)
{
	int random_value = 0;
	unsigned int srandom_value = 0;
	char temp[256]="";
	int count=0;
	int i;
	int number;
	char letter[2]="";

	if ( toload == NULL )
	{
		return -1;
	}
	
	if ( length <= 0 )
	{
		return -2;
	}
    
	if ( length > 255 )
	{
		return -3;
	}
    
	memset(&temp[0], 0, 256); // important for snprintf below since I don't add '\0' everytime
	
	if ( g_srand_done == 0 )
	{
		srandom_value = (unsigned int) time(NULL);
        srand(srandom_value);
		g_srand_done = 1;
	}
	random_value = rand();

	snprintf(temp,255,"%d",random_value);

	while ( count < 20 )
	{	
		if ( strlen(temp) >= (unsigned int) length )
		{
			break;
		}
		random_value = rand();
		snprintf(temp+strlen(temp),255-strlen(temp),"%d",random_value);
		count++;
	}
	
	if ( strlen(temp) < (unsigned int) length )
	{
		length = strlen(temp);
	}

	for (i=0; i<length; i++)
	{
		letter[0] = temp[i];
		letter[1] = '\0';
		number = atoi(letter);
		if ( number>0 && (unsigned int) number < strlen(g_rand_source_num)  )
		{
			toload[i] = g_rand_source_num[number];
		}
		else
		{
			toload[i] = '0';
		}
	}
	if ( length < buffer_size )
	{
		toload[length]='\0';
	}
	else
	{
		toload[buffer_size-1]='\0';
	}

	return 1;
}

int C_Tolower(char * source)
{
	unsigned int i=0;
	if ( source == NULL )
	{
		return -1;
	}

	for (i=0; i<strlen(source); i++)
	{
		if ( source[i]=='\0' )
		{
			break;
		}

		source[i]=tolower(source[i]);
	}

	return 1;
}

int C_memFind(unsigned char * input, unsigned int inputLength, unsigned char * needle, unsigned int needleLength, unsigned char ** out_position_in_input)
{
	unsigned int inputLengthRemaining = inputLength;
	unsigned int inputOffset = 0;
	int iret;

	if ( input == NULL )
	{
		return -1;
	}
	if ( inputLength == 0 )
	{
		return -2;
	}
	if ( needle == NULL )
	{
		return -3;
	}
	if ( needleLength == 0 )
	{
		return -4;
	}
	if ( inputLength < needleLength )
	{
		return 0;
	}

	while (inputLengthRemaining >= needleLength)
	{
		iret = memcmp(input+inputOffset, needle, needleLength);
		if ( iret == 0 )
		{
			// we have a match
			if ( out_position_in_input )
			{
				*out_position_in_input = input + inputOffset;
			}
			return 1;
		}
		inputOffset += 1;
		inputLengthRemaining -= 1;
	}
	
	return 0;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		 
// C_duplicateString
//				
// Written by : Francois Oligny-Lemieux
//    Created : 17.Apr.2007
//   Modified : 
//
//  Description: 
//    > malloc a new string and copy the input content into it.
//    > WARNING: you need to free yourself the string.
//  
//   input (IN) original string
//   output (OUT) will be assigned the new buffer pointer
//   output_size (OUT) will be filled with the new buffer size
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int C_duplicateString(const char * input, char ** output, unsigned int * output_size)
{
	unsigned int strLen;
	unsigned int ouputSize;
	if ( input == NULL )
	{
		return -1;
	}
	if ( output == NULL )
	{
		return -2;
	}

	*output = NULL;
	if ( output_size )
	{
		*output_size = 0;
	}

	strLen = strlen(input);

	// 25mb in hex is 0x1900000 bytes
	if ( strLen > 0x1900000 )
	{
		return -10; // way to big
	}

	ouputSize = strLen+1;

	*output = malloc(sizeof(char) * ouputSize );
	if ( *output == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	if ( output_size )
	{
		*output_size = ouputSize;
	}

	memcpy(*output, input, strLen);
	(*output)[strLen] = '\0';

	return 1;
}

#if C_TOOLBOX_TIMING == 1
unsigned int C_Timestamp()
{
	unsigned int retval=0;
#if ( defined(_MSC_VER) )
	retval = (unsigned int) timeGetTime();
#else
	struct timeval tv_date;
	mtime_t delay; /* delay in msec, signed to detect errors */

	/* see mdate() about gettimeofday() possible errors */
	gettimeofday( &tv_date, NULL );

	/* calculate delay and check if current date is before wished date */
	delay = (mtime_t) tv_date.tv_sec * 1000000 + (mtime_t) tv_date.tv_usec;
	delay = delay/1000;
	retval = (unsigned int) delay;

#endif
	return retval;
}

#endif // end C_TOOLBOX_TIMING


// written 30.Jul.2007
int C_getCompileDate(char * string, int string_size)
{
	char temp[32];
	char date[32] = __DATE__;
	char * month = NULL;
	char * day = NULL;
	char * year = NULL;
	char * strRet;

	if ( string == NULL )
	{
		return -1;
	}
	if ( string_size < 31 )
	{
		return TOOLBOX_ERROR_YOU_PASSED_A_BUFFER_TOO_SMALL;
	}

	temp[0] = '\0';
	temp[31] = '\0';
	string[0] = '\0';
	string[31] = '\0';
	strRet = strchr(date, ' ');
	month = &date[0];

	if ( strRet )
	{
		*strRet = '\0';
		if ( *(strRet+1) == ' ' )
		{	*(strRet+1) = '0';
		}
		day = strRet+1;
		strRet = strchr(strRet+1, ' ');
		if ( strRet )
		{
			*strRet = '\0';
			year = strRet+1;
		}
	}
	snprintf(string, 31, "%s.%s.%s", day, month, year);
	//sprintf(temp, "Build Date: %s", frank_date);

	return 1;
}




#ifdef TOOLBOX_STANDALONE
// unit tests
// unit tests
// unit tests
// unit tests
// unit tests
// unit tests
// unit tests

int TOOLBOX_TIMEOUT = 1800; //seconds before timeout

configArray HTML_Variables;

int logged;
int successReadingAccounts;

int validaccount;
int g_test_number = 1;

char php_message_buffer[200000] = "";
char php_message_buffer_overrun[256] = "";
//char login_database_filename[256]="g:\\tmp\\login.dat";
//char pre_login_database_filename[256]="g:\\tmp\\pre.login.dat";
//#include "constant_strings.c"
//#include "constant_strings_tasman.c"

const char * const html_sample_string = "</td>\
  </tr>\
</table>\
</form><!-- id_form_main -->\
<script language=\"JavaScript1.1\">\
//refreshBothStatus();\
</script>";


const char * const html_sample_string2 = "  Session created - ID : 1\n<br>";
const char * const html_sample_string3 = "  Session created - ID : 1\r\n<br>";

int test_toolbox_system_file_reader(const char * filename, char * loadme, unsigned int loadme_size);


// structures for self tests

typedef struct msString_S
{	char * buffer;
	unsigned int buffersize;
	uint64_t unique_id;
} msString_T;

typedef struct simpleItem_S
{
	unsigned int number;
} simpleItem_T;

static int linkedlist_self_test_sort_callback_01(void * a, void * b)
{
	unsigned int itemA = (unsigned int)a;
	unsigned int itemB = (unsigned int)b;

	if ( itemA > itemB )
	{
		return 1;
	}

	if ( itemA < itemB )
	{
		return -1;
	}

	return 0;
}


static int linkedlist_self_test_sort_callback(void * a, void * b)
{
	simpleItem_T * itemA = (simpleItem_T*)a;
	simpleItem_T * itemB = (simpleItem_T*)b;

	if ( itemA->number > itemB->number )
	{
		return 1;
	}

	if ( itemA->number < itemB->number )
	{
		return -1;
	}

	return 0;
}

// this one is for the unit test only
// will insert into childs of opaque1 treeItem_T
static int fileEntryCallback(const char *name, const WEB_FILESYSTEM_ENTRY *entry, void * opaque1, void * opaque2)
{
   int iret;
   treeItem_T * newElement = NULL;
   treeItem_T * level = (treeItem_T *)opaque1;
   genericTree_T * tree;

   if (opaque1 == NULL) return -3;

   tree = (genericTree_T *)level->tree;

   printf("name:%s\n", name);
   printf("entry->stuff:" I64u "\n", entry->size);

   iret = genericTree_Insert(tree, level, name, 0, NULL, &newElement);
   if ( iret > 0 && newElement )
   {
      newElement->client = malloc(sizeof(WEB_FILESYSTEM_ENTRY));
      if ( newElement->client == NULL )
      {
         genericTree_Delete(tree, newElement);
         return TOOLBOX_ERROR_MALLOC;
      }

      memcpy(newElement->client, entry, sizeof(WEB_FILESYSTEM_ENTRY));
      newElement->parent = level;

      if ( entry->isDirectory )
      {
         traverseDir(name, fileEntryCallback, newElement, NULL);      
      }
      return 1;
   }

   return -10;


}
   

int test_toolbox_GetFileExtension(const char * filename, const char * expected)
{
   char * result = C_GetFileExtension(filename);
   if (result == NULL && expected != NULL) 
   {
        printf("Failed to get extension for filename(%s), got NULL but expected(%s)\n", filename, expected);
        return -1;
   }
   
   if (result == NULL && expected == NULL) 
   {
        printf("Success get extension for filename(%s), got NULL and expected NULL\n", filename);
        return 1;
   }

   if (strcmp(result, expected) != 0)
   {
        printf("Failed get extension for filename(%s), got (%s) but expected(%s)\n", filename, result, expected);
        return -1;
   }

   printf("Success get extension for filename(%s), got (%s) and expected(%s)\n", filename, result, expected);
   return 1;
}


//#include "login.c"

int main(int argc, char *argv[])
{
	int iret, i;
	int a;
	int errors = 0;
	char temp[512] = "";
	char buffer[1024] = "";
	wchar_t bufferW[1024] = L"";
	char document[256] = "";
	char content_string[256] = "";
	char * char_ptr = NULL;
	char * cookie = NULL;
	char * insider = NULL;
	uint64_t fileSize = 0;
	uint64_t fileSize2 = 0;
	MD5_CTX ctx;
	char password[256] = "manager";
	unsigned char digest[32];
	static const char hex[] = "0123456789abcdef";
	char string_with_tag1[1024] = "frank::manager::HASH(21333213)::OBJ(12333)";
	char string_with_tag2[] = "::manager::HASH(21333213)::OBJ(12333)";
	char string_with_tag3[] = "::manager::HASH(21333213)::OBJ(12333)::";
	char string_with_tag4[] = "::manager";
	char string_with_tag5[] = "lonely";
	char string_with_delimiters[] = "frank::manager,:,HASH(21333213)::OBJ(12333)";
	char preset_filename[] = "configuration.audiovideo.001.txt";
	char * tag_ptr;
	char * out_ptr = NULL;
	int strLen;
	char text_buffer[] = "Unsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nCould not find slot=1) data!\nGroup data Invalid SPI=1/1/4Could not find slot=1) data!\nGroup data Invalid SPI=1/1/5\n";
	wchar_t text_bufferW[] = L"Unsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nCould not find slot=1) data!\nGroup data Invalid SPI=1/1/4Could not find slot=1) data!\nGroup data Invalid SPI=1/1/5\n";
	char bigbuf[25000] = "";
	charArray entries;
	charArray html;
	charArray array;
	charArrayW arrayUTF16;
	charArrayW wideArray;
	textFileReader reader;
	textBufferReader breader;
	lineParser parser;
	charArray * splitted = NULL;
	charArray * array_inside = NULL;
	charArray * array_insider = NULL;
	char phrase[10]="";
	//char toBeEscaped[256]="hello how\'s it \"g\"oing ?&&?@\" well i hope";
	char toBeEscaped[256] = "hello \"\"\" oki";
	char toExtractDoubleQuotes[256] = "-sdp_filename \"\"";
	char * strRet;

	unsigned int timeStart = 0;
	unsigned int timeEnd = 0;
	
   g_verbose = 3;

	iret = lineParser_Constructor(&parser);
	charArray_Constructor(&array,0);

	printf("<pre>\n");

	//SANDBOX BEGINs (experimenting stuff unrelated to TOOLBOX_Toolbox)
#if 0
	TOOLBOX_System("ipconfig", &insider);
	return 0;
#endif
	
#if 0
	phrase[9] = '\0';
	snprintf(phrase, 9, "Very long string that will overflow");
	fputs(phrase, stdout);
	return 0;
#endif
	//SANDBOX ENDs

   iret = test_toolbox_GetFileExtension("test.txt", "txt");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.ts", "ts");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.mp4", "mp4");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.jpeg", "jpeg");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.", "");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test", NULL);
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("a.mp4", "mp4");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("a.b", "b");
   if (iret <= 0) return -1;

#if 1 
   {
      genericTree_T genericTree;
      genericTree_Constructor(&genericTree);
      traverseDir("c:/temp", fileEntryCallback, &genericTree.top, NULL);
      genericTree_Destructor(&genericTree);
   }
#endif

	// buffer_to_array test
#if 1
	errors = 0;
	iret = buffer_to_array(html_sample_string, &array);
	if ( iret <= 0 )
	{
		printf("ERROR - buffer_to_array returned iret(%d)\n", iret);
		errors++;
	}
	if ( array.row_amount != 1 )
	{
		printf("ERROR - buffer_to_array row_amount(%d) != 1\n", array.row_amount);
		errors++;
	}
	if ( errors == 0 )
	{
		printf("SUCCESS - buffer_to_array\n");
	}

	charArray_Zero(&array);
	
	errors = 0;
	iret = buffer_to_array(html_sample_string2, &array);
	if ( iret <= 0 )
	{
		printf("ERROR - buffer_to_array returned iret(%d)\n", iret);
		errors++;
	}
	if ( array.row_amount != 2 )
	{
		printf("ERROR - buffer_to_array row_amount(%d) != 2\n", array.row_amount);
		errors++;
	}
	if ( strcmp(array.array[0][0], "  Session created - ID : 1")!=0 )
	{
		printf("ERROR - buffer_to_array first line didn't match\n");
		errors++;
	}
	if ( strcmp(array.array[1][0], "<br>")!=0 )
	{
		printf("ERROR - buffer_to_array second line didn't match\n");
		errors++;
	}
	if ( array.array[2][0] == NULL || array.array[2][0][0] == '\0' )
	{// ok
	}
	else
	{
		printf("ERROR - buffer_to_array third line has text and shouldn't\n");
		errors++;
	}
	if ( errors == 0 )
	{
		printf("SUCCESS - buffer_to_array on sample 2\n");
	}
	
	charArray_Zero(&array);

	errors = 0;
	iret = buffer_to_array(html_sample_string3, &array);
	if ( iret <= 0 )
	{
		printf("ERROR - buffer_to_array returned iret(%d)\n", iret);
		errors++;
	}
	if ( array.row_amount != 2 )
	{
		printf("ERROR - buffer_to_array row_amount(%d) != 2\n", array.row_amount);
		errors++;
	}
	if ( strcmp(array.array[0][0], "  Session created - ID : 1")!=0 )
	{
		printf("ERROR - buffer_to_array first line didn't match\n");
		errors++;
	}
	if ( strcmp(array.array[1][0], "<br>")!=0 )
	{
		printf("ERROR - buffer_to_array second line didn't match\n");
		errors++;
	}
	if ( array.array[2][0] == NULL || array.array[2][0][0] == '\0' )
	{// ok
	}
	else
	{
		printf("ERROR - buffer_to_array third line has text and shouldn't\n");
		errors++;
	}
	if ( errors == 0 )
	{
		printf("SUCCESS - buffer_to_array on sample 3\n");
	}
	charArray_Destructor(&array);
#endif

	// TextFileReader test 
#if 1
	//iret = TextFileReader_Constructor(&reader, "/home/flemieux/TOOLBOX_ToolBox.SVN/hostname_list.txt");
	charArray_Constructor(&entries,0);
	iret = TextFileReader_Constructor(&reader, "regression_test_data\\nightflight.txt");
	printf("TextFileReader_Constructor iret(%d)\n", iret);
	putTextFileReader_into_charArray(&reader, &entries);
	
	if ( entries.row_amount != 355 )
	{
		printf("ERROR - TextFileReader, row_amount(%d) not 355\n", entries.row_amount);
		charArray_print(&entries);
	}

	i=1;
	if ( entries.array[i-1][0]==NULL || entries.array[0][0][0] != '\0' )
	{	printf("ERROR - TextFileReader, first line do not match\n");errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "-----------------------------------")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "           Entity Begins")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "-----------------------------------")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Name: nightflight00@206.162.164.36:5060")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Index: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Encoder Card: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Encoder Port: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Decoder Card: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Decoder Port: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     ~~~~~ Session/Call ~~~~~~~~~~~~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Call ID: 104b71c0-0-13c4-45014-105d-6241bc51-105d")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Call Index: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Call Status: Terminated")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Local Entity Name: nightflight00@206.162.164.36:5060")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Local Entity isRTSP: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Remote Entity Name: 8888@64.71.156.103")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Remote Entity isRTSP: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Conference started since: not started")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Time since last event: 1020502 ms")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Terminated: 1")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       ~~~~~~ RTP Map Local Begins ~~~~~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Name: MP4A-LATM")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Rate: 32000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpAddress: 206.162.164.36")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpPort: 60000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Name: H264")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Rate: 90000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpAddress: 206.162.164.36")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpPort: 60002")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Profile IDC: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Level IDC: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       ConstraintSet012: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       ~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "very long line with special case \\r\\n not in same buffer adsf lookout:tfrAAA1 adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsfE")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);
		printf("strlen(%d) and should be 1022\n", strlen(entries.array[i-1][0]));errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "~~~~~ RTP Map Remote Begins ~~~~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	
	i=318;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Name: H264")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Rate: 90000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;

	i=353;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "-----------------------------------")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;

	if ( entries.row_amount != 355 )
	{
		printf("ERROR - TextFileReader, row_amount(%d) not 355\n", entries.row_amount);errors++;
	}

	if ( errors == 0 )
	{
		printf("SUCCESS - TextFileReader\n");
	}

	TextFileReader_Destructor(&reader);
	charArray_Destructor(&entries);

	//goto end_of_self_tests;
#endif

	// file_to_array test
#if 1
	charArray_Constructor(&entries,0);
	iret = file_to_array("regression_test_data\\substitution.htm", &entries);
	if ( entries.row_amount != 6 )
	{
		printf("ERROR - substitution.htm, row_amount(%d) not 6\n", entries.row_amount);
		charArray_print(&entries);
	}
	else
	{
		iret = charArray_substituteInLineThatMatches(&entries, "REPLACE_IP_ADDRESS", "127.0.0.1", TOOLBOX_ALL);
		if ( iret <= 0 )
		{		
			printf("ERROR - substitution.htm, charArray_substituteInLineThatMatches returned iret(%d)\n", iret);
		}

		if ( strcmp(entries.array[3][0], "var g_address = \"127.0.0.1\";")==0 )
		{		
			printf("Success - substitution.htm, charArray_substituteInLineThatMatches\n");
		}
		else
		{		
			printf("Error - substitution.htm, charArray_substituteInLineThatMatches\n");
		}
	}
	charArray_Destructor(&entries);
#endif
	

	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00";
		iret = TOOLBOX_utf8strlen(utf8string, 11);
		if ( iret != 4 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test1) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test1)\n");	
		}
	}
	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00\0x46";
		iret = TOOLBOX_utf8strlen(utf8string, 12);
		if ( iret != 4 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test2) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test2)\n");	
		}
	}
	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00\0xC0\0x80";
		iret = TOOLBOX_utf8strlen(utf8string, 13);
		if ( iret != 4 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test3) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test3)\n");	
		}
	}
	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00\0xC0\0x80";
		iret = TOOLBOX_utf8strlen(utf8string+1, 12);
		if ( iret != 3 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test4) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test4)\n");
		}
	}
	{
		const char * utf8string = "\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x00";
		iret = TOOLBOX_utf8strlen(utf8string, 10);
		if ( iret != 9 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test5) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test5)\n");
		}
	}


#if 1
	{// Test of charArrayW
		wchar_t wBuffer01[70000];
		wchar_t wBuffer02[70000];
		
		memset(wBuffer01, 0, sizeof(wBuffer01));
		memset(wBuffer02, 0, sizeof(wBuffer02));
		charArray_ConstructorW(&arrayUTF16, 100);
		iret = file_to_arrayW(L"regression_test_data\\UTF8_sample_01.txt", TOOLBOX_TEXT_ENCODING_UTF8, &arrayUTF16);
		if ( iret >= 1 )
		{
			if ( arrayUTF16.row_amount != 212 )
			{
				printf("ERROR - file_to_arrayW on UTF8_sample_01.txt didn't read correct line amount expected(212) actual(%d)\n", arrayUTF16.row_amount);	
			}
			else
			{
				printf("Success - file_to_arrayW on UTF8_sample_01.txt, line amount expected(212) actual(%d)\n", arrayUTF16.row_amount);
			}
		}
		else
		{
			printf("ERROR - test_toolbox_system_file_reader on UTF8_sample_01.txt failed with iret(%d)\n", iret);
		}
		charArray_to_bufferW(&arrayUTF16, wBuffer01, sizeof(wBuffer01));

		charArray_ConstructorW(&wideArray, 100);

		iret = file_to_arrayW(L"regression_test_data\\UTF16_sample_02 (little endian).txt", TOOLBOX_TEXT_ENCODING_UTF16, &wideArray);
		if ( iret >= 1 )
		{
			if ( wideArray.row_amount != 212 )
			{
				printf("ERROR - file_to_arrayW on UTF16_sample_01 (big endian).txt didn't read correct line amount expected(212) actual(%d)\n", wideArray.row_amount);	 // this one fails with row_amount == 211
			}
			else
			{
				printf("Success - file_to_arrayW on UTF16_sample_01 (big endian).txt, line amount expected(212) actual(%d)\n", wideArray.row_amount);
			}
		}
		else
		{
			printf("ERROR - test_toolbox_system_file_reader on UTF16_sample_01 (big endian).txt failed with iret(%d)\n", iret);
		}
		charArray_to_bufferW(&wideArray, wBuffer02, sizeof(wBuffer02));

		iret = charArray_cmpW(&arrayUTF16, &wideArray);
		if ( iret != 1 )
		{
			printf("ERROR - Failed char array compare of both UTF file. They should be the same\n");
		}

		charArray_DestructorW(&arrayUTF16);
		charArray_DestructorW(&wideArray);

		a = wcslen(wBuffer01);
		i = wcslen(wBuffer02);
		if ( a != i )
		{
			printf("ERROR - Failed UTF8 and UTF16 reading do not give the same length.\n");
		}

		//memset(wBuffer01+1005, 0, sizeof(wBuffer01)-2000);
		//memset(wBuffer02+1005, 0, sizeof(wBuffer02)-2000);
		iret = wcscmp(wBuffer01, wBuffer02);
		if ( iret != 0 )
		{
			printf("ERROR - Failed UTF8 and UTF16 reading differ.\n");
		}
		else
		{
			printf("Success - UTF8 and UTF16 reading produce same results.\n");
		}
	}
	
	{// Test of charArrayW
		wchar_t wBuffer01[70000];
		wchar_t wBuffer02[70000];
		
		memset(wBuffer01, 0, sizeof(wBuffer01));
		memset(wBuffer02, 0, sizeof(wBuffer02));
		charArray_ConstructorW(&arrayUTF16, 3000);
		// test an file that ends with no newline (has caused a bug in the past)
		iret = file_to_arrayW(L"regression_test_data\\object.htt", TOOLBOX_TEXT_ENCODING_UTF8, &arrayUTF16);
		if ( iret >= 1 )
		{
			if ( arrayUTF16.row_amount != 2930 )
			{
				printf("ERROR - file_to_arrayW on object.htt didn't read correct line amount expected(2930) actual(%d)\n", arrayUTF16.row_amount);	
			}
			else
			{
				if ( wcscmp(arrayUTF16.array[2929][0], L"</html>") == 0 )
				{
					printf("Success - file_to_arrayW on object.htt, line amount expected(212) actual(%d)\n", arrayUTF16.row_amount);
				}
				else
				{
					printf("Error - last line was not good\n");
				}
			}
		}
		else
		{
			printf("ERROR - test_toolbox_system_file_reader on UTF8_sample_01.txt failed with iret(%d)\n", iret);
		}
		charArray_to_bufferW(&arrayUTF16, wBuffer01, sizeof(wBuffer01));

		charArray_ConstructorW(&wideArray, 100);

	}
#endif

#if 1 // utf8 to wide oveflow test
	{
		char * original = "one two three";
		wchar_t result[13];
		result[12] = L'A';
		iret = TOOLBOX_utf8ToWide(original, result, sizeof(result));
		if ( iret > 0 )
		{
			if ( result[12] != L'\0' )
			{
				printf("ERROR - TOOLBOX_utf8ToWide didn't terminate buffer.\n");
			}
			else
			{
				printf("Success - TOOLBOX_utf8ToWide\n");
			}
		}
		else
		{
			if ( result[12] != L'\0' )
			{
				printf("ERROR - TOOLBOX_utf8ToWide didn't terminate buffer and failed with iret(%d).\n", iret);
			}
			else
			{
				printf("PASS - TOOLBOX_utf8ToWide failed with iret(%d) but terminated buffer.\n", iret);
			}
		}
	}
#endif


#if 1 // snwprintf() with wchar_t
	{
		wchar_t lead[8] = L"a";
		wchar_t result[8] = L"";
		wchar_t bound[8] = L"b";
		int strlen;

		memset(lead, 4, sizeof(lead));
		memset(bound, 5, sizeof(bound));
		memset(result, 6, sizeof(lead));
		//result[sizeof(result)/2-1] = '\0';
		snwprintf(result, sizeof(result)/2-1, L"this is a very long string that will likely overflow");
		strlen = wcslen(result);
		if ( strlen > 7 ) 
		{
			printf("ERROR - snwprintf() bad usage\n");
		}
		wprintf(result);

	}
#endif

	
#if 1 // snwprintf() with wchar_t
	{
		const char * firstPart = "Calavamous moucho coucho one two three jackie go";
		const char * secondPart = " super long artefact that will reach above 30 char, hopeing it will work out ok at the end";
		const wchar_t * firstPartW = L"Calavamous moucho coucho one two three jackie go";
		const wchar_t * secondPartW = L" super long artefact that will reach above 30 char, hopeing it will work out ok at the end";
		wchar_t expectedResult[1024] = L"";
		unsigned int destination_size = 30;
		wchar_t * destination = malloc(30);

		memset(destination, 0, 30);
		C_AppendUtf8toWide(&destination, &destination_size, firstPart, strlen(firstPart), NULL, NULL);
		C_AppendUtf8toWide(&destination, &destination_size, secondPart, strlen(secondPart), NULL, NULL);

		wcscpy(expectedResult, firstPartW);
		wcscat(expectedResult, secondPartW);

		iret = wcscmp(expectedResult, destination);
		if (iret != 0)
		{
			printf("ERROR - Failed C_AppendUtf8toWide\n");
		}
		else 
		{
			printf("Success - C_AppendUtf8toWide\n");
		}
	}
#endif


	// TOOLBOX_CalculateBroadcast test
#if 1
	temp[0] = '\0';
	iret = TOOLBOX_CalculateBroadcast("10.64.10.66", "255.255.0.0", temp, 256);
	if ( iret != 1 || strcmp(temp,"10.64.255.255")!=0 )
	{
		printf("ERROR - TOOLBOX_CalculateBroadcast failed with output(%s) and iret(%d)\n", temp, iret);
	}
	else
	{
		printf("Success - TOOLBOX_CalculateBroadcast output(%s)\n", temp);
	}
#endif


#if 1
	iret = TOOLBOX_GetDefaultGateway(temp, 256);
	printf("TOOLBOX_GetDefaultGateway returned iret(%d) with gateway(%s)\n", iret, temp);
	//return 0;
#endif

	// lineParser_extractDashedParameter test
#if 1
	iret = lineParser_extractDashedParameter(&parser, toExtractDoubleQuotes, "sdp_filename", &insider);
	if ( iret > 0 && insider )
	{	
		printf("SUCCESS - insider(%s)", insider);
	}
	else
	{
		printf("ERROR - Failed extractDashedParameter [line %d]", g_test_number);
	}
	//return 0;
#endif

#if 0
	charArray_Constructor(&entries,0);
	file_to_array("g:\\tmp\\configuration.output.003.txt", &entries);
	printf("%d", entries.row_amount);	
	charArray_Destructor(&entries);
#endif

	// C_strendstr test
#if 1
	strcpy(temp, "abcdefg.264");
	strRet = C_strendstr(temp,".264");
	if ( strRet == NULL )
	{
		printf("FAILED - C_strendstr returned NULL\n");
	}
	else
	{
		printf("SUCCESS - C_strendstr\n");
	}
	g_test_number++;
#endif

	// charArray_to_buffer
#if 1
	errors = 0;
	memset(bigbuf, 0, sizeof(bigbuf));
	charArray_Constructor(&entries, 1024);
	charArray_add(&entries, ".");
	charArray_add(&entries, "..");
	charArray_add(&entries, "VEncSomething1   : some file .txt");
	charArray_add(&entries, "VEncSomething2   : some file .txt");
	charArray_add(&entries, "VEncSomething3   : some file .txt");
	charArray_add(&entries, "VEncSomething4   : some file .txt");
	iret = charArray_to_buffer(&entries, bigbuf, sizeof(bigbuf));
	if ( iret > 0 )
	{
		strLen = strlen(bigbuf);
		if ( strLen != 141 )
		{
			errors++;
			printf("ERROR - charArray_to_buffer output has strLen(%d)\n", strLen);
		}
		else
		{
			printf("SUCCESS - charArray_to_buffer\n");
		}
	}
	else
	{
		errors++;
		printf("ERROR - charArray_to_buffer returned iret(%d)\n", iret);
	}

	charArray_Destructor(&entries);
	g_test_number++;
#endif
	
	// HTTP parsing algorithm test.
#if 0
	{
		char * strRet;
		char charBackup;
		int value_valid;
		strcpy(temp, "b%20x");
		value_valid = strlen(temp);

		if ( strRet = strstr(temp, "%") )
		{
			if ( strRet+2 <= temp+value_valid )
			{
				charBackup = *(strRet+3);
				*(strRet+3) = '\0';
				iret = TOOLBOX_axtoi(strRet+1);
				*strRet = iret;
				*(strRet+3) = charBackup;
				iret = temp-strRet;
				memmove(strRet+1, strRet+3, (value_valid+1)-(strRet-temp+2) ); // (buffer_size) - (consumed)
				printf(temp);
			}
		}
	}
#endif

#if 0	
	iret = changePassword(account_filename,"admin", NULL, password);

	return 0;
#endif

	// TOOLBOX_GetNetworkInformation test
#if 1
	{
		char ip[256];
		char mac[256];
		char subnet[256];
		char broadcast[256];
		char RX_packets[256];
		char RX_errors[256];
		char RX_bytes[256];
		char TX_packets[256];
		char TX_errors[256];
		char TX_bytes[256];
		char collisions[256];
		TOOLBOX_GetNetworkInformation(ip, subnet, broadcast, mac, RX_packets, RX_errors, RX_bytes, TX_packets, TX_errors, TX_bytes, collisions);
		printf("ip(%s)\n",ip);
		printf("subnet(%s)\n",subnet);
		printf("broadcast(%s)\n",broadcast);
		printf("mac(%s)\n",mac);
		printf("RX_packets(%s)\n",RX_packets);
		printf("RX_errors(%s)\n",RX_errors);
		printf("RX_bytes(%s)\n",RX_bytes);
		printf("TX_packets(%s)\n",TX_packets);
		printf("TX_errors(%s)\n",TX_errors);
		printf("TX_bytes(%s)\n",TX_bytes);
		printf("collisions(%s)\n",collisions);
	}
#endif

	// snprintf usage test against overflow.
#if 0
	snprintf(phrase, 10,"123812390123403214453253424342");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"6");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"7");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"8");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"9");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"0");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");

	strcpy(phrase,"");

	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"6");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"7");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"8");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"9");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"0");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");
#endif 

	// TOOLBOX_GetFileSize test
#if 0
	iret = TOOLBOX_GetFileSize("g:\\videoFiles\\CrazyBoss.mpg", &fileSize);
	printf("fileSize("I64u")\n", fileSize);
	//return 0;
#endif

	// TOOLBOX_isMulticast test
#if 1
	iret = TOOLBOX_isMulticast("24.1.1.3");
	if ( iret > 0 )
	{		
		printf("ERROR TOOLBOX_isMulticast returned iret(%d) on 24.1.1.3\n");
	}
	else
	{
		printf("SUCCESS TOOLBOX_isMulticast\n");
	}
	//return 0;
#endif

	TOOLBOX_Random_numeric(2, &bigbuf[0], 256);
	
	
	// lineParser base tests
	iret = lineParser_extractTag(&parser, string_with_tag1, "HASH", &tag_ptr);
	if ( iret <= 0 )
	{
		printf("extractTag returned iret(%d)\n",iret);
	}
	printf("tag_ptr(%s)\n", tag_ptr);

	//char string_with_tag1[1024]="frank::manager::HASH(21333213)::OBJ(12333)";
	//char string_with_tag2[]="::manager::HASH(21333213)::OBJ(12333)";
	//char string_with_tag3[]="::manager::HASH(21333213)::OBJ(12333)::";
	//char string_with_tag4[]="::manager";
	//char string_with_delimiters[]="frank::manager,:,HASH(21333213)::OBJ(12333)";

	printf("======test case 1======\n");
	iret = lineParser_splitTag(&parser, string_with_tag1, "::", &array_inside);
	if ( array_inside->row_amount != 4 )
	{	printf("ERROR lineParser_splitTag on string_with_tag1\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "frank")!=0 
		  || strcmp(array_inside->array[3][0], "OBJ(12333)")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag1\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag1\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 2======\n");
	iret = lineParser_splitTag(&parser, string_with_tag2, "::", &array_inside);	
	if ( array_inside->row_amount != 4 )
	{	printf("ERROR lineParser_splitTag on string_with_tag2\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "")!=0 
		  || strcmp(array_inside->array[3][0], "OBJ(12333)")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag2\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag2\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 3======\n");
	iret = lineParser_splitTag(&parser, string_with_tag3, "::", &array_inside);
	if ( array_inside->row_amount != 5 )
	{	printf("ERROR lineParser_splitTag on string_with_tag3\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "")!=0 
		  || strcmp(array_inside->array[3][0], "OBJ(12333)")!=0
		  || strcmp(array_inside->array[4][0], "")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag3\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag3\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 4======\n");
	iret = lineParser_splitTag(&parser, string_with_tag4, "::", &array_inside);
	if ( array_inside->row_amount != 2 )
	{	printf("ERROR lineParser_splitTag on string_with_tag4\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "")!=0 
		  || strcmp(array_inside->array[1][0], "manager")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag4\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag4\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 5======\n");
	iret = lineParser_splitTag(&parser, string_with_tag5, "::", &array_inside);
	if ( array_inside->row_amount != 1 )
	{	printf("ERROR lineParser_splitTag on string_with_tag5\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "lonely")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag5\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag5\n");
		}
	}
	charArray_print(array_inside);

	iret = lineParser_replaceTag(&parser, string_with_tag1, "HASH", "13", &insider);
	if ( iret <= 0 )
	{
		printf("replaceTag returned iret(%d)\n",iret);
	}
	if ( insider )
	{
		printf("insider(%s)\n", insider);
	}
	else
	{
		printf("there is no insider from replaceTag\n");
	}


	// lineParser_extractFirstWord test
#if 0
	iret = lineParser_extractFirstWord(&parser, "COMMANDO alwoone2 233\n", &insider);

	if ( iret > 0 && insider )
	{
		printf("command(%s)\n",insider);
	}
	return 1;
#endif

	// file_to_variables benchmark
#if 0
	
	timeStart = TOOLBOX_Timestamp();
	configArray_Constructor(&HTML_Variables, 2000);
	file_to_variables("g:\\tmp\\content_dynamic.dat", &HTML_Variables);

	configArray_Destructor(&HTML_Variables);

	timeEnd = TOOLBOX_Timestamp();

	printf("HTML_Variables amount(%d)\n", HTML_Variables.row_amount );
	printf("done, elapsed(%u)\n", timeEnd-timeStart );

	TOOLBOX_Sleep(2000);

	goto end_of_self_tests;

#endif

	// lineParser_setGreediness test
#if 0
	strcpy(temp,"Fri Sep  8 13:23:42 UTC 2006");
	a = lineParser_setGreediness(1);
	iret = lineParser_splitTag(&parser, temp, " ", &array_insider);
	if ( iret > 0 && array_insider )
	{
		charArray_print(array_insider);
	}

	if ( iret && array_insider && array_insider->row_amount >= 6 )
	{
		snprintf(temp,255,"%s.%s.%s", array_insider->array[2][0], array_insider->array[1][0], array_insider->array[5][0]);
	}
	
	lineParser_setGreediness(a);

	goto end_of_self_tests;
#endif

	// lineParser_setGreediness test 
#if 0
	strcpy(temp,"Fri Sep  8 13:23:42 UTC 2006");
	a = lineParser_setGreediness(0);
	iret = lineParser_splitTag(&parser, temp, " ", &array_insider);
	if ( iret > 0 && array_insider )
	{
		charArray_print(array_insider);
	}

	if ( iret && array_insider && array_insider->row_amount >= 6 )
	{
		snprintf(temp,255,"%s.%s.%s", array_insider->array[2][0], array_insider->array[1][0], array_insider->array[5][0]);
	}
	
	lineParser_setGreediness(a);

	goto end_of_self_tests;
#endif

	// lineParser_replaceAll
#if 0
	iret = lineParser_replaceAll(&parser, toBeEscaped, "\"","\\\"", &insider);
	printf("iret(%d) output(%s)\n",iret,insider);
	
	strcpy(toBeEscaped, insider);
	
	iret = lineParser_replaceAll(&parser, toBeEscaped, "'", "\\'", &insider);
	printf("iret(%d) output(%s)\n",iret,insider);				

	goto end_of_self_tests;

#endif



	// MD5 library usage
	{
		our_MD5Init(&ctx);
		our_MD5Update(&ctx, password, strlen(password) );
		our_MD5Final(digest, &ctx);

		for (i=0; i<32; i++)
		{
			buffer[i*2] = hex[digest[i]>>4];
			buffer[i*2+1] = hex[digest[i]&0x0f];
		}
		buffer[32] = '\0';

		printf("digest(%s)\n",buffer);
	}

	// HTTP_Request test
#if 0
	Network_Initialize();
	
	strcpy(document,"/index.html");
	strcpy(document,"/index.cgi");
	//strcpy(document,"/cgi-bin/tasman.cgi");
	//snprintf(content_string, 255, "nickname=&login=admin&password=manager&action_login=Login");
	snprintf(content_string, 255, "username=admin&password=manager");
	iret = HTTP_Request("10.64.1.141", document, content_string, NULL, GET, &insider);
	//iret = HTTP_Request("10.64.10.66", document, content_string, GET, &insider);
	cookie = NULL;
	if ( iret > 0 && insider )
	{
		textBufferReader reader;
		TextBufferReader_Constructor(&reader, insider, strlen(insider)+1);
			
		iret = TextBufferReader_GetLine(&reader, temp, 255);
		while ( iret > 0 )
		{
			if ( TOOLBOX_strcasestr(temp, "Set-Cookie")
				&& strlen(temp)>12 )
			{
				strcpy(buffer, temp+12);
				cookie = buffer;
				break;
			}			
			iret = TextBufferReader_GetLine(&reader, temp, 255);
		}
		TextBufferReader_Destructor(&reader);
		fputs(insider, stdout);
	}

	strcpy(document,"/decoder.cgi");
	snprintf(content_string, 255, "page=stream&apply=1&streamtype=multicast&streamaddress=228.1.1.44&streamport=4568");
	iret = HTTP_Request("10.64.1.141", document, content_string, cookie, GET, &insider);
	if ( iret > 0 && insider )
	{
		fputs(insider, stdout);
	}

	charArray_Constructor(&html, 100);
	charArray_add(&html, "view=audiovideo");
	charArray_add(&html, "VEncBitrate=2424");
	charArray_add(&html, "action=write_these_config");
	charArray_splitTag(&html, "=");
	//HTTP_Request("10.64.1.191","/main.php", &html, POST);
	iret = HTTP_Request_with_charArray("10.64.2.19","/cgi-bin/tasman.cgi", &html, NULL, POST, &insider);
	charArray_Destructor(&html);
#endif

	
#if 0
	a=0;
	
	charArray_Constructor(&entries, 1024);
	file_to_array("g:\\tmp\\sessions.txt", &entries);

#	if 0
	for (i=0; i<99; i++)
	{
		//iret = charArray_removeLineThatMatchesEx(&entries, "Session ID", a, &a);
		iret = charArray_removeLineThatMatchesEx(&entries, "Activation mode", a, &a);
		iret = charArray_removeLineThatMatches(&entries, "Transfer Type");
		iret = charArray_removeLineThatMatches(&entries, "Card ID");
		iret = charArray_removeLineThatMatches(&entries, "Port ID");
		iret = charArray_removeLineThatMatches(&entries, "Remote Host name");
		iret = charArray_removeLineThatMatches(&entries, "Local Host name");
		iret = charArray_removeLineThatMatches(&entries, "Max TS in Packet");
		iret = charArray_removeLineThatMatches(&entries, "Reordering Time");
		iret = charArray_removeLineThatMatchesEx(&entries, "SSRC", a, &a);
		if ( iret <= 0 )
		{
			break;
		}
	}
#	endif
	charArray_to_file(&entries, "g:\\tmp\\outsessions.txt");
	charArray_Destructor(&entries);

	goto end_of_self_tests;
#endif

#if 0 // test lineParser_extractInsider
	lineParser_extractInsider(&parser, preset_filename, "configuration.audiovideo.", ".txt", &insider);
	printf("insider(%s)\n", insider);
	return 0;
#endif


#if 0 // test lineParser_stripSpacing
	lineParser_extractInsider(&parser, "$ something_is_equal_to = <<<EOF", "$", "=", &insider);
	strcpy(insider, "  ");
	lineParser_strpSpacing(&parser, insider, 1, 1, &insider);
	printf("insider(%s)\n", insider);
	return 0;
#endif

#if 0 // test lineParser_stripSpacing
	lineParser_splitTag(&parser, "10:54:14 up  1:29, load average: 0.74, 0.33, 0.17", " ", &array_insider);
	charArray_print(array_insider);

	lineParser_splitTag(&parser, "Tue Jul 4 11:12:22 UTC 2006", " ", &array_insider);
	charArray_print(array_insider);

	goto end_of_self_tests;
#endif


	// charArray test 01 BEGINs
	charArray_Constructor(&html, 100);
	charArray_add(&html, ".");
	charArray_add(&html, "line two with REPLACE_TAG asdl oo hihihi");
	charArray_add(&html, "..");
	charArray_insert(&html, 2, "new insertion");
	charArray_Zero(&html);
	charArray_add(&html, ".");
	charArray_add(&html, "line two with REPLACE_TAG asdl oo hihihi");
	charArray_add(&html, "..");
	charArray_insert(&html, 2, "new insertion");/**/
	charArray_Destructor(&html);
	// charArray test 01 ENDs

	// charArray test 02 BEGINs
	charArray_Constructor(&html, 100);
	charArray_Constructor(&entries, 100);
	charArray_add(&html, ".");
	charArray_add(&html, "line two with REPLACE_TAG asdl oo hihihi");
	charArray_add(&html, "..");
	charArray_add(&entries, "VERY LONG DUAL REPLACEMENT");
	charArray_add(&entries, "VERY LONG DUAL REPLACEMENT");
	charArray_add(&entries, "VERY LONG DUAL REPLACEMENT");
	charArray_substituteArrayInLineThatMatches(&html, "REPLACE_TAG", &entries);
	charArray_Destructor(&entries);
	charArray_Destructor(&html);
	// charArray test 02 ENDs

	// charArray test 03 BEGINs
	charArray_Constructor(&entries, 1024);
	charArray_add(&entries, ".");
	charArray_add(&entries, "..");
	charArray_add(&entries, "VEncSomething1   : some file .txt");
	charArray_add(&entries, "VEncSomething2   : some file .txt");
	charArray_add(&entries, "VEncSomething3   : some file .txt");
	charArray_add(&entries, "VEncSomething4   : some file .txt");

#if 0 // test realloc
	//g_verbose=1;
	for (i=0; i<2000; i++)
	{
		charArray_add(&entries, text_buffer);
	}

	iret = charArray_substituteArrayInLineThatMatches(&html, "REPLACE_TAG", &entries);

	charArray_to_buffer(&entries, bigbuf, 25000);
#endif

	iret = charArray_getLineThatMatches(&entries, "Video Select|VEncSomething", &insider);
	if ( iret > 0 && insider )
	{
		printf("insider(%s)\n",insider);
	}

	printf("=========before=======\n");
	charArray_cout(&entries);

	printf("=========afterremove=======\n");

	//charArray_removeLineWithIndex(&entries,0);
	charArray_removeLineWithIndexAndAllBefore(&entries,3);

	charArray_cout(&entries);

	printf("=========end=======\n");

	iret = TextFileReader_Constructor(&reader, "/home/flemieux/TOOLBOX_ToolBox.SVN/hostname_list.txt");
	//iret = TextFileReader_Constructor(&reader, "C:\\Tasman\\hostname_list.txt");
	printf("TextFileReader_Constructor iret(%d)\n", iret);

	//iret = TextFileReader_GetLine(&reader, buffer, 1024);
	//printf("TextFileReader_Constructor iret(%d), buffer(%s)\n", iret, buffer);

	putTextFileReader_into_charArray(&reader, &entries);

	iret = TextFileReader_Destructor(&reader);
	printf("TextFileReader_Destructor iret(%d)\n", iret);
	charArray_Destructor(&entries);	
	// charArray test 03 ENDs
	
#if 1
	{
		uint64_t last_member;
		msString_T * string;
		binaryTree_T binaryTree;
		bItem_T * bItem;
		bItem_T * bIterator = NULL;

		binaryTree_Constructor(&binaryTree);
		binaryTree.accept_duplicate_names = 1;
		binaryTree.accept_duplicate_unique_ids = 1;

		for (i=0; i<20; i++)
		{
			string = (msString_T*)malloc(sizeof(msString_T));
			memset(string, 0, sizeof(msString_T));
			C_Append(&string->buffer, &string->buffersize, "itemA", -1, NULL);
			iret = binaryTree_Insert(&binaryTree, string->buffer, 0, string, &bItem);
			if ( iret <= 0 )
			{
				printf("ERROR - binaryTree_Insert returned iret(%d) on string(%s)\n", iret, string->buffer);
			}
		}
	
		for (i=0; i<20; i++)
		{
			string = (msString_T*)malloc(sizeof(msString_T));
			memset(string, 0, sizeof(msString_T));
			C_Append(&string->buffer, &string->buffersize, "itemC", -1, NULL);
			iret = binaryTree_Insert(&binaryTree, string->buffer, 0, string, &bItem);
			if ( iret <= 0 )
			{
				printf("ERROR - binaryTree_Insert returned iret(%d) on string(%s)\n", iret, string->buffer);
			}
		}

		for (i=0; i<20; i++)
		{
			string = (msString_T*)malloc(sizeof(msString_T));
			memset(string, 0, sizeof(msString_T));
			C_Append(&string->buffer, &string->buffersize, "itemB", -1, NULL);
			iret = binaryTree_Insert(&binaryTree, string->buffer, 0, string, &bItem);
			if ( iret <= 0 )
			{
				printf("ERROR - binaryTree_Insert returned iret(%d) on string(%s)\n", iret, string->buffer);
			}
		}

		i = 0;
		string = NULL;
		iret = binaryTree_FirstItem(&binaryTree, &string, &bItem, &bIterator);
		while ( iret > 0 && string )
		{
			printf("item:%s\n", string->buffer);

			i++;

			string = NULL;
			iret = binaryTree_NextItem(&binaryTree, &string, &bItem, &bIterator);
		}
		
		i = 0;
		string = NULL;
		iret = binaryTree_FirstItem(&binaryTree, &string, &bItem, &bIterator);
		while ( iret > 0 && string )
		{
			printf("item:%s\n", string->buffer);

			if ( i == 25 )
			{
				break;
			}
			
			i++;

			string = NULL;
			iret = binaryTree_NextItem(&binaryTree, &string, &bItem, &bIterator);
		}

		string = (msString_T*) bItem->client;
		last_member = bItem->unique_id;
		iret = binaryTree_Delete(&binaryTree, bItem);

		iret = binaryTree_Insert(&binaryTree, "dude", last_member, string, &bItem);

		last_member = 0;
		i = 0;
		string = NULL;
		iret = binaryTree_FirstiItem(&binaryTree, &string, &bItem, &bIterator);
		while ( iret > 0 && string )
		{
			printf("item:%s\n", string->buffer);

			if ( last_member > bItem->unique_id )
			{
				printf("ERROR - binaryTree got corrupted\n");
				__asm int 3;
				break;
			}
			last_member = bItem->unique_id;

			string = NULL;
			iret = binaryTree_NextiItem(&binaryTree, &string, &bItem, &bIterator);
		}

		binaryTree_Destructor(&binaryTree);
	}
#endif

	iret = charArray_getLineThatMatches(&entries, "machine_ip::10.64.1.131", &out_ptr);
	if ( iret && out_ptr )
	{
		lineParser_splitTag(&parser, out_ptr, ",:,", &splitted);
		printf("======splitted======\n");
		charArray_print(splitted);
		printf("====================\n");
	}

	charArray_splitTag(splitted, "::");

	printf("======re-splitted======\n");
	charArray_print(splitted);
	printf("====================\n");

	charArray_qsort(splitted, 1, 0, 0, -2);
	printf("======sorted======\n");
	charArray_print(splitted);
	printf("====================\n");

	{
		TextBufferReader_ConstructorEx(&breader, text_buffer, strlen(text_buffer), 1);

		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);

		TextBufferReader_Destructor(&breader);
	}

	{
		TextBufferReader_ConstructorExW(&breaderW, text_bufferW, wcslen(text_bufferW), 1);

		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);

		TextBufferReader_DestructorW(&breaderW);
	}

	// C XML
#if 1
	if (0) 
	{
		xmlParser_T xml;
		xmlElement_T * element = NULL;
		xmlElement_T * element2 = NULL;
		xmlIterator_T iterator;
		int success = 0;
		iret = xml_Constructor_fromFile(&xml, "regression_test_data\\xml_test_alternate_text_tag.txt");
		if ( iret > 0 )
		{
			iret = xml_getFirstElement(&xml.elements, "p", &element, &iterator);
			if ( iret > 0 && element )
			{
				// WARNING incomplete test
				iret = xml_getFirstElement(element->childs, "text", &element2, &iterator);
				if ( iret > 0 && element2 )
				{
					if ( element2->value && strcmp(element2->value, "hello how are you  cool") == 0 )
					{
						// ok
						printf("SUCCESS - XML parsing of alternate text n' tags\n");
						success = 1;
					}
				}
			}
		}
		if ( success == 0 )
		{
			printf("ERROR - XML parsing of alternate text n' tags\n");
		}
		xml_Destructor(&xml);
	}
	if (0)
	{
		xmlParser_T xml;
		xmlElement_T * element = NULL;
		xmlElement_T * element2 = NULL;
		xmlIterator_T iterator;
		int success = 0;
		iret = xml_Constructor_fromFile(&xml, "regression_test_data\\xml_from_odt_01.txt");
		if ( iret > 0 )
		{
			iret = xml_getFirstElement(&xml.elements, "p", &element, &iterator);
			if ( iret > 0 && element )
			{
				iret = xml_getFirstElement(element->childs, "text", &element2, &iterator);
				if ( iret > 0 && element2 )
				{
					if ( element2->value && strcmp(element2->value, "hello how are you  cool") == 0 )
					{
						// ok
						printf("SUCCESS - XML parsing of alternate text n' tags\n");
						success = 1;
					}
				}
			}
		}
		if ( success == 0 )
		{
			printf("ERROR - XML parsing of alternate text n' tags\n");
		}
		xml_Destructor(&xml);
	}
	if (1)
	{
		xmlParser_T xml;
		xmlElement_T * element = NULL;
		xmlElement_T * element2 = NULL;
		xmlIterator_T iterator;
		int success = 0;
		iret = xml_Constructor_fromFile(&xml, "regression_test_data\\xml_from_odt_02.txt");
		if ( iret > 0 )
		{
			iret = xml_getFirstElement(&xml.elements, "text:p", &element, &iterator);
			if ( iret > 0 && element )
			{
				iret = xml_getFirstElement(element->childs, "text:span", &element2, &iterator);
				if ( iret > 0 && element2 )
				{
					if ( element2->value && strcmp(element2->value, "hello how are you  cool") == 0 )
					{
						// ok
						printf("SUCCESS - XML parsing of alternate text n' tags\n");
						success = 1;
					}
				}
			}
		}
		if ( success == 0 )
		{
			printf("ERROR - XML parsing of alternate text n' tags\n");
		}
		xml_Destructor(&xml);
	}
#endif


#if 1 // linkedList TESTs
	{
		linkedList myList;
		simpleItem_T simpleItem[200];
		simpleItem_T * curItem;
		unsigned int lastItem = 0;

		memset(&simpleItem, 0, sizeof(simpleItem));

		linkedList_Constructor(&myList);
		
		for(i=0; i<10; i++)
		{
			simpleItem[i].number = 10-i;
			linkedList_Push(&myList, 10-i); //warning ok
		}
		linkedList_qsort(&myList, linkedlist_self_test_sort_callback_01, 0, 1, -2);

		i = 0;
		errors = 0;
		curItem = NULL;
		iret = linkedList_FirstItem(&myList, &curItem);
		while ( iret > 0 )
		{
			//printf("curItem(%d) = %d\n", i, (unsigned int)curItem);

			if ( lastItem > (unsigned int)curItem )
			{
				printf("ERROR - linkedList, first test sorting assending\n");
				errors++;
				break;
			}
			lastItem = (unsigned int)curItem;
			curItem = NULL;
			iret = linkedList_NextItem(&myList, &curItem);
			i++;
		}
		if ( myList.itemAmount != 10 )
		{	
			printf("ERROR - linkedList, first test has items missing after sort\n");			
		}
		else if ( errors == 0 )
		{
			printf("Success - linkedList, first test\n");
		}

		linkedList_Destructor(&myList);
		linkedList_Constructor(&myList);

		// linkedList TEST 2
		for(i=0; i<200; i++)
		{
			simpleItem[i].number = 200-i;
			linkedList_Push(&myList, &simpleItem[i]);
		}
		linkedList_qsort(&myList, linkedlist_self_test_sort_callback, 0, 1, -2);

		i = 0;
		errors = 0;
		lastItem = 0;
		curItem = NULL;
		iret = linkedList_FirstItem(&myList, &curItem);
		while ( iret > 0 && curItem )
		{
			//printf("curItem(%d) = %d\n", i, curItem->number);
			if ( lastItem >= curItem->number )
			{
				printf("ERROR - linkedList, second test sorting assending\n");
				errors++;
				break;
			}
			lastItem = curItem->number;
			curItem = NULL;
			iret = linkedList_NextItem(&myList, &curItem);
			i++;
		}
		if ( myList.itemAmount != 200 )
		{	
			printf("ERROR - linkedList, second test has items missing after sort\n");			
		}
		else if ( errors == 0 )
		{
			printf("Success - linkedList, second test\n");
		}

		linkedList_Destructor(&myList);
	}
#endif // linkedList TESTs
	
	
end_of_self_tests:
	iret = lineParser_Destructor(&parser);

	printf("finished.\n");
	printf("</pre>\n");

	return 0;

	TOOLBOX_Logger("(startup.c) main(), Starting up.\n");
	TOOLBOX_Logger("   version 2.340, timeout 1.5 sec, retry_max 3\n");

#ifdef TOOLBOX_LOGGER
	if ( g_toolbox_log > 0 )
	{
		fclose(g_toolbox_log);
		g_toolbox_log=NULL;
	}
#endif

	return 0;
}



int test_toolbox_system_file_reader(const char * filename, char * loadme, unsigned int loadme_size)
{
	int fret = 1;
	int size_read = 0;
	unsigned int result_written = 0;
	unsigned int read_size = 24;
	FILE * file;

	file = fopen(filename, "r");

	if ( file == NULL )
	{
		return -13;
	}

	if ( loadme_size-1-result_written < read_size )
	{	read_size = loadme_size-1-result_written;
	}
	size_read = fread(loadme+result_written, 1, read_size, file);
	//printf("after initial fgets, charRet(0x%X) and feof=%d\n", (unsigned int)charRet, feof(outf));
	while ( size_read > 0 )
	{
		result_written += size_read;
		*(loadme+result_written) = '\0';

		if ( loadme_size-1-result_written < read_size )
		{	read_size = loadme_size-1-result_written;
		}

		if ( read_size == 0 )
		{
			fret=-14; // buffer is too small
			break;
		}
		
		size_read = fread(loadme+result_written, 1, read_size, file);	
	}
	fclose(file);

	return fret;
}



#endif// C_TOOLBOX_STANDALONE

