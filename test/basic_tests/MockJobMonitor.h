//
// Created by jkaiser on 22.03.16.
//

#ifndef QOS_PLANNER_MOCKJOBMONITOR_H
#define QOS_PLANNER_MOCKJOBMONITOR_H

#include "gmock/gmock.h"

#include <JobMonitor.h>

namespace common {
class MockJobMonitor : public JobMonitor {
public:
    MOCK_METHOD1(RegisterJob, bool(const Job&));
    MOCK_METHOD1(UnregisterJob, bool(const Job&));
};
}
#endif //QOS_PLANNER_MOCKJOBMONITOR_H
