//
// Created by Jürgen Kaiser on 02.11.16.
//

#ifndef QOS_PLANNER_MOCKRULESETTER_H
#define QOS_PLANNER_MOCKRULESETTER_H


#include "gmock/gmock.h"
#include <RuleSetter.h>

namespace common {
class MockRuleSetter : public RuleSetter {

public:
    MOCK_METHOD4(SetRule, bool(const std::string&, const std::string&, const std::string&, uint32_t));
    MOCK_METHOD3(RemoveRule, bool(const std::string&, const std::string&, const std::string&));
};
}

#endif //QOS_PLANNER_MOCKRULESETTER_H
