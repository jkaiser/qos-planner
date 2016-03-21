//
// Created by jkaiser on 08.03.16.
//


#include "gtest/gtest.h"

#include <ClusterState.h>


TEST(MemClusterState, InitTeardown) {
    common::MemoryClusterState mcs;
    EXPECT_TRUE(mcs.Init());

    std::this_thread::sleep_for(std::chrono::seconds(1)); // give the thread time to start
    EXPECT_TRUE(mcs.TearDown());
};

TEST(MemClusterState, GetState) {
    common::MemoryClusterState mcs;
    EXPECT_TRUE(mcs.Init());

    EXPECT_TRUE(mcs.getNodes()->empty());
    common::NodeState ns = {"n1", 42, 17};
    mcs.UpdateNode("n1", ns);

    auto rt = mcs.getState("n1");
    EXPECT_NE(rt, nullptr);
    EXPECT_STREQ(rt->name.c_str(), "n1");
    EXPECT_EQ(ns.rpcSec, rt->rpcSec);
    EXPECT_EQ(ns.maxRpcSec, rt->maxRpcSec);

    EXPECT_TRUE(mcs.TearDown());
};


