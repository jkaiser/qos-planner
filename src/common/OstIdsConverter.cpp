//
// Created by Jürgen Kaiser on 04.11.16.
//

#include "OstIdsConverter.h"

#include <vector>
#include <memory>

namespace common {


OstIdsConverter::OstIdsConverter(std::shared_ptr<common::Lustre> lustre) : lustre_(lustre) {}

std::string OstIdsConverter::ToUUID(const std::string &id) {
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