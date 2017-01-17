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

#include <gtest/gtest.h>
#include <chrono>

#include <Job.h>
#include <ListJobsFormatter.h>

class ListJobsFormatterTest : public ::testing::Test {

protected:
    common::ListJobsFormatter formatter_;
    std::vector<common::Job*> jobs_;

    virtual void TearDown() {
        for (int i = 0; i < jobs_.size(); ++i) {
            delete jobs_[i];
        }
        jobs_.clear();
    }

};

TEST_F(ListJobsFormatterTest, FormatEmptyListShouldShowSpecialText) {
    std::shared_ptr<std::string> text = formatter_.Format(jobs_);

    ASSERT_STREQ("", text->data());
}

TEST_F(ListJobsFormatterTest, FormattedStringMustNotBeEmpty) {

    std::string id = "id";
    std::chrono::time_point<std::chrono::system_clock> tstart = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> tend = tstart;
    int throughput_mbs = 10;

    common::Job *job = new common::Job(id, tstart, tend, throughput_mbs);
    jobs_.push_back(job);

    auto text = formatter_.Format(jobs_);
    ASSERT_TRUE(text->size() > 0);
}