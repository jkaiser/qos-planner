//
// Created by Jürgen Kaiser on 03.10.16.
//


#include "gtest/gtest.h"

#include <string>
#include <ReserveRequestBuilder.h>
#include <rpc/proto/message.pb.h>


class ReserveRequestBuilderTest : public ::testing::Test {
protected:
    ReserveRequestBuilder rb;
    rpc::Request message;
};

TEST_F(ReserveRequestBuilderTest, ParseInvalidConsoleInputMustFail) {
    ASSERT_FALSE(rb.BuildRequest("", "/foo/bar", 10, 0, message));
    ASSERT_FALSE(rb.BuildRequest("id", "/foo/bar", 10, -1, message));
    ASSERT_FALSE(rb.BuildRequest("id", "/foo/bar", 10, 0, message));
}

TEST_F(ReserveRequestBuilderTest, ParseValidConsoleInputMustSucceed) {

    ASSERT_TRUE(rb.BuildRequest("id", "", 10, 30, message));
    ASSERT_TRUE(rb.BuildRequest("id", "/foo/bar", 0, 30, message));
    ASSERT_TRUE(rb.BuildRequest("id", "/foo/bar", 10, 30, message));
    ASSERT_TRUE(rb.BuildRequest("id", "/foo/bar,/foo/bar2", 10, 30, message));
}

TEST_F(ReserveRequestBuilderTest, CorrectlyParseConsoleInput) {

    rb.BuildRequest("id", "/foo/bar", 10, 30, message);
    ASSERT_EQ(rpc::Request::Type::Request_Type_RESERVE, message.type());
    ASSERT_EQ(10, message.resourcerequest().throughputmb());
    ASSERT_EQ(30, message.resourcerequest().durationsec());

    ASSERT_EQ(1, message.resourcerequest().files_size());
    ASSERT_STREQ("id", message.resourcerequest().id().data());
    ASSERT_STREQ("/foo/bar", message.resourcerequest().files(0).data());

    message.Clear();
    rb.BuildRequest("id", "/foo/bar,/foo/bar2", 10, 30, message);
    ASSERT_EQ(rpc::Request::Type::Request_Type_RESERVE, message.type());
    ASSERT_EQ(10, message.resourcerequest().throughputmb());
    ASSERT_EQ(30, message.resourcerequest().durationsec());

    ASSERT_EQ(2, message.resourcerequest().files_size());
    ASSERT_STREQ("id", message.resourcerequest().id().data());
    ASSERT_STREQ("/foo/bar", message.resourcerequest().files(0).data());
    ASSERT_STREQ("/foo/bar2", message.resourcerequest().files(1).data());
}
