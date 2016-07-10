//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-text-buffer-reader.c
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
// All rights reserved
//
//       Created : 15.May.2006
//      Modified : 12.Mar.2007 (changed m_buffer_size meaning)
//
//  Description: 
//      from a char * buffer, offer a getLine function
//      handles correctly windows and linux end-of-line
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define USE_MUTEX 0

#include "toolbox-text-buffer-reader.h"
#include "toolbox.h" // for C_strlen only

#define TEXT_BUFFER_READER_MAX_LINE_SIZE 1024

int TextBufferReader_Constructor(textBufferReader * reader, const char * const buffer, int buffer_size)
{
	int buffer_valid = 0;

	if ( reader == 0 )
	{
		return -1;
	}
	if ( buffer == 0 )
	{
		return -2;
	}

	reader->buffer = NULL;
	reader->buffer_size = 0;
	reader->offset = 0;
	reader->chop_empty_lines = 1;
	
	buffer_valid = C_strlen(buffer, buffer_size);
	if ( buffer_valid <= -1 )
	{	buffer_valid = buffer_size;
	}
	else
	{	buffer_valid += 1;
	}

	reader->buffer = (char*) malloc ( sizeof(char) * buffer_valid + 16 );
	if ( reader->buffer == 0 )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	memcpy(reader->buffer, buffer, buffer_valid);
	reader->buffer[buffer_valid-1] = '\0';
	memset(reader->buffer+buffer_valid, 0x44, 16);
	reader->internal_buffer = 1;
	reader->buffer_size = buffer_valid;
	
	return 1;
}

// to optimize, same buffer can be used for parsing.
int TextBufferReader_ConstructorEx(textBufferReader * reader, char * const buffer, int buffer_size, int make_your_own_copy_of_buffer)
{
	int buffer_valid = 0;

	if ( reader == 0 )
	{
		return -1;
	}
	if ( buffer == 0 )
	{
		return -2;
	}

	reader->buffer = NULL;
	reader->buffer_size = 0;
	reader->offset = 0;
	
	buffer_valid = C_strlen(buffer, buffer_size);
	if ( buffer_valid <= -1 )
	{	buffer_valid = buffer_size;
	}
	else
	{	buffer_valid += 1;
	}

	if ( make_your_own_copy_of_buffer > 0 )
	{
		reader->buffer = (char*) malloc ( sizeof(char) * buffer_valid + 16 );
		if ( reader->buffer == 0 )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		memcpy(reader->buffer, buffer, buffer_valid);
		reader->buffer[buffer_valid-1] = '\0';
		memset(reader->buffer+buffer_valid, 0x44, 16);
		reader->internal_buffer = 1;
		reader->buffer_size = buffer_valid;
	}
	else 
	{
		reader->buffer = buffer;
		reader->internal_buffer = 0;
		if (C_strlen(buffer, buffer_size) <= 0)
		{
			// ouch we are heading toward madness
			// this look like an unterminated string
			// let's use our own buffer ?
			// or modify user buffer ?

			// decision: abort 
			reader->buffer_size = 0;
			reader->buffer = NULL;
			return -10;
		}
		reader->buffer_size = buffer_valid;
	}

	return 1;
}


int TextBufferReader_Destructor(textBufferReader * reader)
{	
	if ( reader == 0 )
	{
		return -1;
	}

	if ( reader->buffer ) 
	{
		if ( reader->internal_buffer )
		{
			free(reader->buffer);
			reader->buffer = NULL;
		}
		else
		{		
			reader->buffer = NULL;
		}
	}

	reader->buffer_size = 0;

	return 1;
}


