//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// text-file-reader.c
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
//
//        Author : Francois Oligny-Lemieux
//       Created : 15.May.2006
//      Modified : 27.Nov.2006 (didn't detect correctly if file not found)
//      Modified : 08.Aug.2010 (fixed compilation to linux, merged from other projects)
//
//  Description: 
//      read file and offer getLine function
//      handles correctly windows and linux end-of-line
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if ( !defined(_MSC_VER) )
#include <sys/file.h>
#endif

//#include "unistd.h"

#define USE_MUTEX 0
#define TOOLBOX_TFR_DEBUG 0

#include "toolbox.h"
#include "text-file-reader.h"

#define TEXT_FILE_READER_MAX_LINE_SIZE 1024

// DEVELOPER WARNING
//  if you modify this file, also modify text_file_reader.cpp in the liveMedia_VS7 project


int TextFileReader_Constructor(textFileReader * reader,  const char * const filename)
{
	if ( reader == 0 )
	{
		return -1;
	}
	
	reader->m_file = NULL;
	reader->m_offset = 0;
	reader->m_buffer_size = 0;
	reader->m_buffer_valid = 0;
	reader->m_buffer_offset = 0;
	reader->m_buffer_save = 0;
	reader->m_buffer = (char*)malloc(TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE);
	if ( reader->m_buffer )
	{
		reader->m_buffer_size = TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE;
	}

	if ( filename )
	{
		reader->m_file = fopen(filename, "rb");
	}
	else 
	{
		return -1;
	}
	
	if ( reader->m_file == 0 )
	{
		TextFileReader_Destructor(reader);
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}
	
	return 1;
}

int TextFileReader_Destructor(textFileReader * reader)
{	
	if ( reader == 0 )
	{
		return -1;
	}
	if ( reader->m_file ) 
	{
		fclose(reader->m_file);
		reader->m_file = NULL;
	}
	if ( reader->m_buffer ) 
	{
		free(reader->m_buffer);
		reader->m_buffer = NULL;
	}
	reader->m_buffer_size = 0;
	reader->m_buffer_valid = 0;
	reader->m_buffer_offset = 0;
	reader->m_buffer_save = 0;

	return 1;
}

