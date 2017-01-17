/*
 * Copyright (c) 2017 Jürgen Kaiser
 */
/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * GPL HEADER END
 */


#include "gtest/gtest.h"

#include <OSTLimitConfigParser.h>

#include <sstream>

#include <spdlog/spdlog.h>

class OSTLimitParserTest : public ::testing::Test {

protected:
    std::unique_ptr<common::OSTLimitConfigParser> oparser_;

    virtual void SetUp() {
        if (!spdlog::get("console")) {
            auto console = spdlog::stdout_logger_mt("console");
            spdlog::set_level(spdlog::level::critical);
        }

        oparser_.reset(new common::OSTLimitConfigParser);
    }

    virtual void TearDown() {
        oparser_.reset();
    }
};

TEST_F(OSTLimitParserTest, ParseEmptyFileMustFail) {
    std::stringstream ss("");
    ASSERT_FALSE(oparser_->Parse(ss));
}

TEST_F(OSTLimitParserTest, ParseInvalidJsonMustFail) {
    std::stringstream ss("}{");
    ASSERT_FALSE(oparser_->Parse(ss));
}

TEST_F(OSTLimitParserTest, ParsedElementsMustSucceed) {
    std::stringstream ss("[{\"name\" : \"OST_a\", \"max_mbs\" : 100}]");
    ASSERT_TRUE(oparser_->Parse(ss));
}

TEST_F(OSTLimitParserTest, ParsedValuesMustBeCorrect) {
    std::stringstream ss("[{\"name\" : \"OST_a\", \"max_mbs\" : 100}, {\"name\" : \"OST_b\", \"max_mbs\" : 42.0}]");
    oparser_->Parse(ss);

    auto limits = oparser_->GetLimits();

    bool has_entry = limits.find("OST_a") != limits.end();
    ASSERT_TRUE(has_entry);
    ASSERT_EQ(100, limits["OST_a"]);

    has_entry = limits.find("OST_b") != limits.end();
    ASSERT_TRUE(has_entry);
    ASSERT_EQ(42.0, limits["OST_b"]);
}
