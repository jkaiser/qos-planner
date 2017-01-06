//
// Created by jkaiser on 17.03.16.
//

#include "JobPriorityQueue.h"

namespace common {

JobPriorityQueue::JobPriorityQueue() {
    heap_.reserve(100);
}

void JobPriorityQueue::Up(std::vector<WaitingItem *> &queue, int pos) {
    if (pos == 0) {
        return;
    }

    int parent = (pos-1)/2;

    if (queue[parent]->time_of_event < queue[pos]->time_of_event) {
        WaitingItem *tmp = queue[parent];
        queue[parent] = queue[pos];
        queue[pos] = tmp;
        Up(queue, parent);
    }
}

void JobPriorityQueue::Push(JobPriorityQueue::WaitingItem *new_item) {
    heap_.push_back(new_item);
    Up(heap_, (uint32_t) (heap_.size() - 1));
}

JobPriorityQueue::WaitingItem *JobPriorityQueue::Pop() {
    if (heap_.size() == 0) {
        return nullptr;
    }

    WaitingItem *return_item = heap_[0];
    heap_[0] = heap_[heap_.size() - 1];
    heap_.pop_back();

    if (heap_.size() != 0) {
        Down(heap_, 0);
    }

    return return_item;
}

void JobPriorityQueue::Down(std::vector<WaitingItem *> &queue, int pos) {

    int pos_child1 = 2*pos + 1;
    if (pos_child1 >= queue.size() || pos_child1 < 0) {
        return;
    }
    int pos_child2 = 2*pos + 2;
    if (pos_child2 >= queue.size() || pos_child2 < 0) {
        return;
    }

    int pos_to_check;
    if (queue[pos_child1]->time_of_event < queue[pos_child2]->time_of_event) {
        pos_to_check = pos_child1;
    } else {
        pos_to_check = pos_child2;
    }

    if (queue[pos]->time_of_event < queue[pos_to_check]->time_of_event) {   // nothing to do
        return;
    }
    WaitingItem *tmp = queue[pos_to_check];
    queue[pos_to_check] = queue[pos];
    queue[pos] = tmp;
    Down(queue, pos_to_check);
}

bool JobPriorityQueue::Remove(std::string job) {
    for (int i = 0; i < heap_.size(); i++) {
        if (heap_[i]->jobid == job) {
            WaitingItem *tmp = heap_[i];
            heap_[i] = heap_[heap_.size() - 1];

            if (i == heap_.size() -1) {  // it was the last element
                heap_.pop_back();
                return true;
            } else {
                heap_.pop_back();
                Down(heap_, i);
                return true;
            }
        }
    }
    return false;
}

const JobPriorityQueue::WaitingItem *JobPriorityQueue::Peek() const {
    if (heap_.size() == 0) {
        return nullptr;
    }

    return heap_[0];
}
}
