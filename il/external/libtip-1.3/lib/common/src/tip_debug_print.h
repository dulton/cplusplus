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

#ifndef TIP_DEBUG_PRINT_H
#define TIP_DEBUG_PRINT_H

#include <stdio.h>
#include <stdint.h>

namespace LibTip {
    /**
     * Debug flags.  These flags control what header data is printed
     * prior to each log message.
     */
    enum {
        PRINT_ENABLE         = (1 << 0),
        PRINT_TIME           = (1 << 1),
        PRINT_DATE           = (1 << 2),
        PRINT_PROC           = (1 << 3),
        PRINT_AREA           = (1 << 4),
        PRINT_FUNCTION       = (1 << 5),
        PRINT_FULL_FUNCTION  = (1 << 6),
        PRINT_FILE           = (1 << 7),
        PRINT_LINE           = (1 << 8),
        PRINT_NEWLINE        = (1 << 9),
        PRINT_UTC            = (1 << 10),
    };
    extern uint32_t gDebugFlags;

    /**
     * Debug areas.  These control which types of log statements are
     * printed.
     */
    enum {
        DEBUG_XMIT   = 0x00000001, // packets transmitted
        DEBUG_RECV   = 0x00000002, // packets received
        DEBUG_USER   = 0x00000004, // user driven actions
        DEBUG_TIPNEG = 0x00000008, // tip negotiation events
        DEBUG_INTERR = 0x00000010, // internal error
        DEBUG_PKTERR = 0x00000020, // errors related to packet parsing
        DEBUG_ALL    = 0xFFFFFFFF
    };
    extern uint32_t gDebugAreas;

    /**
     * Output file.  Debug output is written to this file, by default
     * stdout.
     */
    extern FILE* gDebugOutput;

    /**
     * Output function.  This function is invoked to print the log
     * statements.  By default this function maps to
     * LibTip::DebugPrintf which calls fprintf() using
     * LibTip::gDebugOutput as the FILE*.
     */
    typedef void (*DebugPrintfFunc)(const char* inFmt, ...);
    extern DebugPrintfFunc gDebugPrintfFunc;
    
    /**
     * Prefix adding function.  This function is invoked to print a
     * header before each log statement.  The function name, area name,
     * source file, and source line are passed as arguments.  By
     * default this function maps to LibTip::DebugPrefix().
     */
    typedef void (*DebugPrefixFunc)(const char* inFunction, const char* area,
                                    const char* inFile, int inLine);
    extern DebugPrefixFunc gDebugPrefixFunc;

    
#define AMDEBUG(area, args)                                                            \
    do {                                                                               \
        if ((LibTip::gDebugAreas & LibTip::DEBUG_##area) == LibTip::DEBUG_##area) { \
            if (LibTip::gDebugFlags & LibTip::PRINT_FULL_FUNCTION) {             \
                LibTip::gDebugPrefixFunc(__PRETTY_FUNCTION__, #area, __FILE__, __LINE__);\
            } else {                                                                   \
                LibTip:: gDebugPrefixFunc(__FUNCTION__, #area, __FILE__, __LINE__); \
            }                                                                          \
            LibTip::gDebugPrintfFunc args;                                          \
        }                                                                              \
    } while (0);

    /**
     * Default prefix function.
     */
    void DebugPrefix(const char* inFunction, const char* area,
                     const char* inFile, int inLine);

    /**
     * Default printf function.
     */
    void DebugPrintf(const char* inFmt, ...);
};

#endif
