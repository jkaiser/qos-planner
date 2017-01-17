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
#include "mock_classes/MockLustre.h"

#include <OstIpsCache.h>
#include <Lustre.h>

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;

class OstIpsCacheTest : public ::testing::Test {
protected:

    std::unique_ptr<common::OstIpsCache> ip_cache_;
    common::MockLustre *mock_lustre_;
    std::shared_ptr<common::MockLustre> mocked_ll_;

    virtual void SetUp() {
        mock_lustre_ = new common::MockLustre();
        mocked_ll_.reset(mock_lustre_);

        ip_cache_.reset(new common::OstIpsCache(mocked_ll_));
    }
};

TEST_F(OstIpsCacheTest, WhenGivenValidUUIDThenReturnCorrectIp) {

    std::string expected_ip = "127.0.0.1";
    std::string uuid = "foo_uuid";

    EXPECT_CALL(*mocked_ll_, GetIPOfOst(_, _)).WillOnce(DoAll(testing::SetArgReferee<1>(expected_ip), Return(true)));

    std::string returned_ip = ip_cache_->GetIp(uuid);
    ASSERT_STREQ(expected_ip.c_str(), returned_ip.c_str());
}

TEST_F(OstIpsCacheTest, WhenGivenInValidUUIDThenReturnEmptyString) {

    std::string expected_ip = "";
    std::string uuid = "foo_uuid";

    EXPECT_CALL(*mocked_ll_, GetIPOfOst(_, _)).WillOnce(DoAll(testing::SetArgReferee<1>(expected_ip), Return(false)));

    std::string returned_ip = ip_cache_->GetIp(uuid);
    ASSERT_STREQ(expected_ip.c_str(), returned_ip.c_str());
}

TEST_F(OstIpsCacheTest, WhenLustreFailsThenReturnEmptyString) {

    std::string expected_ip = "";
    std::string returned_from_lustre = "nonsense";
    std::string uuid = "foo_uuid";

    EXPECT_CALL(*mocked_ll_, GetIPOfOst(_, _)).WillOnce(DoAll(testing::SetArgReferee<1>(returned_from_lustre), Return(false)));

    std::string returned_ip = ip_cache_->GetIp(uuid);
    ASSERT_STREQ(expected_ip.c_str(), returned_ip.c_str());
}