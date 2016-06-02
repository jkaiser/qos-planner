//
// Created by jkaiser on 6/2/16.
//


#include "gtest/gtest.h"
#include <string>

#include <util.h>

TEST(UtilsTest, Exec) {

    std::shared_ptr<std::string> out(new std::string());
    ASSERT_PRED2(common::exec, "ls .", out);
}
