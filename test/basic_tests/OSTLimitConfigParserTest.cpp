//
// Created by Jürgen Kaiser on 01.10.16.
//


#include "gtest/gtest.h"

#include <OSTLimitConfigParser.h>

#include <sstream>

class OSTLimitParserTest : public ::testing::Test {

protected:
    std::unique_ptr<common::OSTLimitConfigParser> oparser;

    virtual void SetUp() {
        oparser.reset(new common::OSTLimitConfigParser);
    }

    virtual void TearDown() {
        oparser.reset();
    }
};

TEST_F(OSTLimitParserTest, ParseEmptyFileMustFail) {
    std::stringstream ss("");
    ASSERT_FALSE(oparser->Parse(ss));
}

TEST_F(OSTLimitParserTest, ParseInvalidJsonMustFail) {
    std::stringstream ss("}{");
    ASSERT_FALSE(oparser->Parse(ss));
}

TEST_F(OSTLimitParserTest, ParsedElementsMustSucceed) {
    std::stringstream ss("[{\"name\" : \"OST_a\", \"max_mbs\" : 100}]");
    ASSERT_TRUE(oparser->Parse(ss));
}

TEST_F(OSTLimitParserTest, ParsedValuesMustBeCorrect) {
    std::stringstream ss("[{\"name\" : \"OST_a\", \"max_mbs\" : 100}, {\"name\" : \"OST_b\", \"max_mbs\" : 42.0}]");
    oparser->Parse(ss);

    auto limits = oparser->GetLimits();

    bool has_entry = limits.find("OST_a") != limits.end();
    ASSERT_TRUE(has_entry);
    ASSERT_EQ(100, limits["OST_a"]);

    has_entry = limits.find("OST_b") != limits.end();
    ASSERT_TRUE(has_entry);
    ASSERT_EQ(42.0, limits["OST_b"]);
}
