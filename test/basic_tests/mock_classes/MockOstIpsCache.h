//
// Created by Jürgen Kaiser on 08.11.16.
//

#ifndef QOS_PLANNER_MOCKOSTIPSCACHE_H
#define QOS_PLANNER_MOCKOSTIPSCACHE_H


#include "gmock/gmock.h"

#include <OstIpsCache.h>

namespace common {
class MockOstIpsCache : public OstIpsCache {

public:
    MockOstIpsCache() : OstIpsCache(std::make_shared<LocalLustre>()) {}

    MOCK_METHOD1(GetIp, std::string (const std::string &));
};
}

#endif //QOS_PLANNER_MOCKOSTIPSCACHE_H
