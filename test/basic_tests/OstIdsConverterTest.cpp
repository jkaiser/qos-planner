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

#include <OstIdsConverter.h>
#include <Lustre.h>

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;

class OstIdsConverterTest : public ::testing::Test {

protected:

    std::unique_ptr<common::OstIdsConverter> converter_;

    virtual void SetUp() {
        mock_lustre = new common::MockLustre();
        mocked_ll.reset(mock_lustre);

        converter_.reset(new common::OstIdsConverter(mocked_ll));
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

    std::string returned_uuid = converter_->ToUUID(id);
    ASSERT_STREQ(expected_uuid.c_str(), returned_uuid.c_str());
}


TEST_F(OstIdsConverterTest, WhenGivenInValidIdThenReturnEmptyString) {
    std::string id = "foo";

    auto ost_list = std::make_shared<std::vector<common::Lustre::getOstsResults>>();

    ON_CALL(*mocked_ll, GetOstList(_, _)).WillByDefault(DoAll(testing::SetArgReferee<1>(ost_list), Return(true)));
    std::string returned_uuid = converter_->ToUUID(id);

    ASSERT_STREQ("", returned_uuid.c_str());
}

TEST_F(OstIdsConverterTest, WhenLustreCallFailsThenDontCrash) {
    std::string id = "foo";

    auto ost_list = std::make_shared<std::vector<common::Lustre::getOstsResults>>();

    ON_CALL(*mocked_ll, GetOstList(_, _)).WillByDefault(DoAll(testing::SetArgReferee<1>(ost_list), Return(false)));
    std::string returned_uuid = converter_->ToUUID(id);
}