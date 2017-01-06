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

    *formatted_string = stream_.str();
    stream_ = std::stringstream();
    return formatted_string;

}

void ListJobsFormatter::AddJobs(const std::vector<Job *> &jobs) {
    for(auto &j : jobs) {
        AddJob(j);
    }
}

void ListJobsFormatter::AddHeader() {
    stream_ << std::left << std::setw(kReservationShift) << "Reserv.-ID";
    stream_ << std::left << std::setw(kThroughputShift) << "Throughput [MB/s]";
    stream_ << std::left << std::setw(kJobstateShift) << "State";
    stream_ << std::left << std::setw(kTimeEndShift) << "TEnd";
    stream_ << std::endl;
}

void ListJobsFormatter::AddJob(Job *j) {
    AddReservationID(j);
    AddThroughput(j);
    AddJobState(j);
    AddTend(j);
    AddEndline();
}

void ListJobsFormatter::AddEndline() { stream_ << std::endl; }

void ListJobsFormatter::AddReservationID(const Job *j) {
    stream_ << std::left << std::setw(kReservationShift) << j->getJobid();
}

void ListJobsFormatter::AddThroughput(const Job *j) {
    stream_ << std::left << std::setw(kThroughputShift) << j->getMin_read_throughput_MB();
}

void ListJobsFormatter::AddTend(const Job *j)  {
    time_t now_c = std::chrono::system_clock::to_time_t(j->GetEndTime());
    stream_ << std::left << std::setw(kTimeEndShift) << std::put_time(localtime(&now_c), "%b %d %T");
}

void ListJobsFormatter::AddJobState(Job *j) {
    stream_ << std::setw(kJobstateShift) << Job::JobStateToString(j->getState());
}

}