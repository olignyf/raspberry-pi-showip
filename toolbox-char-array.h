//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//						 
// toolbox-char-array.h
//					
// Copyright (c) 2006 Francois Oligny-Lemieux
// All rights reserved
//
//      Created: 22.Sep.2006	
//
//  Description: 
//      One line per row, gives easy access to any row.
//      Has substitution function to match and replace.
//      Has search functions to get a line that matches a keyword
//
//  Limitations:
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __C_TOOLBOX_CHAR_ARRAY_H__
#define __C_TOOLBOX_CHAR_ARRAY_H__

#include <stdio.h>

#include "toolbox-errors.h" 

#ifdef __cplusplus
extern "C" {
#endif

enum TOOLBOX_SORT_METHOD
{
	TOOLBOX_SORT_ASC=0,
	TOOLBOX_SORT_DES=1,
};

extern char php_message_buffer[200000];
extern int g_verbose; // -1 means absolutely no output, 0 means only errors, 1 means some debug, 2 means all debug info

#define CHAR_ARRAY_NAME_LENGTH 128

enum TOOLBOX_PARSE_OPTIONS
{
	TOOLBOX_NO_PARSE_OPTIONS = 0,
	TOOLBOX_ALL = 1, 
	TOOLBOX_SINGLE_REPLACE = 2,
	TOOLBOX_LAST_PARSE_OPTION
};

typedef struct charArray
{
	int row_amount;
	int row_amount_max;
	int index; // current index
	int col_amount;
	int col_amount_max;
	char *** array;
	unsigned int ** buffersize;
	char name[CHAR_ARRAY_NAME_LENGTH];
	int cell_length;
	int reader_chop_empty_lines;
	int should_always_be_0x56465646;
} charArray;

typedef struct charArrayW
{
	int row_amount;
	int row_amount_max;
	int index; // current index
	int col_amount;
	int col_amount_max;
	wchar_t *** array;
	unsigned int ** buffersize;
	wchar_t name[CHAR_ARRAY_NAME_LENGTH];
	int cell_length;
	int reader_chop_empty_lines;
	int should_always_be_0x56465647;
} charArrayW;

int charArray_getIndexWithKey(charArray * root, const char * needle, int * key_obj);
int charArray_getColumnWithKey(charArray * root, const char * needle, int column, char ** out_insider);
int charArray_getLineThatMatches(charArray * root, const char * needle, char ** out_insider);
int charArray_getLineThatMatchesEx(charArray * root, const char * needle, int start_at_row, int stop_at_row, int * out_row, char ** out_insider);
int charArray_getLineThatIs(charArray * root, const char * line, char ** out_insider);
int charArray_getLine(charArray * root, int row, char ** out_insider);
int charArray_substituteInLineThatMatches(charArray * root, const char * needle, const char * const replacement, int toolbox_parse_options);
int charArray_substituteArrayInLineThatMatches(charArray * root, const char * needle, charArray * const replacement);
int charArray_removeLineWithIndex(charArray * root, int index);
int charArray_removeLineWithIndexAndAllBefore(charArray * root, int index);
int charArray_removeLineThatIs(charArray * root, char * lookup);
int charArray_removeLineThatMatches(charArray * root, const char * needle);
int charArray_removeLineThatMatchesEx(charArray * root, const char * needle, int start_at_row, int * out_row);
int charArray_add(charArray * root, const char * const string);
int charArray_col_append(charArray * root, int row, const char * const string);
int charArray_appendArray(charArray * root, charArray * const append);
int charArray_insert(charArray * root, int before_position, const char * const string);
int charArray_replace(charArray * root, int row, const char * const string);
int charArray_updateArray(charArray * root, charArray * valid);
int charArray_Constructor(charArray * root, int _amount_max);
int charArray_Destructor(charArray * root);
int charArray_Zero(charArray * root); // used to zero out instead of destructing, useful if needs to be reused.
int charArray_cout(charArray * root);
int charArray_print(charArray * root);
int charArray_splitTag(charArray * root, char * tag);
int charArray_to_file(charArray * root, const char * const filename);
int charArray_to_fileEx(charArray * root, const char * const filename, int lineEnding);
int charArray_to_file_joined(charArray * root, const char * const filename, char * const separator);
int charArray_to_FILE(charArray * root, FILE * const file);
int charArray_to_FILE_joined(charArray * root, FILE * const file, char * const separator);
int charArray_to_buffer(charArray * root, char * buffer, unsigned int buffer_size);
int charArray_to_variable_buffer(charArray * root, char ** inout_buffer, unsigned int * inout_buffer_size);
int charArray_qsort(charArray * root, int column, int order, int first, int last);
int charArray_cmp(const charArray * left, const charArray * right);

int regexp_match(const char * const line, const char * const pattern);

/* charArray Documentation

Index means row number (int) starting count at 0.
Row means row number (int) starting count at 0.
In all instances, we *never* refer to a row starting count at 1.

charArray_substituteInLineThatMatches
 will find and replace all occurances of needle with replacement

charArray_removeLineThatMatches
 will loop strstr on first cell of each row and remove the first row that strstr != NULL

charArray_removeLineThatMatchesEx
 out_row will be assigned the next row that was removed, (and now refering to the next row)
 which makes it suitable to start searching again.
 charArray_removeLineThatMatchesEx


*/


// wide chars //
int charArray_getIndexWithKeyW(charArrayW * root, const wchar_t * needle, int * key_obj);
int charArray_getColumnWithKeyW(charArrayW * root, const wchar_t * needle, int column, wchar_t ** out_insider);
int charArray_getLineThatMatchesW(charArrayW * root, const wchar_t * needle, wchar_t ** out_insider);
int charArray_getLineThatMatchesExW(charArrayW * root, const wchar_t * needle, int start_at_row, int stop_at_row, int * out_row, wchar_t ** out_insider);
int charArray_getLineThatIsW(charArrayW * root, const wchar_t * line, wchar_t ** out_insider);
int charArray_getLineW(charArrayW * root, int row, wchar_t ** out_insider);
int charArray_substituteInLineThatMatchesW(charArrayW * root, const wchar_t * needle, const wchar_t * const replacement, int toolbox_parse_options);
int charArray_substituteArrayInLineThatMatchesW(charArrayW * root, const wchar_t * needle, charArrayW * const replacement);
int charArray_removeLineWithIndexW(charArrayW * root, int index);
int charArray_removeLineWithIndexAndAllBeforeW(charArrayW * root, int index);
int charArray_removeLineThatIsW(charArrayW * root, wchar_t * lookup);
int charArray_removeLineThatMatchesW(charArrayW * root, const wchar_t * needle);
int charArray_removeLineThatMatchesExW(charArrayW * root, const wchar_t * needle, int start_at_row, int * out_row);
int charArray_addW(charArrayW * root, const wchar_t * const string);
int charArray_col_appendW(charArrayW * root, int row, const wchar_t * const string);
int charArray_appendArrayW(charArrayW * root, charArrayW * const append);
int charArray_insertW(charArrayW * root, int before_position, const wchar_t * const string);
int charArray_replaceW(charArrayW * root, int row, const wchar_t * const string);
int charArray_updateArrayW(charArrayW * root, charArrayW * valid);
int charArray_ConstructorW(charArrayW * root, int _amount_max);
int charArray_DestructorW(charArrayW * root);
int charArray_ZeroW(charArrayW * root); // used to zero out instead of destructing, useful if needs to be reused.
int charArray_coutW(charArrayW * root);
int charArray_printW(charArrayW * root);
int charArray_splitTagW(charArrayW * root, wchar_t * tag);
int charArray_to_fileW(charArrayW * root, const wchar_t * const filename);
int charArray_to_fileExW(charArrayW * root, const wchar_t * const filename, int lineEnding);
int charArray_to_file_joinedW(charArrayW * root, const wchar_t * const filename, wchar_t * const separator);
int charArray_to_FILEW(charArrayW * root, FILE * const file);
int charArray_to_FILE_joinedW(charArrayW * root, FILE * const file, wchar_t * const separator);
int charArray_to_bufferW(charArrayW * root, wchar_t * buffer, unsigned int buffer_size);
int charArray_to_variable_bufferW(charArrayW * root, wchar_t ** inout_buffer, unsigned int * inout_buffer_size);
int charArray_qsortW(charArrayW * root, int column, int order, int first, int last);
int charArray_cmpW(const charArrayW * left, const charArrayW * right);

int regexp_matchW(const wchar_t * const line, const wchar_t * const pattern);



#ifdef __cplusplus
}
#endif

#endif //_CHAR_ARRAY_H_

