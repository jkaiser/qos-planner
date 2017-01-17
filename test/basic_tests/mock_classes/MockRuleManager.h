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

#ifndef QOS_PLANNER_MOCKRULEMANAGER_H
#define QOS_PLANNER_MOCKRULEMANAGER_H

#include "gmock/gmock.h"

#include <RuleManager.h>

namespace common {
class MockRuleManager : public RuleManager {

public:
    MOCK_METHOD3(SetRules, bool(std::vector<std::string> &, const std::string &, uint32_t));
    MOCK_METHOD1(RemoveRules, bool(const std::string&));
};
}

#endif //QOS_PLANNER_MOCKRULEMANAGER_H
