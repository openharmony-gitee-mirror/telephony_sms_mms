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

#include "cdma_sms_message.h"
#include "gsm_sms_message.h"
#include "sms_base_message.h"
#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
bool ShortMessage::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString16(visibleMessageBody_)) {
        return false;
    }
    if (!parcel.ReadString16(visibleRawAddress_)) {
        return false;
    }

    int8_t msgClass = 0;
    if (!parcel.ReadInt8(msgClass)) {
        return false;
    }
    messageClass_ = static_cast<ShortMessage::SmsMessageClass>(msgClass);

    int8_t simMsgStatus = 0;
    if (!parcel.ReadInt8(simMsgStatus)) {
        return false;
    }
    simMessageStatus_ = static_cast<ShortMessage::SmsSimMessageStatus>(simMsgStatus);

    if (!parcel.ReadInt32(indexOnSim_)) {
        return false;
    }
    if (!parcel.ReadString16(scAddress_)) {
        return false;
    }
    if (!parcel.ReadInt64(scTimestamp_)) {
        return false;
    }
    if (!parcel.ReadBool(isReplaceMessage_)) {
        return false;
    }
    if (!parcel.ReadInt32(status_)) {
        return false;
    }
    if (!parcel.ReadBool(isSmsStatusReportMessage_)) {
        return false;
    }
    if (!parcel.ReadBool(hasReplyPath_)) {
        return false;
    }
    if (!parcel.ReadInt32(protocolId_)) {
        return false;
    }
    if (!parcel.ReadUInt8Vector(&pdu_)) {
        return false;
    }
    return true;
}

bool ShortMessage::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(visibleMessageBody_)) {
        return false;
    }
    if (!parcel.WriteString16(visibleRawAddress_)) {
        return false;
    }
    if (!parcel.WriteInt8(static_cast<int8_t>(messageClass_))) {
        return false;
    }
    if (!parcel.WriteInt8(static_cast<int8_t>(simMessageStatus_))) {
        return false;
    }
    if (!parcel.WriteInt32(indexOnSim_)) {
        return false;
    }
    if (!parcel.WriteString16(scAddress_)) {
        return false;
    }
    if (!parcel.WriteInt64(scTimestamp_)) {
        return false;
    }
    if (!parcel.WriteBool(isReplaceMessage_)) {
        return false;
    }
    if (!parcel.WriteInt32(status_)) {
        return false;
    }
    if (!parcel.WriteBool(isSmsStatusReportMessage_)) {
        return false;
    }
    if (!parcel.WriteBool(hasReplyPath_)) {
        return false;
    }
    if (!parcel.WriteInt32(protocolId_)) {
        return false;
    }
    if (!parcel.WriteUInt8Vector(pdu_)) {
        return false;
    }
    return true;
}

ShortMessage ShortMessage::UnMarshalling(Parcel &parcel)
{
    ShortMessage param;
    param.ReadFromParcel(parcel);
    return param;
}

std::u16string ShortMessage::GetVisibleMessageBody() const
{
    return visibleMessageBody_;
}

std::u16string ShortMessage::GetVisibleRawAddress() const
{
    return visibleRawAddress_;
}

ShortMessage::SmsMessageClass ShortMessage::GetMessageClass() const
{
    return messageClass_;
}

std::u16string ShortMessage::GetScAddress() const
{
    return scAddress_;
}

int64_t ShortMessage::GetScTimestamp() const
{
    return scTimestamp_;
}

bool ShortMessage::IsReplaceMessage() const
{
    return isReplaceMessage_;
}

int32_t ShortMessage::GetStatus() const
{
    return status_;
}

bool ShortMessage::IsSmsStatusReportMessage() const
{
    return isSmsStatusReportMessage_;
}

bool ShortMessage::HasReplyPath() const
{
    return hasReplyPath_;
}

ShortMessage::SmsSimMessageStatus ShortMessage::GetIccMessageStatus() const
{
    return simMessageStatus_;
}

int32_t ShortMessage::GetIndexOnSim() const
{
    return indexOnSim_;
}

int32_t ShortMessage::GetProtocolId() const
{
    return protocolId_;
}

std::vector<unsigned char> ShortMessage::GetPdu() const
{
    return pdu_;
}

