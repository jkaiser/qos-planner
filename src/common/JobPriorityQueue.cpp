//
// Created by jkaiser on 17.03.16.
//

#include "JobPriorityQueue.h"

namespace common {

JobPriorityQueue::JobPriorityQueue() {
    heap.reserve(100);
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
    heap.push_back(new_item);
    Up(heap, (uint32_t) (heap.size() - 1));
}

JobPriorityQueue::WaitingItem *JobPriorityQueue::Pop() {
    if (heap.size() == 0) {
        return nullptr;
    }

    WaitingItem *return_item = heap[0];
    heap[0] = heap[heap.size() - 1];
    heap.pop_back();

    if (heap.size() != 0) {
        Down(heap, 0);
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
    for (int i = 0; i < heap.size(); i++) {
        if (heap[i]->jobid == job) {
            WaitingItem *tmp = heap[i];
            heap[i] = heap[heap.size() - 1];

            if (i == heap.size() -1) {  // it was the last element
                heap.pop_back();
                return true;
            } else {
                heap.pop_back();
                Down(heap, i);
                return true;
            }
        }
    }
    return false;
}

const JobPriorityQueue::WaitingItem *JobPriorityQueue::Peek() const {
    if (heap.size() == 0) {
        return nullptr;
    }

    return heap[0];
}
}
