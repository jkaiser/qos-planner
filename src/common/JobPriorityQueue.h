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

#ifndef QOS_PLANNER_JOBPRIORITYQUEUE_H
#define QOS_PLANNER_JOBPRIORITYQUEUE_H

#include <string>
#include <vector>
#include <chrono>

#include "Job.h"

namespace common {


/**
 * Implements a simple priority queue for job waiting items
 */
class JobPriorityQueue {


public:
    struct WaitingItem {
        std::string jobid;
        std::chrono::system_clock::time_point time_of_event;
        Job::JobEvent eventType;

        WaitingItem() {};
        WaitingItem(std::string jid, std::chrono::system_clock::time_point time, Job::JobEvent event)
            : jobid{jid},
              time_of_event(time),
              eventType(event) {};
    };



private:
    std::vector<WaitingItem*> heap_;
    void Up(std::vector<WaitingItem*> &queue, int pos);
    void Down(std::vector<WaitingItem*> &queue, int pos);

public:
    JobPriorityQueue();

    void Push(WaitingItem *new_item);

    /**
     * Removes the first item
     */
    WaitingItem *Pop();

    /**
     * Returns the first item in the queue without removing it.
     */
    const WaitingItem *Peek() const;

    /**
     * Removes the given job from the queue
     * Returns:
     *      true: if job exists and it was removed successfully
     *      false: else
     */
    bool Remove(std::string job);
};

}

#endif //QOS_PLANNER_JOBPRIORITYQUEUE_H
