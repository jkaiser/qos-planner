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

#include <ClusterState.h>
#include "mock_classes/MockLustre.h"


using ::testing::_;

class MemClusterStateTest : public ::testing::Test {

protected:

    common::MockLustre *mock_lustre_;
    std::shared_ptr<common::LocalLustre> mocked_ll_;
    std::shared_ptr<common::MemoryClusterState> mem_cluster_state_;

    virtual void TearDown() {
        mem_cluster_state_.reset();
        mocked_ll_.reset();
        mock_lustre_ = nullptr;
    }

    virtual void SetUp() {
        mock_lustre_ = new common::MockLustre();
        mocked_ll_ = std::shared_ptr<common::LocalLustre>(mock_lustre_);
        mem_cluster_state_.reset(new common::MemoryClusterState(mocked_ll_));
    }
};

void compareWorkloads(const common::OSTWorkload &expected_wl, const common::OSTWorkload &returned_wl) {
    EXPECT_STREQ(expected_wl.ost.c_str(), returned_wl.ost.c_str());
    EXPECT_EQ(expected_wl.currentRpcSec, returned_wl.currentRpcSec);
    EXPECT_EQ(expected_wl.maxRpcSec, returned_wl.maxRpcSec);
}

TEST_F(MemClusterStateTest, InitTeardown) {
    ON_CALL(*mock_lustre_, GetOstList(_, _)).WillByDefault(testing::Return(true));
    EXPECT_TRUE(mem_cluster_state_->Init());

    std::this_thread::sleep_for(std::chrono::seconds(1)); // give the thread time to start
    EXPECT_TRUE(mem_cluster_state_->TearDown());
};

TEST_F(MemClusterStateTest, TeardownWithoutInitShouldNotFail) {
    EXPECT_TRUE(mem_cluster_state_->TearDown());
};

TEST_F(MemClusterStateTest, GetStateReturnsCorrectWorkload) {
    mem_cluster_state_->Init();

    ON_CALL(*mock_lustre_, GetOstList(_, _)).WillByDefault(testing::Return(true));

    EXPECT_TRUE(mem_cluster_state_->GetOSTList()->empty());
    common::OSTWorkload workload = {"n1", 42, 17};
    mem_cluster_state_->UpdateNode(workload.ost, workload);

    common::OSTWorkload returned_workload;
    EXPECT_TRUE(mem_cluster_state_->getOstState(workload.ost, &returned_workload));
    compareWorkloads(workload, returned_workload);

    mem_cluster_state_->TearDown();
};


