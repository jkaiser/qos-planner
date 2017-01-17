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

