//
// Created by Jürgen Kaiser on 07.11.16.
//

#include "gtest/gtest.h"
#include "MockLustre.h"

#include <OstIpsCache.h>
#include <Lustre.h>

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;

class OstUpsCacheTest : public ::testing::Test {
protected:

    std::unique_ptr<common::OstIpsCache> ip_cache;

    virtual void SetUp() {
        mock_lustre = new common::MockLustre();
        mocked_ll.reset(mock_lustre);

        ip_cache.reset(new common::OstIpsCache(mocked_ll));
    }

    common::MockLustre *mock_lustre;
    std::shared_ptr<common::MockLustre> mocked_ll;
};

TEST_F(OstUpsCacheTest, WhenGivenValidUUIDThenReturnCorrectIp) {

    std::string expected_ip = "127.0.0.1";
    std::string uuid = "foo_uuid";

    EXPECT_CALL(*mocked_ll, GetIPOfOst(_, _)).WillOnce(DoAll(testing::SetArgReferee<1>(expected_ip), Return(true)));

    std::string returned_ip = ip_cache->GetIp(uuid);
    ASSERT_STREQ(expected_ip.c_str(), returned_ip.c_str());
}

TEST_F(OstUpsCacheTest, WhenGivenInValidUUIDThenReturnEmptyString) {

    std::string expected_ip = "";
    std::string uuid = "foo_uuid";

    EXPECT_CALL(*mocked_ll, GetIPOfOst(_, _)).WillOnce(DoAll(testing::SetArgReferee<1>(expected_ip), Return(false)));

    std::string returned_ip = ip_cache->GetIp(uuid);
    ASSERT_STREQ(expected_ip.c_str(), returned_ip.c_str());
}

TEST_F(OstUpsCacheTest, WhenLustreFailsThenReturnEmptyString) {

    std::string expected_ip = "";
    std::string returned_from_lustre = "nonsense";
    std::string uuid = "foo_uuid";

    EXPECT_CALL(*mocked_ll, GetIPOfOst(_, _)).WillOnce(DoAll(testing::SetArgReferee<1>(returned_from_lustre), Return(false)));

    std::string returned_ip = ip_cache->GetIp(uuid);
    ASSERT_STREQ(expected_ip.c_str(), returned_ip.c_str());
}