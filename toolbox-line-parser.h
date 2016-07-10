//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// line_parser.h
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
// All rights reserved
//
//       Created : 15.May.2006
//
//  Description: 
//      string manipulation and extraction functions 
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#ifndef __C_TOOLBOX_LINE_PARSER_H__
#define __C_TOOLBOX_LINE_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define USE_CHAR_ARRAY_FEATURES 1

#if USE_CHAR_ARRAY_FEATURES == 1
#include "toolbox-char-array.h"
#endif

#define LINE_PARSER_MAX_WORKSPACE 1024
#define LINE_PARSER_MAX_TAGSIZE 256

typedef struct lineParser
{
	char * workspace;
	char * workspace_ptr_end;
	int workspace_length;
#if USE_CHAR_ARRAY_FEATURES == 1
	charArray * char_array;
#endif
	int should_always_be_0x98389389;
} lineParser;


typedef struct lineParserW
{
	wchar_t * workspace;
	wchar_t * workspace_ptr_end;
	int workspace_length;
#if USE_CHAR_ARRAY_FEATURES == 1
	charArrayW * char_array;
#endif
	int should_always_be_0x98389388;
} lineParserW;

int lineParser_Constructor(lineParser * parser);
int lineParser_Destructor(lineParser * parser);
int lineParser_setGreediness(int greedy);
int lineParser_extractDashedParameter(lineParser * parser, const char * const line, char * const parameter, char ** out_insider); 
int lineParser_extractTag(lineParser * parser, const char * const line, char * const tag, char ** out_insider); 
int lineParser_replaceTag(lineParser * parser, const char * const line, char * const tag, const char * const replacement, char ** out_insider); 
int lineParser_extractInsider(lineParser * parser, const char * const line, char * const left, char * const right, char ** out_insider);
#if USE_CHAR_ARRAY_FEATURES == 1
int lineParser_splitTag(lineParser * parser, const char * const line, char * const tag, charArray ** out_array);
int lineParser_mergeTag(lineParser * parser, char ** out_line, const char * const tag, const charArray * const in_array);
#endif
int lineParser_extractKeyValuePair(lineParser * parser, const char * const line, char * const key, char ** out_insider_value);
int lineParser_replace(lineParser * parser, const char * const line, char * const search, char * const replacement, char ** out_insider); 
int lineParser_replaceAll(lineParser * parser, const char * const line, char * const search, char * const _replacement, char ** out_string);
int lineParser_stripSpacing(lineParser * parser, const char * const line, int trim_left, int trim_right, char ** out_insider);
int lineParser_extractFirstWord(lineParser * parser, const char * const line, char ** out_insider);


// Greediness 
// on splitTag, if two delimiters are next to each other, it will generate an empty element whatever the greediness
//   if there are several delimiters next to each other, on greedy==1 it will generate one empty element, on greedy==0 it will generate multiple empty element.
//   greedy == 1 is usefull when matching fixed length date string where the delimiter is spaces.
//   example: "2006  1 Feb, 12h34pm" and 
//            "2006 24 Feb,  1h03am"
// by default greediness is off

// [FUNCTION] lineParser_extractInsider
// [EXAMPLE]
// (input)
// line == "configuration.output.001.txt";
// left = "configuration.output.";
// right = ".txt";
// (output)
// out_insider = "001";

// [FUNCTION] lineParser_extractTag
// [EXAMPLE]
// (input)
// line == "something::HASH(231230302)::blabla";
// tag = "HASH";
// (output)
// out_insider = "231230302";

// [FUNCTION] lineParser_splitTag
// [EXAMPLE.1]
// (input)
// line == "username::password::blabla";
// tag = "::";
// (output)
// charArray[0][0] = "username";
// charArray[1][0] = "password";
// charArray[2][0] = "blabla";
//
// [EXAMPLE.2]
// (input)
// line == "::password::blabla";
// tag = "::";
// (output)
// charArray[0][0] = "";
// charArray[1][0] = "password";
// charArray[2][0] = "blabla";
//
// [EXAMPLE.3]
// (input)
// line == "password::";
// tag = "::";
// (output)
// charArray[0][0] = "password";
// charArray[1][0] = "";

// [FUNCTION] lineParser_mergetTag
// [EXAMPLE.1]
// (input)
// charArray[0][0] = "username";
// charArray[1][0] = "password";
// charArray[2][0] = "blabla";
// tag = "::";
// (output)
// out_line = "username::password::blabla";

// [FUNCTION] lineParser_replace
// [EXAMPLE]
// (input)
// line == "something::HASH(231230302)::blabla";
// search = "HASH(231230302)";
// replacement = "HASH(111)"
// (output)
// out_insider = "something::HASH(111)::blabla";

// [FUNCTION] lineParser_replaceAll
// [EXAMPLE]
// (input)
// line == "abcdefg";
// search = "2";
// replacement = "3"
// (output)
// out_insider = "abcdefg";
// return value (int) = 0 (because no changes)
// [EXAMPLE]
// (input)
// line == "to Frank's PC";
// search = "'";
// replacement = "\\'"
// (output)
// out_insider = "to Frank\'s PC";


int lineParser_ConstructorW(lineParserW * parser);
int lineParser_DestructorW(lineParserW * parser);
int lineParser_extractDashedParameterW(lineParserW * parser, const wchar_t * const line, wchar_t * const parameter, wchar_t ** out_insider); 
int lineParser_extractTagW(lineParserW * parser, const wchar_t * const line, wchar_t * const tag, wchar_t ** out_insider); 
int lineParser_replaceTagW(lineParserW * parser, const wchar_t * const line, wchar_t * const tag, const wchar_t * const replacement, wchar_t ** out_insider); 
int lineParser_extractInsiderW(lineParserW * parser, const wchar_t * const line, wchar_t * const left, wchar_t * const right, wchar_t ** out_insider);
#if USE_CHAR_ARRAY_FEATURES == 1
int lineParser_splitTagW(lineParserW * parser, const wchar_t * const line, wchar_t * const tag, charArrayW ** out_array);
int lineParser_mergeTagW(lineParserW * parser, wchar_t ** out_line, const wchar_t * const tag, const charArrayW * const in_array);
#endif
int lineParser_extractKeyValuePairW(lineParserW * parser, const wchar_t * const line, wchar_t * const key, wchar_t ** out_insider_value);
int lineParser_replaceW(lineParserW * parser, const wchar_t * const line, wchar_t * const search, wchar_t * const replacement, wchar_t ** out_insider); 
int lineParser_replaceAllW(lineParserW * parser, const wchar_t * const line, wchar_t * const search, wchar_t * const _replacement, wchar_t ** out_string);
int lineParser_stripSpacingW(lineParserW * parser, const wchar_t * const line, int trim_left, int trim_right, wchar_t ** out_insider);
int lineParser_extractFirstWordW(lineParserW * parser, const wchar_t * const line, wchar_t ** out_insider);




#ifdef __cplusplus
}
#endif

#endif //_LINE_PARSER_H_
