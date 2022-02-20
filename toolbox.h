//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox.h
//
// Copyright (c) 2006 Francois Oligny-Lemieux
// All rights reserved
//
//        Author : Francois Oligny-Lemieux
//       Created : 15.May.2006
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//  C Toolbox README
//  
//  Description:
//      Written by Francois Oligny-Lemieux in order to have robust
//      array and string manipulation functions in C similar to 
//      the ones found in PHP.
// 
//  Installation:
//      Declare some global variables that toolbox_toolbox will use:
//          char php_message_buffer[200000]="";
//          int g_verbose=0;
//             // 0 is silent
//             // 1 is errors
//             // 2 is warnings
//             // 3 is everything
//      Include toolbox.h
//      If you are under Windows, add these library input:
//          ws2_32.lib winmm.lib
//      Modify toolbox_config.h to add and remove features. For
//          example you might want C_TOOLBOX_LOGIN = 0 
//      
//   Documentation:
//      If you want your int64 printf portable, replace all %I64d, %I64u, %I64X, %I64x, %lld, %llu, %llx, %llX with "I64d", "I64u", ..., "llX"
//      Example, replace:
//           printf("value:%I64d\n", value);
//           printf("%I64d", value);
//      with:
//           printf("value:"I64d"\n", value);
//           printf(I64d, value);
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __C_TOOLBOX_H__
#define __C_TOOLBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "toolbox-flexstring.h"
#include "toolbox-config.h"
#include "toolbox-filesystem.h"

#if C_TOOLBOX_CHARARRAY == 1
#include "toolbox-line-parser.h"
#include "toolbox-char-array.h"
#include "toolbox-text-buffer-reader.h"
typedef struct configArray
{
	int row_amount;
	int row_amount_max;
	int index; // current index
	charArray ** array;
	int cell_length;
} configArray;
#endif 

   
#if C_TOOLBOX_NETWORK == 1 
#	include "toolbox_network.h"
#	if C_TOOLBOX_HTTP_CLIENT == 1
#		include "http_client.h"
#	endif
#	if C_TOOLBOX_SOCKET_SERVER == 1
#		include "socket_server.h"
#	endif
#	if C_TOOLBOX_SOCKET_CLIENT == 1
#		include "socket_client.h"
#	endif
#endif
#if C_TOOLBOX_THREAD_AND_MUTEX == 1
#	include "toolbox_mutex.h"
#	include "toolbox_thread.h"
#endif
#if C_TOOLBOX_CONFIG_INI_MANAGER == 1
#	include "config_ini_manager.h"
#endif
#if C_TOOLBOX_LINKED_LIST == 1
#	include "toolbox_linkedlist.h"
#endif
#if C_TOOLBOX_CONFIG_MD5 == 1
#	include "toolbox_md5.h"
#endif
#if C_TOOLBOX_XML == 1
#	include "toolbox_xml.h"
#endif
#if C_TOOLBOX_BINARY_TREE == 1
#	include "toolbox_btree.h"
#endif
#if C_TOOLBOX_GENERIC_TREE == 1
#	include "toolbox-tree.h"
#endif

typedef enum gnucFlags
{
	TOOLBOX_OVERWRITE_DESTINATION = 1
} gnucFlags_E;

typedef enum gnucEnums
{
	TOOLBOX_CRLF = 1,
	TOOLBOX_LF
} gnucEnums_E;


#define CONFIG_ARRAY_ROW_AMOUNT 100
#define TOOLBOX_DEBUG_LEVEL 0


int TOOLBOX_endSlashDirectory(char * directory_inout); // prepare directory to have filename appended

int wchar_extract_path(const wchar_t * source, wchar_t * destination, int destination_size);
int wchar_extract_filename(const wchar_t * source, wchar_t * destination, int destination_size);


// Wide (wchar_t) is UTF-16
int TOOLBOX_charToWide(const char * original, wchar_t * utf16, unsigned int utf16_size);
int TOOLBOX_utf8ToWide(const char * utf8, wchar_t * utf16, unsigned int utf16_size);
int TOOLBOX_utf8ToWideAlloc(const char * utf8, wchar_t ** utf16, unsigned int * utf16_size);
int TOOLBOX_utf16ToChar(const wchar_t * utf16, char * writeme, unsigned int writeme_size);
int TOOLBOX_utf16ToUtf8(const wchar_t * utf16, char * writeme, unsigned int writeme_size);

// convert &quot; to " and more
int TOOLBOX_DecodeXmlTextBlobs(char * source, char ** temp, unsigned int * temp_size);

// UTF-8 toolbox :)
// utf8_buffer_size can be -1 if you don't know.. but string MUST be terminated.
int TOOLBOX_utf8strlen(const char * utf8, int utf8_buffer_size);
char * TOOLBOX_utf8EndOfString(const char * utf8, int utf8_buffer_size);

