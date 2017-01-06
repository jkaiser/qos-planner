//
// Created by Jürgen Kaiser on 03.10.16.
//

#include <gtest/gtest.h>

#include <RemoveReservationRequestBuilder.h>
#include <rpc/proto/message.pb.h>


class RemReservRequestBuilderTest : public ::testing::Test {
protected:
    virtual void TearDown() {
        message_.Clear();
    }

    rpc::Request message_;
    RemoveReservationRequestBuilder rb_;
};

TEST_F(RemReservRequestBuilderTest, ParseValidInputMustSucceed) {
    ASSERT_TRUE(rb_.BuildRequest("id", message_));
}

TEST_F(RemReservRequestBuilderTest, MessageMustIncludeCorrectVals) {
    rb_.BuildRequest("id", message_);

    ASSERT_EQ(rpc::Request_Type_DELETE, message_.type());
    ASSERT_EQ(1, message_.deleterequest().id_size());
    ASSERT_STREQ("id", message_.deleterequest().id(0).data());
}

TEST_F(RemReservRequestBuilderTest, MessageOnlyContainsDeleteRequest) {

    rpc::Request message;
    rb_.BuildRequest("id", message);

    ASSERT_FALSE(message.has_resourcerequest());
    ASSERT_FALSE(message.has_listjobsrequest());
}