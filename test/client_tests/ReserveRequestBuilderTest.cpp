//
// Created by Jürgen Kaiser on 03.10.16.
//


#include "gtest/gtest.h"

#include <string>
#include <ReserveRequestBuilder.h>
#include <rpc/proto/message.pb.h>


class ReserveRequestBuilderTest : public ::testing::Test {
protected:
    ReserveRequestBuilder rb_;
    rpc::Request message_;
};

TEST_F(ReserveRequestBuilderTest, ParseInvalidConsoleInputMustFail) {
    ASSERT_FALSE(rb_.BuildRequest("", "/foo/bar", 10, 0, message_));
    ASSERT_FALSE(rb_.BuildRequest("id", "/foo/bar", 10, -1, message_));
    ASSERT_FALSE(rb_.BuildRequest("id", "/foo/bar", 10, 0, message_));
}

TEST_F(ReserveRequestBuilderTest, ParseValidConsoleInputMustSucceed) {

    ASSERT_TRUE(rb_.BuildRequest("id", "", 10, 30, message_));
    ASSERT_TRUE(rb_.BuildRequest("id", "/foo/bar", 0, 30, message_));
    ASSERT_TRUE(rb_.BuildRequest("id", "/foo/bar", 10, 30, message_));
    ASSERT_TRUE(rb_.BuildRequest("id", "/foo/bar,/foo/bar2", 10, 30, message_));
}

TEST_F(ReserveRequestBuilderTest, CorrectlyParseConsoleInput) {

    rb_.BuildRequest("id", "/foo/bar", 10, 30, message_);
    ASSERT_EQ(rpc::Request::Type::Request_Type_RESERVE, message_.type());
    ASSERT_EQ(10, message_.resourcerequest().throughputmb());
    ASSERT_EQ(30, message_.resourcerequest().durationsec());

    ASSERT_EQ(1, message_.resourcerequest().files_size());
    ASSERT_STREQ("id", message_.resourcerequest().id().data());
    ASSERT_STREQ("/foo/bar", message_.resourcerequest().files(0).data());

    message_.Clear();
    rb_.BuildRequest("id", "/foo/bar,/foo/bar2", 10, 30, message_);
    ASSERT_EQ(rpc::Request::Type::Request_Type_RESERVE, message_.type());
    ASSERT_EQ(10, message_.resourcerequest().throughputmb());
    ASSERT_EQ(30, message_.resourcerequest().durationsec());

    ASSERT_EQ(2, message_.resourcerequest().files_size());
    ASSERT_STREQ("id", message_.resourcerequest().id().data());
    ASSERT_STREQ("/foo/bar", message_.resourcerequest().files(0).data());
    ASSERT_STREQ("/foo/bar2", message_.resourcerequest().files(1).data());
}
