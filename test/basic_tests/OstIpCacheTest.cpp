//
// Created by Jürgen Kaiser on 07.11.16.
//

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