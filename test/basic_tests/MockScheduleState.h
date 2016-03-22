//
// Created by jkaiser on 22.03.16.
//

#ifndef QOS_PLANNER_MOCKSCHEDULESTATE_H
#define QOS_PLANNER_MOCKSCHEDULESTATE_H

#include "gmock/gmock.h"

#include <ScheduleState.h>

namespace common {
class MockScheduleState : public MemoryScheduleState {
public:
    MOCK_METHOD2(GetJobEnd, bool(std::string, std::chrono::system_clock::time_point*));
    MOCK_METHOD2(GetJobStatus, bool(std::string, Job::JobState*));
    MOCK_METHOD2(UpdateJob, bool(std::string, Job::JobState));
    MOCK_METHOD2(GetJobThroughput,bool(std::string, uint32_t*));
    MOCK_METHOD0(GetAllJobs, std::map<std::string, Job*> *());
    MOCK_METHOD3(AddJob, bool(const std::string&, const Job &, const std::vector<std::string>&));
};
}

#endif //QOS_PLANNER_MOCKSCHEDULESTATE_H
