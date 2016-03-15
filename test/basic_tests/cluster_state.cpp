//
// Created by jkaiser on 08.03.16.
//


#include "gtest/gtest.h"

#include <ClusterState.h>

TEST(MemClusterState, InitTeardown) {
    MemoryClusterState mcs;
    EXPECT_TRUE(mcs.Init());

    std::this_thread::sleep_for(std::chrono::seconds(1)); // give the thread time to start
    EXPECT_TRUE(mcs.TearDown());
};

