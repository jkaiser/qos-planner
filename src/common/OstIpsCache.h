//
// Created by Jürgen Kaiser on 04.11.16.
//

#ifndef QOS_PLANNER_OSTIPSCACHE_H
#define QOS_PLANNER_OSTIPSCACHE_H

#include "Lustre.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace common {
class OstIpsCache {
private:
    std::shared_ptr<Lustre> lustre_;
    std::mutex cache_mut_;
    std::unordered_map<std::string, std::string> uuid_to_ip_cache_;

public:
    OstIpsCache(std::shared_ptr<Lustre> lustre);

    virtual std::string GetIp(const std::string &uuid);
};

}

#endif //QOS_PLANNER_OSTIPSCACHE_H
