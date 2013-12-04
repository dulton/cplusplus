/*
 *   Copyright 2010 IMTC, Inc.  All rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>

#include "tip_debug_print.h"

#define DEBUG_DATE_FORMAT		"%Y-%m-%d"
#define DEBUG_TIME_FORMAT		"%T"
#define DEBUG_DATE_TIME_FORMAT	DEBUG_DATE_FORMAT" "DEBUG_TIME_FORMAT

#ifdef DEBUG_ON
FILE* LibTip::gDebugOutput = NULL;
#else
FILE* LibTip::gDebugOutput = stdout;
#endif
uint32_t LibTip::gDebugAreas = (
                                DEBUG_XMIT   |
                                DEBUG_RECV   |
                                DEBUG_USER   |
                                DEBUG_TIPNEG |
                                DEBUG_INTERR
                                );

uint32_t LibTip::gDebugFlags = (
#ifdef DEBUG_ON
                                PRINT_ENABLE   |
#endif
                                PRINT_TIME     |
                                PRINT_DATE     |
                                PRINT_AREA     |
                                PRINT_FUNCTION |
                                PRINT_NEWLINE  |
                                PRINT_UTC
                                );

LibTip::DebugPrintfFunc LibTip::gDebugPrintfFunc = &LibTip::DebugPrintf;
LibTip::DebugPrefixFunc LibTip::gDebugPrefixFunc = &LibTip::DebugPrefix;

void LibTip::DebugPrefix(const char* inFunction, const char* area,
                         const char* inFile, int inLine)
{
    if (!(gDebugFlags & PRINT_ENABLE)) {
        return;
    }
#ifdef DEBUG_ON
    if(!gDebugOutput){
        gDebugOutput = fopen("/tmp/tip.log","w");
        if(!gDebugOutput)
            return;
    }
#endif
    
	if (gDebugFlags & (PRINT_TIME | PRINT_DATE)) {
		struct timeval now;
		char buffer[128];
		
		(void) gettimeofday(&now, NULL);
		
		if ((gDebugFlags & (PRINT_TIME | PRINT_DATE)) == (PRINT_TIME | PRINT_DATE)) {
			if (gDebugFlags & PRINT_UTC) {
				(void) strftime(buffer, sizeof(buffer), DEBUG_DATE_TIME_FORMAT, gmtime(&now.tv_sec));
			} else {
				(void) strftime(buffer, sizeof(buffer), DEBUG_DATE_TIME_FORMAT, localtime(&now.tv_sec));
			}
			fprintf(gDebugOutput, "%s.%03lu: ", buffer, now.tv_usec / (long unsigned int) 1000);
		} else if (gDebugFlags & PRINT_TIME) {
			if (gDebugFlags & PRINT_UTC) {
				(void) strftime(buffer, sizeof(buffer), DEBUG_TIME_FORMAT, gmtime(&now.tv_sec));
			} else {
				(void) strftime(buffer, sizeof(buffer), DEBUG_TIME_FORMAT, localtime(&now.tv_sec));
			}
			fprintf(gDebugOutput, "%s.%03lu: ", buffer, now.tv_usec / (long unsigned int) 1000);
		} else { // just want the date
			if (gDebugFlags & PRINT_UTC) {
				(void) strftime(buffer, sizeof(buffer), DEBUG_DATE_FORMAT, gmtime(&now.tv_sec));
			} else {
				(void) strftime(buffer, sizeof(buffer), DEBUG_DATE_FORMAT, localtime(&now.tv_sec));
			}
			fprintf(gDebugOutput, "%s: ", buffer);
		}
	}

	if (gDebugFlags & PRINT_PROC) {
		fprintf(gDebugOutput, "[pid %u] ", getpid());
	}
	
	if (gDebugFlags & PRINT_AREA) {
        fprintf(gDebugOutput, "[%-6.6s] ", area);
	}
    
	if (gDebugFlags & PRINT_FUNCTION) {
		fprintf(gDebugOutput, "%s ", inFunction);
	}
	
	if (gDebugFlags & PRINT_FILE) {
		fprintf(gDebugOutput, "%s ", inFile);
	}
	
	if (gDebugFlags & PRINT_LINE) {
		fprintf(gDebugOutput, "%u ", inLine);
	}

	return;
}
  
void LibTip::DebugPrintf(const char* inFmt, ...)
{
    if (!(gDebugFlags & PRINT_ENABLE)) {
        return;
    }
    if(!gDebugOutput){
        gDebugOutput = fopen("/tmp/tip.log","w");
        if(!gDebugOutput)
            return;
    }
    
	va_list ap;
	va_start(ap, inFmt);
	vfprintf(gDebugOutput, inFmt, ap);
	va_end(ap);
	
	if (gDebugFlags & PRINT_NEWLINE) {
		fprintf(gDebugOutput, "\n");
	}

	(void) fflush(gDebugOutput);
	return;
}
