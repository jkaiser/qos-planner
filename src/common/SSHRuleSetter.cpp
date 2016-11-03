//
// Created by Jürgen Kaiser on 02.11.16.
//

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