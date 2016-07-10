//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-text-buffer-reader.h
// 
// Copyright (c) 2006-2010 Francois Oligny-Lemieux
// All rights reserved
//
//       Created : 15.May.2006
//      Modified : 27.Sep.2010 (wide-string version)
//
//  Description: 
//      from a char * buffer, offer a getLine function
//      handles correctly windows and linux end-of-line
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __C_TOOLBOX_TEXT_BUFFER_READER_H__
#define __C_TOOLBOX_TEXT_BUFFER_READER_H__

#include "toolbox-basic-types.h"
#include "toolbox-errors.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#define TEXT_BUFFER_READER_MAX_LINE_SIZE 1024

typedef struct textBufferReader
{
	char * buffer;
	int buffer_size; // this is the malloc'ed size of m_buffer
	int internal_buffer;

	int offset; // points to the next unreaded character.
	int chop_empty_lines;
#if USE_MUTEX == 1
	Mutex mutex;
#endif	
} textBufferReader;


int TextBufferReader_Constructor(textBufferReader * reader, const char * const buffer, int buffer_size );
int TextBufferReader_ConstructorEx(textBufferReader * reader, char * const buffer, int buffer_size, int make_your_own_copy_of_buffer);
int TextBufferReader_Destructor(textBufferReader * reader);
int TextBufferReader_GetLine(textBufferReader * reader, char * out_value, int max_size);



#ifdef __cplusplus
}
#endif

#endif //__C_TOOLBOX_TEXT_BUFFER_READER_H__
