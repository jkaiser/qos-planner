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


#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <tuple>

#include <Lustre.h>


struct parseOstsTestset {
    std::string to_parse;
    std::map<std::string, int> to_find;
};

class ParseOstsTest : public ::testing::TestWithParam<parseOstsTestset> {
};

TEST_P(ParseOstsTest, ParseOsts) {
    auto pt = GetParam();

    std::shared_ptr<std::vector<std::string>> results(new std::vector<std::string>());
    common::Lustre::ParseOstsFromGetStripe(pt.to_parse, results);

        for (auto &it : *results) {
            auto mit = pt.to_find.find(it);
            ASSERT_NE(mit, pt.to_find.end()) << "The element " << it << " was found but not expected.";
            // it must have been found, otherwise the test would have failed
            if (mit->second == 1) {
                pt.to_find.erase(mit);
            } else {
                mit->second--;
            }
        }

        ASSERT_EQ(pt.to_find.size(), 0) << "All elements must be found.";
    }

INSTANTIATE_TEST_CASE_P(OstParsing, ParseOstsTest, ::testing::Values(
        parseOstsTestset{"/mnt/lustret4/test/tacke/stripe4/file1\nlmm_stripe_count:   4\nlmm_stripe_size:    1048576\nlmm_pattern:        1\nlmm_layout_gen:     0\nlmm_stripe_offset:  3\n\tobdidx\t\t objid\t\t objid\t\t group\n\t     3\t             2\t          0x2\t             0\n\t     2\t             2\t          0x2\t             0\n\t     1\t             3\t          0x3\t             0\n\t     0\t             2\t          0x2\t             0\n\n",
        {{"3", 1}, {"2", 1}, {"1", 1}, {"0", 1}}},
        parseOstsTestset{"schlarbm\nlmm_stripe_count:   1\nlmm_stripe_size:    1048576\nlmm_pattern:        1\nlmm_layout_gen:     0\nlmm_stripe_offset:  0\n\tobdidx\t\t objid\t\t objid\t\t group\n\t     0\t      42258134\t    0x284ced6\t             0\n\nschlarbm\nlmm_stripe_count:   1\nlmm_stripe_size:    1048576\nlmm_pattern:        1\nlmm_layout_gen:     0\nlmm_stripe_offset:  0\n\tobdidx\t\t objid\t\t objid\t\t group\n\t     0\t      42258134\t    0x284ced6\t             0\n\n",
                         {{"0", 2}}}
));

struct parseOstsFromLfsOstsTestset {
    std::string to_parse;
    std::vector<std::tuple<std::string, std::string, std::string>> to_find;
};


class ParseOstsFromLfsOstsTest : public ::testing::TestWithParam<parseOstsFromLfsOstsTestset> {
};

TEST_P(ParseOstsFromLfsOstsTest, ParseOsts) {
    auto pt = GetParam();

    std::shared_ptr<std::vector<common::Lustre::getOstsResults>> results (new(std::vector<common::Lustre::getOstsResults>));
    common::Lustre::ParseOstsFromLfsOsts(pt.to_parse, results);

    for (int i = 0; i < results->size(); i++) {
        ASSERT_STREQ(std::get<0>(pt.to_find[i]).c_str(), results->at(i).number.c_str()) << "Found wrong ost id";
        ASSERT_STREQ(std::get<1>(pt.to_find[i]).c_str(), results->at(i).uuid.c_str()) << "Found wrong ost uuid";
        ASSERT_STREQ(std::get<2>(pt.to_find[i]).c_str(), results->at(i).status.c_str()) << "Found wrong ost state";
    }

    ASSERT_EQ(pt.to_find.size(), results->size()) << "Some results are missing";
}

INSTANTIATE_TEST_CASE_P(GetOstParsing, ParseOstsFromLfsOstsTest, ::testing::Values(
        parseOstsFromLfsOstsTestset{"OBDS::\n0: lustret4-OST0000_UUID ACTIVE\n",
                                    {std::make_tuple("0", "lustret4-OST0000_UUID", "ACTIVE")}},
        parseOstsFromLfsOstsTestset{"OBDS::\n0: lustret4-OST0000_UUID ACTIVE\n1: lustret4-OST0001_UUID ACTIVE\n2: lustret4-OST0002_UUID ACTIVE\n3: lustret4-OST0003_UUID ACTIVE\n",
        {std::make_tuple("0", "lustret4-OST0000_UUID", "ACTIVE"),
         std::make_tuple("1", "lustret4-OST0001_UUID", "ACTIVE"),
         std::make_tuple("2", "lustret4-OST0002_UUID", "ACTIVE"),
         std::make_tuple("3", "lustret4-OST0003_UUID", "ACTIVE")}}
));


struct parseOstIpFromGetParamOscTestset {
    std::string input;
    std::string shortened_ost_uuid;
    std::string expected_output;
};

class ParseOstIpFromGetParamTest : public ::testing::TestWithParam<parseOstIpFromGetParamOscTestset> {};

TEST_P(ParseOstIpFromGetParamTest, Convert) {
    auto pt = GetParam();

    std::string out;
    common::Lustre::ParseOstIpFromGetParamOsc(pt.input, pt.shortened_ost_uuid, out);
    ASSERT_STREQ(pt.expected_output.c_str(), out.c_str()) << "should parse '" + pt.expected_output + "' out of '" + pt.input + "'";
}

INSTANTIATE_TEST_CASE_P(OstIDToUUIDConverting, ParseOstIpFromGetParamTest, ::testing::Values(
        parseOstIpFromGetParamOscTestset{"osc.toto-OST0000-osc-ffff88003b90f800.ost_conn_uuid=192.168.122.241@tcp\n", "toto-OST0000", "192.168.122.241"},
        parseOstIpFromGetParamOscTestset{"osc.toto-OST0000-osc-ffff88003b90f800.ost_conn_uuid=192.168.122.241@tcp", "toto-OST0000", "192.168.122.241"},
        parseOstIpFromGetParamOscTestset{"", "toto-OST0000", ""},
        parseOstIpFromGetParamOscTestset{"osc.toto-OST0000-osc-ffff88003b90f800.ost_conn_uuid=192.168.122.241@tcp\nosc.toto-OST0000-osc-ffff88003b90f800.ost_conn_uuid=192.168.122.242@tcp\n", "toto-OST0000", "192.168.122.241"}
));