//
// Created by Jürgen Kaiser on 04.10.16.
//

#ifndef QOS_PLANNER_LISTJOBSFORMATTER_H
#define QOS_PLANNER_LISTJOBSFORMATTER_H

#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include "Job.h"


namespace common {

class ListJobsFormatter {

private:
    std::stringstream stream;

    static const int reservation_shift = 15;
    static const int throughput_shift = 20;
    static const int jobstate_shift = 15;
    static const int time_end_shift = 20;

    void AddJob(Job *j);
    void AddJobState(Job *j);
    void AddTend(const Job *j);
    void AddThroughput(const Job *j);
    void AddReservationID(const Job *j);
    void AddHeader();
    void AddJobs(const std::vector<Job *> &jobs);
    void AddEndline();

public:

    std::shared_ptr<std::string> Format(std::vector<common::Job*> &jobs);
};

}
#endif //QOS_PLANNER_LISTJOBSFORMATTER_H
