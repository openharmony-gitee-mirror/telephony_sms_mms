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
#include "delivery_short_message_callback_stub.h"
#include "send_short_message_callback_stub.h"
#include "string_utils.h"
namespace OHOS {
namespace SMS {
void GsmSmsSenderTest::TestGsmSendShortData(const sptr<SMS::ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = 1;
    std::string dest;
    std::cout << "Please enter the receiver's telephone number\n" << std::endl;
    std::cin >> dest;
    std::string sca("");
    OHOS::sptr<OHOS::SendShortMessageCallbackStub> sendCallBackPtr(new OHOS::SendShortMessageCallbackStub());
    OHOS::sptr<OHOS::DeliveryShortMessageCallbackStub> deliveryCallBackPtr(
        new OHOS::DeliveryShortMessageCallbackStub());
    uint16_t port = 100;
    uint8_t data[] = "hello world";
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca), port, data,
        sizeof(data) / sizeof(data[0]), sendCallBackPtr, deliveryCallBackPtr);
    printf("TestGsmSendShortData\n");
}

void GsmSmsSenderTest::TestMissingParameter(const sptr<SMS::ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = 1;
    std::string dest(" ");
    std::string sca("");
    OHOS::sptr<OHOS::SendShortMessageCallbackStub> sendCallBackPtr(new OHOS::SendShortMessageCallbackStub());
    OHOS::sptr<OHOS::DeliveryShortMessageCallbackStub> deliveryCallBackPtr(
        new OHOS::DeliveryShortMessageCallbackStub());
    uint16_t port = 100;
    uint8_t data[] = "hello world";
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca), port, data,
        sizeof(data) / sizeof(data[0]), sendCallBackPtr, deliveryCallBackPtr);
    printf("TestMissingParameter\n");
}

void GsmSmsSenderTest::TestErrorParameter(const sptr<SMS::ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = 10;
    std::string dest;
    std::cout << "Please enter the receiver's telephone number\n";
    std::cin >> dest;
    std::string sca("");
    OHOS::sptr<OHOS::SendShortMessageCallbackStub> sendCallBackPtr(new OHOS::SendShortMessageCallbackStub());
    OHOS::sptr<OHOS::DeliveryShortMessageCallbackStub> deliveryCallBackPtr(
        new OHOS::DeliveryShortMessageCallbackStub());
    uint16_t port = 100;
    uint8_t data[] = "hello world";
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca), port, data,
        sizeof(data) / sizeof(data[0]), sendCallBackPtr, deliveryCallBackPtr);
    printf("TestErrorParameter\n");
}

void GsmSmsSenderTest::TestGsmSendLongData(const sptr<SMS::ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = 1;
    std::string dest;
    std::cout << "Please enter the receiver's telephone number\n";
    std::cin >> dest;
    std::string sca("");
    OHOS::sptr<OHOS::SendShortMessageCallbackStub> sendCallBackPtr(new OHOS::SendShortMessageCallbackStub());
    OHOS::sptr<OHOS::DeliveryShortMessageCallbackStub> deliveryCallBackPtr(
        new OHOS::DeliveryShortMessageCallbackStub());
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
    printf("TestGsmSendLongData\n");
}

void GsmSmsSenderTest::TestGsmSendShortText(const sptr<SMS::ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = 1;
    std::string dest;
    std::string text;
    std::cout << "Please enter the receiver's telephone number\n";
    getline(std::cin, dest);
    std::cout << "Please enter text\n";
    getline(std::cin, text);
    std::string sca("");
    OHOS::sptr<OHOS::SendShortMessageCallbackStub> sendCallBackPtr(new OHOS::SendShortMessageCallbackStub());
    OHOS::sptr<OHOS::DeliveryShortMessageCallbackStub> deliveryCallBackPtr(
        new OHOS::DeliveryShortMessageCallbackStub());
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca),
        StringUtils::ToUtf16(text), sendCallBackPtr, deliveryCallBackPtr);
    printf("TestGsmSendShortText\n");
}

void GsmSmsSenderTest::TestGsmSendLongText(const sptr<SMS::ISmsServiceInterface> &smsService) const
{
    if (smsService == nullptr) {
        return;
    }
    int32_t cardId = 1;
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
    OHOS::sptr<OHOS::SendShortMessageCallbackStub> sendCallBackPtr(new OHOS::SendShortMessageCallbackStub());
    OHOS::sptr<OHOS::DeliveryShortMessageCallbackStub> deliveryCallBackPtr(
        new OHOS::DeliveryShortMessageCallbackStub());
    smsService->SendMessage(cardId, StringUtils::ToUtf16(dest), StringUtils::ToUtf16(sca),
        StringUtils::ToUtf16(text), sendCallBackPtr, deliveryCallBackPtr);
    printf("TestGsmSendLongText\n");
}
} // namespace SMS
} // namespace OHOS