ShortMessage *ShortMessage::CreateMessage(std::vector<unsigned char> &pdu, std::u16string specification)
{
    std::string indicates = StringUtils::ToUtf8(specification);
    ShortMessage *message = new (std::nothrow) ShortMessage();
    if (message == nullptr) {
        return nullptr;
    }

    std::shared_ptr<SmsBaseMessage> baseMessage;
    if (indicates == "3gpp") {
        baseMessage = GsmSmsMessage::CreateMessage(StringUtils::StringToHex(pdu));
    } else if (indicates == "3gpp2") {
        baseMessage = CdmaSmsMessage::CreateMessage(StringUtils::StringToHex(pdu));
    }

    if (baseMessage == nullptr) {
        delete message;
        message = nullptr;
        return message;
    }

    message->visibleMessageBody_ = StringUtils::ToUtf16(baseMessage->GetVisibleMessageBody());
    message->visibleRawAddress_ = StringUtils::ToUtf16(baseMessage->GetVisibleOriginatingAddress());
    message->messageClass_ = static_cast<ShortMessage::SmsMessageClass>(baseMessage->GetMessageClass());
    message->scAddress_ = StringUtils::ToUtf16(baseMessage->GetSmscAddr());
    message->scTimestamp_ = baseMessage->GetScTimestamp();
    message->isReplaceMessage_ = baseMessage->IsReplaceMessage();
    message->status_ = baseMessage->GetStatus();
    message->isSmsStatusReportMessage_ = baseMessage->IsSmsStatusReportMessage();
    message->hasReplyPath_ = baseMessage->HasReplyPath();
    message->protocolId_ = baseMessage->GetProtocolId();
    message->pdu_ = baseMessage->GetRawPdu();
    return message;
}

ShortMessage ShortMessage::CreateIccMessage(
    std::vector<unsigned char> &pdu, std::string specification, int32_t index)
{
    ShortMessage message;
    message.simMessageStatus_ = SMS_SIM_MESSAGE_STATUS_FREE;
    if (pdu.size() <= MIN_ICC_PDU_LEN) {
        return message;
    }

    unsigned char simStatus = pdu.at(0);
    TELEPHONY_LOGI("simStatus = %{public}d", simStatus);
    if (simStatus == SMS_SIM_MESSAGE_STATUS_FREE) {
        message.simMessageStatus_ = SMS_SIM_MESSAGE_STATUS_FREE;
        return message;
    }
    std::vector<unsigned char> pduTemp(pdu.begin() + MIN_ICC_PDU_LEN, pdu.end());
    std::shared_ptr<SmsBaseMessage> baseMessage;
    if (specification == "3gpp") {
        baseMessage = GsmSmsMessage::CreateMessage(StringUtils::StringToHex(pduTemp));
    } else if (specification == "3gpp2") {
        baseMessage = CdmaSmsMessage::CreateMessage(StringUtils::StringToHex(pduTemp));
    }
    if (baseMessage == nullptr) {
        TELEPHONY_LOGE("baseMessage is nullptr!");
        return message;
    }

    baseMessage->SetIndexOnSim(index);
    if (simStatus == SMS_SIM_MESSAGE_STATUS_READ || simStatus == SMS_SIM_MESSAGE_STATUS_UNREAD ||
        simStatus == SMS_SIM_MESSAGE_STATUS_SENT || simStatus == SMS_SIM_MESSAGE_STATUS_UNSENT) {
        message.simMessageStatus_ = static_cast<SmsSimMessageStatus>(simStatus);
    }

    message.visibleMessageBody_ = StringUtils::ToUtf16(baseMessage->GetVisibleMessageBody());
    message.visibleRawAddress_ = StringUtils::ToUtf16(baseMessage->GetVisibleOriginatingAddress());
    message.messageClass_ = static_cast<ShortMessage::SmsMessageClass>(baseMessage->GetMessageClass());
    message.scAddress_ = StringUtils::ToUtf16(baseMessage->GetSmscAddr());
    message.scTimestamp_ = baseMessage->GetScTimestamp();
    message.isReplaceMessage_ = baseMessage->IsReplaceMessage();
    message.status_ = baseMessage->GetStatus();
    message.isSmsStatusReportMessage_ = baseMessage->IsSmsStatusReportMessage();
    message.hasReplyPath_ = baseMessage->HasReplyPath();
    message.protocolId_ = baseMessage->GetProtocolId();
    message.pdu_ = baseMessage->GetRawPdu();
    message.indexOnSim_ = baseMessage->GetIndexOnSim();
    return message;
}
} // namespace Telephony
} // namespace OHOS