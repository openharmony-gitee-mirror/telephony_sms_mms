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
#ifndef SMS_BASE_MESSAGE__H
#define SMS_BASE_MESSAGE__H
#include <memory>
#include <string>
#include <vector>
#include "gsm_sms_tpdu_codec.h"
namespace OHOS {
namespace SMS {
#define WAP_PUSH_PORT 2948
#define MAX_MSG_TEXT_LEN 1530
#define MAX_REPLAY_PID 8
enum ShortMessageClass {
    /** class0 Indicates an instant message, which is displayed immediately after being received. */
    SMS_INSTANT_MESSAGE = 0,
    /** class1 Indicates an SMS message that can be stored on the device or SIM card based on the storage status. */
    SMS_OPTIONAL_MESSAGE,
    /** class2 Indicates an SMS message containing SIM card information, which is to be stored in a SIM card. */
    SMS_SIM_MESSAGE,
    /** class3 Indicates an SMS message to be forwarded to another device. */
    SMS_FORWARD_MESSAGE,
    /** Indicates an unknown type. */
    SMS_CLASS_UNKNOWN
};

typedef struct {
    uint16_t msgRef;
    uint16_t seqNum;
    uint16_t totalSeg;
    bool is8Bits;
} SmsConcat;

typedef struct {
    uint32_t destPort;
    uint32_t originPort;
    bool is8Bits;
} SmsAppPortAddr;

typedef struct {
    bool bStore;
    uint16_t msgInd;
    uint16_t waitMsgNum;
} SpecialSmsIndication;

class SmsBaseMessage {
public:
    SmsBaseMessage() = default;
    virtual ~SmsBaseMessage() = default;
    virtual void SetScAddress(const std::string &scAddress);
    virtual std::string GetScAddress() const;
    virtual std::string GetOriginatingAddress() const;
    virtual std::string GetVisibleOriginatingAddress() const;
    virtual std::string GetVisibleMessageBody() const;
    virtual enum ShortMessageClass GetMessageClass() const;
    std::vector<uint8_t> GetRawPdu() const;
    std::vector<uint8_t> GetRawUserData() const;
    virtual long GetScTimestamp() const;
    virtual int GetStatus() const;
    virtual int GetProtocolId() const;
    virtual bool IsReplaceMessage();
    virtual bool IsCphsMwi() const;
    virtual bool IsMwiClear() const;
    virtual bool IsMwiSet() const;
    virtual bool IsMwiNotStore() const;
    virtual bool IsSmsStatusReportMessage() const;
    virtual bool HasReplyPath() const;
    virtual std::shared_ptr<SmsConcat> GetConcatMsg();
    virtual std::shared_ptr<SmsAppPortAddr> GetPortAddress();
    virtual std::shared_ptr<SpecialSmsIndication> GetSpecialSmsInd();
    virtual bool IsConcatMsg();
    virtual bool IsWapPushMsg();
    virtual void ConvertMessageClass(enum SMS_MSG_CLASS_E msgClass);

protected:
    std::string scAddress_;
    std::string originatingAddress_;
    std::string visibleOriginatingAddress_;
    std::string visibleMessageBody_;
    enum ShortMessageClass msgClass_ = SMS_CLASS_UNKNOWN;
    long scTimestamp_;
    int status_;
    int protocolId_;
    bool bReplaceMessage_;
    bool bStatusRoretMessage_;
    bool bMwi_;
    bool bMwiSense_;
    bool bCphsMwi_;
    bool bMwiClear_;
    bool bMwiSet_;
    bool bMwiNotStore_;
    bool hasReplayPath_;
    bool bMoreMsg_;
    bool bHeaderInd_;
    int headerCnt_;
    int headerDataLen_;
    int msgRef_;
    bool bCompressed_;
    bool bIndActive_;
    int codingScheme_;
    int codingGroup_;
    std::vector<uint8_t> rawPdu_;
    std::vector<uint8_t> rawUserData_;
    struct SmsUserData smsUserData_;
    std::shared_ptr<SmsConcat> smsConcat_;
    std::shared_ptr<SmsAppPortAddr> portAddress_;
    std::shared_ptr<SpecialSmsIndication> specialSmsInd_;

private:
    constexpr static int pid87_ = 0xc0;
    constexpr static int pid7_ = 0x40;
    constexpr static int pid10Low_ = 0x3f;
};
} // namespace SMS
} // namespace OHOS
#endif