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

#ifndef NAPI_SMS_H
#define NAPI_SMS_H
#include <codecvt>
#include <locale>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "i_sms_service_interface.h"
#include "base_context.h"
#include "short_message.h"
#include "core_manager.h"

namespace OHOS {
namespace Telephony {
const int32_t DEFAULT_ERROR = -1;
const int32_t MESSAGE_UNKNOWN_STATUS = -1;
const int32_t MESSAGE_PARAMETER_NOT_MATCH = 0;
const int32_t TEXT_MESSAGE_PARAMETER_MATCH = 1;
const int32_t RAW_DATA_MESSAGE_PARAMETER_MATCH = 2;
constexpr int32_t MAX_TEXT_SHORT_MESSAGE_LENGTH = 4096;
constexpr int32_t DEFAULT_PORT = 8888;
constexpr size_t BUFF_LENGTH = 31;
constexpr int32_t PROPERTY_NAME_SIZE = 32;
constexpr int32_t NORMAL_STRING_SIZE = 64;

enum class ShortMessageClass {
    /** Indicates an unknown type. */
    UNKNOWN,
    /** Indicates an instant message, which is displayed immediately after being received. */
    INSTANT_MESSAGE,
    /** Indicates an SMS message that can be stored on the device or SIM card based on the storage status. */
    OPTIONAL_MESSAGE,
    /** Indicates an SMS message containing SIM card information, which is to be stored in a SIM card. */
    SIM_MESSAGE,
    /** Indicates an SMS message to be forwarded to another device. */
    FORWARD_MESSAGE
};

enum SendSmsResult {
    /**
     * Indicates that the SMS message is successfully sent.
     */
    SEND_SMS_SUCCESS = 0,

    /**
     * Indicates that sending the SMS message fails due to an unknown reason.
     */
    SEND_SMS_FAILURE_UNKNOWN = 1,

    /**
     * Indicates that sending the SMS fails because the modem is powered off.
     */
    SEND_SMS_FAILURE_RADIO_OFF = 2,

    /**
     * Indicates that sending the SMS message fails because the network is unavailable
     * or does not support sending or reception of SMS messages.
     */
    SEND_SMS_FAILURE_SERVICE_UNAVAILABLE = 3
};

struct SendMessageContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::u16string destinationHost = u"";
    std::u16string serviceCenter = u"";
    std::u16string textContent = u"";
    std::vector<uint8_t> rawDataContent;
    napi_ref thisVarRef = nullptr;
    napi_async_work work = nullptr;
    napi_ref sendCallbackRef = nullptr;
    napi_ref deliveryCallbackRef = nullptr;
    int32_t messageType = MESSAGE_PARAMETER_NOT_MATCH;
    uint16_t destinationPort = 0;
    int resolved = false;
};

struct CreateMessageContext : BaseContext {
    std::vector<unsigned char> pdu;
    std::string specification = "";
    ShortMessage *shortMessage = nullptr;
};

struct SetDefaultSmsSlotIdContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    bool setResult;
};

struct GetDefaultSmsSlotIdContext : BaseContext {
    int32_t defaultSmsSlotId = CoreManager::DEFAULT_SLOT_ID;
};

struct SetSmscAddrContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::string smscAddr = "";
    bool setResult = false;
};

struct GetSmscAddrContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::string smscAddr = "";
};

struct AddSimMessageContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::string smsc = "";
    std::string pdu = "";
    int32_t status = MESSAGE_UNKNOWN_STATUS;
    bool addResult = false;
};

struct DelSimMessageContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    int32_t msgIndex = DEFAULT_ERROR;
    bool deleteResult = false;
};

struct UpdateSimMessageContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    int32_t msgIndex = DEFAULT_ERROR;
    bool updateResult = false;
    int32_t newStatus = MESSAGE_UNKNOWN_STATUS;
    std::string pdu = "";
    std::string smsc = "";
};

struct SimShortMessage {
    ShortMessage message;
    ShortMessage::SmsSimMessageStatus simMessageStatus;
};

struct GetAllSimMessagesContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    std::vector<ShortMessage> messageArray;
};

struct CBRangeConfigContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    bool enable = false;
    int32_t startMessageId = DEFAULT_ERROR;
    int32_t endMessageId = DEFAULT_ERROR;
    int32_t ranType = DEFAULT_ERROR;
    bool setResult = false;
};

struct CBConfigContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    bool enable = false;
    int32_t startMessageId = DEFAULT_ERROR;
    int32_t endMessageId = DEFAULT_ERROR;
    int32_t ranType = DEFAULT_ERROR;
    bool setResult = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_SMS_H