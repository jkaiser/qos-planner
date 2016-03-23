//
// Created by jkaiser on 21.03.16.
//

#include "Lustre.h"

namespace common {


bool LocalLustre::StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) {
    // $ lctl set_param x.x.x.nrs_tbf_rule="[reg|hp] start rule_name jobid_list) rpc_rate_limit"
    // $ lctl set_param ost.OSS.ost_io.nrs_tbf_rule="[reg|hp] start rule_name jobid_list) rpc_rate_limit"
    return false;
}

bool LocalLustre::StopJobTbfRule(std::string jobid, std::string rule_name) {
    // lctl set_param x.x.x.nrs_tbf_rule= "[reg|hp] change rule_name rate"
    // lctl set_param ost.OSS.ost_io.nrs_tbf_rule="stop loginnode"
    return false;
}

uint32_t LocalLustre::MBsToRPCs(const uint32_t mb_per_sec) const {
    return mb_per_sec/max_rpc_size;   // TODO:
}

uint32_t LocalLustre::RPCsToMBs(const uint32_t rpc_per_sec) const {
    return rpc_per_sec * max_rpc_size;   // TODO:
}

bool LocalLustre::Init() {
    /**
     * TODO: implement logic to convert MB/sec -> RPC/sec
     *   - is the configuration of lustre visible somewhere?
     */

    return true;
}



}
