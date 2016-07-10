//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-char-array.c
//
// Copyright (c) 2006 Francois Oligny-Lemieux
// All rights reserved
//
//    Written by : Francois Oligny-Lemieux
//          Date : 22.Sep.2006
//      Modified : 25.Jan.2008 (to have array malloc'ed and growable)
//      Modified : 13.May.2008 (merged memory-leak fix done in mediaBox.svn for function charArray_insert)
//
//  Description: see .h
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "toolbox-char-array.h"
#include "toolbox-line-parser.h" // for charArray_splitTag
#include "toolbox-basic-types.h"
#include "toolbox.h" // for TOOLBOX_Append and TOOLBOX_LF

#define CHAR_ARRAY_ROW_AMOUNT 100
#define CHAR_ARRAY_COL_AMOUNT 30
#define CHAR_ARRAY_CELL_LENGTH 1024

#define CHAR_ARRAY_DEBUG_LEVEL 0
#define CHAR_ARRAY_DEBUG_MALLOC 0 // log to file allocations

#if CHAR_ARRAY_DEBUG_MALLOC == 1
#	include "filelog.h"
char temp[256] = "";
#endif

// return -999 : malloc error
int charArray_Constructor(charArray * root, int _row_amount_hint)
{
	int i=0;
	int j=0;
	//int index;
	//char * new_array;
	//char ** new_double_array;

	if ( root == NULL )
	{
		return -1;
	}

#if _DEBUG
	if ( root->should_always_be_0x56465646 == 0x56465646 )
	{
#	if defined(_MSC_VER)
		__asm int 3;
#	endif
		return TOOLBOX_ERROR_ALREADY_INITIALIZED;
	}
#endif

	memset(root->name, 0, sizeof(char)*CHAR_ARRAY_NAME_LENGTH);

	root->index = -1;

	root->col_amount = 1;
	root->col_amount_max = CHAR_ARRAY_COL_AMOUNT;

	root->row_amount = 0;
	if ( _row_amount_hint > 0 )
	{
		root->row_amount_max = _row_amount_hint;
	}
	else
	{
		root->row_amount_max = CHAR_ARRAY_ROW_AMOUNT;
	}


	root->array = (char***) malloc( sizeof(char**) * root->row_amount_max );
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
#if CHAR_ARRAY_DEBUG == 1
	snprintf(temp, sizeof(temp)-1, "[charArray_Constructor] charArray(0x%X) root->array(0x%X)\n", (unsigned int)root, (unsigned int)root->array);
	Log_C(temp, 0);
#endif
	root->buffersize = (unsigned int**) malloc( sizeof(unsigned int*) * root->row_amount_max );
	if ( root->buffersize == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	for ( i=0; i<root->row_amount_max; i++ )
	{
		root->array[i] = (char**) malloc( sizeof(char*) * root->col_amount_max );
#if CHAR_ARRAY_DEBUG == 1
		snprintf(temp, sizeof(temp)-1, "[charArray_Constructor] root->array[%d](0x%X)\n", i, (unsigned int)root->array[i]);
		Log_C(temp, 0);
#endif
		root->buffersize[i] = (unsigned int*) malloc( sizeof(unsigned int) * root->col_amount_max );
		if ( root->array[i] == NULL || root->buffersize[i] == NULL )
		{
			// destruction of temporary initialization
			for ( j=0; j<i; j++ )
			{
				if ( root->array[j] )
				{
					free(root->array[j]);
				}
				if ( root->buffersize[j] )
				{
					free(root->buffersize[j]);
				}
			}
			free(root->array);
			free(root->buffersize);
			root->array = NULL;
			root->buffersize = NULL;
			return TOOLBOX_ERROR_MALLOC;
		}
		memset(root->array[i], 0, sizeof(char*) * root->col_amount_max );
		memset(root->buffersize[i], 0, sizeof(unsigned int) * root->col_amount_max );
	}

	root->cell_length = CHAR_ARRAY_CELL_LENGTH;

	root->reader_chop_empty_lines = 0;

	root->should_always_be_0x56465646 = 0x56465646;

	return 1;	
}


// return -1 : malloc error
int charArray_Destructor(charArray * root)
{
	int i;
	int j;

	if ( root == NULL )
	{
		return -1;
	}
	
#if CHAR_ARRAY_DEBUG == 1
	snprintf(temp, sizeof(temp)-1, "[charArray_Destructor] charArray(0x%X) root->array(0x%X) 0x56465646(0x%X)\n", (unsigned int)root, (unsigned int)root->array, root->should_always_be_0x56465646);
	Log_C(temp, 0);
#endif

	if ( root->should_always_be_0x56465646 != 0x56465646 )
	{
#if _DEBUG
		__asm int 3;
#endif
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}

	root->should_always_be_0x56465646 = 0;

	if ( root->array )
	{
		for (i=0; i<root->row_amount_max; i++)
		{
			if ( root->array[i] )
			{
				for (j=0; j<root->col_amount_max; j++ )
				{
#if _DEBUG
					if ( root->array[i][j] && root->buffersize[i][j] == 0 )
					{
						__asm int 3;
					}
#endif
					if ( root->buffersize[i][j] ) // it's faster CPU wise to check for the buffersize[i][j] value
					{
#if CHAR_ARRAY_DEBUG == 1
						snprintf(temp, sizeof(temp)-1, "[charArray_Destructor] freeing root->array[%d][%d](0x%X)\n", i, j, (unsigned int)root->array[i][j]);
						Log_C(temp, 0);
#endif

						free(root->array[i][j]);
						root->array[i][j] = NULL;
						root->buffersize[i][j] = 0;
					}
				}
				
#if CHAR_ARRAY_DEBUG == 1
				snprintf(temp, sizeof(temp)-1, "[charArray_Destructor] freeing root->array[%d](0x%X)\n", i, (unsigned int)root->array[i]);
				Log_C(temp, 0);
#endif

				free(root->array[i]);
				free(root->buffersize[i]);

				root->array[i] = NULL;
				root->buffersize[i] = NULL;
			}
		}

		free(root->array);
		free(root->buffersize);
		root->array = NULL;
		root->buffersize = NULL;
	}

	root->row_amount = 0;
	root->col_amount = 0;

	return 1;
}

// return -1 : malloc error
int charArray_Zero(charArray * root)
{
	int i,j;

	if ( root == NULL )
	{
		return -1;
	}

	if ( root->should_always_be_0x56465646 != 0x56465646 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}

	if ( root->array )
	{
		for (i=0; i<root->row_amount_max; i++)
		{
			if ( root->array[i] )
			{
				if ( root->array[i][0] )
				{
					memset(root->array[i][0], 0, root->buffersize[i][0]);
				}

				// free all columns j>=1
				for (j=1; j<root->col_amount_max; j++ )
				{
					if ( root->buffersize[i][j] )
					{
#if CHAR_ARRAY_DEBUG == 1
		snprintf(temp, sizeof(temp)-1, "[charArray_Zero] charArray(0x%X) root->array(0x%X) freeing cell[%d][%d](0x%X)\n", (unsigned int)root, (unsigned int)root->array, i, j, (unsigned int)root->array[i][j]);
		Log_C(temp, 0);
#endif
						free(root->array[i][j]);
						root->array[i][j] = NULL;
						root->buffersize[i][j] = 0;
					}
				}
			}
		}
	}

	root->row_amount = 0;

	return 1;
}


#if 0 // old way
static int charArray_realloc(charArray * root, int newsize)
{
	int i=0;
	int j=0;
	char ** new_double_array;
	char *** new_triple_array;
	unsigned int * new_double_buffersize;
	unsigned int ** new_triple_buffersize;

	if ( g_verbose >= 2 )
	{
		printf("<!-- WARNING - charArray_realloc, root(0x%X) newsize(%d)-->\n", (unsigned int)root, newsize);
	}

	// realloc
	new_triple_array = (char***) malloc( sizeof(char**) * newsize );
	if ( new_triple_array == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	new_triple_buffersize = (unsigned int**) malloc( sizeof(unsigned int*) * newsize );
	if ( new_triple_buffersize == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

#if CHAR_ARRAY_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[charArray_realloc] copy old into new\n");
	}
#endif

	// copy old into new
	for (i=0; i<root->row_amount_max; i++)
	{
		new_double_array = (char**) malloc( sizeof(char*) * root->col_amount_max );
		new_double_buffersize = (unsigned int*) malloc( sizeof(unsigned int) * root->col_amount_max );
		if ( new_double_array == NULL || new_double_buffersize == NULL )
		{
			// destruction of temporary initialization
			for (j=0; j<i; j++)
			{
				if ( new_triple_array[j] )
				{
					free(new_triple_array[j]);
					new_triple_array[j] = NULL;
				}
				if ( new_triple_buffersize[j] )
				{
					free(new_triple_buffersize[j]);
					new_triple_buffersize[j] = NULL;
				}
			}
			if ( new_double_array )
			{
				free(new_double_array);
			}
			if ( new_double_buffersize )
			{
				free(new_double_buffersize);
			}
			if ( new_triple_array )
			{
				free(new_triple_array);
			}
			if ( new_triple_buffersize )
			{
				free(new_triple_buffersize);
			}
			return TOOLBOX_ERROR_MALLOC;
		}
		for(j=0; j<root->col_amount_max; j++)
		{
			new_double_array[j] = root->array[i][j]; // copy char pointer
			new_double_buffersize[j] = root->buffersize[i][j]; // copy value
		}
		new_triple_array[i] = new_double_array;
		new_triple_buffersize[i] = new_double_buffersize;
	}
#if CHAR_ARRAY_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[charArray_realloc] continue with new rows\n");
	}
#endif
	// continue with new rows
	for (i=root->row_amount_max; i<newsize; i++)
	{
		new_double_array = (char**) malloc( sizeof(char*) * root->col_amount_max );
		new_double_buffersize = (unsigned int*) malloc( sizeof(unsigned int) * root->col_amount_max );
#if CHAR_ARRAY_DEBUG_LEVEL >= 2
		if ( g_verbose >= 1 )
		{
			printf("[charArray_realloc] new row pointer (0x%X)\n",(unsigned int)new_double_array);
		}
#endif

		if ( new_double_array == NULL || new_double_buffersize == NULL )
		{
			// destruction of temporary initialization
			if ( g_verbose >= 1 )
			{
				printf("<!-- [charArray_realloc] ERROR destruction of temporary initialization -->\n");
			}

			for (j=0; j<i; j++)
			{
				if ( new_triple_array[j] )
				{
					free(new_triple_array[j]);
					new_triple_array[j] = NULL;
				}
				if ( new_triple_buffersize[j] )
				{
					free(new_triple_buffersize[j]);
					new_triple_buffersize[j] = NULL;
				}
			}
			if ( new_double_array )
			{
				free(new_double_array);
			}
			if ( new_double_buffersize )
			{
				free(new_double_buffersize);
			}
			if ( new_triple_array )
			{
				free(new_triple_array);
			}
			if ( new_triple_buffersize )
			{
				free(new_triple_buffersize);
			}
			return TOOLBOX_ERROR_MALLOC;
		}
		memset(new_double_array, 0, sizeof(char*) * root->col_amount_max);
		memset(new_double_buffersize, 0, sizeof(unsigned int) * root->col_amount_max);
		new_triple_array[i] = new_double_array;
		new_triple_buffersize[i] = new_double_buffersize;
	}
	
#if CHAR_ARRAY_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[charArray_realloc] cleanup of old array structure (not char data)\n");
	}
#endif

	// cleanup of old array structure (not char data)
	for (i=0; i<root->row_amount_max; i++)
	{	
		if ( root->array[i] )
		{
			free(root->array[i]);
			free(root->buffersize[i]);
			root->array[i] = NULL;
			root->buffersize[i] = NULL;
		}
	}

#if CHAR_ARRAY_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[charArray_realloc] last three lines ahead\n");
	}
