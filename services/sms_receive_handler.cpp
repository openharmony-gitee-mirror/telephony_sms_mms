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

#include "sms_receive_handler.h"

#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "want.h"

#include "string_utils.h"
#include "gsm_sms_message.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
using namespace EventFwk;
SmsReceiveHandler::SmsReceiveHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : AppExecFwk::EventHandler(runner), slotId_(slotId)
{}

SmsReceiveHandler::~SmsReceiveHandler() {}

void SmsReceiveHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SmsReceiveHandler::ProcessEvent event == nullptr");
        return;
    }

    int eventId = 0;
    eventId = event->GetInnerEventId();
    switch (eventId) {
        case ObserverHandler::RADIO_GSM_SMS: {
            std::shared_ptr<SmsBaseMessage> message = nullptr;
            message = TransformMessageInfo(event->GetSharedObject<SmsMessageInfo>());
            if (message != nullptr) {
                TELEPHONY_LOGD("[raw pdu size] =%{public}zu", message->GetRawPdu().size());
            }
            HandleReceivedSms(message);
            break;
        }
        default:
            break;
    }
}

void SmsReceiveHandler::HandleReceivedSms(const std::shared_ptr<SmsBaseMessage> &smsBaseMessage)
{
    if (smsBaseMessage == nullptr) {
        TELEPHONY_LOGE("smsBaseMessage is nullptr");
        return;
    }
    ReplySmsToSmsc(HandleSmsByType(smsBaseMessage), nullptr);
}

void SmsReceiveHandler::CombineMessagePart(const std::shared_ptr<SmsReceiveIndexer> &indexer)
{
    if (indexer == nullptr) {
        TELEPHONY_LOGE("indexer is nullptr");
        return;
    }
    std::shared_ptr<vector<string>> pdus = make_shared<vector<string>>();
    if (pdus == nullptr) {
        return;
    }
    if (indexer->IsSingleMsg()) {
        string pdu = StringUtils::StringToHex(indexer->GetPdu());
        pdus->push_back(pdu);
    } else {
        pdus->assign(MAX_SEGMENT_NUM, "");
        int msgSeg = indexer->GetMsgCount();
        int8_t count = 0;
        int8_t notNullPart = msgSeg;
        string key = indexer->GetEraseRefId();
        auto iter = receiveMap_.find(key);
        for (; iter != receiveMap_.end(); ++iter) {
            ++count;
            string pdu = StringUtils::StringToHex(iter->second->GetPdu());
            if ((iter->second->GetMsgSeqId() - PDU_POS_OFFSET >= MAX_SEGMENT_NUM) ||
                (iter->second->GetMsgSeqId() - PDU_POS_OFFSET < 0)) {
                receiveMap_.erase(indexer->GetEraseRefId());
                return;
            }
            pdus->at(iter->second->GetMsgSeqId() - PDU_POS_OFFSET) = pdu;
            if (iter->second->GetPdu().size() == 0) {
                --notNullPart;
            }
        }
        if (count != msgSeg) {
            return;
        }
        if (pdus->empty()) {
            TELEPHONY_LOGI("pdu list count is 0");
            return;
        }
        if (notNullPart != msgSeg) {
            TELEPHONY_LOGI("pdu list contains is null");
            return;
        }
    }
    if (indexer->GetIsWapPushMsg()) {
        receiveMap_.erase(indexer->GetEraseRefId());
        return;
    }
    receiveMap_.erase(indexer->GetEraseRefId());
    SendBroadcast(pdus);
}

bool SmsReceiveHandler::IsRepeatedMessagePart(const shared_ptr<SmsReceiveIndexer> &smsIndexer)
{
    if (smsIndexer != nullptr) {
        // emulate find
        string key = smsIndexer->GetOriginatingAddress() + to_string(smsIndexer->GetMsgRefId()) +
            to_string(smsIndexer->GetMsgCount());
        auto iter = receiveMap_.find(key);
        for (; iter != receiveMap_.end(); ++iter) {
            if (iter->second->GetMsgSeqId() == smsIndexer->GetMsgSeqId()) {
                return true;
            }
        }
    }
    return false;
}

void SmsReceiveHandler::SendBroadcast(const shared_ptr<vector<string>> &pdus)
{
    if (pdus == nullptr) {
        TELEPHONY_LOGE("pdus is nullptr");
        return;
    }

    std::string fullPdu;
    for (const auto &it : *pdus) {
        if (!it.empty()) {
            fullPdu.append(it);
            fullPdu.append(":");
        }
    }
    if (!fullPdu.empty()) {
        fullPdu.pop_back();
    }

    Want want;
    want.SetAction("usual.event.SMS_RECEIVE_COMPLETED");
    want.SetParam("pdu", *pdus);
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(MSG_RECEIVE_CODE);
    data.SetData(fullPdu);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    if (!publishResult) {
        TELEPHONY_LOGE("SendBroadcast PublishBroadcastEvent result fail");
    }
}

std::shared_ptr<Core> SmsReceiveHandler::GetCore() const
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        return core;
    }
    return nullptr;
}
} // namespace Telephony
} // namespace OHOS