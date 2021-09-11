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

#ifndef CDMA_SMS_MESSAGE_H
#define CDMA_SMS_MESSAGE_H
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "sms_base_message.h"

namespace OHOS {
namespace Telephony {
#define CDMA_MAX_UD_HEADER_NUM 7

class CdmaSmsMessage : public SmsBaseMessage {
public:
    CdmaSmsMessage() = default;
    virtual ~CdmaSmsMessage() = default;
    virtual int GetProtocolId() const;
    virtual bool IsReplaceMessage();
    virtual bool IsCphsMwi() const;
    virtual bool IsWapPushMsg();
    virtual int GetTransMsgType() const;
    virtual int GetTransTeleService() const;
    virtual bool AddUserDataHeader(const struct SmsUDH &header);

    virtual std::shared_ptr<SpecialSmsIndication> GetSpecialSmsInd();
    static std::shared_ptr<CdmaSmsMessage> CreateMessage(const std::string &pdu);

private:
    std::unique_ptr<SmsTransAddr> address_;
    std::unique_ptr<SmsTeleSvcAddr> callbackNumber_;
    std::vector<struct SmsUDH> userHeaders_;
    std::unique_ptr<struct SmsTransMsg> transMsg_;

    bool PduAnalysis(const std::string &pduHex);
    void AnalysisP2pMsg(const SmsTransP2PMsg &p2pMsg);
    void AnalysisCbMsg(const SmsTransBroadCastMsg &cbMsg);
    void AnalsisAckMsg(const SmsTransAckMsg &ackMsg);
    void AnalsisDeliverMwi(const SmsTransP2PMsg &p2pMsg);
    void AnalsisDeliverMsg(const SmsTeleSvcDeliver &deliver);
    void AnalsisDeliverAck(const SmsTeleSvcDeliverAck &deliverAck);
    void AnalsisSubmitReport(const SmsTeleSvcDeliverReport &report);
    void AnalsisUserData(const SmsTeleSvcUserData &userData);
    void AnalsisCMASMsg(const SmsTeleSvcDeliver &deliver);
};
} // namespace Telephony
} // namespace OHOS
#endif