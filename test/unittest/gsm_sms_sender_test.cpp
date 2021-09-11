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

#include "gsm_sms_sender_test.h"

#include <iostream>

#include "core_manager.h"
#include "delivery_short_message_callback_stub.h"
#include "send_short_message_callback_stub.h"
#include "short_message.h"
#include "string_utils.h"

namespace OHOS {
namespace Telephony {
void GsmSmsSenderTest::TestGsmSendShortData(const sptr<ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = CoreManager::DEFAULT_SLOT_ID;
    std::string dest;
    std::cout << "Please enter the receiver's telephone number\n" << std::endl;
    std::cin >> dest;
    std::string sca("");
    OHOS::sptr<SendShortMessageCallbackStub> sendCallBackPtr(new SendShortMessageCallbackStub());
    OHOS::sptr<DeliveryShortMessageCallbackStub> deliveryCallBackPtr(new DeliveryShortMessageCallbackStub());
    uint16_t port = 100;
    uint8_t data[] = "hello world";
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca), port, data,
        sizeof(data) / sizeof(data[0]), sendCallBackPtr, deliveryCallBackPtr);
    std::cout << "TestGsmSendShortData" << std::endl;
}

void GsmSmsSenderTest::TestGsmSendLongData(const sptr<ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = CoreManager::DEFAULT_SLOT_ID;
    std::string dest;
    std::cout << "Please enter the receiver's telephone number\n";
    std::cin >> dest;
    std::string sca("");
    OHOS::sptr<SendShortMessageCallbackStub> sendCallBackPtr(new SendShortMessageCallbackStub());
    OHOS::sptr<DeliveryShortMessageCallbackStub> deliveryCallBackPtr(new DeliveryShortMessageCallbackStub());
    uint8_t data[] =
        R"(This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!)";
    uint16_t port = 100;

    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca), port, data,
        sizeof(data) / sizeof(data[0]), sendCallBackPtr, deliveryCallBackPtr);
    std::cout << "TestGsmSendLongData" << std::endl;
}

void GsmSmsSenderTest::TestGsmSendShortText(const sptr<ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = CoreManager::DEFAULT_SLOT_ID;
    std::string dest;
    std::string text;
    std::cout << "Please enter the receiver's telephone number\n";
    getline(std::cin, dest);
    std::cout << "Please enter text\n";
    getline(std::cin, text);
    std::string sca("");
    OHOS::sptr<SendShortMessageCallbackStub> sendCallBackPtr(new SendShortMessageCallbackStub());
    OHOS::sptr<DeliveryShortMessageCallbackStub> deliveryCallBackPtr(new DeliveryShortMessageCallbackStub());
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca),
        StringUtils::ToUtf16(text), sendCallBackPtr, deliveryCallBackPtr);
    std::cout << "TestGsmSendShortText" << std::endl;
}

void GsmSmsSenderTest::TestGsmSendLongText(const sptr<ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = CoreManager::DEFAULT_SLOT_ID;
    std::string dest;
    std::cout << "Please enter the receiver's telephone number\n";
    std::cin >> dest;
    std::string sca("");
    std::string text(
        R"(This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!
        This is the test data!This is the test data!This is the test data!)");
    OHOS::sptr<SendShortMessageCallbackStub> sendCallBackPtr(new SendShortMessageCallbackStub());
    OHOS::sptr<DeliveryShortMessageCallbackStub> deliveryCallBackPtr(new DeliveryShortMessageCallbackStub());
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca),
        StringUtils::ToUtf16(text), sendCallBackPtr, deliveryCallBackPtr);
    std::cout << "TestGsmSendLongText" << std::endl;
}

void GsmSmsSenderTest::TestSetSmscAddr(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = CoreManager::DEFAULT_SLOT_ID;
    std::string sca;
    std::cout << "Please enter smsc" << std::endl;
    std::cin >> sca;
    result = smsService->SetSmscAddr(cardId, StringUtils::ToUtf16(sca));
    if (result) {
        std::cout << "TestSetSmscAddr OK!" << std::endl;
    } else {
        std::cout << "TestSetSmscAddr failed!" << std::endl;
    }
}

void GsmSmsSenderTest::TestGetSmscAddr(const sptr<ISmsServiceInterface> &smsService) const
{
    std::string result;
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = CoreManager::DEFAULT_SLOT_ID;
    result = StringUtils::ToUtf8(smsService->GetSmscAddr(cardId));
    std::cout << "TestGetSmscAddr:" << result << std::endl;
}

void GsmSmsSenderTest::TestAddSimMessage(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::string smsc;
    std::cout << "Please enter smsc" << std::endl;
    std::cin >> smsc;
    std::string pdu;
    std::cout << "Please enter pdu" << std::endl;
    std::cin >> pdu;
    std::string input;
    std::cout << "Please enter status" << std::endl;
    std::cin >> input;
    uint32_t status = std::atoi(input.c_str());
    result = smsService->AddSimMessage(slotId, StringUtils::ToUtf16(smsc), StringUtils::ToUtf16(pdu),
        static_cast<ISmsServiceInterface::SimMessageStatus>(status));
    if (result) {
        std::cout << "TestAddSimMessage OK!" << std::endl;
    } else {
        std::cout << "TestAddSimMessage failed!" << std::endl;
    }
}

void GsmSmsSenderTest::TestDelSimMessage(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::string input;
    std::cout << "Please enter msgIndex" << std::endl;
    std::cin >> input;
    uint32_t msgIndex = std::atoi(input.c_str());
    result = smsService->DelSimMessage(slotId, msgIndex);
    if (result) {
        std::cout << "TestDelSimMessage OK!" << std::endl;
    } else {
        std::cout << "TestDelSimMessage failed!" << std::endl;
    }
}

