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
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "short_message_test.h"
#include "sms_broadcast_subscriber.h"
#include "gsm_sms_sender_test.h"
using namespace OHOS;
using namespace OHOS::SMS;
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
    matchingSkills.AddEvent(SMS_ACTION_RECEIVE);
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

std::unique_ptr<std::vector<TestStruct>> GetFunArray(const sptr<SMS::ISmsServiceInterface> smsService)
{
    static GsmSmsSenderTest gsmSmsSenderTest;
    static ShortMessageTest shortMessageTest;
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
    funArray->emplace_back(
        "TestMissingParameter", std::bind(&GsmSmsSenderTest::TestMissingParameter, gsmSmsSenderTest, smsService));
    funArray->emplace_back(
        "TestErrorParameter", std::bind(&GsmSmsSenderTest::TestErrorParameter, gsmSmsSenderTest, smsService));
    funArray->emplace_back("TestCreateMessage", std::bind(&ShortMessageTest::TestCreateMessage, shortMessageTest));
    funArray->emplace_back("TestGetVisbleMessageBody",
        std::bind(&ShortMessageTest::TestGetVisbleMessageBody, shortMessageTest, *::g_shortMessage));
    funArray->emplace_back("TestShowShortMessage",
        std::bind(&ShortMessageTest::TestShowShortMessage, shortMessageTest, *::g_shortMessage));
    return funArray;
}

int main()
{
    TestRecev();
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        printf("SmsServiceTest Get ISystemAbilityManager failed.\n");
        return 0;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    if (remote) {
        sptr<SMS::ISmsServiceInterface> smsService = iface_cast<SMS::ISmsServiceInterface>(remote);
        if (smsService != nullptr) {
            std::unique_ptr<std::vector<TestStruct>> testFunArray = GetFunArray(smsService);
            int8_t caseCount = 0;
            if (testFunArray == nullptr || ((caseCount = testFunArray->size()) <= 0)) {
                std::cout << "Failed to get testFunArray data!\n";
                return 0;
            }
            std::string hint = "[-1]:Exit\n";
            for (int index = 0; index < caseCount; ++index) {
                hint += "[" + std::to_string(index) + "]:" + (*testFunArray)[index].funName + "\n";
            }
            while (smsService != nullptr) {
                std::cout << hint;
                std::cout << "Please input test case number!\n";
                std::string input;
                int caseNumber = 0;
                std::cin >> input;
                caseNumber = std::atoi(input.c_str());
                std::cin.clear();
                std::cin.ignore();
                std::cin.sync();
                if (caseNumber < -1 || caseNumber >= caseCount) {
                    std::cout << "test case is not exist!\n";
                    continue;
                }
                if (caseNumber == -1) {
                    break;
                }
                std::cout << "Enter the " << (*testFunArray)[caseNumber].funName << " case!\n";
                (*testFunArray)[caseNumber].fun();
            }
        } else if (smsService == nullptr) {
            printf("get smsService fail!\n");
        }
    }
    if (::g_shortMessage != nullptr) {
        delete ::g_shortMessage;
        ::g_shortMessage = nullptr;
    }
    return 0;
}