#endif

	free(root->array);
	free(root->buffersize);
	root->array = new_triple_array;
	root->buffersize = new_triple_buffersize;
	root->row_amount_max = newsize;
	
#if CHAR_ARRAY_DEBUG_LEVEL >= 2
	if ( g_verbose >= 1 )
	{
		printf("[charArray_realloc] end of func");
	}
#endif

	return 1;
}
#else
// new efficient way
static int charArray_realloc(charArray * root, int newsize)
{
	int i=0;
	char ** new_double_array;
	char *** new_triple_array;
	unsigned int * new_double_buffersize;
	unsigned int ** new_triple_buffersize;
	int failure_at_row = -1;

	if ( g_verbose >= 3 )
	{
		printf("WARNING need to realloc with newsize(%d)\n", newsize);
	}
	else if ( g_verbose >= 2 )
	{
		printf("<!-- WARNING - charArray_realloc -->\n");
	}

	// realloc
	new_triple_array = (char***) realloc(root->array, sizeof(char**) * newsize );
	if ( new_triple_array == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	new_triple_buffersize = (unsigned int**) realloc(root->buffersize, sizeof(unsigned int*) * newsize );
	if ( new_triple_buffersize == NULL )
	{
		//free(new_triple_array); don't free it so it stays consistent.
		return TOOLBOX_ERROR_MALLOC;
	}
	
	// copy old into new... no already done

	// continue with new rows
	for (i=root->row_amount_max; i<newsize; i++)
	{
		new_double_array = (char**) malloc( sizeof(char*) * root->col_amount_max  );
		new_double_buffersize = (unsigned int*) malloc( sizeof(unsigned int) * root->col_amount_max  );

		if ( new_double_array == NULL )
		{
			// stop here
			new_triple_array[i] = NULL;
			new_triple_buffersize[i] = NULL;
			failure_at_row = i;
			if ( g_verbose >= 1 )
			{
				printf("[charArray_realloc] ERROR malloc of new double_array\n");
			}
			break;
		}
		if ( new_double_buffersize == NULL )
		{
			// stop here
			new_triple_array[i] = NULL;
			new_triple_buffersize[i] = NULL;
			failure_at_row = i;
			if ( g_verbose >= 1 )
			{
				printf("[charArray_realloc] ERROR malloc of new double_array\n");
			}
			break;
		}
		
		memset(new_double_array, 0, sizeof(char*) * root->col_amount_max);
		memset(new_double_buffersize, 0, sizeof(unsigned int) * root->col_amount_max);
		new_triple_array[i] = new_double_array;
		new_triple_buffersize[i] = new_double_buffersize;
	}

	root->array = new_triple_array;
	root->buffersize = new_triple_buffersize;
	root->row_amount_max = newsize;

	if ( failure_at_row >= 0 )
	{
		root->row_amount_max = failure_at_row+1;
		return TOOLBOX_ERROR_MALLOC;
	}
	
	return 1;
}
#endif

// private function
static void free_row(char ** row, unsigned int * buffersize, int col_max)
{
	int j=0;
	if ( row == NULL ) 
	{
		return;
	}

	for ( j=0; j<col_max; j++ )
	{
		if ( row[j] )
		{
			free(row[j]);
			row[j] = NULL;
			buffersize[j] = 0;
		}
	}
}


int regexp_match(const char * const line, const char * const pattern)
{
	int iret;
	//int fret=-1;
	int i;
	//int j=0;
	int found = 0;
	char * str_ret;
	lineParser parser;
	charArray * array_insider = NULL;
	int single_fail = 0;
	//int debug=1;
	
	if ( strstr(pattern, ".*") )
	{
		iret = lineParser_Constructor(&parser);
		if ( iret == TOOLBOX_ERROR_MALLOC )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		else if ( iret <= 0 )
		{
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		// we need to crank up
		iret = lineParser_splitTag(&parser, pattern, ".*", &array_insider);
		if ( iret <= 0 )
		{
			lineParser_Destructor(&parser);
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		if ( array_insider == NULL )
		{
			// never came across this error
			lineParser_Destructor(&parser);
			return -10;
		}

		// Chapter 1 -- Find what row needs to be returned
		single_fail = 0;
		found = 0;
		for (i=0; i<array_insider->row_amount; i++)
		{
			str_ret = strstr(line, array_insider->array[i][0]); // frank this is BROKEN, it finds it in all orders
			//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
			if ( str_ret==NULL )
			{
				single_fail++; //we require ALL to match
				break;
			}
			else
			{
				// one match
			}
		}
		if ( single_fail == 0 && array_insider->row_amount > 0 )
		{
			// real found
			found = 1;
		}
		lineParser_Destructor(&parser);

	}
	else
	{
		// SIMPLE VERSION
		// SIMPLE VERSION
		// SIMPLE VERSION

		// Chapter 1 -- Find what row needs to be returned
		str_ret = strstr(line, pattern);
		//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
		if ( str_ret )
		{
			found = 1;
		}
	}

	if ( found )
	{
		return 1;
	}

	return 0;
}


int charArray_getIndexWithKey(charArray * root, const char * needle, int * key_obj)
{
	//charArray * root = (charArray*) _root;
	int i=0;
	//int j=0;
	int found=0;
	char * str_ret;

	if ( root == 0 )
	{
		return -2;
	}
	if ( needle == 0 )
	{
		return -3;
	}
	if ( key_obj == 0 )
	{
		return -5;
	}
	if ( strlen(needle) == 0 )
	{
		return -6;
	}

	// Chapter 1 -- Find what row needs to be returned
	for (i=0; i<root->row_amount; i++)
	{
		str_ret = strstr(root->array[i][0], needle);
		//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
		if ( str_ret )
		{
			found = 1;
			break;
		}
	}

	if ( found == 0 )
	{
		return 0;
	}

	*key_obj = i;

	return 1;
}

// column is zero-based.
int charArray_getColumnWithKey(charArray * root, const char * needle, int column, char ** out_insider)
{
	//charArray * root = (charArray*) _root;
	int i=0;
	int j=0;
	int a;
	int iret;
	int found=0;
	lineParser parser;
	charArray * array_insider = NULL;
	int single_fail = 0;
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	int debug = 0;
#endif

	if ( root == 0 )
	{
		return -2;
	}
	if ( needle == 0 )
	{
		return -3;
	}
	if ( column < 0 )
	{
		return -4;
	}
	if ( out_insider == 0 )
	{
		return -5;
	}
	if ( strlen(needle) == 0 )
	{
		return -6;
	}
	
	*out_insider = NULL;
	
#if CHAR_ARRAY_DEBUG_LEVEL >= 3
	if ( debug )
	{
		printf("[charArray_getColumnWithKey] entering with needle(%s)\n", needle);
	}
#endif

	if ( strstr(needle, "|") )
	{
		// COMPLICATED VERSION
		// COMPLICATED VERSION
		// COMPLICATED VERSION
#if CHAR_ARRAY_DEBUG_LEVEL >= 3
		if ( debug )
		{
			printf("[charArray_getColumnWithKey] complicated version\n");
			fflush(stdout);
		}
#endif

		iret = lineParser_Constructor(&parser);
		if ( iret == TOOLBOX_ERROR_MALLOC )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		else if ( iret <= 0 )
		{
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		// we need to crank up
		iret = lineParser_splitTag(&parser, needle, "|", &array_insider);
		if ( iret <= 0 )
		{
			lineParser_Destructor(&parser);
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		if ( array_insider == NULL )
		{
			// never came across this error
			lineParser_Destructor(&parser);
			return -10;
		}

		// Chapter 1 -- Find what row needs to be returned
		for (i=0; i<root->row_amount; i++)
		{
			single_fail = 0;
			found = 0;
			for (a=0; a<array_insider->row_amount; a++)
			{
				//iret = regexp_match(root->array[i][0], array_insider->array[a][0]);
				iret = strcmp(root->array[i][0], array_insider->array[a][0]);
				//str_ret = strstr(root->array[i][0], array_insider->array[a][0]);
				//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
				if ( iret!=0 )
				{
					single_fail++;
				}
				else
				{
					// one match
					found = 1;
					break;
				}
			}
			if ( found )
			{
				// real found
				found = 1;
				break;
			}
		}
		lineParser_Destructor(&parser);
	}
	else
	{
		// SIMPLE VERSION
		// SIMPLE VERSION
		// SIMPLE VERSION		
#if CHAR_ARRAY_DEBUG_LEVEL >= 3
		if ( debug )
		{
			printf("[charArray_getColumnWithKey] simple version\n");
			fflush(stdout);
		}
#endif

		// Chapter 1 -- Find what row needs to be returned
		for (i=0; i<root->row_amount; i++)
		{
#if CHAR_ARRAY_DEBUG_LEVEL >= 3
			if ( debug )
			{
				if ( root->array[i][j] )
				{
					printf("[charArray_getColumnWithKey] searching for (%s) into (%s)\n", needle, root->array[i][j]);
				}
				else 
				{
					printf("[charArray_getColumnWithKey] ERROR, row_amount(%d), root->array[%d][0] == NULL\n", root->row_amount, i);
					fflush(stdout);
				}
			}
#endif
			iret = strcmp(root->array[i][j], needle);
			//printf("[getColumnWithKey] searching for (%s) into (%s), result(%d)\n", needle, root->array[i][j], iret);
			//snprintf(php_message_buffer+strlen(php_message_buffer), 255, "[getLineThatMatches] searching for (%s) into (%s), result(%d)\n", needle, root->array[i][j], iret);
			if ( iret == 0 )
			{
				found = 1;
				break;
			}
		}
	}

	//printf("[getColumnWithKey] after break, found(%d), i(%d)\n",found,i);

	if ( found == 0 )
	{
		return 0;
	}

	j = column;

	if ( root->array[i][j] )
	{
		*out_insider = root->array[i][j];
		//printf("[getColumnWithKey] done success assigning value(%s)\n", *out_insider );
	}
	else
	{
		//printf("[getColumnWithKey] root->array[%d][%d] is NULL\n", i,j );
		*out_insider = NULL;
		return 0;
	}
	return 1;
}

int charArray_getLineThatMatchesEx(charArray * root, const char * needle, int start_at_row, int stop_at_row, int * out_row, char ** out_insider)
{
	int i = 0;
	int a;
	int iret;
	int found = 0;
	lineParser parser;
	charArray * array_insider = NULL;
	int single_fail = 0;

	if ( root == 0 )
	{
		return -1;
	}
	if ( needle == 0 )
	{
		return -2;
	}
	if ( start_at_row < 0 )
	{
		return -3;
	}
	if ( stop_at_row >= 0 && stop_at_row < start_at_row )
	{
		return -4;
	}
	if ( stop_at_row < 0 )
	{
		stop_at_row = root->row_amount-1;
	}
	if ( out_row == 0 )
	{
		// allow it return -5;
	}
	if ( out_insider == 0 ) 
	{
		return -6;
	}
	if ( strlen(needle) == 0 )
	{
		return -7; // empty needle
	}

	if ( out_row ) *out_row = 0;
	*out_insider = NULL;

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_YOU_PASSED_AN_EMPTY_ARRAY;
	}

	if ( start_at_row < 0 )
	{
		start_at_row = 0;
	}
	if ( start_at_row >= root->row_amount )
	{
		start_at_row = root->row_amount-1;
	}
	if ( stop_at_row >= root->row_amount )
	{
		stop_at_row = root->row_amount-1;
	}

	if ( strstr(needle, "|") )
	{
		// COMPLICATED VERSION
		// COMPLICATED VERSION
		// COMPLICATED VERSION

		iret = lineParser_Constructor(&parser);
		if ( iret == TOOLBOX_ERROR_MALLOC )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		else if ( iret <= 0 )
		{
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		// we need to crank up
		iret = lineParser_splitTag(&parser, needle, "|", &array_insider);
		if ( iret <= 0 )
		{
			lineParser_Destructor(&parser);
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		if ( array_insider == NULL )
		{
			// never came across this error
			lineParser_Destructor(&parser);
			return -10;
		}

		// Chapter 1 -- Find what row needs to be returned
		for (i=start_at_row; i<=stop_at_row; i++)
		{
			single_fail = 0;
			found = 0;
			for (a=0; a<array_insider->row_amount; a++)
			{
				iret = regexp_match(root->array[i][0], array_insider->array[a][0]);
				//str_ret = strstr(root->array[i][0], array_insider->array[a][0]);
				//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
				if ( iret <= 0 )
				{
					single_fail++;
				}
				else
				{
					// one match
					found = 1;
					break;
				}
			}
			if ( found )
			{
				// real found
				found = 1;
				break;
			}
		}
		lineParser_Destructor(&parser);
	}
	else
	{
		// SIMPLE VERSION
		// SIMPLE VERSION
		// SIMPLE VERSION

		// Chapter 1 -- Find what row needs to be returned
		for (i=start_at_row; i<=stop_at_row; i++)
		{
			iret = regexp_match(root->array[i][0], needle);
			//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
			if ( iret > 0 )
			{
				found = 1;
				break;
			}
		}
	}

	if ( found == 0 )
	{
		return 0;
	}

	*out_insider = root->array[i][0];
	if ( out_row ) *out_row = i;

	return 1;

}

// return -2 to -4 : NULL pointer in parameters error
// return -5 : needle is empty
int charArray_getLineThatMatches(charArray * root, const char * needle, char ** out_insider)
{
	//charArray * root = (charArray*) _root;
	int i = 0;
	int a;
	int iret;
	int found = 0;
	lineParser parser;
	charArray * array_insider = NULL;
	int single_fail = 0;
	//int debug=0;

	if ( root == 0 )
	{
		return -2;
	}
	if ( needle == 0 )
	{
		return -3;
	}
	if ( out_insider == 0 )
	{
		return -4;
	}
	if ( strlen(needle) == 0 )
	{
		return -5;
	}

	*out_insider = NULL;

	if ( strstr(needle, "|") )
	{
		// COMPLICATED VERSION
		// COMPLICATED VERSION
		// COMPLICATED VERSION

		iret = lineParser_Constructor(&parser);
		if ( iret == TOOLBOX_ERROR_MALLOC )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		else if ( iret <= 0 )
		{
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		// we need to crank up
		iret = lineParser_splitTag(&parser, needle, "|", &array_insider);
		if ( iret <= 0 )
		{
			lineParser_Destructor(&parser);
			return TOOLBOX_ERROR_INTERNAL_DESIGN_PROBLEM;
		}
		if ( array_insider == NULL )
		{
			// never came across this error
			lineParser_Destructor(&parser);
			return -10;
		}

		// Chapter 1 -- Find what row needs to be returned
		for (i=0; i<root->row_amount; i++)
		{
			single_fail=0;
			found=0;
			for (a=0; a<array_insider->row_amount; a++)
			{
				iret = regexp_match(root->array[i][0], array_insider->array[a][0]);
				//str_ret = strstr(root->array[i][0], array_insider->array[a][0]);
				//if ( debug ) printf("[getLineThatMatches] searching for (%s) into (%s), result(%d)\n", needle, root->array[i][j], iret);
				if ( iret <= 0 )
				{
					single_fail++;
				}
				else
				{
					// one match
					found = 1;
					break;
				}
			}
			if ( found )
			{
				// real found
				found = 1;
				break;
			}
		}
		lineParser_Destructor(&parser);
	}
	else// frank fixme, split the simple version in REGEXP SIMPLE and PLAIN SIMPLE
	{
		// SIMPLE VERSION
		// SIMPLE VERSION
		// SIMPLE VERSION

		// Chapter 1 -- Find what row needs to be returned
		for (i=0; i<root->row_amount; i++)
		{
			iret = regexp_match(root->array[i][0], needle);
			//if ( debug ) printf("[getLineThatMatches] searching for (%s) into (%s), result(%d)\n", needle, root->array[i][j], iret);
			if ( iret > 0 )
			{
				found = 1;
				break;
			}
		}
	}

	if ( found == 0 )
	{
		return 0;
	}

	*out_insider = root->array[i][0];

	return 1;
}

int charArray_getLineThatIs(charArray * root, const char * line, char ** out_insider)
{
	int i = 0;
	//int a;
	int iret;
	int found = 0;
	//lineParser parser;
	charArray * array_insider = NULL;
	int single_fail = 0;
	//int debug=0;

	if ( root == NULL )
	{
		return -2;
	}
	if ( line == NULL )
	{
		return -3;
	}
	if ( out_insider == 0 )
	{
		return -4;
	}
	if ( strlen(line) == 0 )
	{
		return -5;
	}

	*out_insider = NULL;
	
	// Chapter 1 -- Find what row needs to be returned
	for (i=0; i<root->row_amount; i++)
	{
		iret = strcmp(root->array[i][0], line);
		//if ( debug ) printf("[getLineThatMatches] searching for (%s) into (%s), result(%d)\n", needle, root->array[i][j], iret);

		if ( iret == 0 )
		{
			found = 1;
			break;
		}
	}
	
	if ( found == 0 )
	{
		return 0;
	}

	*out_insider = root->array[i][0];

	return 1;

}

int charArray_getLine(charArray * root, int row, char ** out_insider)
{
	//int i=0;
	//int j=0;
	//int found=0;

	if ( root == 0 )
	{
		return -2;
	}
	if ( row < 0 )
	{
		row = root->row_amount-1;
	}
	if ( out_insider == 0 )
	{
		return -3;
	}

	if ( row >= root->row_amount )
	{
		*out_insider = NULL;
		return -10;
		// row not there
	}

	*out_insider = root->array[row][0];

	return 1;
}


// will do several substitutions until exhausted.
int charArray_substituteInLineThatMatches(charArray * root, const char * needle, const char * const replacement, int toolbox_parse_options)
{
	int i = 0;
	int index;
	int iret;
	int row;
	int found = 0;
	char *** array;
	char * str_ret;
	int replacement_amount = 0;
	charArray * tempArray_ptr = NULL;	
	lineParser parser;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( replacement == NULL )
	{
		return -3;
	}
	if ( needle == NULL || needle[0] == '\0' )
	{
		return -5;
	}

	array = root->array;

	row=0;
	lineParser_Constructor(&parser);

hunt_needle:
	found = 0;

	// Chapter 1 -- find out what row needs to be spliced
	for (i=row; i<root->row_amount; i++)
	{
		str_ret = strstr(root->array[i][0], needle);
		//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
		if ( str_ret )
		{
			found = 1;
			break;
		}
	}

	if ( found == 0 )
	{
		if ( replacement_amount > 0 )
		{
			goto func_end;
		}
		lineParser_Destructor(&parser);
		return 0;
	}

	row = i;
	index = i;

	iret = lineParser_splitTag(&parser, root->array[row][0], needle, &tempArray_ptr);
	if ( iret > 0 && tempArray_ptr )
	{// frank fixme do strlen(replacement)*amount_found_on_line and for needle too!!!
		if ( strlen(replacement) + strlen(root->array[row][0]) - strlen(needle) > CHAR_ARRAY_CELL_LENGTH )
		{	// split line in two lines
			if ( tempArray_ptr->row_amount >= 1 )
			{
				charArray_replace(root, row, tempArray_ptr->array[0][0]); // first part of line
				row++;
				charArray_insert(root, row, replacement); // middle part (which is new)
			}
			else
			{
				charArray_replace(root, row, replacement); // middle part (which is new)
			}

			for (i=1; i<tempArray_ptr->row_amount; i++)
			{
				row++;// frank fixme, insert replacement before or after every cell
				charArray_insert(root, row, tempArray_ptr->array[i][0]); // middle part (which is new)
			}
		}
		else
		{
			// work on single line
			memset(root->array[row][0], 0, root->cell_length);

			if ( tempArray_ptr->row_amount == 0 )
			{
				strcpy(root->array[row][0], tempArray_ptr->array[0][0]);
			}
			else if ( tempArray_ptr->row_amount == 1 )
			{
				strcpy(root->array[row][0], replacement);
				strcat(root->array[row][0], tempArray_ptr->array[0][0]);
			}
			else if ( tempArray_ptr->row_amount > 1 )
			{
				if ( tempArray_ptr->array[0][0] )
				{
					strcpy(root->array[row][0], tempArray_ptr->array[0][0]);
				}

				for (i=1; i<tempArray_ptr->row_amount; i++)
				{
					strcat(root->array[row][0], replacement);
					strcat(root->array[row][0], tempArray_ptr->array[i][0]);
				}
			}
		}
		replacement_amount++;
		row++;
		if ( toolbox_parse_options == TOOLBOX_ALL )
		{
			goto hunt_needle;
		}
	}

func_end:
	lineParser_Destructor(&parser);

	return 1;
}

int charArray_substituteArrayInLineThatMatches(charArray * root, const char * needle, charArray * const replacement)
{
	int i=0;
	int j=0;
	int index;
	int iret;
	int row;
	int found=0;
	char *** array;
	char * str_ret;
	charArray * tempArray_ptr = NULL;	
	lineParser parser;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( replacement == NULL )
	{
		return -3;
	}
	if ( strlen(needle) == 0 )
	{
		return -5;
	}

	array = root->array;

	j = 0;

	// Chapter 1 -- find out what row needs to be spliced
	for (i=0; i<root->row_amount; i++)
	{
		str_ret = strstr(root->array[i][j], needle);
		//printf("[getLineThatMatches] searching for (%s) into (%s), result(0x%X)\n", needle, root->array[i][j], str_ret);
		if ( str_ret )
		{
			found = 1;
			break;
		}
	}

	if ( found == 0 )
	{
		return 0;
	}

	row = i;
	index = row;

	lineParser_Constructor(&parser);

	iret = lineParser_splitTag(&parser, root->array[row][j], needle, &tempArray_ptr);
	if ( iret > 0 && tempArray_ptr )
	{
		i=0;
		if ( tempArray_ptr->row_amount >= 1 )
		{
			charArray_replace(root, row, tempArray_ptr->array[0][0]); // first part of line
		}
		else
		{
			i=1; // frank when does this happens ?? it should never!!
			if ( replacement->array[0][0] ) // bypasses when replacement is empty
			{
				charArray_replace(root, row, replacement->array[0][0]); 
			}
			else
			{
				charArray_replace(root, row, ""); 
			}
		}

		for (; i<replacement->row_amount; i++)
		{
			row++;
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
			if ( g_verbose >= 1 )
			{
				printf("[charArray_substituteArrayInLineThatMatches] charArray_insert(%s)\n",replacement->array[i][0]);
			}
#endif
			charArray_insert(root, row, replacement->array[i][0]); // middle part (which is new)
		}
		for (i=1; i<tempArray_ptr->row_amount; i++)
		{
			row++;
			charArray_insert(root, row, tempArray_ptr->array[i][0]); // middle part (which is new)
		}
	}

	lineParser_Destructor(&parser);

	return 1;
}



int charArray_removeLineWithIndex(charArray * root, int index)
{
	//charArray * root = (charArray*) _root;
	int i=0;
	//int j=0;
	//int found=0;
	char ** row;
	unsigned int * buffersize;

	if ( root == 0 )
	{
		return -2;
	}

	if ( index == root->row_amount && index == 0 )
	{
		return 0;
	}

	if ( index >= root->row_amount )
	{
		index = root->row_amount-1;
	}
	if ( index < 0 )
	{
		index = root->row_amount-1;
	}

	i = index;

	free_row(root->array[i], root->buffersize[i], root->col_amount_max);

	// root->array[i] is going to be a zombie (it exists but has no place)
	row = root->array[i];
	buffersize = root->buffersize[i];

	// shift down the rest
	for (i=i+1; i<root->row_amount; i++)
	{
		root->array[i-1] = root->array[i];
		root->buffersize[i-1] = root->buffersize[i];
	}

	// now root->array[i] is a zombie

	root->array[root->row_amount-1] = row;
	root->buffersize[root->row_amount-1] = buffersize;

	// no zombie anymore

	root->row_amount--;

	return 1;
}

int charArray_removeLineWithIndexAndAllBefore(charArray * root, int index)
{
	//charArray * root = (charArray*) _root;
	int i=0;
	//int j=0;
	int a=0;
	//int found=0;
	char ** row;
	unsigned int * buffersize;

	if ( root == 0 )
	{
		return -2;
	}
	if ( index >= root->row_amount )
	{
		index = root->row_amount-1;
	}
	if ( index < 0 )
	{
		index = root->row_amount-1;
	}

	i = index;
	a = i;

	while ( i >= 0 )
	{
		free_row(root->array[i], root->buffersize[i], root->col_amount_max);

		// root->array[i] is going to be a zombie (it exists but has no place)
		row = root->array[i];
		buffersize = root->buffersize[i];

		// shift down the rest
		a = i;
		for (a=a+1; a<root->row_amount; a++)
		{
			root->array[a-1] = root->array[a];
			root->buffersize[a-1] = root->buffersize[a];
		}

		// now root->array[i] is a zombie

		root->array[root->row_amount-1] = row;
		root->buffersize[root->row_amount-1] = buffersize;

		// no zombie anymore
	
		root->row_amount--;

		i--;
	}

	return 1;
}

int charArray_removeLineThatIs(charArray * root, char * lookup)
{
	//charArray * root = (charArray*) _root;
	int i=0;
	int j=0;
	int found=0;
	int iret;
	char ** row;
	unsigned int * buffersize;

	if ( root == 0 )
	{
		return -2;
	}
	if ( lookup == 0 )
	{
		return -3;
	}
	if ( strlen(lookup) == 0 )
	{
		return -5;
	}

	for (i=0; i<root->row_amount; i++)
	{
		iret = strcmp(root->array[i][j], lookup);
		//printf("[removeLineThatMatches] searching for (%s) into (%s), result(%d)\n", lookup, root->array[i][j], iret);
		if ( iret == 0 )
		{
			found = 1;
			break;
		}
	}

	if ( found == 0 )
	{
		return 0;
	}

	free_row(root->array[i], root->buffersize[i], root->col_amount_max);

	// root->array[i] is going to be a zombie (it exists but has no place)
	row = root->array[i];
	buffersize = root->buffersize[i];


	// shift down the rest
	for (i=i+1; i<root->row_amount; i++)
	{
		root->array[i-1] = root->array[i];
		root->buffersize[i-1] = root->buffersize[i];
	}

	// now root->array[i] is a zombie

	root->array[root->row_amount-1] = row;
	root->buffersize[root->row_amount-1] = buffersize;

	// no zombie anymore

	root->row_amount--;

	return 1;
}

// return -2 to -4 : NULL pointer in parameters error
// return -5 : needle is empty
int charArray_removeLineThatMatches(charArray * root, const char * needle)
{
	//charArray * root = (charArray*) _root;
	int i=0;
	//int j=0;
	int found=0;
	char * str_ret;
	char ** row;
	unsigned int * buffersize;
	//int debug = 0;

	if ( root == 0 )
	{
		return -2;
	}
	if ( needle == 0 )
	{
		return -3;
	}
	if ( strlen(needle) == 0 )
	{
		return -5;
	}

	for (i=0; i<root->row_amount; i++)
	{
		str_ret = strstr(root->array[i][0], needle);
		
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
		if ( debug ) printf("[removeLineThatMatches] searching for (%s) into (%s), result(0x%X)\n",
			needle, root->array[i][0], (unsigned int)str_ret);
#endif

		if ( str_ret )
		{
			found = 1;
			break;
		}
	}

	if ( found == 0 )
	{
		return 0;
	}

	free_row(root->array[i], root->buffersize[i], root->col_amount_max);

	// root->array[i] is going to be a zombie (it exists but has no place)
	row = root->array[i];
	buffersize = root->buffersize[i];

	// shift down the rest
	for (i=i+1; i<root->row_amount; i++)
	{
		root->array[i-1] = root->array[i];
		root->buffersize[i-1] = root->buffersize[i];
	}

	// now root->array[i] is a zombie

	root->array[root->row_amount-1] = row;
	root->buffersize[root->row_amount-1] = buffersize;

	// no zombie anymore

	root->row_amount--;

	return 1;

}

// return -2 to -4 : NULL pointer in parameters error
// return -5 : needle is empty
int charArray_removeLineThatMatchesEx(charArray * root, const char * needle, int start_at_row, int * out_row)
{
	//charArray * root = (charArray*) _root;
	int i=0;
	//int j=0;
	int found = 0;
	char * str_ret;
	char ** row;
	unsigned int * buffersize;
	//int debug = 0;

	if ( root == 0 )
	{
		return -2;
	}
	if ( needle == 0 )
	{
		return -3;
	}
	if ( strlen(needle) == 0 )
	{
		return -5;
	}
	
	if ( start_at_row < 0 )
	{
		start_at_row = 0;
	}
	if ( start_at_row >= root->row_amount )
	{
		start_at_row = root->row_amount-1;
	}

	for (i=start_at_row; i<root->row_amount; i++)
	{
		str_ret = strstr(root->array[i][0], needle);
		
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
		if ( debug ) printf("[removeLineThatMatches] searching for (%s) into (%s), result(0x%X)\n",
			needle, root->array[i][0], (unsigned int)str_ret);
#endif

		if ( str_ret )
		{
			found = 1;
			break;
		}
	}

	if ( found == 0 )
	{
		return 0;
	}

	free_row(root->array[i], root->buffersize[i], root->col_amount_max);

	// root->array[i] is going to be a zombie (it exists but has no place)
	row = root->array[i];
	buffersize = root->buffersize[i];

	if ( out_row ) 
	{
		*out_row = i;
	}

	// shift down the rest
	for (i=i+1; i<root->row_amount; i++)
	{
		root->array[i-1] = root->array[i];
		root->buffersize[i-1] = root->buffersize[i];
	}

	// now root->array[i] is a zombie

	root->array[root->row_amount-1] = row;
	root->buffersize[root->row_amount-1] = buffersize;

	// no zombie anymore

	root->row_amount--;
	
	if ( out_row && root->row_amount <= *out_row ) 
	{
		*out_row = root->row_amount-1;
	}

	return 1;

}


// Description: will malloc a new line, strcpy the line, increment root->row_amount
// return -1 : malloc error
int charArray_add(charArray * root, const char * const string)
{
	//int i=0;
	int index;
	int iret;
	char * new_array;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( root->row_amount >= root->row_amount_max )
	{
		// realloc
		iret = charArray_realloc(root, root->row_amount_max+CHAR_ARRAY_ROW_AMOUNT);
		if ( iret <= 0 )
		{
			if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
			{	return iret;
			}
			else
			{	return -20;
			}
		}
	}

	root->row_amount++;	
	index = root->row_amount-1;
	if ( root->array[index][0] == NULL )
	{
		new_array = (char*) malloc( sizeof(char)*(unsigned int)root->cell_length );
		if ( new_array == NULL )
		{
			root->row_amount--;
			if ( g_verbose >= 1 )
			{
				printf("[charArray_add] ERROR malloc of size(%d) failed\n", root->cell_length);
			}
			return TOOLBOX_ERROR_MALLOC;
		}
#if CHAR_ARRAY_DEBUG_MALLOC == 1
		snprintf(temp, sizeof(temp)-1, "[charArray_add] charArray(0x%X) root->array(0x%X) row(0x%X)\n", (unsigned int)root, (unsigned int)root->array, (unsigned int)new_array);
		Log_C(temp, 0);
#endif
		root->array[index][0] = new_array;
		root->buffersize[index][0] = root->cell_length;
	}

	if ( strlen(string) > (unsigned int) root->cell_length )
	{
		strncpy(root->array[index][0], string, (size_t) root->cell_length-1);
	}
	else
	{
		strcpy(root->array[index][0], string);
	}
	root->array[index][0][root->cell_length-1] = '\0';

	return 1;

}

int charArray_appendArray(charArray * root, charArray * const append)
{
	int i=0;
	int j=0;
	int iret;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( append == NULL )
	{
		return -3;
	}
	if ( append == root )
	{
		return -4;
	}

	for (i=0; i<append->row_amount; i++)
	{
		j=0;
		iret = charArray_add(root, append->array[i][j]);
		if ( iret > 0 )
		{
			// to prevent the case where add new line didn't work and we append valid col on previous row by mistake
			for (j=1; j<append->col_amount_max; j++)
			{
				if ( append->array[i][j] )
				{
					iret = charArray_col_append(root, root->row_amount-1, append->array[i][j]);
				}
			}
		}
	}

	return 1;
}


int charArray_insert(charArray * root, int before_position, const char * const string)
{
	int i=0;
	int j=0;
	int index;
	int iret;
	char * new_cell = NULL;
	char ** row_temp;
	unsigned int * row_sizes_temp;
	char *** array;
	unsigned int ** buffersize;
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	int inside_verbose=0;
#endif

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( before_position < 0 )
	{
		return -3;
	}
	if ( before_position > root->row_amount )
	{
		before_position = root->row_amount;
	}
	array = root->array;
	buffersize = root->buffersize;

	if ( root->row_amount >= root->row_amount_max )
	{
		// realloc		
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
		if ( g_verbose >= 1 )
		{
			printf("[charArray_insert] going to charArray_realloc() because root->row_amount(%d)\n",root->row_amount);
			inside_verbose = 1;
		}
#endif
		iret = charArray_realloc(root, root->row_amount_max+CHAR_ARRAY_ROW_AMOUNT); // will change row_amount_max but not row_amount
		array = root->array;
		buffersize = root->buffersize;
		if ( iret <= 0 )
		{
			if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
			{	return iret;
			}
			else
			{	return -20;
			}
		}
	}

	// shift towards end
	row_temp = array[root->row_amount];
	row_sizes_temp = buffersize[root->row_amount];

	if ( row_temp[0] == NULL )
	{
		new_cell = (char*) malloc( sizeof(char)*(unsigned int)root->cell_length );
		if ( new_cell == NULL )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
#if CHAR_ARRAY_DEBUG_MALLOC == 1
		snprintf(temp, sizeof(temp)-1, "[charArray_insert] charArray(0x%X) root->array(0x%X) row(0x%X)\n", (unsigned int)root, (unsigned int)root->array, (unsigned int)new_cell);
		Log_C(temp, 0);
#endif

		row_temp[0] = new_cell;
		row_sizes_temp[0] = root->cell_length;
	}
	
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	if ( inside_verbose >= 1 )
	{
		printf("[charArray_insert] after malloc of new_col\n");
		inside_verbose = 1;
	}
#endif
	
	// shift towards end
	for (i=root->row_amount; i>before_position; i--)
	{
		array[i] = array[i-1];
		buffersize[i] = buffersize[i-1];
	}

#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	if ( inside_verbose >= 1 )
	{
		printf("[charArray_insert] after shift down\n");
		inside_verbose = 1;
	}
#endif

	root->row_amount++;

	j = 0;
	
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	if ( inside_verbose >= 1 )
	{
		printf("[charArray_insert] before_position(%d)\n",before_position);
	}
#endif

	index = before_position;

#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	if ( inside_verbose >= 1 )
	{
		printf("[charArray_insert] root->array[%d](0x%X) = row_temp(0x%X)\n",
			index, (unsigned int)root->array[index], (unsigned int)row_temp );
	}
#endif

	root->array[index] = row_temp;
	root->buffersize[index] = row_sizes_temp;
	
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	if ( inside_verbose >= 1 )
	{
		printf("[charArray_insert] root->array[%d][%d](0x%X) = new_col(0x%X)\n",
			index, j, (unsigned int)root->array[index][j], (unsigned int)new_col );
	}
#endif
	// already done
	//root->array[index][j] = new_cell;
	//root->buffersize[index][j] = root->cell_length;

	// copy string into cell
	if ( strlen(string) >= (unsigned int) root->buffersize[index][j] )
	{
		root->array[index][j][0] = '\0';
		C_Append(&root->array[index][j], &root->buffersize[index][j], string, -1, NULL);
	}
	else
	{
		strcpy(root->array[index][j], string);
	}
	root->array[index][j][root->cell_length-1] = '\0';
	
#if CHAR_ARRAY_DEBUG_LEVEL >= 1
	if ( inside_verbose >= 1 )
	{
		printf("[charArray_insert] end of func\n");
		inside_verbose = 1;
	}
#endif

	return 1;
}

// 
int charArray_replace(charArray * root, int row, const char * const string)
{
	//int i=0;
	int j=0;
	int index;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( string == NULL )
	{
		return -2;
	}

	j=0;
	index = row;

	if ( index >= root->row_amount_max )
	{
		return TOOLBOX_OVERFLOW_OF_SOME_KIND;
	}

	if ( index >= root->row_amount )
	{
		// row to replace doesn't exists
		return -10;
	}

	if ( root->array[index] )
	{
		if ( root->array[index][j] )
		{
			memset(root->array[index][j], 0, root->cell_length);
			if ( strlen(string) > (unsigned int) root->cell_length )
			{
				strncpy(root->array[index][j], string, (size_t) root->cell_length-1);
			}
			else
			{
				strcpy(root->array[index][j], string);
			}
			root->array[index][j][root->cell_length-1] = '\0';
		}
		else
		{
			return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
		}
	}
	else
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
	}

	return 1;
}


int charArray_updateArray(charArray * root, charArray * valid)
{
	int iret;

	if ( root == 0 )
	{
		return -1;
	}
	if ( valid == 0 )
	{
		return -2;
	}
	if ( root->array == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( valid->array == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	charArray_Zero(root);

	iret = charArray_appendArray(root, valid);

	if ( iret > 0 )
	{
		return 1;
	}
	else
	{
		return iret;
	}
}



// return -2 to -4 : NULL pointer in parameters error
// return -5 : needle is empty
int charArray_cout(charArray * root)
{
	int i=0;
	int j=0;
	//int found=0;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	// copy old into new
	for (i=0; i<root->row_amount; i++)
	{
		if ( root->array[i] )
		{
			//printf("[charArray_print] row(%d): ", i);
			for ( j=0; j<root->col_amount_max; j++ )
			{
				if ( root->array[i][j] )
				{
					fputs(root->array[i][j], stdout);
				}
			}
			printf("\n");
		}
	}

	return 1;
}


// return -2 to -4 : NULL pointer in parameters error
// return -5 : needle is empty
int charArray_print(charArray * root)
{
	int i=0;
	int j=0;
	//int found=0;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	// copy old into new
	for (i=0; i<root->row_amount; i++)
	{
		if ( root->array[i] )
		{
			printf("[charArray_print] row(%d): ", i);
			for ( j=0; j<root->col_amount; j++ )
			{
				if ( root->array[i][j] )
				{
					printf("col(%d)=(%s)\t",j, root->array[i][j]);
				}
			}
			printf("\n");
		}
	}

	return 1;
}	

// if row == -1, means append on the last row
int charArray_col_append(charArray * root, int row, const char * const string)
{
	//int i=0;
	int j=0;
	int index;
	int found=0;
	char * new_array;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( string == NULL )
	{
		return -3;
	}

	if ( root->row_amount == 0 )
	{
		// use charArray_add first
		return TOOLBOX_ERROR_NOT_USING_CORRECTLY;
	}

	new_array = (char*) malloc( sizeof(char)*(unsigned int)root->cell_length );

	if ( new_array == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	j=0;
	index = row;

	if ( index >= root->row_amount_max )
	{
		return TOOLBOX_OVERFLOW_OF_SOME_KIND;
	}

	if ( index >= root->row_amount )
	{
		// row to append to doesn't exists
		return -10;
	}

	if ( index < 0 )
	{
		// select last row
		index = root->row_amount-1;
	}

	if ( root->array[index] == NULL )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
	}

	found=0;
	for ( j=0; j<root->col_amount_max; j++ )
	{
		if ( root->array[index][j] == NULL )
		{
			found=1;
			break;
		}
	}
	if ( found == 0 )
	{
		// did not find a free slot
		free(new_array);
		return -110;
	}

	root->array[index][j] = new_array;
	if ( strlen(string) > (unsigned int) root->cell_length )
	{
		strncpy(root->array[index][j], string, (size_t) root->cell_length-1);
	}
	else
	{
		strcpy(root->array[index][j], string);
	}
	root->array[index][j][root->cell_length-1] = '\0';
	root->buffersize[index][j] = root->cell_length;
	
#if CHAR_ARRAY_DEBUG_MALLOC == 1
	snprintf(temp, sizeof(temp)-1, "[charArray_col_append] charArray(0x%X) root->array(0x%X) jcell(0x%X)\n", (unsigned int)root, (unsigned int)root->array, (unsigned int)new_array);
	Log_C(temp, 0);
#endif
	if ( root->col_amount <= j )
	{
		root->col_amount = j+1;
	}

	return 1;

}

int charArray_splitTag(charArray * root, char * delimiter)
{
	charArray * tempLine_ptr=NULL;
	lineParser parser;
	int i,j;
	int iret;
	int a;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( delimiter == NULL )
	{
		return -3;
	}

	lineParser_Constructor(&parser);

	a = lineParser_setGreediness(0);

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			iret = lineParser_splitTag(&parser, root->array[i][0], delimiter, &tempLine_ptr);
			if ( iret > 0 && tempLine_ptr )
			{
				charArray_replace(root, i, tempLine_ptr->array[0][0]);
				for (j=1; j<tempLine_ptr->row_amount; j++)
				{
					charArray_col_append(root, i, tempLine_ptr->array[j][0]);
				}
			}
		}
	}

	lineParser_setGreediness(a);

	lineParser_Destructor(&parser);

	return 1;
}

int charArray_to_file(charArray * root, const char * const filename)
{
	FILE * file;
	int i,j;
	unsigned int str_len;
	//int debug = 0;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( filename == NULL )
	{
		return -2;
	}
	if ( strlen(filename) <= 0 )
	{
		return -3;
	}

	file = fopen(filename,"wb");

	if ( file == NULL )
	{
		if ( g_verbose >= 0 )
		{
			printf("Cannot open filename(%s)\n", filename);
		}
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			for (j=0; j<root->col_amount; j++)
			{
				if ( root->array[i][j] )
				{
					if (j>0)
					{
						fwrite("\t", 1, 1, file);
					}
					str_len = (unsigned int)strlen(root->array[i][j]);
					if ( str_len > 0 )
					{
						fwrite(root->array[i][j], 1, str_len, file);
					}
				}
			}
			fwrite("\n", 1, 1, file);
		}
	}

	fclose(file);

	return 1;
}

int charArray_to_fileEx(charArray * root, const char * const filename, int lineEnding)
{
	FILE * file;
	int i,j;
	unsigned int str_len;
	//int debug = 0;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( filename == NULL )
	{
		return -2;
	}
	if ( strlen(filename) <= 0 )
	{
		return -3;
	}

	file = fopen(filename,"wb");

	if ( file == NULL )
	{
		if ( g_verbose >= 0 )
		{
			printf("Cannot open filename(%s)\n", filename);
		}
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			for (j=0; j<root->col_amount; j++)
			{
				if ( root->array[i][j] )
				{
					if (j>0)
					{
						fwrite("\t", 1, 1, file);
					}
					str_len = (unsigned int)strlen(root->array[i][j]);
					if ( str_len > 0 )
					{
						fwrite(root->array[i][j], 1, str_len, file);
					}
				}
			}
			if ( lineEnding == TOOLBOX_LF )
			{
				fwrite("\n", 1, 1, file);
			}
			else
			{
				fwrite("\r\n", 1, 2, file);
			}
		}
	}

	fclose(file);

	return 1;
}


int charArray_to_file_joined(charArray * root, const char * const filename, char * const separator)
{
	FILE * file;
	int i,j;
	unsigned int str_len;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( filename == NULL )
	{
		return -2;
	}
	if ( strlen(filename) <= 0 )
	{
		return -3;
	}

	file = fopen(filename,"wb");

	if ( file == NULL )
	{
		if ( g_verbose >= 0 )
		{
			printf("Cannot open filename(%s)\n", filename);
		}
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			for (j=0; j<root->col_amount; j++)
			{
				if ( root->array[i][j] )
				{
					if (j>0)
					{
						fwrite(separator, 1, strlen(separator), file);
					}
					str_len = (unsigned int)strlen(root->array[i][j]);
					if ( str_len > 0 )
					{
						fwrite(root->array[i][j], 1, str_len, file);
					}
				}
			}
			fwrite("\n", 1, 1, file);
		}
	}
	
	fclose(file);

	return 1;
}

int charArray_to_FILE(charArray * root, FILE * const file)
{
	int i,j;
	unsigned int str_len;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( file == NULL )
	{
		return -2;
	}

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			for (j=0; j<root->col_amount; j++)
			{
				if ( root->array[i][j] )
				{
					if (j>0)
					{
						fwrite("\t", 1, 1, file);
					}
					str_len = (unsigned int)strlen(root->array[i][j]);
					if ( str_len > 0 )
					{
						fwrite(root->array[i][j], 1, str_len, file);
					}
				}
			}
			fwrite("\n", 1, 1, file);
		}
	}

	return 1;
}


int charArray_to_FILE_joined(charArray * root, FILE * const file, char * const separator)
{
	int i,j;
	unsigned int str_len;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( file == NULL )
	{
		return -2;
	}

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			for (j=0; j<root->col_amount; j++)
			{
				if ( root->array[i][j] )
				{
					if (j>0)
					{
						fwrite(separator, 1, strlen(separator), file);
					}
					str_len = (unsigned int)strlen(root->array[i][j]);
					if ( str_len > 0 )
					{
						fwrite(root->array[i][j], 1, str_len, file);
					}
				}
			}
			fwrite("\n", 1, 1, file);
		}
	}

	return 1;
}

