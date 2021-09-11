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

#ifndef GSM_SMS_CB_HANDLER_H
#define GSM_SMS_CB_HANDLER_H

#include <map>
#include <memory>

#include "core_manager.h"
#include "event_handler.h"
#include "event_runner.h"

#include "hril_sms_parcel.h"
#include "sms_cb_message.h"

#define MAX_CB_MSG_TEXT_LEN 4200
#define MAX_CB_MSG_LANGUAGE_TYPE_LEN 3
#define MAX_ETWS_WARNING_DATA_INFO_LEN 50

enum SmsCbType { SMS_CB_TYPE = 0, SMS_ETWS_TYPE };

struct SmsCbMsgInfo {
    unsigned char type;
    unsigned short messageId;
    long int receivedTime;
    unsigned short serialNum;
    unsigned char dcs;
    int cbTextLen;
    unsigned char cbText[MAX_CB_MSG_TEXT_LEN];
    unsigned char languageType[MAX_CB_MSG_LANGUAGE_TYPE_LEN];
};

struct SmsEtwsMsgInfo {
    unsigned char type;
    unsigned short messageId;
    long int receivedTime;
    unsigned short serialNum;
    unsigned short etwsWarningType;
    unsigned char etwsWarningData[MAX_ETWS_WARNING_DATA_INFO_LEN];
};

struct SmsCbMessageInfo {
    SmsCbType type;
    union {
        SmsCbMsgInfo cbInfo;
        SmsEtwsMsgInfo etwsInfo;
    };
};

namespace OHOS {
namespace Telephony {
using SmsCbInfo = struct CbInfo {
    CbInfo(const std::shared_ptr<SmsCbMessageHeader> &headPtr,
        const std::map<unsigned char, std::shared_ptr<SmsCbMessage>> &cbPtr)
        : header(headPtr), cbMsgs(cbPtr)
    {}
    CbInfo() {}
    std::shared_ptr<SmsCbMessageHeader> header;
    std::map<unsigned char, std::shared_ptr<SmsCbMessage>> cbMsgs;
};

class GsmSmsCbHandler : public AppExecFwk::EventHandler {
public:
    GsmSmsCbHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    ~GsmSmsCbHandler();
    void Init();
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    int32_t slotId_;
    std::vector<SmsCbInfo> cbMsgList_;
    int32_t msgReceiveCode_ = 0;

    bool CheckCbActive(const std::shared_ptr<SmsCbMessage> &cbMessage);
    unsigned char CheckCbMessage(const std::shared_ptr<SmsCbMessage> &cbMessage);
    std::unique_ptr<SmsCbInfo> FindCbMessage(const std::shared_ptr<SmsCbMessage> &cbMessage);
    bool AddCbMessageToList(const std::shared_ptr<SmsCbMessage> &cbMessage);
    bool RemoveCbMessageFromList(const std::shared_ptr<SmsCbMessage> &cbMessage);
    bool SendCbMessageBroadcast(const std::shared_ptr<SmsCbMessage> &cbMessage);
    void HandleCbMessage(std::shared_ptr<CellBroadcastReportInfo> &message);
    std::shared_ptr<Core> GetCore() const;
};
} // namespace Telephony
} // namespace OHOS
#endif