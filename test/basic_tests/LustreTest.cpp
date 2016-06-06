//
// Created by jkaiser on 6/1/16.
//

#include "gtest/gtest.h"


#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>

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