void GsmSmsSenderTest::TestUpdateSimMessage(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::string input;
    std::cout << "Please enter msgIndex" << std::endl;
    std::cin >> input;
    uint32_t msgIndex = std::atoi(input.c_str());
    std::string pdu;
    std::cout << "Please enter pdu" << std::endl;
    std::cin >> pdu;
    std::string smsc;
    std::cout << "Please enter smsc" << std::endl;
    std::cin >> smsc;
    std::u16string pduData = StringUtils::ToUtf16(pdu);
    std::u16string smscData = StringUtils::ToUtf16(smsc);
    result = smsService->UpdateSimMessage(
        slotId, msgIndex, ISmsServiceInterface::SimMessageStatus::SIM_MESSAGE_STATUS_READ, pduData, smscData);
    if (result) {
        std::cout << "TestUpdateSimMessage OK!" << std::endl;
    } else {
        std::cout << "TestUpdateSimMessage failed!" << std::endl;
    }
}

void GsmSmsSenderTest::TestGetAllSimMessages(const sptr<ISmsServiceInterface> &smsService) const
{
    std::vector<ShortMessage> result;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    int32_t index = 0;
    result = smsService->GetAllSimMessages(slotId);
    std::cout << "TestUpdateSimMessage Begin:" << std::endl;
    for (auto &item : result) {
        if (item.GetIccMessageStatus() != ShortMessage::SMS_SIM_MESSAGE_STATUS_FREE &&
            item.GetIccMessageStatus() != ShortMessage::SMS_SIM_MESSAGE_STATUS_UNKNOWN) {
            std::cout << "[" << index << "] " << StringUtils::StringToHex(item.GetPdu()) << std::endl;
            std::cout << "status:" << item.GetIccMessageStatus()
                      << " message:" << StringUtils::ToUtf8(item.GetVisibleMessageBody()) << std::endl;
        }
        index++;
    }
    std::cout << "TestUpdateSimMessage End!" << std::endl;
}

void GsmSmsSenderTest::TestEnableCBRangeConfig(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    bool enable = true;
    uint32_t startMessageId = 0;
    uint32_t endMessageId = 0;
    uint8_t ranType = 1;
    std::string input;
    std::cout << "Please enter startMessageId" << std::endl;
    std::cin >> input;
    startMessageId = std::atoi(input.c_str());
    std::cout << "Please enter endMessageId" << std::endl;
    std::cin >> input;
    endMessageId = std::atoi(input.c_str());
    result = smsService->SetCBConfig(slotId, enable, startMessageId, endMessageId, ranType);
    std::string ret = result ? "true" : "false";
    std::cout << "TestEnableCBRangeConfig:" << ret << std::endl;
}

void GsmSmsSenderTest::TestDisableCBRangeConfig(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    bool enable = false;
    uint32_t startMessageId = 0;
    uint32_t endMessageId = 0;
    uint8_t ranType = 1;
    std::string input;
    std::cout << "Please enter startMessageId" << std::endl;
    std::cin >> input;
    startMessageId = std::atoi(input.c_str());
    std::cout << "Please enter endMessageId" << std::endl;
    std::cin >> input;
    endMessageId = std::atoi(input.c_str());
    result = smsService->SetCBConfig(slotId, enable, startMessageId, endMessageId, ranType);
    std::string ret = result ? "true" : "false";
    std::cout << "TestDisableCBRangeConfig:" << ret << std::endl;
}

void GsmSmsSenderTest::TestEnableCBConfig(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    bool enable = true;
    uint32_t identifier = 0;
    uint8_t ranType = 1;
    std::string input;
    std::cout << "Please enter identifier" << std::endl;
    std::cin >> input;
    identifier = std::atoi(input.c_str());
    result = smsService->SetCBConfig(slotId, enable, identifier, identifier, ranType);
    std::string ret = result ? "true" : "false";
    std::cout << "TestEnableCBConfig:" << ret << std::endl;
}

void GsmSmsSenderTest::TestDisableCBConfig(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    bool enable = false;
    uint32_t identifier = 0;
    uint8_t ranType = 1;
    std::string input;
    std::cout << "Please enter identifier" << std::endl;
    std::cin >> input;
    identifier = std::atoi(input.c_str());
    result = smsService->SetCBConfig(slotId, enable, identifier, identifier, ranType);
    std::string ret = result ? "true" : "false";
    std::cout << "TestDisableCBConfig:" << ret << std::endl;
}

void GsmSmsSenderTest::TestSetDefaultSmsSlotId(const sptr<ISmsServiceInterface> &smsService) const
{
    bool result = false;
    if (smsService == nullptr) {
        return;
    }
    int32_t slotId;
    std::string input;
    std::cout << "Please enter Slot Id" << std::endl;
    std::cin >> input;
    slotId = std::atoi(input.c_str());
    result = smsService->SetDefaultSmsSlotId(slotId);
    std::string ret = result ? "true" : "false";
    std::cout << "TestSetDefaultSmsSlotId:" << ret << std::endl;
}

void GsmSmsSenderTest::TestGetDefaultSmsSlotId(const sptr<ISmsServiceInterface> &smsService) const
{
    int32_t result;
    if (smsService == nullptr) {
        return;
    }
    result = smsService->GetDefaultSmsSlotId();
    std::cout << "TestGetDefaultSmsSlotId:" << result << std::endl;
}
} // namespace Telephony
} // namespace OHOS