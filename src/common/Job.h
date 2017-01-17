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

#ifndef QOS_PLANNER_JOB_H
#define QOS_PLANNER_JOB_H


#include <string>
#include <chrono>
#include <vector>

namespace common {

/**
 * A given resource request for a given time is represented as Job.
 */
class Job {

public:
    enum JobEvent {
        JOBSTART = 1,
        JOBSTOP
    };

    enum JobState {
        INITIALIZED = 0,
        SCHEDULED,
        ACTIVE,
        DONE
    };

    static std::string JobStateToString(JobState s) {
        switch (s) {
            case INITIALIZED:
                return "INITIALIZED";
            case SCHEDULED:
                return "SCHEDULED";
            case ACTIVE:
                return "ACTIVE";
            case DONE:
                return "DONE";
        }
    }

    Job(const std::string &jobid,
        std::chrono::time_point<std::chrono::system_clock> tstart,
        std::chrono::time_point<std::chrono::system_clock> tend,
        uint32_t min_read_throughput_MB)
            : state_(INITIALIZED), jobid_(jobid), tstart_(tstart), tend_(tend), min_read_throughput_MB_(min_read_throughput_MB) { }

    Job(const std::string &jobid) : jobid_(jobid), state_(INITIALIZED), min_read_throughput_MB_(0) {}

private:
    JobState state_;
    std::string jobid_;
    std::chrono::time_point<std::chrono::system_clock> tstart_;
    std::chrono::time_point<std::chrono::system_clock> tend_;
    std::vector<std::string> osts_;  // relevant osts for this. A OST becomes relevant if one (part) of the input files reside on it.
    uint32_t min_read_throughput_MB_;

public:

    void setState(const JobState &state) {
        Job::state_ = state;
    }

    const JobState &getState() const {
        return state_;
    }

    const std::string &getJobid() const {
        return jobid_;
    }

    const std::chrono::time_point<std::chrono::system_clock> &GetStartTime() const {
        return tstart_;
    }

    const std::chrono::time_point<std::chrono::system_clock> &GetEndTime() const {
        return tend_;
    }

    const std::vector<std::string> &getOsts() const {
        return osts_;
    }

    void setOsts(const std::vector<std::string> &osts) {
        Job::osts_ = osts;
    }

    uint32_t getMin_read_throughput_MB() const {
        return min_read_throughput_MB_;
    }
};

}

#endif //QOS_PLANNER_JOB_H