// first written: ??.2005 or 2006
// re-written: 28.Jan.2008 (time is getting short)
int TextFileReader_GetLine(textFileReader * reader, char ** buffer, unsigned int * buffersize, unsigned int * out_read_length, int realloc_if_necessary)
{
#if TOOLBOX_TFR_DEBUG == 1
	static int count = 0;
#endif
	unsigned int _written;
	int fret;
	int iret;
	char * str_ret;
	int read_size;
	int length;
	int skipped = 0; // skipped is usally length + number of \r and \n bytes

	if ( reader == NULL )
	{
		return -1;
	}
	if ( reader->m_file == NULL
	  || reader->m_buffer == NULL 
	  || reader->m_buffer_size == 0 )
	{
		// failed to initialize m_file
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( buffer == NULL )
	{
		return -2;
	}
	if ( buffersize == NULL )
	{
		return -3;
	}
	if ( out_read_length == NULL )
	{
		out_read_length = &_written;
	}
	if ( realloc_if_necessary <= 0 && *buffersize == 0 )
	{
		// overflow possible.
		return -3;
	}

#if USE_MUTEX == 1
	reader->m_mutex.Acquire();
#endif

	fret = 1;

	if ( *buffer == NULL )
	{
		if ( realloc_if_necessary <= 0 )
		{	
			fret = -5; // if you pass NULL buffer, you must allow realloc'ation.
			goto end_of_getline;
		}
		*buffer = (char*)malloc(10);
		if ( *buffer == NULL )
		{
			fret = TOOLBOX_ERROR_MALLOC;
			goto end_of_getline;
		}
		*buffersize = 10;
		memset(*buffer, 0, 10);
	}
	else
	{
		(*buffer)[0] = '\0';
	}
	*out_read_length = 0;

	while (1) 
	{
		char * const buffer_ptr = &reader->m_buffer[0];
		char * const buffer_ptr_end = &reader->m_buffer[reader->m_buffer_size-1];
		char * buffer_ptr_end_temp = &reader->m_buffer[reader->m_buffer_size-1];
		char * buffer_ptr_temp;
		
#if TOOLBOX_TFR_DEBUG == 1
		count++;
		if ( count == 364 )
		{
			//__asm int 3;
		}
#endif

		if ( reader->m_buffer_save > 0 )
		{
			memcpy(buffer_ptr, buffer_ptr+reader->m_buffer_valid, reader->m_buffer_save);
			reader->m_buffer_valid = reader->m_buffer_save;
			buffer_ptr[reader->m_buffer_save] = '\0';
			reader->m_buffer_offset = 0;
			reader->m_buffer_save = 0;
		}

		if ( (int)reader->m_buffer_offset >= reader->m_buffer_valid )
		{
			// epuisé all valid, reset buffer
			reader->m_buffer_valid = 0;
			reader->m_buffer_offset = 0;
		}

		buffer_ptr_temp = buffer_ptr + reader->m_buffer_offset;
		read_size = reader->m_buffer_size-reader->m_buffer_valid-1;
		if ( read_size > 0 )
		{
			read_size = fread(buffer_ptr + reader->m_buffer_valid, 1, reader->m_buffer_size-reader->m_buffer_valid-1, reader->m_file);
			buffer_ptr_end_temp = buffer_ptr + reader->m_buffer_valid + read_size;

			if ( buffer_ptr_end_temp > buffer_ptr_end )
			{
				fret = TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM; // internal error
				goto end_of_getline;
			}

			buffer_ptr_end_temp[0] = '\0';

			if ( read_size == 0 && reader->m_buffer_valid == 0 )
			{
				fret = -13;
				// finished
				goto end_of_getline;
			}
			reader->m_buffer_valid += read_size;
		}

		str_ret = strchr(buffer_ptr_temp, '\n');
		if ( str_ret )
		{
			length = str_ret - buffer_ptr_temp;
			if ( length == 0 )
			{
				// empty line
				skipped++;
				buffer_ptr_temp++;
			}
			else
			{
				skipped += length + 1; // + 1 is for endline
			}

			if ( realloc_if_necessary == 0 )
			{
				if ( length + *out_read_length >= *buffersize )
				{
					// input buffer too small.
					// frank make sure we have a validation test for that algorithm
					length = *buffersize - *out_read_length - 1;
					fret = TOOLBOX_WARNING_CONTINUE_READING;
					// frank fixme, why dont we adjust skipped.
					// frank fixme, there is a bug here, but it is not affecting since nobody uses a buffer smaller than TextFileReader private buffer size.
				}
			}
			
			if ( length > 0 )
			{
				//if ( (*buffer)[*out_read_length+length-1] == '\r' )
				//{
				//	(*buffer)[*out_read_length+length-1] = '\0';
				//	length--;
				//	//*out_read_length += length-1;	
				//}
				
				if ( buffer_ptr_temp[length-1] == '\r' )
				{
					length--;
				}
				
				if ( realloc_if_necessary == 0 )
				{	strncpy(*buffer+*out_read_length, buffer_ptr_temp, length);
				}
				else
				{
					iret = C_Append(buffer, buffersize, buffer_ptr_temp, length, NULL/*separator*/);
					if ( iret == TOOLBOX_ERROR_MALLOC )
					{
						fret = TOOLBOX_ERROR_MALLOC;
						goto end_of_getline;
					}
				}
			}
			(*buffer)[*out_read_length+length] = '\0';

			//else
			//{
			//	iret = C_Append(buffer, buffersize, buffer_ptr_temp, length, NULL/*separator*/);
			//	if ( iret == TOOLBOX_ERROR_MALLOC )
			//	{
			//		fret = TOOLBOX_ERROR_MALLOC;
			//		goto end_of_getline;
			//	}
			//}

			reader->m_buffer_offset += skipped;
			if ( (int)reader->m_buffer_offset >= reader->m_buffer_valid )
			{
				// reset buffer
				reader->m_buffer_valid = 0;
				reader->m_buffer_offset = 0;
			}
			*out_read_length += length;

			break; // because got endline
		}
		else
		{
			// did not match \n
			// end of file ?
			if ( feof(reader->m_file) )
			{
				// end of file
				if ( read_size == 0 && reader->m_buffer_valid - reader->m_buffer_offset == 0 )
				{
					fret = -11;
					break;
					// finished
				}
				else if ( reader->m_buffer_valid - reader->m_buffer_offset > 0 
				       && strstr(buffer_ptr_temp, "\r") 
				        )
				{
					// only \r found and no \n
					str_ret = strstr(buffer_ptr_temp, "\r");
					if ( str_ret )
					{
						*str_ret = '\0';
					}
				}

				if ( realloc_if_necessary == 0 )
				{
					if ( *out_read_length + strlen(buffer_ptr_temp) >= *buffersize )
					{
						strncpy(*buffer+*out_read_length, buffer_ptr_temp, *buffersize-*out_read_length-1 );
						(*buffer)[*buffersize-1] = '\0';
						*out_read_length += *buffersize-*out_read_length-1;
					}
					else
					{
						strcat(*buffer, buffer_ptr_temp);
						*out_read_length += strlen(buffer_ptr_temp);
					}
				}
				else
				{
					C_Append(buffer, buffersize, buffer_ptr_temp, -1, NULL/*separator*/);
				}

				// finished, reset
				reader->m_buffer_valid = 0;
				reader->m_buffer_offset = 0;
				break;
			}
			else
			{
				// append and continue to work
				if ( buffer_ptr_temp[reader->m_buffer_valid-reader->m_buffer_offset-1] == '\r' )
				{
					reader->m_buffer_save++; // lookout tfrAAA1
					reader->m_buffer_valid--; // save \r for later
				}

				length = reader->m_buffer_valid - reader->m_buffer_offset;
				
				if ( realloc_if_necessary == 0 )
				{
					if ( length + *out_read_length >= *buffersize )
					{
						length = *buffersize - *out_read_length - 1;
						fret = TOOLBOX_WARNING_CONTINUE_READING;
					}
					strncpy(*buffer+*out_read_length, buffer_ptr_temp, length);
					(*buffer)[*out_read_length+length] = '\0';
				}
				else
				{
					// want to append length
					C_Append(buffer, buffersize, buffer_ptr_temp, length, NULL/*separator*/);
				}

				reader->m_buffer_offset += length;
				*out_read_length += length;
			}
		}
		if ( fret < 0 )
		{
			break;
		}
	}

end_of_getline:;

#if USE_MUTEX == 1
	reader->m_mutex.Release();
#endif

	return fret;
	
}



// The following is intended to support seeking in files > 2^32 bytes in size.
#if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
#include <io.h>
#else
#include <stdio.h>
#endif
