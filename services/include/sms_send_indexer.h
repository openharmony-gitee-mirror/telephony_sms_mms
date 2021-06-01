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
#ifndef SMS_SEND_INDEXER_H
#define SMS_SEND_INDEXER_H
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "i_sms_service_interface.h"
#include "sms_common.h"
namespace OHOS {
namespace SMS {
class SmsSendIndexer {
public:
    SmsSendIndexer(const std::string &desAddr, const std::string &scAddr, const std::string &text,
        const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveCallback);
    SmsSendIndexer(const std::string &desAddr, const std::string &scAddr, int32_t port, const uint8_t *data,
        uint32_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveCallback);
    ~SmsSendIndexer();
    uint8_t GetPsResendCount() const;
    void SetPsResendCount(uint8_t resendCount);
    uint8_t GetCsResendCount() const;
    void SetCsResendCount(uint8_t resendCount);
    uint8_t GetMsgRefId() const;
    void SetMsgRefId(uint8_t msgRefId);
    int64_t GetMsgRefId64Bit() const;
    void SetMsgRefId64Bit(int64_t msgRefId64Bit);
    std::vector<uint8_t> &GetAckPdu();
    void SetAckPdu(const std::vector<uint8_t> &ackPdu);
    std::vector<uint8_t> &GetData();
    void SetData(const std::vector<uint8_t> &data);
    uint8_t GetErrorCode() const;
    void SetErrorCode(uint8_t errorCode);
    bool GetIsText() const;
    std::string GetSmcaAddr() const;
    void SetSmcaAddr(const std::string &scAddr);
    std::string GetDestAddr() const;
    void SetDestAddr(const std::string &destAddr);
    int32_t GetDestPort() const;
    void SetDestPort(int32_t destPort);
    sptr<ISendShortMessageCallback> GetSendCallback() const;
    void SetSendCallback(const sptr<ISendShortMessageCallback> &sendCallback);
    sptr<IDeliveryShortMessageCallback> GetDeliveryCallback() const;
    void SetDeliveryCallback(const sptr<IDeliveryShortMessageCallback> &deliveCallback);
    std::string GetText() const;
    void SetText(const std::string &text);
    std::shared_ptr<uint8_t> GetUnSentCellCount() const;
    void SetUnSentCellCount(const std::shared_ptr<uint8_t> &unSentCellCount);
    std::shared_ptr<bool> GetHasCellFailed() const;
    void SetHasCellFailed(const std::shared_ptr<bool> &hasCellFailed);
    bool GetIsFailure() const;
    void SetIsFailure(bool isFailure);
    bool GetHasMore() const;
    void SetHasMore(bool hasMore);
    long GetTimeStamp() const;
    void SetTimeStamp(long timeStamp);
    void SetNetWorkType(NetWorkType netWorkType);
    NetWorkType GetNetWorkType();
    void SetEncodeSmca(const std::vector<uint8_t> &smac);
    std::vector<uint8_t> &GetEncodeSmca();
    void SetEncodePdu(const std::vector<uint8_t> &pdu);
    std::vector<uint8_t> &GetEncodePdu();
    void UpdatePduForResend();
    void SetEncodePdu(const std::vector<uint8_t> &&pdu);
    void SetEncodeSmca(const std::vector<uint8_t> &&smac);
    void SetAckPdu(const std::vector<uint8_t> &&ackPdu);
    void SetData(const std::vector<uint8_t> &&data);

private:
    std::vector<uint8_t> pdu_;
    std::vector<uint8_t> smca_;
    std::vector<uint8_t> ackPdu_;
    std::vector<uint8_t> data_;
    uint8_t errorCode_;
    bool isText_ = false;
    std::string text_;
    NetWorkType netWorkType_;
    std::string scAddr_;
    std::string destAddr_;
    int32_t destPort_;
    bool hasMore_ = false;
    long timeStamp_;
    uint8_t csResendCount_ = 0;
    uint8_t psResendCount_ = 0;
    uint8_t msgRefId_;
    int64_t msgRefId64Bit_;
    std::shared_ptr<uint8_t> unSentCellCount_;
    std::shared_ptr<bool> hasCellFailed_;
    sptr<ISendShortMessageCallback> sendCallback_;
    sptr<IDeliveryShortMessageCallback> deliveryCallback_;
    bool isFailure_ = false;
};
} // namespace SMS
} // namespace OHOS
#endif