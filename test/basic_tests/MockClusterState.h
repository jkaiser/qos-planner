//
// Created by jkaiser on 22.03.16.
//

#ifndef QOS_PLANNER_MOCKCLUSTERSTATE_H
#define QOS_PLANNER_MOCKCLUSTERSTATE_H

#include "gmock/gmock.h"

#include <ClusterState.h>

namespace common {
class MockClusterState : public MemoryClusterState {
public:
    MOCK_METHOD0(GetOSTList, std::vector<std::string>*());

    MOCK_METHOD2(getOstState, bool(const std::string&, OSTWorkload*));
    MOCK_METHOD2(UpdateNode, void(const std::string&, const OSTWorkload&));
};
}

#endif //QOS_PLANNER_MOCKCLUSTERSTATE_H
