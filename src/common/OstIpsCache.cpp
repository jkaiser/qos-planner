/*
 * Copyright (c) 2017 Jürgen Kaiser
 */
/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * GPL HEADER END
 */

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