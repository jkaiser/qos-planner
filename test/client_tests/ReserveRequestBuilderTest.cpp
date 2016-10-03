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
    rpc::Request request;
};

TEST_F(ReserveRequestBuilderTest, ParseInvalidConsoleInputMustFail) {
    ASSERT_FALSE(rb.Parse("/foo/bar", 10, "", request));
}

TEST_F(ReserveRequestBuilderTest, ParseValidConsoleInputMustSucceed) {
    ASSERT_TRUE(rb.Parse("", 10, "30", request));
    ASSERT_TRUE(rb.Parse("/foo/bar", 0, "30", request));
    ASSERT_TRUE(rb.Parse("/foo/bar", 10, "30", request));
    ASSERT_TRUE(rb.Parse("/foo/bar,/foo/bar2", 10, "30", request));
}


TEST_F(ReserveRequestBuilderTest, CorrectlyParseConsoleInput) {

    rb.Parse("/foo/bar", 10, "30", request);
    ASSERT_EQ(rpc::Request::Type::Request_Type_RESERVE, request.type());
    ASSERT_EQ(10, request.resourcerequest().throughputmb());
    ASSERT_EQ(30, request.resourcerequest().tstop());

    ASSERT_EQ(1, request.resourcerequest().files_size());
    ASSERT_STREQ("/foo/bar", request.resourcerequest().files(0).data());


    request.Clear();
    rb.Parse("/foo/bar,/foo/bar2", 10, "30", request);
    ASSERT_EQ(rpc::Request::Type::Request_Type_RESERVE, request.type());
    ASSERT_EQ(10, request.resourcerequest().throughputmb());
    ASSERT_EQ(30, request.resourcerequest().tstop());

    ASSERT_EQ(2, request.resourcerequest().files_size());
    ASSERT_STREQ("/foo/bar", request.resourcerequest().files(0).data());
    ASSERT_STREQ("/foo/bar2", request.resourcerequest().files(1).data());
}
