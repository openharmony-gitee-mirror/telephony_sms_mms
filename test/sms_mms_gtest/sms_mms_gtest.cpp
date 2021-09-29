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

#include "sms_mms_gtest.h"

#include "string_utils.h"

namespace OHOS {
namespace Telephony {
sptr<ISmsServiceInterface> SmsMmsGtest::g_telephonyService = nullptr;

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

HWTEST_F(SmsMmsGtest, GetProxy_0001, TestSize.Level1)
{
    g_telephonyService = SmsMmsGtest::GetProxy();
    EXPECT_FALSE(g_telephonyService == nullptr);
}

HWTEST_F(SmsMmsGtest, OpenCellBroadcast_0001, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = -1;
        bool enable = true;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 10;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, OpenCellBroadcast_0002, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = true;
        uint32_t fromMsgId = 20;
        uint32_t toMsgId = 10;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, OpenCellBroadcast_0003, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = true;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 10;
        uint8_t netType = 3;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, OpenCellBroadcast_0004, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = true;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 10;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, OpenCellBroadcast_0005, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = true;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 1000;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, OpenCellBroadcast_0006, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = true;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 0;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, CloseCellBroadcast_0001, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = -1;
        bool enable = false;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 10;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, CloseCellBroadcast_0002, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = false;
        uint32_t fromMsgId = 20;
        uint32_t toMsgId = 10;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, CloseCellBroadcast_0003, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = false;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 10;
        uint8_t netType = 3;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, CloseCellBroadcast_0004, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = false;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 10;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, CloseCellBroadcast_0005, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = false;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 1000;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, CloseCellBroadcast_0006, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        bool enable = false;
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 0;
        uint8_t netType = 1;
        result = g_telephonyService->SetCBConfig(slotId, enable, fromMsgId, toMsgId, netType);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, SetDefaultSmsSlotId_0001, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        result = g_telephonyService->SetDefaultSmsSlotId(slotId);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, SetDefaultSmsSlotId_0002, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 1;
        result = g_telephonyService->SetDefaultSmsSlotId(slotId);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, SetDefaultSmsSlotId_0003, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 2;
        result = g_telephonyService->SetDefaultSmsSlotId(slotId);
        EXPECT_TRUE(result);
    }
}

HWTEST_F(SmsMmsGtest, SetDefaultSmsSlotId_0004, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 3;
        result = g_telephonyService->SetDefaultSmsSlotId(slotId);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, SetDefaultSmsSlotId_0005, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 4;
        result = g_telephonyService->SetDefaultSmsSlotId(slotId);
        EXPECT_FALSE(result);
    }
}

HWTEST_F(SmsMmsGtest, GetDefaultSmsSlotId_0001, TestSize.Level1)
{
    const int32_t error = -1;
    if (g_telephonyService != nullptr) {
        int32_t slotId = 0;
        slotId = g_telephonyService->GetDefaultSmsSlotId();
        EXPECT_TRUE(slotId != error);
    }
}

HWTEST_F(SmsMmsGtest, SetSmscAddr_0001, TestSize.Level1)
{
    bool result = false;
    if (g_telephonyService != nullptr) {
        int32_t slotId = -1;
        std::string scAddr("13333333333");
        result = g_telephonyService->SetSmscAddr(slotId, StringUtils::ToUtf16(scAddr));
        EXPECT_FALSE(result);
    }
}
} // namespace Telephony
} // namespace OHOS