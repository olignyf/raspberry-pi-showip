//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// text-file-reader.h
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
//				
//        Author : Francois Oligny-Lemieux
//       Created : 15.May.2006
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __C_TOOLBOX_TEXT_FILE_READER_H__
#define __C_TOOLBOX_TEXT_FILE_READER_H__

#include "toolbox-basic-types.h"
#include "toolbox-errors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE 1024

enum gnucTextEncoding_E
{
	TOOLBOX_TEXT_ENCODING_ASCII = 0,
	TOOLBOX_TEXT_ENCODING_AUTO_DETECT = 1,
	TOOLBOX_TEXT_ENCODING_UTF8 = 2,
	TOOLBOX_TEXT_ENCODING_UTF16 = 3
};


typedef struct textFileReader
{
	FILE * m_file;
	int64_t m_offset;
	char * m_buffer;
	unsigned int m_buffer_size;
	unsigned int m_buffer_valid;
	unsigned int m_buffer_offset; // in 
	unsigned int m_buffer_save; // in char* unit, means buffer amount (at the end) that needs to be memmove'd to the begining before next read.
#if USE_MUTEX == 1
	Mutex m_mutex;
#endif
} textFileReader;

typedef struct textFileReaderW
{
	FILE * m_file;
	int64_t m_offset;
	wchar_t * m_buffer;
	//unsigned int m_buffer_size;
	unsigned int m_buffer_length; // in unit of string length of wchar_t
	unsigned int m_buffer_valid; // in unit of string length of wchar_t
	unsigned int m_buffer_offset; // in unit of string length of wchar_t
	unsigned int m_buffer_save; // means buffer amount (at the end) that needs to be memmove'd to the begining before next read.
	int textEncoding; 
	int littleEndian;
#if USE_MUTEX == 1
	Mutex m_mutex;
#endif
} textFileReaderW;

int TextFileReader_Constructor(textFileReader * reader, const char * const filename);
int TextFileReader_Destructor(textFileReader * reader);
// old int TextFileReader_GetLine(textFileReader * reader, char * out_value, int * out_read_length, int max_size);
int TextFileReader_GetLine(textFileReader * reader, char ** buffer, unsigned int * buffersize, unsigned int * out_read_length, int realloc_if_necessary);


// textEncoding should be among gnucTextEncoding_E
int TextFileReader_ConstructorW(textFileReaderW * reader, const wchar_t * const filename, int textEncoding);
int TextFileReader_DestructorW(textFileReaderW * reader);
int TextFileReader_GetLineW(textFileReaderW * reader, wchar_t ** buffer, unsigned int * buffersize, unsigned int * out_read_length, int realloc_if_necessary);



#ifdef __cplusplus
}
#endif

#endif //__C_TOOLBOX_TEXT_FILE_READER_H__
