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
    std::unique_ptr<common::RuleManager> rm_;

    common::MockRuleSetter *mock_ruleSetter_;
    common::MockOstIpsCache *mock_ost_ip_cache_;
    common::MockOstIdsConverter *mock_id_converter_;
    std::shared_ptr<common::MockRuleSetter> mocked_ruleSetter_;
    std::shared_ptr<common::MockOstIpsCache> mocked_ost_ip_cache_;
    std::shared_ptr<common::MockOstIdsConverter> mocked_id_converter_;

    std::vector<std::string> ost_ids_;
    std::vector<std::string> ost_uuids_;
    std::vector<std::string> ost_ips_;
    std::string job_id_;
    uint32_t min_throughput_mbs_;


protected:
    virtual void TearDown() {
        ost_ids_.clear();
        ost_uuids_.clear();
        job_id_ = "";
        min_throughput_mbs_ = 0;
    }

    virtual void SetUp() {
        mock_ruleSetter_ = new common::MockRuleSetter();
        mock_ost_ip_cache_ = new common::MockOstIpsCache();
        mock_id_converter_ = new common::MockOstIdsConverter();

        mocked_ruleSetter_.reset(mock_ruleSetter_);
        mocked_ost_ip_cache_.reset(mock_ost_ip_cache_);
        mocked_id_converter_.reset(mock_id_converter_);
        rm_.reset(new common::RuleManager(mocked_ruleSetter_, mocked_ost_ip_cache_, mocked_id_converter_));
    }

    void createSingleOstSetup() {
        ost_ids_ = {"ost1"};
        ost_uuids_ = {"ost1_uuid"};
        ost_ips_ = {"127.0.0.1"};
        job_id_ = "jobid";
        min_throughput_mbs_ = 42;

        ON_CALL(*mocked_id_converter_, ToUUID(ost_ids_[0])).WillByDefault(testing::Return(ost_uuids_[0]));;
        ON_CALL(*mocked_ost_ip_cache_, GetIp(ost_uuids_[0])).WillByDefault(testing::Return(ost_ips_[0]));
    }

    void createDoubleOstSetup() {
        ost_ids_ = {"ost1", "ost2"};
        ost_uuids_ = {"ost1_uuid", "ost2_uuid"};
        ost_ips_ = {"127.0.0.1", "127.0.0.2"};
        job_id_ = "jobid";
        min_throughput_mbs_ = 42;

        ON_CALL(*mocked_id_converter_, ToUUID(ost_ids_[0])).WillByDefault(testing::Return(ost_uuids_[0]));
        ON_CALL(*mocked_id_converter_, ToUUID(ost_ids_[1])).WillByDefault(testing::Return(ost_uuids_[1]));
        ON_CALL(*mocked_ost_ip_cache_, GetIp(ost_uuids_[0])).WillByDefault(testing::Return(ost_ips_[0]));
        ON_CALL(*mocked_ost_ip_cache_, GetIp(ost_uuids_[1])).WillByDefault(testing::Return(ost_ips_[1]));
    }

    void createTwoOstOnSameOSSSetup() {
        ost_ids_ = {"ost1", "ost2"};
        ost_uuids_ = {"ost1_uuid", "ost2_uuid"};
        ost_ips_ = {"127.0.0.1", "127.0.0.1"};
        job_id_ = "jobid";
        min_throughput_mbs_ = 42;

        ON_CALL(*mocked_id_converter_, ToUUID(ost_ids_[0])).WillByDefault(testing::Return(ost_uuids_[0]));
        ON_CALL(*mocked_id_converter_, ToUUID(ost_ids_[1])).WillByDefault(testing::Return(ost_uuids_[1]));
        ON_CALL(*mocked_ost_ip_cache_, GetIp(ost_uuids_[0])).WillByDefault(testing::Return(ost_ips_[0]));
        ON_CALL(*mocked_ost_ip_cache_, GetIp(ost_uuids_[1])).WillByDefault(testing::Return(ost_ips_[1]));
    }
};

