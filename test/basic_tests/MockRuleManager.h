//
// Created by Jürgen Kaiser on 08.11.16.
//

#ifndef QOS_PLANNER_MOCKRULEMANAGER_H
#define QOS_PLANNER_MOCKRULEMANAGER_H

#include "gmock/gmock.h"

#include <RuleManager.h>

namespace common {
class MockRuleManager : public RuleManager {

public:
    MOCK_METHOD3(SetRules, bool(std::vector<std::string> &, const std::string &, uint32_t));
    MOCK_METHOD1(RemoveRules, bool(const std::string&));
};
}

#endif //QOS_PLANNER_MOCKRULEMANAGER_H
