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

#ifndef TIP_PROFILE_H
#define TIP_PROFILE_H

#include "tip_system.h"

namespace LibTip {

    /**
     * Single screen video endpoint profile.  Provides API to
     * customize a CTipSystem class to represent a single screen
     * video endpoint (e.g. CTS1000).
     */
    class CSingleScreenProfile {
    public:
        /**
         * Configure the system object as a single screen system.
         * Customizes the system object by adding transmitters,
         * receivers, and media options as is required by a single
         * screen system such as a CTS1000.
         *
         * @param system system object to configure
         * @param aux30FPS set to true if the system should support
         * 30FPS aux (presentation)
         * @param secure set to true if the system is secure
         */
        static void Configure(CTipSystem& system, bool aux30FPS, bool secure);

    private:
        CSingleScreenProfile();
        ~CSingleScreenProfile();
    };
    
    /**
     * Single screen extended reach video endpoint profile.  Provides
     * API to customize a CTipSystem class to represent a single
     * screen video endpoint (e.g. CTS500 on a T1 connection).
     */
    class CSingleScreenExtendedReachProfile {
    public:
        /**
         * Configure the system object as a single screen extended
         * reach system.  Customizes the system object by adding
         * transmitters, receivers, and media options as is required
         * by a single screen system such as a CTS500 on a T1 connection.
         *
         * @param system system object to configure
         * @param secure set to true if the system is secure
         */
        static void Configure(CTipSystem& system, bool secure);

    private:
        CSingleScreenExtendedReachProfile();
        ~CSingleScreenExtendedReachProfile();
    };

    /**
     * Triple screen video endpoint profile.  Provides API to
     * customize a CTipSystem class to represent a triple screen
     * video endpoint (e.g. CTS3000).
     */
    class CTripleScreenProfile {
    public:
        /**
         * Configure the system object as a triple screen system.
         * Customizes the system object by adding transmitters,
         * receivers, and media options as is required by a triple
         * screen system such as a CTS3000.
         *
         * @param system system object to configure
         * @param aux30FPS set to true if the system should support
         * 30FPS aux (presentation)
         * @param secure set to true if the system is secure
         */
        static void Configure(CTipSystem& system, bool aux30FPS, bool secure);

    private:
        CTripleScreenProfile();
        ~CTripleScreenProfile();
    };
         
};

#endif
