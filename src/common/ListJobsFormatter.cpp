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
    stream << std::left << std::setw(reservation_shift) << "Reserv.-ID";
    stream << std::left << std::setw(throughput_shift) << "Throughput [MB/s]";
    stream << std::left << std::setw(jobstate_shift) << "State";
    stream << std::left << std::setw(time_end_shift) << "TEnd";
    stream << std::endl;
}

void ListJobsFormatter::AddJob(Job *j) {
    AddReservationID(j);
    AddThroughput(j);
    AddJobState(j);
    AddTend(j);
    AddEndline();
}

void ListJobsFormatter::AddEndline() const { stream << std::endl; }

void ListJobsFormatter::AddReservationID(const Job *j) {
    stream << std::left << std::setw(reservation_shift) << j->getJobid();
}

void ListJobsFormatter::AddThroughput(const Job *j) {
    stream << std::left << std::setw(throughput_shift) << j->getMin_read_throughput_MB();
}

void ListJobsFormatter::AddTend(const Job *j)  {
    time_t now_c = std::chrono::system_clock::to_time_t(j->GetEndTime());
    stream << std::left << std::setw(time_end_shift) << std::put_time(localtime(&now_c), "%b %d %T");
}

void ListJobsFormatter::AddJobState(Job *j) {
    stream << std::setw(jobstate_shift) << Job::JobStateToString(j->getState());
}

}