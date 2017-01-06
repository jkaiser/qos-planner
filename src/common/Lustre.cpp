//
// Created by jkaiser on 21.03.16.
//

#include "Lustre.h"

#include <sys/stat.h>
#include <regex>
#include <sstream>
#include <unordered_set>

#include <spdlog/spdlog.h>

namespace common {


bool LocalLustre::StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) {
    // $ lctl set_param x.x.x.nrs_tbf_rule="[reg|hp] start rule_name jobid_list) rpc_rate_limit" // NOTE: hp = high priority, reg = regular (?)
    // $ lctl set_param ost.OSS.ost_io.nrs_tbf_rule="[reg|hp] start rule_name jobid_list) rpc_rate_limit"

    std::string cmd = "lctl set_param ost.OSS.ost_io.nrs_tbf_rule=\"start " + rule_name + " {" + jobid + "} " +
                      std::to_string(rpc_rate_limit) + "\"";
    std::shared_ptr<std::string> out(new std::string());

    spdlog::get("console")->debug("lustre: will call '{}'", cmd);
    if (!exec(cmd.c_str(), out)) {
        spdlog::get("console")->error("starting tbf rule failed for reservation '{}'", jobid);
        return false;
    }
    return true;
}

bool LocalLustre::ChangeJobTbfRule(std::string jobid, std::string rule_name, uint32_t new_rpc_rate_limit) {
    std::string cmd = "lctl set_param ost.OSS.ost_io.nrs_tbf_rule=\"change " + rule_name + " " +
                      std::to_string(new_rpc_rate_limit) + "\"";
    std::shared_ptr<std::string> out(new std::string());

    spdlog::get("console")->debug("lustre: will call '{}'", cmd);
    if (!exec(cmd.c_str(), out)) {
        spdlog::get("console")->error("changing tbf rule failed for reservation '{}'", jobid);
        return false;
    }
    return true;
}

bool LocalLustre::StopJobTbfRule(std::string jobid, std::string rule_name) {
    // lctl set_param x.x.x.nrs_tbf_rule= "[reg|hp] change rule_name rate"
    // lctl set_param ost.OSS.ost_io.nrs_tbf_rule="stop loginnode"

    std::string cmd = "lctl set_param ost.OSS.ost_io.nrs_tbf_rule=\"stop " + rule_name + "\"";
    std::shared_ptr<std::string> out(new std::string());

    spdlog::get("console")->debug("lustre: will call '{}'", cmd);
    if (!exec(cmd.c_str(), out)) {
        spdlog::get("console")->error("stopping tbf rule failed for reservation '{}'", jobid);
        return false;
    }

    return true;
}

uint32_t LocalLustre::MBsToRPCs(const uint32_t mb_per_sec) const {
    return mb_per_sec / max_rpc_size_;   // TODO:
}

uint32_t LocalLustre::RPCsToMBs(const uint32_t rpc_per_sec) const {
    return rpc_per_sec * max_rpc_size_;   // TODO:
}

bool LocalLustre::Init() {
    /**
     * TODO: implement logic to convert MB/sec -> RPC/sec
     *   - is the configuration of lustre visible somewhere?
     */

    return true;
}

bool LocalLustre::GetOstsForFile(const std::string &file, std::shared_ptr<std::vector<std::string>> osts) {

    struct stat buffer;
    if(stat(file.c_str(), &buffer) != 0) {
        spdlog::get("console")->error("file '{}' doesn't exist!", file);
        return false;
    }

    std::shared_ptr<std::string> getstripe_out(new std::string());
    std::string cmd = "lfs getstripe " + file;

    if (!exec(cmd.c_str(), getstripe_out)) {
        spdlog::get("console")->error("lfs getstripe failed for file {}", file);
        return false;
    }

    ParseOstsFromGetStripe(getstripe_out->c_str(), osts);

    return true;
}

