//
// Created by Jürgen Kaiser on 04.11.16.
//

#ifndef QOS_PLANNER_OSTIDSCONVERTER_H
#define QOS_PLANNER_OSTIDSCONVERTER_H

#include <unordered_map>
#include <mutex>
#include "Lustre.h"

namespace common {
class OstIdsConverter {

private:
    std::shared_ptr<Lustre> lustre_;
    std::mutex cache_mut_;
    std::unordered_map<std::string, std::string> id_to_uuid_;

    void UpdateCache();

public:

    OstIdsConverter(std::shared_ptr<Lustre> lustre);

    virtual std::string ToUUID(const std::string &id);
};
}

#endif //QOS_PLANNER_OSTIDSCONVERTER_H
