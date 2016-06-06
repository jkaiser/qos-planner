//
// Created by jkaiser on 08.03.16.
//

#include "ClusterState.h"

namespace common {

bool MemoryClusterState::getState(const std::string &id, NodeState *state) {
    std::unique_lock<std::mutex> lck(state_mut);

    auto it = this->nodeMap.find(id);
    if (it == this->nodeMap.end()) {
        return false;
    }
    *state = it->second;
    return true;
}

std::vector<std::string> *MemoryClusterState::getNodes() {
    std::vector<std::string> *names = new std::vector<std::string>();
    names->reserve(this->nodeMap.size());

    for (auto i = this->nodeMap.begin(); i != this->nodeMap.end(); i++) {
        names->push_back(i->second.name);
    }

    return names;
}



bool MemoryClusterState::Init() {

    if (!Update()) {    // perform an initial update first
        return false;
    }

    // start updating thread.
    if (not update_thread_started) {
        update_thread = std::thread(&MemoryClusterState::updateRepeatedly, this);
        update_thread_started = true;
    } else { // something's wrong. Was it initialized before?
        return false;
    }

    return true;
}

bool MemoryClusterState::TearDown() {
    std::unique_lock<std::mutex> lck(state_mut);

    if (!update_thread_exit_flag) {
        update_thread_exit_flag = true;
        std::chrono::seconds max_wait_time(30); // 30 seconds to wait
        auto tstart = std::chrono::system_clock::now();

        bool successful_joined = false;
        while (std::chrono::system_clock::now() < tstart + max_wait_time) {
            if (update_thread.joinable()) {
                update_thread.join();
                successful_joined = true;
                break;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        if (not successful_joined) {
            return false;
        }
    }

    return true;
}



void MemoryClusterState::updateRepeatedly() {

    while(!update_thread_exit_flag) {

        if (!Update()) {
           //TODO: add error logging here, but don't exit the thread as it might be an temporary error
        };
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

}

bool MemoryClusterState::Update() {
    std::shared_ptr<std::vector<Lustre::getOstsResults>> r (new std::vector<Lustre::getOstsResults>());

    if (!lustre->GetOstList("", r)) {
        return false;
    }

    std::unique_lock<std::mutex> lck(state_mut);
    for (auto &it : *r) {
        this->nodeMap[it.number] = {it.uuid, 0, 0}; // TODO: obviously the performance values are wrong. Replace with correct ones!
    }
    return true;
}

void MemoryClusterState::UpdateNode(const std::string &name, const NodeState &node_state) {
    nodeMap[name] = node_state;
}

MemoryClusterState::MemoryClusterState(std::shared_ptr<common::Lustre> l) {
    lustre = l;
}

MemoryClusterState::MemoryClusterState() { }


}
