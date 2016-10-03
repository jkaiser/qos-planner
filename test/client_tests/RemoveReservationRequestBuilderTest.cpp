//
// Created by Jürgen Kaiser on 03.10.16.
//

#include <gtest/gtest.h>

#include <RemoveReservationRequestBuilder.h>
#include <rpc/proto/message.pb.h>


class RemReservRequestBuilderTest : public ::testing::Test {
protected:
    virtual void TearDown() {
        message.Clear();
    }

    rpc::Request message;
    RemoveReservationRequestBuilder rb;
};

TEST_F(RemReservRequestBuilderTest, ParseValidInputMustSucceed) {
    ASSERT_TRUE(rb.BuildRequest("id", message));
}

TEST_F(RemReservRequestBuilderTest, MessageMustIncludeCorrectVals) {
    rb.BuildRequest("id", message);

    ASSERT_EQ(rpc::Request_Type_DELETE, message.type());
    ASSERT_EQ(1, message.deleterequest().id_size());
    ASSERT_STREQ("id", message.deleterequest().id(0).data());
}

TEST_F(RemReservRequestBuilderTest, MessageOnlyContainsDeleteRequest) {

    rpc::Request message;
    rb.BuildRequest("id", message);

    ASSERT_FALSE(message.has_resourcerequest());
    ASSERT_FALSE(message.has_listjobsrequest());
}