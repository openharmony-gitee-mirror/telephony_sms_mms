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
#include "sms_send_indexer.h"
namespace OHOS {
namespace SMS {
using namespace std;
SmsSendIndexer::SmsSendIndexer(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
    : text_(text), scAddr_(scAddr), destAddr_(desAddr), sendCallback_(sendCallback),
      deliveryCallback_(deliveryCallback)
{
    isFailure_ = false;
}

SmsSendIndexer::SmsSendIndexer(const string &desAddr, const string &scAddr, int32_t port, const uint8_t *data,
    uint32_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
    : data_(std::vector<uint8_t>(data, data + dataLen)), scAddr_(scAddr), destAddr_(desAddr), destPort_(port),
      sendCallback_(sendCallback), deliveryCallback_(deliveryCallback)
{
    isFailure_ = false;
}

SmsSendIndexer::~SmsSendIndexer() {}

void SmsSendIndexer::SetEncodeSmca(const std::vector<uint8_t> &smac)
{
    smca_ = smac;
}

void SmsSendIndexer::SetEncodeSmca(const std::vector<uint8_t> &&smac)
{
    smca_ = std::forward<const std::vector<uint8_t>>(smac);
}

std::vector<uint8_t> &SmsSendIndexer::GetEncodeSmca()
{
    return smca_;
}

void SmsSendIndexer::SetEncodePdu(const std::vector<uint8_t> &pdu)
{
    pdu_ = pdu;
}

void SmsSendIndexer::SetEncodePdu(const std::vector<uint8_t> &&pdu)
{
    pdu_ = std::forward<const std::vector<uint8_t>>(pdu);
}

std::vector<uint8_t> &SmsSendIndexer::GetEncodePdu()
{
    return pdu_;
}

bool SmsSendIndexer::GetIsFailure() const
{
    return isFailure_;
}

void SmsSendIndexer::SetIsFailure(bool isFailure)
{
    isFailure_ = isFailure;
}

bool SmsSendIndexer::GetHasMore() const
{
    return hasMore_;
}

void SmsSendIndexer::SetHasMore(bool hasMore)
{
    hasMore_ = hasMore;
}

long SmsSendIndexer::GetTimeStamp() const
{
    return timeStamp_;
}

void SmsSendIndexer::SetTimeStamp(long timeStamp)
{
    timeStamp_ = timeStamp;
}

void SmsSendIndexer::SetNetWorkType(NetWorkType netWorkType)
{
    netWorkType_ = netWorkType;
}
NetWorkType SmsSendIndexer::GetNetWorkType()
{
    return netWorkType_;
}

std::shared_ptr<bool> SmsSendIndexer::GetHasCellFailed() const
{
    return hasCellFailed_;
}

void SmsSendIndexer::SetHasCellFailed(const std::shared_ptr<bool> &hasCellFailed)
{
    hasCellFailed_ = hasCellFailed;
}

std::shared_ptr<uint8_t> SmsSendIndexer::GetUnSentCellCount() const
{
    return unSentCellCount_;
}

void SmsSendIndexer::SetUnSentCellCount(const std::shared_ptr<uint8_t> &unSentCellCount)
{
    unSentCellCount_ = unSentCellCount;
}

uint8_t SmsSendIndexer::GetCsResendCount() const
{
    return csResendCount_;
}

void SmsSendIndexer::SetCsResendCount(uint8_t csResendCount)
{
    csResendCount_ = csResendCount;
}

std::string SmsSendIndexer::GetText() const
{
    return text_;
}

void SmsSendIndexer::SetText(const std::string &text)
{
    text_ = text;
}

sptr<IDeliveryShortMessageCallback> SmsSendIndexer::GetDeliveryCallback() const
{
    return deliveryCallback_;
}

void SmsSendIndexer::SetDeliveryCallback(const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    deliveryCallback_ = deliveryCallback;
}

sptr<ISendShortMessageCallback> SmsSendIndexer::GetSendCallback() const
{
    return sendCallback_;
}

void SmsSendIndexer::SetSendCallback(const sptr<ISendShortMessageCallback> &sendCallback)
{
    sendCallback_ = sendCallback;
}

int32_t SmsSendIndexer::GetDestPort() const
{
    return destPort_;
}

void SmsSendIndexer::SetDestPort(int32_t destPort)
{
    destPort_ = destPort;
}

std::string SmsSendIndexer::GetDestAddr() const
{
    return destAddr_;
}

void SmsSendIndexer::SetDestAddr(const std::string &destAddr)
{
    destAddr_ = destAddr;
}

std::string SmsSendIndexer::GetSmcaAddr() const
{
    return scAddr_;
}

void SmsSendIndexer::SetSmcaAddr(const std::string &scAddr)
{
    scAddr_ = scAddr;
}

bool SmsSendIndexer::GetIsText() const
{
    return isText_;
}

uint8_t SmsSendIndexer::GetErrorCode() const
{
    return errorCode_;
}

void SmsSendIndexer::SetErrorCode(uint8_t errorCode)
{
    errorCode_ = errorCode;
}

std::vector<uint8_t> &SmsSendIndexer::GetData()
{
    return data_;
}

void SmsSendIndexer::SetData(const std::vector<uint8_t> &data)
{
    data_ = data;
}

void SmsSendIndexer::SetData(const std::vector<uint8_t> &&data)
{
    data_ = std::forward<const std::vector<uint8_t>>(data);
}

std::vector<uint8_t> &SmsSendIndexer::GetAckPdu()
{
    return ackPdu_;
}

void SmsSendIndexer::SetAckPdu(const std::vector<uint8_t> &ackPdu)
{
    ackPdu_ = ackPdu;
}

void SmsSendIndexer::SetAckPdu(const std::vector<uint8_t> &&ackPdu)
{
    ackPdu_ = std::forward<const std::vector<uint8_t>>(ackPdu);
}

uint8_t SmsSendIndexer::GetMsgRefId() const
{
    return msgRefId_;
}

void SmsSendIndexer::SetMsgRefId(uint8_t msgRefId)
{
    msgRefId_ = msgRefId;
}

int64_t SmsSendIndexer::GetMsgRefId64Bit() const
{
    return msgRefId64Bit_;
}
void SmsSendIndexer::SetMsgRefId64Bit(int64_t msgRefId64Bit)
{
    msgRefId64Bit_ = msgRefId64Bit;
}

uint8_t SmsSendIndexer::GetPsResendCount() const
{
    return psResendCount_;
}

void SmsSendIndexer::SetPsResendCount(uint8_t psResendCount)
{
    psResendCount_ = psResendCount;
}

void SmsSendIndexer::UpdatePduForResend()
{
    if (pdu_.size() < 0x02) {
        return;
    }
    if (((0x01 & pdu_[0]) == 0x01)) {
        pdu_[0] |= 0x04; // TP-RD
        pdu_[1] = msgRefId_; // TP-MR
    }
}
} // namespace SMS
} // namespace OHOS