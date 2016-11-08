//
// Created by Jürgen Kaiser on 04.11.16.
//


#include "gtest/gtest.h"
#include "mock_classes/MockLustre.h"

#include <OstIdsConverter.h>
#include <Lustre.h>

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;

class OstIdsConverterTest : public ::testing::Test {

protected:

    std::unique_ptr<common::OstIdsConverter> converter;

    virtual void SetUp() {
        mock_lustre = new common::MockLustre();
        mocked_ll.reset(mock_lustre);

        converter.reset(new common::OstIdsConverter(mocked_ll));
    }

    common::MockLustre *mock_lustre;
    std::shared_ptr<common::MockLustre> mocked_ll;

    std::shared_ptr<std::vector<common::Lustre::getOstsResults>>
    CreateSingleElementOutputList(const std::string &id, const std::string &expected_uuid) const {
        auto list = std::make_shared<std::vector<common::Lustre::getOstsResults>>();
        list->push_back(common::Lustre::getOstsResults{id, expected_uuid, ""});
        return list;
    }
};

TEST_F(OstIdsConverterTest, WhenGivenValidIdThenReturnUUID) {

    std::string id = "foo";
    std::string expected_uuid = "foo_uuid";

    auto ost_list = CreateSingleElementOutputList(id, expected_uuid);

    ON_CALL(*mocked_ll, GetOstList(_, _)).WillByDefault(DoAll(testing::SetArgReferee<1>(ost_list), Return(true)));

    std::string returned_uuid = converter->ToUUID(id);
    ASSERT_STREQ(expected_uuid.c_str(), returned_uuid.c_str());
}


TEST_F(OstIdsConverterTest, WhenGivenInValidIdThenReturnEmptyString) {
    std::string id = "foo";

    auto ost_list = std::make_shared<std::vector<common::Lustre::getOstsResults>>();

    ON_CALL(*mocked_ll, GetOstList(_, _)).WillByDefault(DoAll(testing::SetArgReferee<1>(ost_list), Return(true)));
    std::string returned_uuid = converter->ToUUID(id);

    ASSERT_STREQ("", returned_uuid.c_str());
}

TEST_F(OstIdsConverterTest, WhenLustreCallFailsThenDontCrash) {
    std::string id = "foo";

    auto ost_list = std::make_shared<std::vector<common::Lustre::getOstsResults>>();

    ON_CALL(*mocked_ll, GetOstList(_, _)).WillByDefault(DoAll(testing::SetArgReferee<1>(ost_list), Return(false)));
    std::string returned_uuid = converter->ToUUID(id);
}