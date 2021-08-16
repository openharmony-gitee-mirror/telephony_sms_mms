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

#include "sms_receive_indexer.h"

namespace OHOS {
namespace Telephony {
SmsReceiveIndexer::SmsReceiveIndexer()
{
    timestamp_ = 0;
    destPort_ = 0;
    msgSeqId_ = 0;
    msgRefId_ = -1;
    msgCount_ = 1;
    isCdma_ = false;
    isCdmaWapPdu_ = false;
}

SmsReceiveIndexer::SmsReceiveIndexer(const std::vector<uint8_t> &pdu, long timestamp, int16_t destPort,
    bool isCdma, const std::string &address, const std::string &visibleAddress, int16_t msgRefId,
    uint16_t msgSeqId, uint16_t msgCount, bool isCdmaWapPdu, const std::string &messageBody)
    : pdu_(pdu), timestamp_(timestamp), destPort_(destPort), isCdma_(isCdma), isCdmaWapPdu_(isCdmaWapPdu),
      visibleMessageBody_(messageBody), originatingAddress_(address), msgRefId_(msgRefId), msgSeqId_(msgSeqId),
      msgCount_(msgCount), visibleAddress_(visibleAddress)
{}

SmsReceiveIndexer::SmsReceiveIndexer(const std::vector<uint8_t> &pdu, long timestamp, int16_t destPort,
    bool isCdma, bool isCdmaWapPdu, const std::string &address, const std::string &visibleAddress,
    const std::string &messageBody)
    : pdu_(pdu), timestamp_(timestamp), destPort_(destPort), isCdma_(isCdma), isCdmaWapPdu_(isCdmaWapPdu),
      visibleMessageBody_(messageBody), originatingAddress_(address), visibleAddress_(visibleAddress)
{
    if (isCdma_ && isCdmaWapPdu_) {
        msgSeqId_ = 0;
    } else {
        msgSeqId_ = 1;
    }
    msgRefId_ = -1;
    msgCount_ = 1;
}

std::string SmsReceiveIndexer::GetVisibleAddress() const
{
    return visibleAddress_;
}

void SmsReceiveIndexer::SetVisibleAddress(const std::string &visibleAddress)
{
    visibleAddress_ = visibleAddress;
}

std::string SmsReceiveIndexer::GetEraseRefId() const
{
    return eraseRefId_;
}

void SmsReceiveIndexer::SetEraseRefId(const std::string &eraseRefId)
{
    eraseRefId_ = eraseRefId;
}

uint16_t SmsReceiveIndexer::GetMsgCount() const
{
    return msgCount_;
}

void SmsReceiveIndexer::SetMsgCount(uint16_t msgCount)
{
    msgCount_ = msgCount;
}

uint16_t SmsReceiveIndexer::GetMsgSeqId() const
{
    return msgSeqId_;
}

void SmsReceiveIndexer::SetMsgSeqId(uint16_t msgSeqId)
{
    msgSeqId_ = msgSeqId;
}

uint16_t SmsReceiveIndexer::GetMsgRefId() const
{
    return msgRefId_;
}

void SmsReceiveIndexer::SetMsgRefId(uint16_t msgRefId)
{
    msgRefId_ = msgRefId;
}

std::string SmsReceiveIndexer::GetOriginatingAddress() const
{
    return originatingAddress_;
}

void SmsReceiveIndexer::SetOriginatingAddress(const std::string &address)
{
    originatingAddress_ = address;
}

std::string SmsReceiveIndexer::GetVisibleMessageBody() const
{
    return visibleMessageBody_;
}

void SmsReceiveIndexer::SetVisibleMessageBody(const std::string &messageBody)
{
    visibleMessageBody_ = messageBody;
}

bool SmsReceiveIndexer::GetIsCdmaWapPdu() const
{
    return isCdmaWapPdu_;
}

void SmsReceiveIndexer::SetIsCdmaWapPdu(bool isCdmaWapPdu)
{
    isCdmaWapPdu_ = isCdmaWapPdu;
}

bool SmsReceiveIndexer::GetIsCdma() const
{
    return isCdma_;
}

void SmsReceiveIndexer::SetIsCdma(bool isCdma)
{
    isCdma_ = isCdma;
}

int16_t SmsReceiveIndexer::GetDestPort() const
{
    return destPort_;
}

void SmsReceiveIndexer::SetDestPort(int16_t destPort)
{
    destPort_ = destPort;
}

long SmsReceiveIndexer::GetTimestamp() const
{
    return timestamp_;
}

void SmsReceiveIndexer::SetTimestamp(long timestamp)
{
    timestamp_ = timestamp;
}

const std::vector<uint8_t> &SmsReceiveIndexer::GetPdu() const
{
    return pdu_;
}

void SmsReceiveIndexer::SetPdu(const std::vector<uint8_t> &pdu)
{
    pdu_ = pdu;
}

void SmsReceiveIndexer::SetPdu(const std::vector<uint8_t> &&pdu)
{
    pdu_ = std::forward<const std::vector<uint8_t>>(pdu);
}

bool SmsReceiveIndexer::GetIsText() const
{
    return (destPort_ == TEXT_PORT_NUM);
}

bool SmsReceiveIndexer::GetIsWapPushMsg() const
{
    return (destPort_ == WAP_PUSH_PORT);
}

bool SmsReceiveIndexer::IsSingleMsg() const
{
    return msgCount_ == 1;
}
} // namespace Telephony
} // namespace OHOS