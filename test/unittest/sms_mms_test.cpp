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

#include <memory>
#include <vector>

#include "common_event_support.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"

#include "gsm_sms_sender_test.h"
#include "short_message_test.h"
#include "sms_broadcast_subscriber.h"
#include "sms_cb_message_test.h"

using namespace OHOS;
using namespace OHOS::Telephony;

namespace {
static const ShortMessage *g_shortMessage = nullptr;
}
using TestStruct = struct FunStruct {
    std::string funName;
    std::function<void(void)> fun;
    FunStruct(std::string name, std::function<void(void)> function)
    {
        funName = name;
        fun = function;
    }
};

void TestRecev()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent("usual.event.SMS_RECEIVE_COMPLETED");
    matchingSkills.AddEvent("ohos.action.telephonySmsETWSCBReceiveFinished");
    matchingSkills.AddEvent("ohos.action.telephonySmsCBReceiveFinished");
    // make subcriber info
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    // make a subcriber object
    std::shared_ptr<SmsBroadcastSubscriber> subscriberTest =
        std::make_shared<SmsBroadcastSubscriber>(subscriberInfo);
    if (subscriberTest == nullptr) {
        return;
    }
    // subscribe a common event
    bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriberTest);
    std::cout << "subscribeResult is " << (subscribeResult ? "true" : "false") << std::endl;
}

static std::unique_ptr<std::vector<TestStruct>> GetFunArray(const sptr<ISmsServiceInterface> smsService)
{
    static GsmSmsSenderTest gsmSmsSenderTest;
    static ShortMessageTest shortMessageTest;
    static SmsCbMessageTest smsCbMessageTest;
    if (::g_shortMessage == nullptr) {
        ::g_shortMessage = shortMessageTest.TestCreateMessage();
    }
    std::unique_ptr<std::vector<TestStruct>> funArray = std::make_unique<std::vector<TestStruct>>();
    if (smsService == nullptr || ::g_shortMessage == nullptr || funArray == nullptr) {
        return funArray;
    }
    funArray->emplace_back(
        "TestGsmSendShortText", std::bind(&GsmSmsSenderTest::TestGsmSendShortText, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestGsmSendShortData", std::bind(&GsmSmsSenderTest::TestGsmSendShortData, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestGsmSendLongText", std::bind(&GsmSmsSenderTest::TestGsmSendLongText, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestGsmSendLongData", std::bind(&GsmSmsSenderTest::TestGsmSendLongData, gsmSmsSenderTest, smsService));
    funArray->emplace_back("TestCreateMessage", std::bind(&ShortMessageTest::TestCreateMessage, shortMessageTest));
    funArray->emplace_back("TestGetVisibleMessageBody",
        std::bind(&ShortMessageTest::TestGetVisibleMessageBody, shortMessageTest, *::g_shortMessage));
    funArray->emplace_back("TestShowShortMessage",
        std::bind(&ShortMessageTest::TestShowShortMessage, shortMessageTest, *::g_shortMessage));
    funArray->emplace_back(
        "TestSetSmscAddr", std::bind(&GsmSmsSenderTest::TestSetSmscAddr, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestGetSmscAddr", std::bind(&GsmSmsSenderTest::TestGetSmscAddr, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestAddSimMessage", std::bind(&GsmSmsSenderTest::TestAddSimMessage, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestDelSimMessage", std::bind(&GsmSmsSenderTest::TestDelSimMessage, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestUpdateSimMessage", std::bind(&GsmSmsSenderTest::TestUpdateSimMessage, gsmSmsSenderTest, smsService));
    funArray->emplace_back("TestGetAllSimMessages",
        std::bind(&GsmSmsSenderTest::TestGetAllSimMessages, gsmSmsSenderTest, smsService));
    funArray->emplace_back("TestCbMessage", std::bind(&SmsCbMessageTest::ProcessTest, smsCbMessageTest));
    funArray->emplace_back("TestEnableCBRangeConfig",
        std::bind(&GsmSmsSenderTest::TestEnableCBRangeConfig, gsmSmsSenderTest, smsService));
    funArray->emplace_back("TestDisableCBRangeConfig",
        std::bind(&GsmSmsSenderTest::TestDisableCBRangeConfig, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestEnableCBConfig", std::bind(&GsmSmsSenderTest::TestEnableCBConfig, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestDisableCBConfig", std::bind(&GsmSmsSenderTest::TestDisableCBConfig, gsmSmsSenderTest, smsService));
    funArray->emplace_back("TestSetDefaultSmsSlotId",
        std::bind(&GsmSmsSenderTest::TestSetDefaultSmsSlotId, gsmSmsSenderTest, smsService));
    funArray->emplace_back("TestGetDefaultSmsSlotId",
        std::bind(&GsmSmsSenderTest::TestGetDefaultSmsSlotId, gsmSmsSenderTest, smsService));
    return funArray;
}

int main()
{
    TestRecev();
    std::string hint = "[-1]:Exit\n";
    int8_t caseCount = 0;
    std::unique_ptr<std::vector<TestStruct>> testFunArray = nullptr;
    sptr<IRemoteObject> remote = nullptr;
    sptr<ISmsServiceInterface> smsService = nullptr;
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if ((systemAbilityMgr == nullptr) ||
        ((remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID)) == nullptr) ||
        ((smsService = iface_cast<ISmsServiceInterface>(remote))) == nullptr) {
        std::cout << "connect to service failed." << std::endl;
        return 0;
    }
    testFunArray = GetFunArray(smsService);
    if (testFunArray == nullptr || ((caseCount = testFunArray->size()) <= 0)) {
        std::cout << "Failed to get testFunArray data!" << std::endl;
        if (::g_shortMessage != nullptr) {
            delete ::g_shortMessage;
            ::g_shortMessage = nullptr;
        }
        return 0;
    }
    for (int index = 0; index < caseCount; ++index) {
        hint += "[" + std::to_string(index) + "]:" + (*testFunArray)[index].funName + "\n";
    }
    while (smsService != nullptr) {
        std::cout << hint << "Please input test case number!" << std::endl;
        std::string input;
        int caseNumber = 0;
        std::cin >> input;
        caseNumber = std::atoi(input.c_str());
        std::cin.clear();
        std::cin.ignore();
        std::cin.sync();
        if (caseNumber < -1 || caseNumber >= caseCount) {
            continue;
        }
        if (caseNumber == -1) {
            break;
        }
        if ((*testFunArray)[caseNumber].fun != nullptr) {
            (*testFunArray)[caseNumber].fun();
        }
    }
    if (::g_shortMessage != nullptr) {
        delete ::g_shortMessage;
        ::g_shortMessage = nullptr;
    }
    return 0;
}
