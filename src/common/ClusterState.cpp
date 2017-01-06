//
// Created by jkaiser on 08.03.16.
//

#include "ClusterState.h"

namespace common {

    bool MemoryClusterState::getOstState(const std::string &id, OSTWorkload *state) {
        std::lock_guard<std::mutex> lck(state_mut_);

        auto it = this->ost_state_map_.find(id);
        if (it == this->ost_state_map_.end()) {
            return false;
        }
        *state = it->second;
        return true;
    }

    std::vector<std::string> *MemoryClusterState::GetOSTList() {
        std::lock_guard<std::mutex> lck(state_mut_);
        std::vector<std::string> *names = new std::vector<std::string>();
        names->reserve(this->ost_state_map_.size());

        for (auto i = this->ost_state_map_.begin(); i != this->ost_state_map_.end(); i++) {
            names->push_back(i->second.ost);
        }

        return names;
    }


    bool MemoryClusterState::Init() {

        if (!Update()) {    // perform an initial update first
            return false;
        }

        if (not update_thread_started_) {
            update_thread_ = std::thread(&MemoryClusterState::updateRepeatedly, this);
            update_thread_started_ = true;
        } else { // something's wrong. Was it initialized before?
            return false;
        }

        return true;
    }

    bool MemoryClusterState::TearDown() {
        std::unique_lock<std::mutex> lck(state_mut_);

        update_thread_exit_flag_ = true;
        while (update_thread_is_active_) {
            update_thread_finish_cv_.wait(lck);
        }
        lck.unlock();

        if (update_thread_.joinable()){
            update_thread_.join();
        }

        return true;
    }


    void MemoryClusterState::updateRepeatedly() {

        std::unique_lock<std::mutex> lck(state_mut_);
        if (update_thread_exit_flag_) {
            lck.unlock();
            return;
        }
        update_thread_is_active_ = true;
        update_thread_finish_cv_.notify_all();
        lck.unlock();

        while (!update_thread_exit_flag_) {

            if (!Update()) {
                //TODO: add error logging here, but don't exit the thread as it might be an temporary error
            };
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        lck.lock();
        update_thread_is_active_ = false;
        update_thread_finish_cv_.notify_all();
        lck.unlock();
    }

    bool MemoryClusterState::Update() {
        std::shared_ptr<std::vector<Lustre::getOstsResults>> r(new std::vector<Lustre::getOstsResults>());

        if (!lustre_->GetOstList("", r)) {
            return false;
        }

        std::unique_lock<std::mutex> lck(state_mut_);
        for (auto &it : *r) {
            // TODO: obviously, the following performance values are wild guesses. Replace with correct ones! Implement some heuristic to derive some initial max value?
            //this->ost_state_map_[it.number] = {it.uuid, 0, 0};
            this->ost_state_map_[it.number] = {it.uuid, 0, default_rpc_rate_};
        }

        lck.unlock();

        return true;
    }

    void MemoryClusterState::UpdateNode(const std::string &name, const OSTWorkload &node_state) {
        ost_state_map_[name] = node_state;
    }

    MemoryClusterState::MemoryClusterState(std::shared_ptr<common::Lustre> l) : update_thread_started_(false),
                                                                                update_thread_is_active_(false),
                                                                                update_thread_exit_flag_(false),
                                                                                lustre_(l) {}

    MemoryClusterState::MemoryClusterState() : update_thread_started_(false),
                                               update_thread_is_active_(false),
                                               update_thread_exit_flag_(false) {}

}
