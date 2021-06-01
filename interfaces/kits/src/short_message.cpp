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

#include "short_message.h"
#include "gsm_sms_message.h"
#include "string_utils.h"
namespace OHOS {
namespace SMS {
std::u16string ShortMessage::GetVisibleMessageBody() const
{
    std::string visibleMessageBody;
    if (baseMessage_ != nullptr) {
        visibleMessageBody = baseMessage_->GetVisibleMessageBody();
    }
    return SMS::StringUtils::ToUtf16(visibleMessageBody);
}

std::u16string ShortMessage::GetVisibleRawAddress() const
{
    std::string visibleRawAddress;
    if (baseMessage_ != nullptr) {
        visibleRawAddress = baseMessage_->GetOriginatingAddress();
    }
    return SMS::StringUtils::ToUtf16(visibleRawAddress);
}

ShortMessage::ShortMessageClass ShortMessage::GetMessageClass() const
{
    ShortMessage::ShortMessageClass msgClase = SMS_CLASS_UNKNOWN;
    if (baseMessage_ != nullptr) {
        msgClase = (ShortMessage::ShortMessageClass)baseMessage_->GetMessageClass();
    }
    return msgClase;
}

std::u16string ShortMessage::GetScAddress() const
{
    std::string scAddress;
    if (baseMessage_ != nullptr) {
        scAddress = baseMessage_->GetScAddress();
    }
    return SMS::StringUtils::ToUtf16(scAddress);
}

int64_t ShortMessage::GetScTimestamp() const
{
    if (baseMessage_ != nullptr) {
        return baseMessage_->GetScTimestamp();
    }
    return 0;
}

bool ShortMessage::IsReplaceMessage() const
{
    if (baseMessage_ != nullptr) {
        return baseMessage_->IsReplaceMessage();
    }
    return false;
}

int32_t ShortMessage::GetStatus() const
{
    if (baseMessage_ != nullptr) {
        return baseMessage_->GetStatus();
    }
    return 0;
}

bool ShortMessage::IsSmsStatusReportMessage() const
{
    if (baseMessage_ != nullptr) {
        return baseMessage_->IsSmsStatusReportMessage();
    }
    return false;
}

bool ShortMessage::HasReplyPath() const
{
    if (baseMessage_ != nullptr) {
        return baseMessage_->HasReplyPath();
    }
    return false;
}

int32_t ShortMessage::GetProtocolId() const
{
    if (baseMessage_ != nullptr) {
        return baseMessage_->GetProtocolId();
    }
    return 0;
}

std::vector<unsigned char> ShortMessage::GetPdu() const
{
    std::vector<unsigned char> pdu;
    if (baseMessage_ != nullptr) {
        pdu = baseMessage_->GetRawPdu();
    }
    return pdu;
}

ShortMessage *ShortMessage::CreateMessage(std::vector<unsigned char> &pdu, std::u16string specification)
{
    std::string indicates = SMS::StringUtils::ToUtf8(specification);
    ShortMessage *message = nullptr;
    if (indicates == "3gpp") {
        message = new (std::nothrow) ShortMessage();
        if (message != nullptr) {
            message->baseMessage_ = SMS::GsmSmsMessage::CreateMessage(SMS::StringUtils::StringToHex(pdu));
            if (message->baseMessage_ == nullptr) {
                delete message;
                message = nullptr;
            }
        }
    }
    return message;
}
} // namespace SMS
} // namespace OHOS