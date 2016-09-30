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

    update_thread_exit_flag = true;
    while (update_thread_is_active) {
        update_thread_finish_cv.wait(lck);
    }
    update_thread.join();
    lck.unlock();
    return true;
}



void MemoryClusterState::updateRepeatedly() {

    std::unique_lock<std::mutex> lck(state_mut);
    update_thread_is_active = true;
    update_thread_finish_cv.notify_all();
    lck.unlock();

    while(!update_thread_exit_flag) {

        if (!Update()) {
           //TODO: add error logging here, but don't exit the thread as it might be an temporary error
        };
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    lck.lock();
    update_thread_is_active = false;
    update_thread_finish_cv.notify_all();
    lck.unlock();
}

bool MemoryClusterState::Update() {
    std::shared_ptr<std::vector<Lustre::getOstsResults>> r (new std::vector<Lustre::getOstsResults>());

    if (!lustre->GetOstList("", r)) {
        return false;
    }

    std::lock_guard<std::mutex> lck(state_mut);
    for (auto &it : *r) {
        // TODO: obviously, the following performance values are wild guesses. Replace with correct ones! Implement some heuristic to derive some initial max value?
        //this->nodeMap[it.number] = {it.uuid, 0, 0};
        this->nodeMap[it.number] = {it.uuid, 0, default_rpc_rate_};
    }

    return true;
}

void MemoryClusterState::UpdateNode(const std::string &name, const NodeState &node_state) {
    nodeMap[name] = node_state;
}

MemoryClusterState::MemoryClusterState(std::shared_ptr<common::Lustre> l) :  update_thread_started(false),
                                                                             update_thread_is_active(false),
                                                                             update_thread_exit_flag(false) {
    lustre = l;
}

MemoryClusterState::MemoryClusterState() : update_thread_started(false),
    update_thread_is_active(false),
    update_thread_exit_flag(false) {}

}
