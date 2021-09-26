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

#include "sms_base_message.h"

namespace OHOS {
namespace Telephony {
using namespace std;
string SmsBaseMessage::GetSmscAddr() const
{
    return scAddress_;
}

void SmsBaseMessage::SetSmscAddr(const string &address)
{
    scAddress_ = address;
}

string SmsBaseMessage::GetOriginatingAddress() const
{
    return originatingAddress_;
}

string SmsBaseMessage::GetVisibleOriginatingAddress() const
{
    return originatingAddress_;
}

enum SmsMessageClass SmsBaseMessage::GetMessageClass() const
{
    return msgClass_;
}

string SmsBaseMessage::GetVisibleMessageBody() const
{
    return visibleMessageBody_;
}

std::vector<uint8_t> SmsBaseMessage::GetRawPdu() const
{
    return rawPdu_;
}

std::vector<uint8_t> SmsBaseMessage::GetRawUserData() const
{
    return rawUserData_;
}

long SmsBaseMessage::GetScTimestamp() const
{
    return scTimestamp_;
}

// 3GPP TS 23.040 V5.1.0 9.2.3.9 TP Protocol Identifier (TP PID)
bool SmsBaseMessage::IsReplaceMessage()
{
    uint8_t temp = static_cast<uint8_t>(protocolId_);
    uint8_t tempPid = temp & PID_10_LOW;
    bReplaceMessage_ = ((temp & PID_87) == PID_7) && (tempPid > 0) && (tempPid < MAX_REPLY_PID);
    return bReplaceMessage_;
}

// Message Waiting Indication Status storage on the USIM
bool SmsBaseMessage::IsCphsMwi() const
{
    return bCphsMwi_;
}

// 3GPP TS 23.040 V5.1.0 3.2.6 Messages Waiting
bool SmsBaseMessage::IsMwiClear() const
{
    return bMwiClear_;
}

// 3GPP TS 23.040 V5.1.0 3.2.6 Messages Waiting
bool SmsBaseMessage::IsMwiSet() const
{
    return bMwiSet_;
}

// 3GPP TS 23.040 V5.1.0 3.2.6 Messages Waiting
bool SmsBaseMessage::IsMwiNotStore() const
{
    return bMwiNotStore_;
}

int SmsBaseMessage::GetStatus() const
{
    return status_;
}

bool SmsBaseMessage::IsSmsStatusReportMessage() const
{
    return bStatusReportMessage_;
}

bool SmsBaseMessage::HasReplyPath() const
{
    return hasReplyPath_;
}

int SmsBaseMessage::GetProtocolId() const
{
    return protocolId_;
}

std::shared_ptr<SmsConcat> SmsBaseMessage::GetConcatMsg()
{
    smsConcat_ = nullptr;
    for (int i = 0; i < smsUserData_.headerCnt; i++) {
        if (smsUserData_.header[i].udhType == SMS_UDH_CONCAT_8BIT) {
            smsConcat_ = std::make_shared<SmsConcat>();
            smsConcat_->is8Bits = true;
            smsConcat_->totalSeg = smsUserData_.header[i].udh.concat8bit.totalSeg;
            smsConcat_->seqNum = smsUserData_.header[i].udh.concat8bit.seqNum;
            smsConcat_->msgRef = smsUserData_.header[i].udh.concat8bit.msgRef;
            break;
        } else if (smsUserData_.header[i].udhType == SMS_UDH_CONCAT_16BIT) {
            smsConcat_ = std::make_shared<SmsConcat>();
            smsConcat_->is8Bits = false;
            smsConcat_->totalSeg = smsUserData_.header[i].udh.concat16bit.totalSeg;
            smsConcat_->seqNum = smsUserData_.header[i].udh.concat16bit.seqNum;
            smsConcat_->msgRef = smsUserData_.header[i].udh.concat16bit.msgRef;
            break;
        }
    }
    return smsConcat_;
}

std::shared_ptr<SmsAppPortAddr> SmsBaseMessage::GetPortAddress()
{
    portAddress_ = nullptr;
    for (int i = 0; i < smsUserData_.headerCnt; i++) {
        if (smsUserData_.header[i].udhType == SMS_UDH_APP_PORT_8BIT) {
            portAddress_ = std::make_shared<SmsAppPortAddr>();
            portAddress_->is8Bits = true;
            portAddress_->destPort = smsUserData_.header[i].udh.appPort8bit.destPort;
            portAddress_->originPort = smsUserData_.header[i].udh.appPort8bit.originPort;
            break;
        } else if (smsUserData_.header[i].udhType == SMS_UDH_APP_PORT_16BIT) {
            portAddress_ = std::make_shared<SmsAppPortAddr>();
            portAddress_->is8Bits = false;
            portAddress_->destPort = smsUserData_.header[i].udh.appPort16bit.destPort;
            portAddress_->originPort = smsUserData_.header[i].udh.appPort16bit.originPort;
            break;
        }
    }
    return portAddress_;
}

std::shared_ptr<SpecialSmsIndication> SmsBaseMessage::GetSpecialSmsInd()
{
    specialSmsInd_ = nullptr;
    for (int i = 0; i < smsUserData_.headerCnt; i++) {
        if (smsUserData_.header[i].udhType == SMS_UDH_SPECIAL_SMS) {
            specialSmsInd_ = std::make_shared<SpecialSmsIndication>();
            specialSmsInd_->bStore = smsUserData_.header[i].udh.specialInd.bStore;
            specialSmsInd_->msgInd = smsUserData_.header[i].udh.specialInd.msgInd;
            specialSmsInd_->waitMsgNum = smsUserData_.header[i].udh.specialInd.waitMsgNum;
            break;
        }
    }
    return specialSmsInd_;
}

bool SmsBaseMessage::IsConcatMsg()
{
    return (GetConcatMsg() == nullptr) ? false : true;
}

bool SmsBaseMessage::IsWapPushMsg()
{
    std::shared_ptr<SmsAppPortAddr> portAddress = GetPortAddress();
    if (portAddress != nullptr && !portAddress->is8Bits) {
        return portAddress->destPort == WAP_PUSH_PORT;
    }
    return false;
}

void SmsBaseMessage::ConvertMessageClass(enum SmsMessageClass msgClass)
{
    switch (msgClass) {
        case SmsMessageClass::SMS_SIM_MESSAGE:
            msgClass_ = SmsMessageClass::SMS_SIM_MESSAGE;
            break;
        case SmsMessageClass::SMS_INSTANT_MESSAGE:
            msgClass_ = SmsMessageClass::SMS_INSTANT_MESSAGE;
            break;
        case SmsMessageClass::SMS_OPTIONAL_MESSAGE:
            msgClass_ = SmsMessageClass::SMS_OPTIONAL_MESSAGE;
            break;
        case SmsMessageClass::SMS_FORWARD_MESSAGE:
            msgClass_ = SmsMessageClass::SMS_FORWARD_MESSAGE;
            break;
        default:
            msgClass_ = SmsMessageClass::SMS_CLASS_UNKNOWN;
            break;
    }
}

int SmsBaseMessage::GetMsgRef()
{
    return msgRef_;
}

int SmsBaseMessage::GetIndexOnSim() const
{
    return indexOnSim_;
}

void SmsBaseMessage::SetIndexOnSim(int index)
{
    indexOnSim_ = index;
}
} // namespace Telephony
} // namespace OHOS