// if buffer is NULL it will be malloced
// WARNING malloc not implemented
// WARNING duplicate function with array_to_buffer in toolbox_toolbox.c
// fixed 30.Mar.2010 (written-1 instead of written)
int charArray_to_buffer(charArray * root, char * buffer, unsigned int buffer_size)
{
	int i,j;
	unsigned int str_len_base;
	unsigned int str_len;
	unsigned int written = 1; // 1 is for \0
	uint8_t buffer_size_too_small = 0;
	uint8_t buffer_malloced = 0;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( buffer == NULL )
	{
		buffer_malloced = 1;
	}

	buffer[0] = '\0';
	buffer[buffer_size-1] = '\0';

	str_len_base = 0;

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			for (j=0; j<root->col_amount; j++)
			{
				if ( root->array[i][j] )
				{
					if (j>0)
					{
						written += 1;
						if ( written >= buffer_size )
						{
							buffer_size_too_small = 1;
							goto end_of_func;
						}
						strcpy(buffer+str_len_base, "\t");
						str_len_base = written;
					}
					str_len = strlen(root->array[i][j]);
					if ( str_len > 0 )
					{
						written += str_len;
						if ( written >= buffer_size )
						{
							buffer_size_too_small = 1;
							goto end_of_func;
						}
						strcpy(buffer+str_len_base, root->array[i][j]);
						str_len_base = written-1;
					}
				}
			}
			
#if 0
			written += 5; // frank fixed 28.Feb.2007 was 1 before
			if ( written >= buffer_size )
			{
				buffer_size_too_small = 1;
				goto end_of_func;
			}
			strcpy(buffer+str_len_base, "<br>\n");
#else
			written += 1;
			if ( written >= buffer_size )
			{
				buffer_size_too_small = 1;
				goto end_of_func;
			}
			strcpy(buffer+str_len_base, "\n");
#endif
			str_len_base = written-1;
		}
	}

