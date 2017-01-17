/*
 * Copyright (c) 2017 Jürgen Kaiser
 */
/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * GPL HEADER END
 */

#ifndef QOS_PLANNER_UTIL_H
#define QOS_PLANNER_UTIL_H

#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <stdio.h>

namespace common {

static bool exec(const char* cmd, std::shared_ptr<std::string> out) {
    char buffer[128];

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        //TODO: add error message: "couldn't execute cmd
        //  WEXITSTATUS(pclose(fp))
        return false;
    }

    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL) {
                out->append(buffer);
            }
        }
    } catch (std::exception e) {
        //TODO: add error logging here
        // WEXITSTATUS(pclose(pipe))
        pclose(pipe);

        return false;
    }

    pclose(pipe);
    return true;
}


}

#endif //QOS_PLANNER_UTIL_H
