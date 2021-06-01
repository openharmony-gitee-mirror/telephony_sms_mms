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
#include "want.h"
#include "string_utils.h"
#include "gsm_sms_message.h"
#include "sms_hilog_wrapper.h"
namespace OHOS {
namespace SMS {
using namespace std;
using namespace EventFwk;
SmsReceiveHandler::SmsReceiveHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : AppExecFwk::EventHandler(runner), slotId_(slotId)
{}

SmsReceiveHandler::~SmsReceiveHandler() {}

void SmsReceiveHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    int eventId = 0;
    HILOG_DEBUG("SmsReceiveHandler::ProcessEvent");
    eventId = event->GetInnerEventId();
    switch (eventId) {
        case ObserverHandler::RADIO_GSM_SMS: {
            std::shared_ptr<SmsBaseMessage> message = nullptr;
            message = TransformMessageInfo(event->GetSharedObject<SmsMessageInfo>());
            if (message != nullptr) {
                HILOG_DEBUG("[raw pdu] size = [%{public}d] pdu = %{public}s", message->GetRawPdu().size(),
                    message->GetRawPdu().data());
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
        return;
    }
    ReplySmsToSmsc(HandleSmsByType(smsBaseMessage), nullptr);
}

void SmsReceiveHandler::CombineMessagePart(const std::shared_ptr<SmsReceiveIndexer> &indexer)
{
    if (indexer == nullptr) {
        return;
    }
    int messageCount = indexer->GetMsgCount();
    std::shared_ptr<vector<string>> pdus = make_shared<vector<string>>();
    if (pdus == nullptr) {
        return;
    }
    if (messageCount == 1) {
        string pdu = StringUtils::StringToHex(indexer->GetPdu());
        pdus->push_back(pdu);
    } else {
        int8_t count = 0;
        int8_t notNullPart = messageCount;
        string key = indexer->GetEraseRefId();
        auto iter = receiveMap_.find(key);
        for (; iter != receiveMap_.end(); ++iter) {
            ++count;
            string pdu = StringUtils::StringToHex(iter->second->GetPdu());
            pdus->emplace(pdus->begin() + iter->second->GetMsgSeqId() - pduPosOffset_, pdu);
            if (iter->second->GetPdu().size() == 0) {
                --notNullPart;
            }
        }
        if (count != messageCount) {
            return;
        }
        if (pdus->empty()) {
            HILOG_INFO("pdu list count is 0");
            return;
        }
        if (notNullPart != messageCount) {
            HILOG_INFO("pdu list contains is null");
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
        return;
    }
    Want want;
    want.SetAction("ohos.action.telephonySmsReceiveFinished");
    want.SetParam("pdu", *pdus);
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(msgReceiveCode_);
    data.SetData("SMS MSG RECEIVE");
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    if (!publishResult) {
        HILOG_ERROR("SendBroadcast PublishBroadcastEvent result fail");
    }
}
} // namespace SMS
} // namespace OHOS