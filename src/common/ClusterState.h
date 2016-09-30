//
// Created by jkaiser on 08.03.16.
//

#ifndef QOS_PLANNER_CLUSTERSTATE_H
#define QOS_PLANNER_CLUSTERSTATE_H

#include <string>

#include <map>
#include <vector>
#include <stdint.h>
#include <mutex>

#include <thread>

#include "Lustre.h"

namespace common {

/**
 * Simple representation of a node's current workload (the actual measured one)
 */
struct OSTWorkload {
    std::string name;
    uint32_t rpcSec;
    uint32_t maxRpcSec;
};


/**
 * The ClusterState holds the current performance metrics of each Lustre node.
 */
class ClusterState {

protected:
    // performs an update of the internal state of the actual used resources of the cluster
    virtual bool Update() = 0;

public:

    virtual bool Init() = 0;
    virtual bool TearDown() = 0;

    /**
     * Get the state of the OST with the given id
     */
    virtual bool getOstState(const std::string &ost_id, OSTWorkload *ost_workload) = 0;
    virtual std::vector<std::string> *GetOSTList() = 0;

    virtual void UpdateNode(const std::string &name, const OSTWorkload &ost_workload) = 0;
};

/**
 * ClusterState implementation which stores everything in main memory.
 */
class MemoryClusterState : public ClusterState {

private:
    static const uint32_t default_rpc_rate_ = 500;
    std::map<std::string, OSTWorkload> ost_state_map;
    std::shared_ptr<common::Lustre> lustre;

    bool update_thread_started;
    std::thread update_thread;
    bool update_thread_exit_flag = false;
    bool update_thread_is_active = false;
    std::condition_variable update_thread_finish_cv;

    std::mutex state_mut;

    void updateRepeatedly();

protected:
    virtual bool Update() override;


public:
    MemoryClusterState();
    MemoryClusterState(std::shared_ptr<common::Lustre> l);

    virtual bool Init() override;
    virtual bool TearDown() override;
    virtual std::vector<std::string> *GetOSTList() override;
    virtual bool getOstState(const std::string &id, OSTWorkload *state) override;
    virtual void UpdateNode(const std::string &name, const OSTWorkload &node_state) override;
};
}
#endif //QOS_PLANNER_CLUSTERSTATE_H

