//
// Created by Jürgen Kaiser on 08.11.16.
//

#ifndef QOS_PLANNER_MOCKOSTIDSCONVERTER_H
#define QOS_PLANNER_MOCKOSTIDSCONVERTER_H


#include "gmock/gmock.h"

#include <OstIdsConverter.h>

namespace common {
class MockOstIdsConverter : public OstIdsConverter {

public:
    MockOstIdsConverter() : OstIdsConverter(std::make_shared<LocalLustre>()) {}

    MOCK_METHOD1(ToUUID, std::string(const std::string &));
};
}

#endif //QOS_PLANNER_MOCKOSTIDSCONVERTER_H
