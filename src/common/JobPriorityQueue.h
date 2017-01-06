//
// Created by jkaiser on 17.03.16.
//

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