end_of_func:

	if ( buffer_size_too_small )
	{
		return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
	}

	return 1;
}

// will not clear buffer (thus will append to the buffer)
// buffer must be terminated (if last byte is not \0, we will erase all the buffer)
int charArray_to_variable_buffer(charArray * root, char ** inout_string, unsigned int * inout_string_length)
{
	int i,j;
	unsigned int str_len;
	unsigned int new_size;
	char * new_string;
	unsigned int written = 1; // 1 is to save one byte for last \0
	unsigned int currentOffset;
	unsigned int initialOffset;
	unsigned int bytes_left = 0;

	if ( root == 0 )
	{
		return -1;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( inout_string == NULL || *inout_string == NULL || inout_string_length == NULL || *inout_string_length == 0 )
	{
		return -2;
	}

	if ( *inout_string_length > 0 )
	{
		// look at last byte of buffer to see if buffer content is well terminated
		// if not we consider buffer corrupted or uninitialized.
		if ( (*inout_string)[(*inout_string_length)-1] != '\0' )
		{
			memset(*inout_string, 0x00, *inout_string_length);
		}
	}

	if ( *inout_string_length < 1024*10 )
	{ 
		// too small
		new_string = (char*) realloc(*inout_string, 1024*10);
		if ( new_string == NULL )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		
		// do not memset, old content need to be preserved.
		// we assume old content was terminated.
		*inout_string = new_string;
		memset(*inout_string+*inout_string_length, 0x00, 1024*10 - *inout_string_length);
		*inout_string_length = 1024*10;
	}
	
	initialOffset = (unsigned int)strlen(*inout_string);
	currentOffset = initialOffset;
	bytes_left = (*inout_string_length) - initialOffset;

	for (i=0; i<root->row_amount; i++ )
	{	
		if ( root->array[i] )
		{
			for (j=0; j<root->col_amount; j++)
			{
				if ( root->array[i][j] )
				{
					if (j>0)
					{
						written += 1;
						if ( 1 >= bytes_left )
						{
							new_size = (*inout_string_length) + 1024;
							new_string = (char*) realloc(*inout_string, new_size);
							if ( new_string == NULL )
							{
								return TOOLBOX_ERROR_MALLOC;
							}
							// do not memset, old content need to be preserved.
							// we assume old content was terminated.
							*inout_string = new_string;
							memset(*inout_string+*inout_string_length, 0x00, new_size - *inout_string_length);
							*inout_string_length = new_size;
							bytes_left += 1024;
						}
						strcpy((*inout_string)+currentOffset, "\t");
						currentOffset += 1;
						bytes_left -= 1;
					}
					str_len = (unsigned int)strlen(root->array[i][j]);
					if ( str_len > 0 )
					{
						written += str_len;
						if ( str_len >= bytes_left )
						{
							new_size = (*inout_string_length) + str_len + 1; // I know the +1 is unnecessary
							new_string = (char*) realloc(*inout_string, new_size);
							if ( new_string == NULL )
							{
								return TOOLBOX_ERROR_MALLOC;
							}
							// do not memset, old content need to be preserved.
							// we assume old content was terminated.
							*inout_string = new_string;
							memset(*inout_string+*inout_string_length, 0x00, new_size - *inout_string_length);
							*inout_string_length = new_size;
							bytes_left += str_len + 1;
						}
						strcpy((*inout_string)+currentOffset, root->array[i][j]);
						currentOffset += str_len;
						bytes_left -= str_len;
					}
				}
			}
			
			written += 1;
			if ( 1 >= bytes_left )
			{
				new_size = (*inout_string_length) + 1024;
				new_string = (char*) realloc(*inout_string, new_size);
				if ( new_string == NULL )
				{
					return TOOLBOX_ERROR_MALLOC;
				}
				// do not memset, old content need to be preserved.
				// we assume old content was terminated.
				*inout_string = new_string;
				memset((*inout_string)+*inout_string_length, 0x00, new_size - *inout_string_length);
				*inout_string_length = new_size;
				bytes_left += 1024;
			}
			strcpy((*inout_string)+currentOffset, "\n");
			currentOffset += 1;
			bytes_left -= 1;
		}
	}

//end_of_func:

	return 1;
}



enum SORT_METHOD
{
	SORT_ASC=0,
	SORT_DES=1,
};

#if 1
//int array_qsort( charArray * root, int column=0, enum SORT_METHOD order=SORT_ASC, int first=0, int last=-2 );
int charArray_qsort(charArray * root, int column, int order, int first, int last)
{
	// $array  - the array to be sorted
	// $column - index (column) on which to sort
	//           can be a string if using an associative array
	// $order  - SORT_ASC (default) for ascending or SORT_DESC for descending
	// $first  - start index (row) for partial array sort
	// $last  - stop  index (row) for partial array sort
	// $keys  - array of key values for hash array sort

	//int i=0,j=0;

	char *** array;
	char ** row_alpha;
	char ** row_omega;
	char ** row_temporary;
	char * guess;

	int alpha;
	int omega;

	if ( root == 0 )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	array = root->array;

	//keys = array_keys(array);

	if (last == -2)
	{  
		last = root->row_amount - 1;
		//count($array) - 1;
	}
	if ( last > first )
	{
		alpha = first;
		omega = last;
		row_alpha = array[alpha];
		row_omega = array[omega];
		guess = row_alpha[column];
		if ( guess == NULL )
		{
			return -90;
		}
		while(omega >= alpha)
		{
			row_alpha = array[alpha]; // added this 16.Feb.2006, removed from end of loop
			row_omega = array[omega]; // added this 16.Feb.2006, removed from end of loop
			if (order == 0)// SORT_ASC
			{
				//while(row_alpha[column] < guess)
				while( strcasecmp(row_alpha[column],guess) < 0 )
				{
					alpha++; row_alpha = array[alpha]; 
				}
				//while(row_omega[column] > guess)
				while( strcasecmp(row_omega[column],guess) > 0 )
				{
					omega--; row_omega = array[omega]; 
				}
			}
			else
			{
				//while(row_alpha[column] > guess)
				while( strcasecmp(row_alpha[column],guess) > 0 )
				{
					alpha++; row_alpha = array[alpha]; 
				}
				//while(row_omega[column] < guess)
				while( strcasecmp(row_omega[column],guess) < 0 )
				{
					omega--; row_omega = array[omega]; 
				}
			}
			if (alpha > omega)
			{
				break;
			}
			row_temporary = array[alpha];
			array[alpha] = array[omega];
			// removed 16.Feb.2006, $key_alpha = $keys[$alpha];
			array[omega] = row_temporary;
			alpha++;
			omega--;
			// removed 16.Feb.2006, $key_omega = $keys[$omega];
		}
		charArray_qsort(root, column, order, first, omega);
		charArray_qsort(root, column, order, alpha, last);
	}

	return 1;
}
#endif

// written 06.May.2009
// returns 0 when both have the same array content
int charArray_cmp(const charArray * left, const charArray * right)
{
	int i;
	int iret;

	if ( left == NULL )
	{	return -1;
	}
	if ( right == NULL )
	{	return -2;
	}

	if ( left->row_amount != right->row_amount )
	{
		return -10;
	}
	
	for (i=0; i<left->row_amount; i++)
	{
		iret = strcmp(left->array[i][0], right->array[i][0]);
		if ( iret != 0 )
		{
			return -20;
		}
	}

	return 1;
}



