//
// Created by Jürgen Kaiser on 05.10.16.
//

#include <gtest/gtest.h>
#include <StorageReqParser.h>

#include <sstream>


class StorageReqParserTest : public ::testing::Test {

protected:
    common::StorageReqParser p;
    std::stringstream ss;

    virtual void TearDown() {
        ss.str("");
    }
};

TEST_F(StorageReqParserTest, MustFailAtEmptyInput) {
    ss.str("");
    ASSERT_FALSE(p.Parse(ss));
}

TEST_F(StorageReqParserTest, MustFailAtInvalidJSON) {

    ss.str("}{");
    ASSERT_FALSE(p.Parse(ss));

    ss.str("{\"foo\": 42");
    ASSERT_FALSE(p.Parse(ss));
}

TEST_F(StorageReqParserTest, MustAcceptValidFile) {

    std::string json = "{\"throughput_mbs\" : 10, \"read_files\" : [\"foo\"]}";
    ss.str(json);

    ASSERT_TRUE(p.Parse(ss));
}

TEST_F(StorageReqParserTest, MustDeliverValidValues) {

    std::string json = "{\"throughput_mbs\" : 10, \"read_files\" : [\"foo\"]}";
    ss.str(json);

    p.Parse(ss);

    ASSERT_EQ(10, p.getThroughput_mbs());
    ASSERT_STREQ("foo", p.getRead_files().data());
}