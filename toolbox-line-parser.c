//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-line-parser.c
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
// All rights reserved
//
//    Written by : Francois Oligny-Lemieux
//       Created : 15.May.2006
//      Modified : 22.Nov.2006 (added lineParser_extractFirstWord)
//      Modified : 07.Mar.2007 (modified lineParser_extractInsider to allow just left or right)
//
//  Description: 
//      string manipulation and extraction functions 
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "toolbox-line-parser.h"

int g_greedy = 0;

int lineParser_Constructor(lineParser * parser)
{
	if ( parser == 0 )
	{
		return -1;
	}

	parser->char_array = (charArray*) malloc( sizeof(charArray) );
	if ( parser->char_array == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	charArray_Constructor(parser->char_array, 0);

	parser->workspace = (char*) malloc( sizeof(char) * LINE_PARSER_MAX_WORKSPACE );
	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	parser->workspace[0]='\0';
	parser->should_always_be_0x98389389 = 0x98389389;

	return 1;
}

int lineParser_Destructor(lineParser * parser)
{
	if ( parser == 0 )
	{
		return -1;
	}

	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( parser->char_array )
	{
		if ( parser->char_array->array )
		{
			charArray_Destructor(parser->char_array);
		}
		free(parser->char_array);
		parser->char_array = NULL;
	}

	if ( parser->workspace )
	{
		free(parser->workspace);
		parser->workspace = NULL;
	}

	return 1;
}

int lineParser_setGreediness(int greedy)
{
	int oldval=g_greedy;
	if ( greedy > 0 )
	{
		g_greedy = 1;
	}
	else
	{
		g_greedy = 0;
	}

	return oldval;
}


int lineParser_extractDashedParameter(lineParser * parser, const char * const line, char * const parameter, char ** out_value)
{
	char * workspace_ptr;
	char * str_ret;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	int length;
	unsigned char doubleQuotes = 0;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( parameter == NULL )
	{
		return -3;
	}
	if ( out_value == NULL )
	{
		return -4;
	}

	*out_value = NULL;
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( strlen(parameter) > LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}

	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	workspace_ptr = parser->workspace;
	strcpy(tag_start, "-");
	strcat(tag_start, parameter);
	strcat(tag_start, " ");

	str_ret = strstr(line, tag_start);
	if ( str_ret == NULL )
	{
		// cannot find HASH tag
		return -30;
	}

	if ( str_ret != line
		&& str_ret[-1] != ' ' )
	{
		// not allowed, there must be a space before - or it must be the beginning of the line
		return -31;
	}

	if ( strlen(str_ret) < LINE_PARSER_MAX_WORKSPACE )
	{
		strcpy(workspace_ptr, str_ret);
	}
	else
	{
		// internal error
		return -32; 
	}

	workspace_ptr += strlen(tag_start);

	if ( workspace_ptr[0] == '"' )
	{
		doubleQuotes = 1;
	}

	if ( doubleQuotes )
	{
		workspace_ptr++;
		str_ret = strchr(workspace_ptr, '"');
	}
	else
	{
		str_ret = strstr(workspace_ptr, " ");
		if ( str_ret == NULL )
		{
			str_ret = workspace_ptr + strlen(workspace_ptr);
		}
	}

	if ( str_ret == NULL )
	{
		// cannot find the end of HASH tag
		return -40;
	}

	length = str_ret - workspace_ptr;

	if ( length < 0 )
	{
		// invalid length
		return -41;
	}

	workspace_ptr[length] = '\0';

	*out_value = workspace_ptr;	

	return 1;
}

int lineParser_extractTag(lineParser * parser, const char * const line, char * const tag, char ** out_string)
{
	char * workspace_ptr;
	char * str_ret;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	int length;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( tag == NULL )
	{
		return -3;
	}
	if ( out_string == NULL )
	{
		return -4;
	}
	
	*out_string = NULL;
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( strlen(tag) > LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}

	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	workspace_ptr = parser->workspace;
	strcpy(tag_start, tag);
	strcat(tag_start, "(");

	str_ret = strstr(line, tag_start);
	if ( str_ret == NULL )
	{
		// cannot find HASH tag
		return -30;
	}

	if ( strlen(str_ret) < LINE_PARSER_MAX_WORKSPACE )
	{
		strcpy(workspace_ptr, str_ret);
	}
	else
	{
		// internal error
		return -31; 
	}

	workspace_ptr += strlen(tag_start);

	str_ret = strstr(workspace_ptr, ")");

	if ( str_ret == NULL )
	{
		// cannot find the end of HASH tag
		return -40;
	}

	length = str_ret - workspace_ptr;

	if ( length <= 0 )
	{
		// invalid length
		return -41;
	}

	workspace_ptr[length] = '\0';

	*out_string = workspace_ptr;	

	return 1;
}

int lineParser_replaceTag(lineParser * parser, const char * const line, char * const tag, const char * const replacement, char ** out_insider)
{
	char * workspace_ptr;
	char * str_ret;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	char line_reconstructed[LINE_PARSER_MAX_WORKSPACE];
	int length;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( tag == NULL )
	{
		return -3;
	}
	if ( out_insider == NULL )
	{
		return -4;
	}

	*out_insider = NULL;
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( strlen(tag) > LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}

	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	workspace_ptr = parser->workspace;
	strcpy(tag_start, tag);
	strcat(tag_start, "(");

	str_ret = strstr(line, tag_start);
	if ( str_ret == NULL )
	{
		// cannot find HASH tag
		return -30;
	}

	strcpy(line_reconstructed, line);

	length = str_ret - line;
 	if ( length < LINE_PARSER_MAX_WORKSPACE )
	{
		line_reconstructed[length] = '\0';
	}
	else
	{
		return -31;  // no enough space
	}

	if ( strlen(line_reconstructed) + strlen(tag_start) < LINE_PARSER_MAX_WORKSPACE )
	{
		strcat(line_reconstructed, tag_start);
	}
	else
	{
		return -32;  // no enough space
	}

	if ( strlen(line_reconstructed) + strlen(replacement) < LINE_PARSER_MAX_WORKSPACE )
	{
		strcat(line_reconstructed, replacement);
	}
	else
	{
		return -33;  // no enough space for your replacement
	}

	str_ret = strstr(str_ret, ")");

	if ( str_ret == NULL )
	{
		// cannot find the end of HASH tag
		return -40;
	}

	if ( strlen(line_reconstructed) + strlen(str_ret) < LINE_PARSER_MAX_WORKSPACE )
	{
		strcat(line_reconstructed, str_ret);
	}
	else
	{
		return -34;  // no enough space for your replacement
	}

	strcpy(parser->workspace, line_reconstructed);
	workspace_ptr[LINE_PARSER_MAX_WORKSPACE-1] = '\0';

	*out_insider = parser->workspace;

	return 1;
}




// (input)
// line == "configuration.output.001.txt";
// left = "configuration.output.";
// right = ".txt";
// (output)
// out_insider = "001";
int lineParser_extractInsider(lineParser * parser, const char * const line, char * const left, char * const right, char ** out_insider)
{
	char * workspace_ptr;
	char * str_ret;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	//int length;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( left == NULL && right == NULL )
	{
		return -3;
	}
	if ( out_insider == NULL )
	{
		return -5;
	}

	*out_insider = NULL;
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( strlen(line) >= LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( left && strlen(left) >= LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}
	if ( right && strlen(right) >= LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -12;
	}

	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	workspace_ptr = parser->workspace;
	if ( left )
	{
		strcpy(tag_start, left);
		str_ret = strstr(line, tag_start);
		if ( str_ret == NULL )
		{
			// cannot find left
			return -30;
		}
		
		if ( strlen(str_ret) < LINE_PARSER_MAX_WORKSPACE )
		{
			strcpy(workspace_ptr, str_ret);
		}
		else
		{
			// internal error
			return -31; 
		}
		workspace_ptr += strlen(tag_start);
	}
	else
	{
		strcpy(workspace_ptr, line);
	}

	if ( right && strlen(right) > 0 )
	{
		str_ret = strstr(workspace_ptr, right);
	}
	else
	{
		str_ret = workspace_ptr + strlen(workspace_ptr);
	}

	if ( str_ret == NULL )
	{
		// cannot find right
		// we just removed the left part...
		*out_insider = workspace_ptr;
		return -40;
	}

	str_ret[0] = '\0';

	*out_insider = workspace_ptr;

	return 1;
}

#if USE_CHAR_ARRAY_FEATURES == 1
int lineParser_splitTag(lineParser * parser, const char * const line, char * const delimiter, charArray ** array)
{
	int iret;
	//char * workspace_ptr;
	char * str_ret;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	char temp[LINE_PARSER_MAX_WORKSPACE];	
	char next_add[LINE_PARSER_MAX_WORKSPACE];
	char * temp_ptr = temp;
	int length;
	int greedy=g_greedy;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( parser->char_array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( delimiter == NULL )
	{
		return -3;
	}
	if ( array == NULL )
	{
		return -4;
	}

	*array = NULL;
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( parser->char_array->array )
	{
		charArray_Destructor(parser->char_array);
		memset(parser->char_array, 0, sizeof(charArray) );
	}

	iret = charArray_Constructor(parser->char_array, 0);
	if ( iret <= 0 )
	{
		return iret;
	}

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( strlen(delimiter) > LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}
	if ( strlen(delimiter) == 0 )
	{
		// empty or corrupted input tag
		return -12;
	}

	strcpy(tag_start, delimiter);
	strcpy(temp, line);

	while ( strlen(temp_ptr) > 0 )
	{
		str_ret = strstr(temp_ptr, tag_start);
		if ( str_ret == NULL )
		{
			// cannot find delimiter tag
			if ( strlen(temp_ptr) > 0 && strlen(temp_ptr) < LINE_PARSER_MAX_WORKSPACE )
			{
				strcpy(next_add, temp_ptr);
				charArray_add(parser->char_array, next_add);
			}
			break;
		}

		length = str_ret - temp_ptr;

		if ( length > 0 )
		{
			strncpy(next_add, temp_ptr, length);
			next_add[length]='\0';
			charArray_add(parser->char_array, next_add);
			temp_ptr += length;
			if ( strcmp(temp_ptr,tag_start)==0 )
			{
				// ends with tag
				next_add[0]='\0';
				charArray_add(parser->char_array, next_add);
			}
			temp_ptr += strlen(tag_start);
		}
		else if ( length==0 && strlen(temp_ptr) > (unsigned int) length )
		{
			// empty field
			if ( greedy )
			{
				// do nothing
			}	
			else 
			{				
				next_add[0]='\0';
				charArray_add(parser->char_array, next_add);
			}
			temp_ptr += strlen(tag_start);
		}
		else
		{
			if ( parser->char_array->row_amount == 0 )
			{
				// probably TAG is on an empty line or line begins with TAG
				// skip first TAG
				if ( strlen(temp_ptr) > strlen(tag_start) )
				{
					charArray_add(parser->char_array, "");
					temp_ptr += strlen(tag_start);
				}
				else
				{
					// only have tag on line
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	*array = parser->char_array;

	return 1;
}

int lineParser_mergeTag(lineParser * parser, char ** out_line, const char * const tag, const charArray * const in_array)
{	
	int i;
	int error=0;
	char workspace[LINE_PARSER_MAX_WORKSPACE]="";
	char * element;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( parser->char_array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( out_line == NULL )
	{
		return -2;
	}
	if ( tag == NULL )
	{
		return -3;
	}
	if ( in_array == NULL )
	{
		return -4;
	}
	if (  in_array->array == NULL)
	{
		return -5; // argument 4 not initialized
	}
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( parser->char_array == in_array )
	{
		// im merging my own array
	}
	
	for ( i=0; i < in_array->row_amount; i++ )
	{
		element = in_array->array[i][0];
		if ( strlen(workspace)+strlen(element)+2 < LINE_PARSER_MAX_WORKSPACE )
		{
			strcat(workspace, element);
			strcat(workspace, "::");
		}
		else
		{
			error = -15;
			break;
		}
	}
	workspace[LINE_PARSER_MAX_WORKSPACE-1]='\0';
	if ( strlen(workspace)>2 )
	{
		workspace[strlen(workspace)-2]='\0';
	}

	strcpy(parser->workspace, workspace);

	*out_line = parser->workspace;

	if ( error < 0 )
	{
		return error;
	}

	return 1;
}
#endif // USE_CHAR_ARRAY_FEATURES == 1

int lineParser_extractKeyValuePair(lineParser * parser, const char * const line, char * const key, char ** out_insider_value)
{
	char * workspace_ptr;
	char * str_ret;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	//int length;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( key == NULL )
	{
		return -3;
	}
	if ( out_insider_value == NULL )
	{
		return -5;
	}

	*out_insider_value = NULL;

	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( strlen(key) > LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}

	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	workspace_ptr = parser->workspace;
	strcpy(tag_start, key);

	str_ret = strstr(line, tag_start);
	if ( str_ret == NULL )
	{
		// cannot find left
		return -30;
	}

	if ( strlen(str_ret) < LINE_PARSER_MAX_WORKSPACE )
	{
		strcpy(workspace_ptr, str_ret);
	}
	else
	{
		// internal error
		return -31; 
	}

	workspace_ptr += strlen(tag_start);

	str_ret = strstr(workspace_ptr, ";");
	if ( str_ret == NULL )
	{
		str_ret = workspace_ptr + strlen(workspace_ptr);
	}

	if ( str_ret == NULL )
	{
		// cannot find right
		// we just removed the left part...
		*out_insider_value = workspace_ptr;
		return -40;
	}

	str_ret[0] = '\0';

	*out_insider_value = workspace_ptr;

	return 1;
}

int lineParser_replace(lineParser * parser, const char * const line, char * const search, char * const _replacement, char ** out_string)
{
	char * workspace_ptr;
	char * str_ret;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	char input[LINE_PARSER_MAX_WORKSPACE];
	char replacement[LINE_PARSER_MAX_WORKSPACE];
	int length;
	int fret=1;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( search == NULL )
	{
		return -3;
	}
	if ( _replacement == NULL )
	{
		return -4;
	}
	if ( out_string == NULL )
	{
		return -5;
	}

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( strlen(search) > LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( parser->workspace == NULL )
	{
		// malloc error
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	strcpy(input,line);
	strcpy(replacement,_replacement);

	workspace_ptr = parser->workspace;
	workspace_ptr[0] = '\0';

	if ( strlen(search) == 0 )
	{		
		*out_string = workspace_ptr;	
		return -6;
	}

	strcpy(tag_start, search);

	str_ret = strstr(input, tag_start);
	if ( str_ret == NULL )
	{
		// cannot find HASH tag
		strcpy(workspace_ptr, input);
		*out_string = workspace_ptr;	
		return 0;
	}	

	length = str_ret - input;
	if ( length )
	{
		strncpy(workspace_ptr, input, length);
		workspace_ptr[length] = '\0';
	}

	if ( strlen(replacement) + strlen(input) - strlen(search) < LINE_PARSER_MAX_WORKSPACE )
	{
		strcat(workspace_ptr, replacement);
	}
	else
	{
		printf(" strlen(replacement)=%u, strlen(input)=%u, strlen(search)=%u<br>\n", strlen(replacement),strlen(input),strlen(search));
		printf(" replacement(%s), input(%s), search(%s)<br>\n", replacement,input,search);
		fret = -12;
	}

	str_ret += strlen(search);

	if ( strlen(str_ret) < strlen(input) )
	{
		// should never overflow if was ok initially because we only removed
		strcat(workspace_ptr, str_ret);
	}

	workspace_ptr[LINE_PARSER_MAX_WORKSPACE-1] = '\0';

	*out_string = workspace_ptr;	

	return fret;
}


int lineParser_replaceAll(lineParser * parser, const char * const line, char * const search, char * const _replacement, char ** out_string)
{
	char * workspace_ptr;
	char * str_ret;
	char * str_ret_base;
	char tag_start[LINE_PARSER_MAX_TAGSIZE];
	char input[LINE_PARSER_MAX_WORKSPACE];
	char replacement[LINE_PARSER_MAX_WORKSPACE];
	int length;
	unsigned int total_length=0;
	int fret=1;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( search == NULL )
	{
		return -3;
	}
	if ( _replacement == NULL )
	{
		return -4;
	}
	if ( out_string == NULL )
	{
		return -5;
	}

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( strlen(search) > LINE_PARSER_MAX_TAGSIZE )
	{
		// too large or corrupted input tag
		return -11;
	}
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( parser->workspace == NULL )
	{
		// malloc error
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	strcpy(input,line);
	strcpy(replacement,_replacement);

	workspace_ptr = parser->workspace;
	workspace_ptr[0] = '\0';

	if ( strlen(search) == 0 )
	{		
		*out_string = workspace_ptr;	
		return -6;
	}

	strcpy(tag_start, search);

	str_ret = strstr(input, tag_start);
	if ( str_ret == NULL )
	{
		// cannot find HASH tag
		strcpy(workspace_ptr, input);
		*out_string = workspace_ptr;	
		return 0;
	}	

	str_ret_base = &input[0];

	while (1)
	{
		length = str_ret - str_ret_base;
		if ( length > 0 )
		{
			total_length += (unsigned int) length;
			// copy part before tag
			strncat(workspace_ptr, str_ret_base, length);
			workspace_ptr[total_length] = '\0';
		}

		if ( strlen(replacement) + strlen(str_ret_base) - strlen(search) < LINE_PARSER_MAX_WORKSPACE )
		{
			strcat(workspace_ptr, replacement);
			total_length += strlen(replacement);
		}
		else
		{
			printf(" strlen(replacement)=%u, strlen(str_ret_base)=%u, strlen(search)=%u<br>\n", strlen(replacement),strlen(str_ret_base),strlen(search));
			printf(" replacement(%s), str_ret_base(%s), search(%s)<br>\n", replacement,str_ret_base,search);
			fret = -12;
		}

		str_ret += strlen(search);

		// now str_ret points to after the tag.

		if ( strlen(str_ret) < strlen(str_ret_base) )
		{
			// should never overflow if was ok initially because we only removed
			//strcat(workspace_ptr, str_ret); // append the rest of the string
		}

		str_ret_base = str_ret;
		str_ret = strstr(str_ret_base, tag_start);
		if ( str_ret == NULL )
		{
			// cannot find HASH tag
			// append rest of string	
			if ( strlen(str_ret_base) + strlen(workspace_ptr) < LINE_PARSER_MAX_WORKSPACE )
			{
				strcat(workspace_ptr, str_ret_base);
				total_length += strlen(str_ret_base);
			}
			else
			{
				//error, remplacement is too big for input string.
				fret = -13;
			}
			break;
		}
	}	


	workspace_ptr[LINE_PARSER_MAX_WORKSPACE-1] = '\0';

	*out_string = workspace_ptr;	

	return fret;
}



int lineParser_stripSpacing(lineParser * parser, const char * const line, int trim_left, int trim_right, char ** out_insider)
{
	char * workspace_ptr;
	//char * str_ret;
	char temp[LINE_PARSER_MAX_WORKSPACE];
	char * temp_ptr = &temp[0];
	char * const temp_ptr_end = &temp[LINE_PARSER_MAX_WORKSPACE-1];
	int length;
	unsigned int str_len;

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( out_insider == NULL )
	{
		return -5;
	}

	*out_insider = NULL;

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}
	
	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	workspace_ptr = parser->workspace;
	temp[LINE_PARSER_MAX_WORKSPACE-1] = '\0';
	strncpy(temp, line, LINE_PARSER_MAX_WORKSPACE);

	if ( trim_left > 0 )
	{
		while ( temp_ptr < temp_ptr_end && *temp_ptr == ' ' )
		{
			temp_ptr++;
		}
	}

	str_len = strlen(temp_ptr);
	length = temp_ptr_end - temp_ptr;
	if ( str_len <= (unsigned int) length )
	{
		memmove(temp, temp_ptr, strlen(temp_ptr)+1 );
	}

	memset(workspace_ptr, 0, LINE_PARSER_MAX_WORKSPACE);
	strcpy(workspace_ptr, temp);

	if ( trim_right > 0 )
	{
		temp_ptr = temp_ptr_end;
		while ( temp_ptr > &temp[0] )
		{
			if ( *temp_ptr == '\0' )
			{
				temp_ptr--;
			}
			else if ( *temp_ptr == ' ' )
			{
				*temp_ptr = '\0';
				temp_ptr--;
			}
			else if ( *temp_ptr == '\n' )
			{
				*temp_ptr = '\0';
				temp_ptr--;
			}
			else
			{
				break;
			}
		}
		strcpy(workspace_ptr, temp);
	}

	*out_insider = workspace_ptr;

	return 1;
}


int lineParser_extractFirstWord(lineParser * parser, const char * const line, char ** out_insider)
{
	char * workspace_ptr;
	char * str_ret;
	char temp[LINE_PARSER_MAX_WORKSPACE];
	char * temp_ptr = &temp[0];

	if ( parser == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( out_insider == NULL )
	{
		return -5;
	}

	*out_insider = NULL;

	if ( strlen(line) > LINE_PARSER_MAX_WORKSPACE )
	{
		// too large or corrupted input line
		return -10;
	}

	if ( parser->should_always_be_0x98389389 != 0x98389389 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( parser->workspace == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	workspace_ptr = parser->workspace;
	temp[LINE_PARSER_MAX_WORKSPACE-1] = '\0';
	strncpy(temp, line, LINE_PARSER_MAX_WORKSPACE);

	str_ret = strstr(temp_ptr, " ");

	if ( str_ret == NULL )
	{
		if ( strlen(temp_ptr) == 0 )
		{
			// empty string;
			*out_insider = NULL;
			return 0;
		}
		else
		{
			strcpy(workspace_ptr, temp_ptr);
			*out_insider = workspace_ptr;
			return 1;
		}
	}

	*str_ret = '\0';

	strcpy(workspace_ptr, temp_ptr);
	*out_insider = workspace_ptr;

	return 1;

}

