//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-flexstring.h
//					
// Copyright (c) 2015 Francois Oligny-Lemieux
// All rights reserved
//
//      Created: 12.Apr.2015
//
//  Description: Used fixed buffer if big enough, otherwise allocate.
//               The fixed buffer size should be chosen to get the best tradeoff from malloc calls and fixed memory usage.
//               Since most case of string use are small strings, we don't need to
//               dynamically malloc new char buffer for each flexstring object. We can
//               allocate a flexstring fully on the stack if small. However
//               in the case where very long string is saved, it will be dynamically
//               allocated under the hood. You are able to use the same functions to
//               work on both kind.
//
//  Limitations:
//
//  License: Yipp Dual Personal Open Source License and Business Monetary License
//           http://yipp.ca/licenses/dual-personal-open-source-business-monetary-license/
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __C_TOOLBOX_FLEXSTRING__
#define __C_TOOLBOX_FLEXSTRING__

#ifdef __cplusplus
extern "C" {
#endif

#define TOOLBOX_FLEXSTRING_LENGTH 256

typedef struct flexString_S
{
   char fixed[TOOLBOX_FLEXSTRING_LENGTH];
   char * buffer; // used if fixed buffer is not big enough, always has more priority than fixed.
	unsigned int buffersize; // if non-zero, this is the valid string
} flexString_T;

int flexstrcpy(flexString_T * base, const char * str);
int flexstrcasecmp(flexString_T * base, const char * str);
int flexstrcmp(flexString_T * base, const char * str);

#ifdef __cplusplus
}
#endif

#endif