// will realloc the string if too small, or malloc it if NULL
// separator is optional and can be NULL
// other parameters are mandatory
int C_Append(char ** string, unsigned int * buffersize, const char * append, int max_length, const char * separator);
int C_AppendUtf8toWide(wchar_t ** string, unsigned int * buffersize, const char * append, int max_length, const wchar_t * separator, unsigned int * appendedLength);

#if C_TOOLBOX_LOG_COMMAND == 1
extern FILE * g_toolbox_toolbox_log_file;
#endif

int C_itoa( unsigned int i, char * toLoad, unsigned int toLoad_size );
int C_axtoi(const char * hex);
#if defined(_MSC_VER)
#	if defined(atoi64)
#		define TOOLBOX_atoi64(a) atoi64(a)
#		define TOOLBOX_atoi64W(a) wtoi64(a)
#		define TOOLBOX_atoiW(a) wtoi(a)
#	else
#		define TOOLBOX_atoi64(a) _atoi64(a)
#		define TOOLBOX_atoi64W(a) _wtoi64(a)
#		define TOOLBOX_atoiW(a) _wtoi(a)
#	endif
#else
#	define TOOLBOX_atoi64(a) atoll(a)
#endif
char* C_strreplace(char * source, const char token, const char replacement);
int C_strncpy(char * destination, const char * source, int dst_max_length);

char * C_strcasestr(const char * string , const char * needle);
// C_strncasestr :: string_length is to allow an unterminated and/or binary data string.
// however needle must be terminated with '\0'
char * C_strncasestr(const char * string, int string_length, const char * needle);
char * C_strendstr(const char * string, const char * needle);
char * C_striendstr(const char * string, const char * needle);
//C_strlen == binary-safe string length
int C_strlen(const char * string, int string_buffer_size);
const char * C_eos(const char * string);
void * C_memfind(const unsigned char * buffer, int buffer_length, const unsigned char * needle, int needle_length);


// filesystem functions
int C_System(const char * command, char ** insider, int * status);
int C_System2(const char * command, char * loadme, unsigned int loadme_size, int * status);
int C_FileTouch(const char * filename);
int C_FileExists(const char * filename);
int C_isFileOpened(const char * filename);
int C_GetFileSize(const char * filename, uint64_t * loadme);
int C_GetFileCreationTime(const char * filename, uint64_t * loadme);
const char * C_GetFileExtension(const char * filename);
int C_DirectoryExists(const char * directory);
int C_CreateDirectory(const char * directory);
int C_DeleteFile(const char * filename);
int C_CopyFile(const char * source, const char * destination);
int C_CopyFileEx(const char * source, const char * destination, int flags); // flags are from gnucFlags_E
int C_MoveFile(const char * source, const char * destination); // will overwrite destination
int C_MoveFileEx(const char * source, const char * destination, int flags); // flags are from gnucFlags_E


#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
// Returns 0 if not found and which_interface was provided
// Set "which_interface" to NULL or empty string to not specify any interface
// otherwise will pickup first interface (normally first wired interface)
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
							   const char * which_interface );
#endif

#if defined(__TOOLBOX_NETWORK_H__)
int C_GetHostname(char * hostname, int hostname_bufsize);
#endif

#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
int C_GetDefaultGateway(char * gateway, int gateway_bufsize);
#endif

// timing functions
int C_Sleep(int milliseconds);
//int C_usleep(int microseconds);

// basic string manipulation functions
int C_Random_alpha(int length, char * toload, int buffer_size);
int C_Random_numeric(int length, char * toload, int buffer_size);
int C_Tolower(char * source);
int C_memFind(unsigned char * input, unsigned int inputLength, unsigned char * needle, unsigned int needleLength, unsigned char ** out_position_in_input);
int C_duplicateString(const char * input, char ** output, unsigned int * output_size);

#if C_TOOLBOX_TIMING == 1
unsigned int C_Timestamp();
int C_Timestring(char * loadme, unsigned int loadme_size);
int C_TimestringW(wchar_t * loadme, unsigned int loadme_size);
// year with four letters
// month with three letters (first one capitalized)
// day numeric with 0 padding.
int C_epoch2date(int64_t epoch, char * year, char * month, char * month_digit, char * day, int * hours, int * minutes, int * seconds);
int C_epoch2dateW(int64_t epoch, wchar_t * year, wchar_t * month, wchar_t * month_digit, wchar_t * day, int * hours, int * minutes, int * seconds);
int C_dateToEpoch(const char * date, uint64_t * epoch);
#endif

int C_getCompileDate(char * string, int string_size);

int C_GetTempFilename(const char * path, char * out_filename);

int C_isDigit(const char * string);
int C_isMulticast(const char * string);

int C_hexToString(unsigned char * hex, unsigned int length, char * loadme, unsigned int loadme_length);



#ifdef __cplusplus
}
#endif

#endif