int TextBufferReader_GetLine(textBufferReader * reader, char * out_value, int max_size)
{
	char buffer[TEXT_BUFFER_READER_MAX_LINE_SIZE];	
	char * const buffer_ptr = &buffer[0];
	const char * const buffer_ptr_end = &buffer[TEXT_BUFFER_READER_MAX_LINE_SIZE-1];
	char * buffer_ptr_end_temp = &buffer[TEXT_BUFFER_READER_MAX_LINE_SIZE-1]; // the ptr to last byte.
	char * buffer_ptr_temp = &buffer[0];

	char * str_ret;
	int fret;
	int length;
	int memcpy_size=0;
	int skipped=0;

	int out_value_written = 0;

	if ( reader == 0 )
	{
		return -1;
	}
	if ( reader->buffer == 0 )
	{
		// failed to initialize m_file
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( max_size == 0 )
	{
		return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
	}
	if ( out_value == 0 )
	{
		return -2;
	}

	fret = 1;

#if USE_MUTEX == 1
	reader->mutex.Acquire();
#endif

	memcpy_size = TEXT_BUFFER_READER_MAX_LINE_SIZE-1;
	if ( memcpy_size >= reader->buffer_size - reader->offset )
	{
		memcpy_size = reader->buffer_size - reader->offset - 1; // -1 is to not copy \0
	}
	if ( memcpy_size < 0 )
	{
		memcpy_size = 0;
	}
	memcpy(buffer_ptr, reader->buffer + reader->offset, memcpy_size);
	buffer_ptr[memcpy_size] = '\0';
	buffer_ptr_temp = buffer_ptr;
	buffer_ptr_end_temp = buffer_ptr + memcpy_size; // points to '\0'

	if ( reader->buffer_size <= 0  )
	{
		out_value[0] =  '\0';
		fret = 0;
		goto end_of_getline;
	}
	if ( memcpy_size <= 0 )
	{
		out_value[0] =  '\0';
		fret = -10;
		goto end_of_getline;
	}

continue_seeking_data:;
	str_ret = strchr(buffer_ptr_temp, '\n');
	if ( str_ret )
	{
		length = (int) (str_ret - buffer_ptr_temp);
		skipped += length;
		if (    length == 0 
			|| ( length == 1 && *(str_ret-1) == '\r' ) 
			)
		{
			// empty line
			skipped -= length;
			skipped++;
			buffer_ptr_temp++;
			if ( buffer_ptr_temp < buffer_ptr_end && buffer_ptr_temp < buffer_ptr_end_temp )
			{
				// ok
				if ( reader->chop_empty_lines )
				{	goto continue_seeking_data;
				}
			}
			else
			{
				// fail
#if USE_MUTEX == 1
				reader->mutex.Release();
#endif
				return -21;
				// weird error, probably 1024 empty lines ??
			}
		}
		else
		{
			skipped++; // for endline
		}
		if ( out_value_written + length >= max_size )
		{
			// error
			fret = TOOLBOX_ERROR_BUFFER_TOO_SMALL;
			goto end_of_getline;
		}
		strncpy( out_value, buffer_ptr_temp, length );
		out_value_written += length;
		if ( out_value[length-1] == '\r' )
		{
			out_value[length-1] = '\0';
		}
		out_value[length] = '\0';
		reader->offset += (int) skipped;
	}
	else
	{
		// no \n matched
		// end of buffer ? or very long line
		length = strlen(buffer_ptr_temp);
		if ( length < 0 )
		{
			// error strlen bigger than INT_MAX
			fret = -16;
		}
		else
		{
			if ( out_value_written + length >= max_size )
			{
				// WARNING user buffer too small
				strncpy(out_value, buffer_ptr_temp, max_size-out_value_written-1 );
				out_value_written += (int) (max_size-out_value_written-1);
				out_value[buffer_ptr_end_temp - buffer_ptr_temp] = '\0';
				reader->offset = reader->buffer_size-1;
				fret = TOOLBOX_ERROR_BUFFER_TOO_SMALL; // normal thing to do for caller is to consider it was \n and continue reading, or increase its buffer and continue reading.
			}
			else
			{
				strcpy(out_value, buffer_ptr_temp);
				out_value[length] = '\0';
				out_value_written += length;
			
				if ( length + reader->offset + 1 == reader->buffer_size )// +1 is for '\0'
				{
					//end of buffer
					//success
					reader->offset += length;
				}
				else
				{
                   // append and continue to work
					if ( buffer_ptr_temp[length-1] == '\r' )
					{
						length--; // save \r for later
					}
					out_value[length] = '\0';
					reader->offset += length;
					fret = TOOLBOX_WARNING_CONTINUE_READING;
				}
			}	
		}
	}

end_of_getline:;

#if USE_MUTEX == 1
	reader->mutex.Release();
#endif

	return fret;
}

