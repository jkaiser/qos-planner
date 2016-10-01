//
// Created by jkaiser on 08.03.16.
//


#include "gtest/gtest.h"

#include <ClusterState.h>
#include "MockLustre.h"


using ::testing::_;

class MemClusterStateTest : public ::testing::Test {

protected:

    common::MockLustre *mock_lustre;
    std::shared_ptr<common::LocalLustre> mocked_ll;
    std::shared_ptr<common::MemoryClusterState> mem_cluster_state;

    virtual void TearDown() {
        mem_cluster_state.reset();
        mocked_ll.reset();
        mock_lustre = nullptr;
    }

    virtual void SetUp() {
        mock_lustre = new common::MockLustre();
        mocked_ll = std::shared_ptr<common::LocalLustre>(mock_lustre);
        mem_cluster_state.reset(new common::MemoryClusterState(mocked_ll));
    }
};

void compareWorkloads(const common::OSTWorkload &expected_wl, const common::OSTWorkload &returned_wl) {
    EXPECT_STREQ(expected_wl.ost.c_str(), returned_wl.ost.c_str());
    EXPECT_EQ(expected_wl.currentRpcSec, returned_wl.currentRpcSec);
    EXPECT_EQ(expected_wl.maxRpcSec, returned_wl.maxRpcSec);
}

TEST_F(MemClusterStateTest, InitTeardown) {
    ON_CALL(*mock_lustre, GetOstList(_, _)).WillByDefault(testing::Return(true));
    EXPECT_TRUE(mem_cluster_state->Init());

    std::this_thread::sleep_for(std::chrono::seconds(1)); // give the thread time to start
    EXPECT_TRUE(mem_cluster_state->TearDown());
};

TEST_F(MemClusterStateTest, TeardownWithoutInitShouldNotFail) {
    EXPECT_TRUE(mem_cluster_state->TearDown());
};

TEST_F(MemClusterStateTest, GetStateReturnsCorrectWorkload) {
    mem_cluster_state->Init();

    ON_CALL(*mock_lustre, GetOstList(_, _)).WillByDefault(testing::Return(true));

    EXPECT_TRUE(mem_cluster_state->GetOSTList()->empty());
    common::OSTWorkload workload = {"n1", 42, 17};
    mem_cluster_state->UpdateNode(workload.ost, workload);

    common::OSTWorkload returned_workload;
    EXPECT_TRUE(mem_cluster_state->getOstState(workload.ost, &returned_workload));
    compareWorkloads(workload, returned_workload);

    mem_cluster_state->TearDown();
};


