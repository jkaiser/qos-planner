//
// Created by Jürgen Kaiser on 05.10.16.
//

#include <gtest/gtest.h>
#include <StorageReqParser.h>

#include <sstream>


class StorageReqParserTest : public ::testing::Test {

protected:
    common::StorageReqParser sq_parser_;
    std::stringstream ss_;

    virtual void TearDown() {
        ss_.str("");
    }
};

TEST_F(StorageReqParserTest, MustFailAtEmptyInput) {
    ss_.str("");
    ASSERT_FALSE(sq_parser_.Parse(ss_));
}

TEST_F(StorageReqParserTest, MustFailAtInvalidJSON) {

    ss_.str("}{");
    ASSERT_FALSE(sq_parser_.Parse(ss_));

    ss_.str("{\"foo\": 42");
    ASSERT_FALSE(sq_parser_.Parse(ss_));
}

TEST_F(StorageReqParserTest, MustAcceptValidFile) {

    std::string json = "{\"throughput_mbs\" : 10, \"read_files\" : [\"foo\"]}";
    ss_.str(json);

    ASSERT_TRUE(sq_parser_.Parse(ss_));
}

TEST_F(StorageReqParserTest, MustDeliverValidValues) {

    std::string json = "{\"throughput_mbs\" : 10, \"read_files\" : [\"foo\"]}";
    ss_.str(json);

    sq_parser_.Parse(ss_);

    ASSERT_EQ(10, sq_parser_.getThroughput_mbs());
    ASSERT_STREQ("foo", sq_parser_.getRead_files().data());
}