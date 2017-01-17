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

#ifndef QOS_PLANNER_CLUSTERSTATE_H
#define QOS_PLANNER_CLUSTERSTATE_H

#include <string>

#include <map>
#include <vector>
#include <stdint.h>
#include <mutex>

#include <thread>
#include <condition_variable>

#include "Lustre.h"

namespace common {

/**
 * Simple representation of a node's current workload (the actual measured one)
 */
struct OSTWorkload {
    std::string ost;
    uint32_t currentRpcSec;
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
    std::map<std::string, OSTWorkload> ost_state_map_;
    std::shared_ptr<common::Lustre> lustre_;

    bool update_thread_started_;
    std::thread update_thread_;
    bool update_thread_exit_flag_ = false;
    bool update_thread_is_active_ = false;
    std::condition_variable update_thread_finish_cv_;

    std::mutex state_mut_;

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

