//
// Created by jkaiser on 08.03.16.
//

#ifndef QOS_PLANNER_CLUSTERSTATE_H
#define QOS_PLANNER_CLUSTERSTATE_H

#include <string>
#include <map>
#include <vector>
#include <stdint.h>

#include <thread>

namespace common {

/**
 * Simple representation of a node's current workload (the actual measured one)
 */
struct NodeState {
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
    virtual bool getState(const std::string &id, NodeState *state) const = 0;
    virtual std::vector<std::string> *getNodes() = 0;

    virtual void UpdateNode(const std::string &name, const NodeState &node_state) = 0;
};

class MemoryClusterState : public ClusterState {

private:
    std::map<std::string, NodeState> nodeMap;

    bool update_thread_started;
    std::thread update_thread;
    bool update_thread_exit_flag = false;

    void updateRepeatedly();

protected:
    virtual bool Update() override;


public:
    virtual bool Init() override;
    virtual bool TearDown() override;
    virtual std::vector<std::string> *getNodes() override;
    virtual bool getState(const std::string &id, NodeState *state) const override;
    virtual void UpdateNode(const std::string &name, const NodeState &node_state) override;
};
}
#endif //QOS_PLANNER_CLUSTERSTATE_H

