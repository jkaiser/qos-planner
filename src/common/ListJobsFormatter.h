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
    std::stringstream stream_;

    static const int kReservationShift = 15;
    static const int kThroughputShift = 20;
    static const int kJobstateShift = 15;
    static const int kTimeEndShift = 20;

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
