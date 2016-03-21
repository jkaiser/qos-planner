//
// Created by jkaiser on 21.03.16.
//

#ifndef QOS_PLANNER_LUSTRE_H
#define QOS_PLANNER_LUSTRE_H

#include <string>

namespace common {
class Lustre {

public:
    virtual bool Init() = 0;

    virtual uint32_t MBsToRPCs(const uint32_t mb_per_sec) const = 0;

    virtual bool StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) = 0;

    virtual bool StopJobTbfRule(std::string jobid, std::string rule_name) = 0;
};



class LocalLustre : public Lustre {

private:
    uint32_t max_rpc_size;  //in MB

public:
    virtual bool Init() override;

    LocalLustre() : max_rpc_size(1) { }

    virtual uint32_t MBsToRPCs(const uint32_t mb_per_sec) const override;

    virtual bool StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) override;

    virtual bool StopJobTbfRule(std::string jobid, std::string rule_name) override;
};

}

#endif //QOS_PLANNER_LUSTRE_H
