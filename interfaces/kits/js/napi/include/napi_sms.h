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
namespace OHOS {
namespace TelephonyNapi {
const int DEFAULT_ERROR = -1;
const int RESOLVED = 1;
const int REJECT = 0;
const int NONE_PARAMTER = 0;
const int ONE_PARAMTER = 1;
const int TWO_PARAMTER = 2;
const int THREE_PARAMTER = 3;
const int FOUR_PARAMTER = 4;

constexpr int32_t ZERO = 0;
constexpr int32_t ONE = 1;
constexpr int32_t TWO = 2;
constexpr int32_t THREE = 3;
constexpr size_t SIZE_T_ZERO = 0;

constexpr int32_t MAX_TEXT_SHORT_MESSAGE_LENGTH = 4096;
constexpr int32_t DEFALUT_PORT = 8888;
constexpr size_t BUFF_LENGTH = 31;
constexpr int32_t PROPERTY_NAME_SIZE = 32;

struct AsyncContext {
    napi_env env;
    napi_async_work work;
    std::vector<unsigned char> pdu;
    char specification[PROPERTY_NAME_SIZE];
    size_t specificationLen;
    napi_value value;
    napi_deferred deferred;
    napi_ref callbackRef;
    int status;
};

struct TextMessageParameter {
    int32_t slotId;
    std::u16string destinationHost;
    std::u16string serviceCenter;
    napi_value contentValue;
    napi_value object;
};

enum ShortMessageClass {
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
} // namespace TelephonyNapi
} // namespace OHOS
#endif // NAPI_SMS_H