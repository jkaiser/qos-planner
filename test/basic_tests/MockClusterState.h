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
    MOCK_METHOD0(getNodes, std::vector<std::string>*());
    MOCK_METHOD2(getState, bool(const std::string&, NodeState*));
    MOCK_METHOD2(UpdateNode, void(const std::string&, const NodeState&));
};
}

#endif //QOS_PLANNER_MOCKCLUSTERSTATE_H