bool LocalLustre::GetOstsForFile(const std::vector<std::string> &files,
                                 std::shared_ptr<std::vector<std::string>> osts) {
    std::shared_ptr<std::string> getstripe_out(new std::string());
    std::ostringstream oss;
    if (!osts->empty()) {
        std::copy(osts->begin(), osts->end(), std::ostream_iterator<std::string> (oss, " "));   // theoretically, there is a " " too much at the end, but it doesn't hurt us
    }
    std::string cmd = "lfs getstripe " + oss.str();

    if (!exec(cmd.c_str(), getstripe_out)) {
        spdlog::get("console")->error("lfs getstripe failed for list of files");
        return false;
    }

    ParseOstsFromGetStripe(getstripe_out->c_str(), osts);

    // there may be double entries in the vector
    std::unordered_set<std::string> ost_set;
    for (auto &o : *osts) {
        ost_set.insert(o);
    }

    osts->clear();
    for (auto &o : ost_set) {
        osts->push_back(o);
    }
    return true;
}

bool LocalLustre::GetOstList(const std::string &path, std::shared_ptr<std::vector<getOstsResults>> &output) {
        std::shared_ptr<std::string> cmd_out(new std::string());
    std::string cmd = "lfs osts " + path;

    if (!exec(cmd.c_str(), cmd_out)) {
        spdlog::get("console")->error("couldn't get list of osts");
        return false;
    }

    ParseOstsFromLfsOsts(*cmd_out, output);

    return true;
}

bool LocalLustre::GetIPOfOst(const std::string &ost_uuid, std::string &ip_address) {

    auto pos = ost_uuid.find("_UUID");
    std::string shortened_id = (pos == std::string::npos)? ost_uuid : ost_uuid.substr(0, pos);

    std::shared_ptr<std::string> cmd_out(new std::string());
    std::string cmd = "lctl get_param osc." + shortened_id + "*.ost_conn_uuid";
    if (!exec(cmd.c_str(), cmd_out)) {
        spdlog::get("console")->error("couldn't get ip of osts '{}'", ost_uuid);
        return false;
    }

    ParseOstIpFromGetParamOsc(*cmd_out, shortened_id, ip_address);

    return true;
}


void Lustre::ParseOstsFromGetStripe(std::string lfs_out, std::shared_ptr<std::vector<std::string>> osts) {

    // example: '\t     3\t             2\t          0x2\t             0'
    std::regex r("\t.+([0-9]+)\t.+\t.+\t.*");

    std::istringstream stream(lfs_out);
    for (std::string line; std::getline(stream, line);) {

        std::smatch base_match;
        if (std::regex_match(line, base_match, r)) {
            if (base_match.size() == 2) {
                osts->push_back(base_match[1].str());
            }
        }
    }
}

void Lustre::ParseOstsFromLfsOsts(const std::string &lfs_out,
                                  std::shared_ptr<std::vector<common::Lustre::getOstsResults>> out) {

    // example: "OBDS::\n0: lustret4-OST0000_UUID ACTIVE\n1: lustret4-OST0001_UUID ACTIVE\n2: lustret4-OST0002_UUID ACTIVE\n3: lustret4-OST0003_UUID ACTIVE\n"
    std::regex r("([0-9]+): ([_[:alnum:]-]+) ([[:alnum:]]+)");

    std::istringstream stream(lfs_out);
    for (std::string line; std::getline(stream, line);) {

        std::smatch base_match;
        if (std::regex_match(line, base_match, r)) {
            if (base_match.size() == 4) {
                common::Lustre::getOstsResults foo = {base_match[1].str(), base_match[2].str(), base_match[3].str()};
                out->push_back(foo);
            }
        }
    }
}


void
Lustre::ParseOstIpFromGetParamOsc(const std::string &to_parse, const std::string &ost_id, std::string &ip_address) {
    // example: "osc.toto-OST0000-osc-ffff88003b90f800.ost_conn_uuid=192.168.122.241@tcp\n"
    std::regex r(".+" + ost_id + ".+ost_conn_uuid=(.+)@tcp$");
    std::istringstream stream(to_parse);
    for (std::string line; std::getline(stream, line);) {

        std::smatch base_match;
        if (std::regex_match(line, base_match, r)) {
            if (base_match.size() == 2) {
                ip_address = base_match[1].str();
                return;
            }
        }
    }
}

}



