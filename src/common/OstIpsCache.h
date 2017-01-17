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
