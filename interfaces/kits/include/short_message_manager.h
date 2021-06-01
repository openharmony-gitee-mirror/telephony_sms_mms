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
#ifndef SHORT_MESSAGE_MANAGER_H
#define SHORT_MESSAGE_MANAGER_H
#include <string>
#include <vector>
#include "i_sms_service_interface.h"
#include "refbase.h"
namespace OHOS {
class ShortMessageManager {
public:
    ShortMessageManager();
    virtual ~ShortMessageManager();
    void SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr,
        const std::u16string text, const sptr<SMS::ISendShortMessageCallback> &callback,
        const sptr<SMS::IDeliveryShortMessageCallback> &deliverCallback);
    void SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr, uint16_t port,
        const uint8_t *data, uint16_t dataLen, const sptr<SMS::ISendShortMessageCallback> &callback,
        const sptr<SMS::IDeliveryShortMessageCallback> &deliverCallback);

private:
    sptr<SMS::ISmsServiceInterface> smsServiceInterface_;
    int32_t ConnectService();
};

enum class SmsSendResult {
    /**
     *  Indicates that the SMS message is successfully sent.
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
} // namespace OHOS
#endif // SHORT_MESSAGE_MANAGER_H