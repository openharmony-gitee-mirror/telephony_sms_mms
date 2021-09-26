/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SMS_MMS_GTEST_H
#define SMS_MMS_GTEST_H

#include "gtest/gtest.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "i_sms_service_interface.h"
#include "sms_service_proxy.h"
#include "system_ability_definition.h"

#include "string_utils.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class SmsMmsGtest : public testing::Test {
public:
    static sptr<ISmsServiceInterface> g_telephonyService;
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static sptr<ISmsServiceInterface> GetProxy();
};
} // namespace Telephony
} // namespace OHOS
#endif
