//
// Created by jkaiser on 15.03.16.
//

#ifndef QOS_PLANNER_JOB_H
#define QOS_PLANNER_JOB_H


#include <chrono>
#include <vector>

namespace common {

/**
 * A given resource request for a given time is represented as Job.
 */
class Job {

private:
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

    Job(const std::string &jobid,
        std::chrono::time_point<std::chrono::system_clock> tstart,
        std::chrono::time_point<std::chrono::system_clock> tend,
        uint32_t min_read_throughput_MB)
            : jobid(jobid), tstart(tstart), tend(tend), min_read_throughput_MB(min_read_throughput_MB) { }

    Job(const std::string &jobid) : jobid(jobid){
//        this->jobid = jobid;
    }

private:
    JobState state;
    std::string jobid;
    std::chrono::time_point<std::chrono::system_clock> tstart;
    std::chrono::time_point<std::chrono::system_clock> tend;
    std::vector<std::string> osts;  // relevant osts for this. A OST becomes relevant if one (part) of the input files reside on it.
    uint32_t min_read_throughput_MB;

public:

    void setState(const JobState &state) {
        Job::state = state;
    }

    const JobState &getState() const {
        return state;
    }

    const std::string &getJobid() const {
        return jobid;
    }

    const std::chrono::time_point<std::chrono::system_clock> &getTstart() const {
        return tstart;
    }

    const std::chrono::time_point<std::chrono::system_clock> &getTend() const {
        return tend;
    }

    const std::vector<std::string> &getOsts() const {
        return osts;
    }

    void setOsts(const std::vector<std::string> &osts) {
        Job::osts = osts;
    }


    uint32_t getMin_read_throughput_MB() const {
        return min_read_throughput_MB;
    }
};

}

#endif //QOS_PLANNER_JOB_H
