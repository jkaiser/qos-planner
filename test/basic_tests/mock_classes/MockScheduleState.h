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

#ifndef QOS_PLANNER_MOCKSCHEDULESTATE_H
#define QOS_PLANNER_MOCKSCHEDULESTATE_H

#include "gmock/gmock.h"

#include <MemoryScheduleState.h>

namespace common {
class MockScheduleState : public MemoryScheduleState {
public:
    MOCK_METHOD2(GetJobEnd, bool(std::string, std::chrono::system_clock::time_point*));
    MOCK_METHOD2(GetJobStatus, bool(std::string, Job::JobState*));
    MOCK_METHOD2(UpdateJob, bool(std::string, Job::JobState));
    MOCK_METHOD2(GetJobOstIds, bool(const std::string&, std::vector<std::string>&));
    MOCK_METHOD2(GetJobThroughput,bool(std::string, uint32_t*));
    MOCK_METHOD0(GetAllJobs, std::map<std::string, Job*> *());
    MOCK_METHOD1(RemoveJob, bool (const std::string&));
    MOCK_METHOD3(AddJob, bool(const std::string&, const Job &, const std::vector<std::string>&));
};
}

#endif //QOS_PLANNER_MOCKSCHEDULESTATE_H
