//
// Created by Jürgen Kaiser on 04.11.16.
//

#include "OstIpsCache.h"

namespace common {

OstIpsCache::OstIpsCache(std::shared_ptr<common::Lustre> lustre) : lustre_(lustre) {}

std::string OstIpsCache::GetIp(const std::string &uuid) {

    std::lock_guard<std::mutex> lock(cache_mut_);

    auto it = uuid_to_ip_cache_.find(uuid);
    if (it != uuid_to_ip_cache_.end()) {
        return it->second;
    }

    std::string ip;
    if (!lustre_->GetIPOfOst(uuid, ip)) {
        return "";
    }

    uuid_to_ip_cache_[uuid] = ip;
    return ip;
}

}