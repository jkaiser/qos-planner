//
// Created by jkaiser on 22.03.16.
//

#ifndef QOS_PLANNER_MOCKLUSTRE_H
#define QOS_PLANNER_MOCKLUSTRE_H

#include "gmock/gmock.h"

#include <Lustre.h>

namespace common {

class MockLustre : public LocalLustre {
public:
    MOCK_METHOD3(StartJobTbfRule, bool(std::string, std::string, uint32_t));
    MOCK_METHOD2(StopJobTbfRule, bool(std::string, std::string));
    MOCK_METHOD2(GetOstList, bool(const std::string&, std::shared_ptr<std::vector<getOstsResults>>&));
    MOCK_METHOD2(GetIPOfOst, bool(const std::string&, std::string&));
};
}

#endif //QOS_PLANNER_MOCKLUSTRE_H
