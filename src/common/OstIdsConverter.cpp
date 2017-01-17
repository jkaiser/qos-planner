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

#include "OstIdsConverter.h"

#include <vector>
#include <memory>

namespace common {


OstIdsConverter::OstIdsConverter(std::shared_ptr<common::Lustre> lustre) : lustre_(lustre) {}

std::string OstIdsConverter::ToUUID(const std::string &id) {
    std::lock_guard<std::mutex> lock(cache_mut_);
    auto it = id_to_uuid_.find(id);
    if(it != id_to_uuid_.end()) {
        return it->second;
    }

    UpdateCache();

    it = id_to_uuid_.find(id);
    if(it != id_to_uuid_.end()) {
        return it->second;
    }

    return std::string();
}

void OstIdsConverter::UpdateCache() {
    std::shared_ptr<std::vector<Lustre::getOstsResults>> ost_list = std::make_shared<std::vector<Lustre::getOstsResults>>();
    if (lustre_->GetOstList("", ost_list)) {
        id_to_uuid_.clear();
        for (auto &r : *ost_list) {
            id_to_uuid_[r.number] = r.uuid;
        }
    }
}
}