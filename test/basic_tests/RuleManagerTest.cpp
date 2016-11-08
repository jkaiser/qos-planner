//
// Created by Jürgen Kaiser on 02.11.16.
//


#include "gtest/gtest.h"

#include <RuleManager.h>
#include "mock_classes/MockRuleSetter.h"
#include "mock_classes/MockLustre.h"
#include "mock_classes/MockOstIpsCache.h"
#include "mock_classes/MockOstIdsConverter.h"

using ::testing::_;
using ::testing::StrEq;
using ::testing::Not;
using ::testing::InSequence;

class RuleManagerTest : public ::testing::Test {
protected:
    std::unique_ptr<common::RuleManager> rm;

    common::MockRuleSetter *mock_ruleSetter;
    common::MockOstIpsCache *mock_ost_ip_cache;
    common::MockOstIdsConverter *mock_id_converter;
    std::shared_ptr<common::MockRuleSetter> mocked_ruleSetter;
    std::shared_ptr<common::MockOstIpsCache> mocked_ost_ip_cache;
    std::shared_ptr<common::MockOstIdsConverter> mocked_id_converter;

    std::vector<std::string> ost_ids;
    std::vector<std::string> ost_uuids;
    std::vector<std::string> ost_ips;
    std::string job_id;
    uint32_t min_throughput_mbs;


protected:
    virtual void TearDown() {
        ost_ids.clear();
        ost_uuids.clear();
        job_id = "";
        min_throughput_mbs = 0;
    }

    virtual void SetUp() {
        mock_ruleSetter = new common::MockRuleSetter();
        mock_ost_ip_cache = new common::MockOstIpsCache();
        mock_id_converter = new common::MockOstIdsConverter();

        mocked_ruleSetter.reset(mock_ruleSetter);
        mocked_ost_ip_cache.reset(mock_ost_ip_cache);
        mocked_id_converter.reset(mock_id_converter);
        rm.reset(new common::RuleManager(mocked_ruleSetter, mocked_ost_ip_cache, mocked_id_converter));
    }

    void createSingleOstSetup() {
        ost_ids = {"ost1"};
        ost_uuids = {"ost1_uuid"};
        ost_ips = {"127.0.0.1"};
        job_id = "jobid";
        min_throughput_mbs = 42;

        ON_CALL(*mocked_id_converter, ToUUID(ost_ids[0])).WillByDefault(testing::Return(ost_uuids[0]));;
        ON_CALL(*mocked_ost_ip_cache, GetIp(ost_uuids[0])).WillByDefault(testing::Return(ost_ips[0]));
    }

    void createDoubleOstSetup() {
        ost_ids = {"ost1", "ost2"};
        ost_uuids = {"ost1_uuid", "ost2_uuid"};
        ost_ips = {"127.0.0.1", "127.0.0.2"};
        job_id = "jobid";
        min_throughput_mbs = 42;

        ON_CALL(*mocked_id_converter, ToUUID(ost_ids[0])).WillByDefault(testing::Return(ost_uuids[0]));
        ON_CALL(*mocked_id_converter, ToUUID(ost_ids[1])).WillByDefault(testing::Return(ost_uuids[1]));
        ON_CALL(*mocked_ost_ip_cache, GetIp(ost_uuids[0])).WillByDefault(testing::Return(ost_ips[0]));
        ON_CALL(*mocked_ost_ip_cache, GetIp(ost_uuids[1])).WillByDefault(testing::Return(ost_ips[1]));
    }

    void createTwoOstOnSameOSSSetup() {
        ost_ids = {"ost1", "ost2"};
        ost_uuids = {"ost1_uuid", "ost2_uuid"};
        ost_ips = {"127.0.0.1", "127.0.0.1"};
        job_id = "jobid";
        min_throughput_mbs = 42;

        ON_CALL(*mocked_id_converter, ToUUID(ost_ids[0])).WillByDefault(testing::Return(ost_uuids[0]));
        ON_CALL(*mocked_id_converter, ToUUID(ost_ids[1])).WillByDefault(testing::Return(ost_uuids[1]));
        ON_CALL(*mocked_ost_ip_cache, GetIp(ost_uuids[0])).WillByDefault(testing::Return(ost_ips[0]));
        ON_CALL(*mocked_ost_ip_cache, GetIp(ost_uuids[1])).WillByDefault(testing::Return(ost_ips[1]));
    }
};

