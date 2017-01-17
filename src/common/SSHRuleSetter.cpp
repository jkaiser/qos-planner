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

#include "SSHRuleSetter.h"

#include "util.h"
#include <spdlog/spdlog.h>

namespace common {

bool SSHRuleSetter::SetRule(const std::string &ip, const std::string &job_id, const std::string &rule_name,
                            uint32_t rpc_rate_limit) {
    if (ip.size() == 0) {
        return false;
    }

    std::string cmd = "ssh " + ip + " lctl set_param ost.OSS.ost_io.nrs_tbf_rule=\"start " + rule_name + " {" + job_id + "} " +
                      std::to_string(rpc_rate_limit) + "\"";
    std::shared_ptr<std::string> out(new std::string());

    spdlog::get("console")->debug("sshRuleSetter: will call '{}'", cmd);
    if (!exec(cmd.c_str(), out)) {
        spdlog::get("console")->error("starting tbf rule failed for reservation '{}'", job_id);
        return false;
    }
    return true;
}

bool SSHRuleSetter::RemoveRule(const std::string &ip, const std::string &rule_name, const std::string &job_id) {

    std::string cmd = "ssh " + ip + "lctl set_param ost.OSS.ost_io.nrs_tbf_rule=\"stop " + rule_name + "\"";
    std::shared_ptr<std::string> out(new std::string());

    spdlog::get("console")->debug("lustre: will call '{}'", cmd);
    if (!exec(cmd.c_str(), out)) {
        spdlog::get("console")->error("stopping tbf rule failed for reservation '{}'", job_id);
        return false;
    }

    return true;
}

}