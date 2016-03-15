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

struct NodeState {
    std::string name;
    uint32_t rpcSec;
    uint32_t maxRpcSec;
};

class ClusterState {

protected:
    // performs an update of the internal state of the actual used resources of the cluster
    virtual bool Update() = 0;

public:

    virtual bool Init() = 0;
    virtual bool TearDown() = 0;
    virtual const NodeState * getState(std::string id) = 0;
    virtual std::vector<std::string> *getNodes() = 0;

    virtual void UpdateNode(std::string name, const NodeState &node_state) = 0;
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
    virtual const NodeState *getState(std::string id) override;
    virtual void UpdateNode(std::string name, const NodeState &node_state) override;
};
}
#endif //QOS_PLANNER_CLUSTERSTATE_H

