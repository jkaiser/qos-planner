//
// Created by Jürgen Kaiser on 04.10.16.
//

#include "ListJobsFormatter.h"
#include <iomanip>

namespace common {

std::shared_ptr<std::string> ListJobsFormatter::Format(std::vector<Job *> &jobs) {

    std::shared_ptr<std::string> formatted_string(new std::string());
    if (jobs.empty()) {
        *formatted_string = "";
        return formatted_string;
    }

    AddHeader();
    AddJobs(jobs);

    *formatted_string = stream.str();
    stream = std::stringstream();
    return formatted_string;

}

void ListJobsFormatter::AddJobs(const std::vector<Job *> &jobs) {
    for(auto &j : jobs) {
        AddJob(j);
    }
}

void ListJobsFormatter::AddHeader() {
    stream << std::setw(10) << "Job ID";
    stream << std::setw(20) << "Reserv. ID";
    stream << std::setw(20) << "TEnd";
    stream << std::setw(20) << "Throughput [MB/s]";
    stream << std::setw(10) << "State";
    stream << std::endl;
}

void ListJobsFormatter::AddJob(Job *j) {
    AddJobID(j);
    AddTend(j);
    AddThroughput(j);
    AddJobState(j);
}

void ListJobsFormatter::AddJobID(const Job *j) { stream << std::setw(10) << j->getJobid(); }

void ListJobsFormatter::AddThroughput(const Job *j) { stream << std::setw(20) << j->getMin_read_throughput_MB(); }

void ListJobsFormatter::AddTend(const Job *j)  {
    time_t now_c = std::chrono::system_clock::to_time_t(j->GetEndTime());
    stream << std::setw(20) << std::put_time(localtime(&now_c), "%b %d %T");
}

void ListJobsFormatter::AddJobState(Job *j) {
    stream << std::setw(10) << Job::JobStateToString(j->getState());
}

}