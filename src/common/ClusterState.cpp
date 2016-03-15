//
// Created by jkaiser on 08.03.16.
//

#include "ClusterState.h"


//MemoryClusterState::MemoryClusterState() {}

const NodeState *MemoryClusterState::getState(std::string id) {
    auto it = this->nodeMap.find(id);
    if (it == this->nodeMap.end()) {
        return nullptr;
    }
    return &it->second;

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

    if (not update_thread_started) {
        update_thread = std::thread(&MemoryClusterState::updateRepeatedly, this);
        update_thread_started = true;
    } else { // something's wrong. Was it initialized before?
        return false;
    }


    return true;
}

bool MemoryClusterState::TearDown() {

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

        Update();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

}

bool MemoryClusterState::Update() {
    return false;
}
