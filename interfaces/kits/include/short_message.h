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

#ifndef SHORT_MESSAGE_H
#define SHORT_MESSAGE_H
#include <codecvt>
#include <locale>
#include <memory>
#include "sms_base_message.h"
namespace OHOS {
namespace SMS {
class ShortMessage {
    enum ShortMessageClass {
        /** class0 Indicates an instant message, which is displayed immediately after being received. */
        SMS_INSTANT_MESSAGE = 0,
        /** class1 Indicates an SMS message that can be stored on the device or SIM card based on the storage
           status. */
        SMS_OPTIONAL_MESSAGE,
        /** class2 Indicates an SMS message containing SIM card information, which is to be stored in a SIM card. */
        SMS_SIM_MESSAGE,
        /** class3 Indicates an SMS message to be forwarded to another device. */
        SMS_FORWARD_MESSAGE,
        /** Indicates an unknown type. */
        SMS_CLASS_UNKNOWN
    };

public:
    std::u16string GetVisibleMessageBody() const;
    std::u16string GetVisibleRawAddress() const;
    ShortMessageClass GetMessageClass() const;
    std::u16string GetScAddress() const;
    int64_t GetScTimestamp() const;
    bool IsReplaceMessage() const;
    int32_t GetStatus() const;
    bool IsSmsStatusReportMessage() const;
    bool HasReplyPath() const;
    int32_t GetProtocolId() const;
    std::vector<unsigned char> GetPdu() const;
    static ShortMessage *CreateMessage(std::vector<unsigned char> &pdu, std::u16string specification);
    ~ShortMessage() = default;

private:
    ShortMessage() = default;
    std::shared_ptr<SMS::SmsBaseMessage> baseMessage_;
};
} // namespace SMS
} // namespace OHOS
#endif