TEST_F(RuleManagerTest, WhenGivenEmptyIDListThenFail) {
    std::vector<std::string> ids;
    std::string jobid = "jobid";

    ASSERT_FALSE(rm->SetRules(ids, jobid, 42));
}

TEST_F(RuleManagerTest, WhenGivenValidInputThenCallRuleSetter) {

    createSingleOstSetup();

    EXPECT_CALL(*mock_ruleSetter, SetRule(StrEq(ost_ips[0]),job_id,Not(StrEq("")),min_throughput_mbs)).Times(1).WillRepeatedly(testing::Return(true));
    ASSERT_TRUE(rm->SetRules(ost_ids, job_id, min_throughput_mbs));
}

TEST_F(RuleManagerTest, WhenTryToResetExistingRuleFail) {

    createSingleOstSetup();

    ON_CALL(*mock_ruleSetter, SetRule(_,_,_,_)).WillByDefault(testing::Return(true));
    EXPECT_CALL(*mock_ruleSetter, SetRule(StrEq(ost_ips[0]),job_id,Not(StrEq("")),min_throughput_mbs)).Times(1).WillRepeatedly(testing::Return(true));
    rm->SetRules(ost_ids, job_id, min_throughput_mbs);
    ASSERT_FALSE(rm->SetRules(ost_ids, job_id, min_throughput_mbs));
}

TEST_F(RuleManagerTest, WhenGivenTwoValidInputThenCallRuleSetterTwice) {

    createDoubleOstSetup();

    EXPECT_CALL(*mock_ruleSetter, SetRule(_,_,_,_)).Times(2).WillRepeatedly(testing::Return(true));
    ASSERT_TRUE(rm->SetRules(ost_ids, job_id, min_throughput_mbs));
}

TEST_F(RuleManagerTest, WhenTwoOstOnSameOSSThenUseDoubleMinMBs) {

    createTwoOstOnSameOSSSetup();

    EXPECT_CALL(*mock_ruleSetter, SetRule(_,_,_, 2*min_throughput_mbs)).WillOnce(testing::Return(true));
    ASSERT_TRUE(rm->SetRules(ost_ids, job_id, min_throughput_mbs));
}

TEST_F(RuleManagerTest, WhenCallRuleSetterFailsThenFail) {

    createDoubleOstSetup();

    ON_CALL(*mock_ruleSetter, SetRule(_,_,_,_)).WillByDefault(testing::Return(false));
    EXPECT_FALSE(rm->SetRules(ost_ids, job_id, min_throughput_mbs));
}

TEST_F(RuleManagerTest, WhenSecondCallRuleSetterFailsThenCleanUp) {

    createDoubleOstSetup();

    {
        InSequence s;
        EXPECT_CALL(*mock_ruleSetter, SetRule(_, _, _, _)).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_ruleSetter, SetRule(_, _, _, _)).Times(1).WillOnce(testing::Return(false));
    }
    EXPECT_CALL(*mock_ruleSetter, RemoveRule(Not(StrEq("")),Not(StrEq("")),StrEq(job_id))).Times(1).WillRepeatedly(testing::Return(true));

    rm->SetRules(ost_ids, job_id, min_throughput_mbs);
}

TEST_F(RuleManagerTest, WhenRemovingNonExistingJobThenFail) {

    std::string job_id = "jobid";
    ASSERT_FALSE(rm->RemoveRules(job_id));
}

TEST_F(RuleManagerTest, WhenRemovingValidJobThenCallRuleSetter) {

    createSingleOstSetup();

    ON_CALL(*mock_ruleSetter, SetRule(_,_,_,_)).WillByDefault(testing::Return(true));
    rm->SetRules(ost_ids, job_id, min_throughput_mbs);

    EXPECT_CALL(*mock_ruleSetter, RemoveRule(StrEq(ost_ips[0]),Not(StrEq("")),StrEq(job_id))).Times(1).WillRepeatedly(testing::Return(true));
    ASSERT_TRUE(rm->RemoveRules(job_id));
}

TEST_F(RuleManagerTest, WhenRemovingValidJobTwiceThenFail) {

    createSingleOstSetup();

    rm->SetRules(ost_ids, job_id, min_throughput_mbs);
    rm->RemoveRules(job_id);

    ASSERT_FALSE(rm->RemoveRules(job_id));
}