TEST_F(RuleManagerTest, WhenGivenEmptyIDListThenFail) {
    std::vector<std::string> ids;
    std::string jobid = "jobid";

    ASSERT_FALSE(rm_->SetRules(ids, jobid, 42));
}

TEST_F(RuleManagerTest, WhenGivenValidInputThenCallRuleSetter) {

    createSingleOstSetup();

    EXPECT_CALL(*mock_ruleSetter_, SetRule(StrEq(ost_ips_[0]),job_id_,Not(StrEq("")),min_throughput_mbs_)).Times(1).WillRepeatedly(testing::Return(true));
    ASSERT_TRUE(rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_));
}

TEST_F(RuleManagerTest, WhenTryToResetExistingRuleFail) {

    createSingleOstSetup();

    ON_CALL(*mock_ruleSetter_, SetRule(_,_,_,_)).WillByDefault(testing::Return(true));
    EXPECT_CALL(*mock_ruleSetter_, SetRule(StrEq(ost_ips_[0]),job_id_,Not(StrEq("")),min_throughput_mbs_)).Times(1).WillRepeatedly(testing::Return(true));
    rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_);
    ASSERT_FALSE(rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_));
}

TEST_F(RuleManagerTest, WhenGivenTwoValidInputThenCallRuleSetterTwice) {

    createDoubleOstSetup();

    EXPECT_CALL(*mock_ruleSetter_, SetRule(_,_,_,_)).Times(2).WillRepeatedly(testing::Return(true));
    ASSERT_TRUE(rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_));
}

TEST_F(RuleManagerTest, WhenTwoOstOnSameOSSThenUseDoubleMinMBs) {

    createTwoOstOnSameOSSSetup();

    EXPECT_CALL(*mock_ruleSetter_, SetRule(_,_,_, 2*min_throughput_mbs_)).WillOnce(testing::Return(true));
    ASSERT_TRUE(rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_));
}

TEST_F(RuleManagerTest, WhenCallRuleSetterFailsThenFail) {

    createDoubleOstSetup();

    ON_CALL(*mock_ruleSetter_, SetRule(_,_,_,_)).WillByDefault(testing::Return(false));
    EXPECT_FALSE(rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_));
}

TEST_F(RuleManagerTest, WhenSecondCallRuleSetterFailsThenCleanUp) {

    createDoubleOstSetup();

    {
        InSequence s;
        EXPECT_CALL(*mock_ruleSetter_, SetRule(_, _, _, _)).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_ruleSetter_, SetRule(_, _, _, _)).Times(1).WillOnce(testing::Return(false));
    }
    EXPECT_CALL(*mock_ruleSetter_, RemoveRule(Not(StrEq("")),Not(StrEq("")),StrEq(job_id_))).Times(1).WillRepeatedly(testing::Return(true));

    rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_);
}

TEST_F(RuleManagerTest, WhenRemovingNonExistingJobThenFail) {

    std::string job_id = "jobid";
    ASSERT_FALSE(rm_->RemoveRules(job_id));
}

TEST_F(RuleManagerTest, WhenRemovingValidJobThenCallRuleSetter) {

    createSingleOstSetup();

    ON_CALL(*mock_ruleSetter_, SetRule(_,_,_,_)).WillByDefault(testing::Return(true));
    rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_);

    EXPECT_CALL(*mock_ruleSetter_, RemoveRule(StrEq(ost_ips_[0]),Not(StrEq("")),StrEq(job_id_))).Times(1).WillRepeatedly(testing::Return(true));
    ASSERT_TRUE(rm_->RemoveRules(job_id_));
}

TEST_F(RuleManagerTest, WhenRemovingValidJobTwiceThenFail) {

    createSingleOstSetup();

    rm_->SetRules(ost_ids_, job_id_, min_throughput_mbs_);
    rm_->RemoveRules(job_id_);

    ASSERT_FALSE(rm_->RemoveRules(job_id_));
}

