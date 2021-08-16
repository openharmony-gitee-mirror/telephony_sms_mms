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
#include "sms_service_proxy.h"
#include "system_ability_definition.h"

#include "i_sms_service_interface.h"

namespace OHOS {
namespace Telephony {
namespace {
sptr<ISmsServiceInterface> g_telephonyService = nullptr;
} // namespace

using namespace testing::ext;
class SmsMmsGtest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static bool CloseCellBroadcast();
    static bool OpenCellBroadcast();
    static sptr<ISmsServiceInterface> GetProxy();
};

void SmsMmsGtest::TearDownTestCase() {}

void SmsMmsGtest::SetUp() {}

void SmsMmsGtest::TearDown() {}

void SmsMmsGtest::SetUpTestCase()
{
    g_telephonyService = GetProxy();
    if (g_telephonyService == nullptr) {
        return;
    }
}

sptr<ISmsServiceInterface> SmsMmsGtest::GetProxy()
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        return nullptr;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    if (remote) {
        sptr<ISmsServiceInterface> smsService = iface_cast<ISmsServiceInterface>(remote);
        return smsService;
    }
    return nullptr;
}

bool SmsMmsGtest::CloseCellBroadcast()
{
    bool result = false;
    if (g_telephonyService == nullptr) {
        return result;
    }
    int32_t slotId = 1;
    bool enable = false;
    uint32_t fromMsgId = 10;
    uint32_t toMsgId = 0;
    uint8_t netType = 1;
    return g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
}

bool SmsMmsGtest::OpenCellBroadcast()
{
    bool result = false;
    if (g_telephonyService == nullptr) {
        return result;
    }
    int32_t slotId = -1;
    bool enable = true;
    uint32_t fromMsgId = 0;
    uint32_t toMsgId = 10;
    uint8_t netType = 1;
    return g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
}

HWTEST_F(SmsMmsGtest, GetProxy, TestSize.Level1)
{
    g_telephonyService = SmsMmsGtest::GetProxy();
    ASSERT_FALSE(g_telephonyService == nullptr);
}

HWTEST_F(SmsMmsGtest, CloseCellBroadcast, TestSize.Level1)
{
    auto result = SmsMmsGtest::CloseCellBroadcast();
    ASSERT_FALSE(result);
}

HWTEST_F(SmsMmsGtest, OpenCellBroadcast, TestSize.Level1)
{
    auto result = SmsMmsGtest::OpenCellBroadcast();
    ASSERT_FALSE(result);
}
} // namespace Telephony
} // namespace OHOS
#endif