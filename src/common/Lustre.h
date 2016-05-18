//
// Created by jkaiser on 21.03.16.
//

#ifndef QOS_PLANNER_LUSTRE_H
#define QOS_PLANNER_LUSTRE_H

#include <string>
#include <vector>

namespace common {

/**
 * Abstract class for Lustre interactions
 */
class Lustre {

public:
    virtual bool Init() = 0;

    virtual uint32_t MBsToRPCs(const uint32_t mb_per_sec) const = 0;
    virtual uint32_t RPCsToMBs(const uint32_t rpc_per_sec) const = 0;

    virtual bool StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) = 0;

    virtual bool StopJobTbfRule(std::string jobid, std::string rule_name) = 0;

    /**
     * Determines the osts for the given file and appends them to the given vector.
     */
    virtual bool GetOstsForFile(const std::string &file, std::shared_ptr<std::vector<std::string>> osts) = 0;
};


/**
 * Class for Lustre interactions where there are local (and already configured) cmd-line tools for Lustre.
 */
class LocalLustre : public Lustre {

private:
    uint32_t max_rpc_size;  //in MB

public:
    virtual bool Init() override;

    LocalLustre() : max_rpc_size(1) { }

    virtual uint32_t MBsToRPCs(const uint32_t mb_per_sec) const override;
    virtual uint32_t RPCsToMBs(const uint32_t rpc_per_sec) const override;

    virtual bool StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) override;

    virtual bool StopJobTbfRule(std::string jobid, std::string rule_name) override;
    virtual bool GetOstsForFile(const std::string &file, std::shared_ptr<std::vector<std::string>> osts) override;
};

}

#endif //QOS_PLANNER_LUSTRE_H
