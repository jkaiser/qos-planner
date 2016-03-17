//
// Created by jkaiser on 17.03.16.
//

#include "gtest/gtest.h"

#include <JobPriorityQueue.h>

TEST(JobPriorityQueue, EmptyPop) {

    common::JobPriorityQueue jq;

    EXPECT_EQ(nullptr, jq.Pop());
    EXPECT_FALSE(jq.Remove("foo"));
}

TEST(JobPriorityQueue, PushPop) {
    common::JobPriorityQueue jq;
    common::JobPriorityQueue::WaitingItem *wt, *wt_2, *wt_3;
    wt = new common::JobPriorityQueue::WaitingItem();
    wt->jobid = "wt_1";
    wt->time_of_event = std::chrono::system_clock::now();

    jq.Push(wt);

    auto tmp = jq.Pop();
    EXPECT_STREQ("wt_1", tmp->jobid.c_str());
    EXPECT_EQ(nullptr, jq.Pop()) << "heap must be empty";

    jq.Push(wt);
    wt_2 = new common::JobPriorityQueue::WaitingItem();
    wt_2->jobid = "wt_2";
    wt_2->time_of_event = wt->time_of_event - std::chrono::seconds(10);
    jq.Push(wt_2);

    wt_3 = new common::JobPriorityQueue::WaitingItem();
    wt_3->jobid = "wt_3";
    wt_3->time_of_event = wt->time_of_event + std::chrono::seconds(10);
    jq.Push(wt_3);

    // second one should be returned
    tmp = jq.Pop();
    EXPECT_STREQ("wt_2", wt_2->jobid.c_str());
    tmp = jq.Pop();
    EXPECT_STREQ("wt_1", wt->jobid.c_str());
    tmp = jq.Pop();
    EXPECT_STREQ("wt_3", wt_3->jobid.c_str());
    EXPECT_EQ(nullptr, jq.Pop());
}

TEST(JobPriorityQueue, Remove) {
    common::JobPriorityQueue jq;
    common::JobPriorityQueue::WaitingItem *wt, *wt_2;
    wt = new common::JobPriorityQueue::WaitingItem();
    wt->jobid = "wt_1";
    wt->time_of_event = std::chrono::system_clock::now();
    jq.Push(wt);

    wt_2 = new common::JobPriorityQueue::WaitingItem();
    wt_2->jobid = "wt_2";
    wt_2->time_of_event = wt->time_of_event - std::chrono::seconds(10);
    jq.Push(wt_2);

    EXPECT_TRUE(jq.Remove("wt_2"));
    EXPECT_FALSE(jq.Remove("wt_2"));

    auto tmp = jq.Pop();
    EXPECT_STREQ("wt_1", wt->jobid.c_str());
    EXPECT_EQ(nullptr, jq.Pop());
}

