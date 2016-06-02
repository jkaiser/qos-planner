//
// Created by jkaiser on 6/2/16.
//

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
