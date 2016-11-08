//
// Created by jkaiser on 21.03.16.
//

#ifndef QOS_PLANNER_LUSTRE_H
#define QOS_PLANNER_LUSTRE_H

#include <string>
#include <vector>
#include <memory>

#include "util.h"

namespace common {

/**
 * Abstract class for Lustre interactions
 */
class Lustre {

public:
    virtual bool Init() = 0;

    virtual uint32_t MBsToRPCs(const uint32_t mb_per_sec) const = 0;
    virtual uint32_t RPCsToMBs(const uint32_t rpc_per_sec) const = 0;

    struct getOstsResults {
        std::string number;
        std::string uuid;
        std::string status;
    };
    /**
     * lists all osts of Lustre. This is the equivalent of an "lfs osts <path>" call.
     *
     * params:
     *  path:   If a path is provided (= nonempty string) that is located on a lustre mounted file system then only the OSTs belonging to that filesystem are displayed.
     *
     * Returns:
     *      true, if successfull
     */
    virtual bool GetOstList(const std::string &path, std::shared_ptr<std::vector<getOstsResults>> &output) = 0;
    static void ParseOstsFromLfsOsts(const std::string &lfs_out, std::shared_ptr<std::vector<getOstsResults>> osts);

    virtual bool StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) = 0;
    virtual bool ChangeJobTbfRule(std::string jobid, std::string rule_name, uint32_t new_rpc_rate_limit) = 0;

    virtual bool StopJobTbfRule(std::string jobid, std::string rule_name) = 0;

    /**
     * Determines the osts for the given file and appends them to the given vector.
     * Returns
     *  true, if no error occured.
     *  else, otherwise
     */
    virtual bool GetOstsForFile(const std::string &file, std::shared_ptr<std::vector<std::string>> osts) = 0;

    /**
     * Given the uuid of an OST, determines the ip address of the node serving the OST.
     * @param ost_uuid
     * @param ip_address Output parameter.
     * @return True, if successful
     */
    virtual bool GetIPOfOst(const std::string &ost_uuid, std::string &ip_address) = 0;

    /**
     * Determines the osts for the given files and appends them to the given vector.
     */
    virtual bool GetOstsForFile(const std::vector<std::string> &files, std::shared_ptr<std::vector<std::string>> osts) = 0;

    /**
     * Parses the output of an lfs getstripe <file> command. It is assumed that it is the command
     * for a single file and that the call didn't return with an error.
     */
    static void ParseOstsFromGetStripe(std::string lfs_out, std::shared_ptr<std::vector<std::string>> osts);

    /**
     * Parses the ip address of an "lctl get_param osc.fsname-OSTnumber*.ost_conn_uuid" call.
     * @param to_parse The stdout output of the command
     * @param ip_address Output parameter
     */
    static void ParseOstIpFromGetParamOsc(const std::string &to_parse, const std::string &ost_id, std::string &ip_address);
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

    virtual bool GetOstList(const std::string &path, std::shared_ptr<std::vector<getOstsResults>> &output) override;

    virtual bool StartJobTbfRule(std::string jobid, std::string rule_name, uint32_t rpc_rate_limit) override;
    virtual bool ChangeJobTbfRule(std::string jobid, std::string rule_name, uint32_t new_rpc_rate_limit) override;
    virtual bool StopJobTbfRule(std::string jobid, std::string rule_name) override;

    virtual bool GetIPOfOst(const std::string &ost_uuid, std::string &ip_address) override;
    virtual bool GetOstsForFile(const std::string &file, std::shared_ptr<std::vector<std::string>> osts) override;
    virtual bool GetOstsForFile(const std::vector<std::string> &files, std::shared_ptr<std::vector<std::string>> osts) override;
};

}

#endif //QOS_PLANNER_LUSTRE_H
