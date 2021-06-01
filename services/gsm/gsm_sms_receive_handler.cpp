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
#include "gsm_sms_receive_handler.h"
#include <map>
#include "gsm_sms_message.h"
#include "sms_base_message.h"
#include "sms_hilog_wrapper.h"
#include "sms_receive_indexer.h"
#include "string_utils.h"
namespace OHOS {
namespace SMS {
using namespace std;
GsmSmsReceiveHandler::GsmSmsReceiveHandler(const shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : SmsReceiveHandler(runner, slotId)
{}

GsmSmsReceiveHandler::~GsmSmsReceiveHandler() {}

void GsmSmsReceiveHandler::RegisterHandler()
{
    rilManager_ = PhoneManager ::GetInstance().phone_[1]->rilManager_;
    if (rilManager_ != nullptr) {
        PhoneManager ::GetInstance().phone_[1]->rilManager_->RegisterPhoneNotify(
            shared_from_this(), ObserverHandler::RADIO_GSM_SMS, nullptr);
        HILOG_DEBUG("SmsReceiveHandler::RegisterHandler::slotid= %{public}d", slotId_);
    }
}

void GsmSmsReceiveHandler::UnRegisterHandler()
{
    if (rilManager_ != nullptr) {
        HILOG_DEBUG("SmsReceiveHandler::UnRegisterHandler::slotid= %{public}d", slotId_);
        rilManager_->UnRegisterPhoneNotify(ObserverHandler::RADIO_GSM_SMS);
    }
}

int32_t GsmSmsReceiveHandler::HandleSmsByType(const shared_ptr<SmsBaseMessage> &smsBaseMessage)
{
    HILOG_DEBUG("GsmSmsReceiveHandler:: HandleSmsByType");
    if (smsBaseMessage == nullptr) {
        return SMS_RESULT_UNKOWN_ERROR;
    }
    GsmSmsMessage *message = (GsmSmsMessage *)smsBaseMessage.get();
    if (message->IsSpecialMessage()) {
        HILOG_DEBUG("GsmSmsReceiveHandler:: IsSpecialMessage");
        return SMS_RESULT_PROCESSED;
    }
    // Encapsulate key information to Tracker
    shared_ptr<SmsReceiveIndexer> indexer;
    if (!message->IsConcatMsg()) {
        indexer = make_shared<SmsReceiveIndexer>(message->GetRawPdu(), message->GetScTimestamp(),
            message->GetDestPort(), message->GetGsm(), false, message->GetOriginatingAddress(),
            message->GetVisibleOriginatingAddress(), message->GetVisibleMessageBody());
    } else {
        std::shared_ptr<SmsConcat> smsConcat = message->GetConcatMsg();
        if (smsConcat == nullptr) {
            return SMS_RESULT_UNKOWN_ERROR;
        }
        indexer = make_shared<SmsReceiveIndexer>(message->GetRawPdu(), message->GetScTimestamp(),
            message->GetDestPort(), message->GetGsm(), message->GetOriginatingAddress(),
            message->GetVisibleOriginatingAddress(), smsConcat->msgRef, smsConcat->seqNum, smsConcat->totalSeg,
            false, message->GetVisibleMessageBody());
    }
    // add messages to the database
    if (indexer == nullptr) {
        return SMS_RESULT_UNKOWN_ERROR;
    }

    if (indexer->GetIsText() && IsRepeatedMessagePart(indexer)) {
        return SMS_RESULT_REPEATED_ERROR;
    }
    string key =
        indexer->GetOriginatingAddress() + to_string(indexer->GetMsgRefId()) + to_string(indexer->GetMsgCount());
    indexer->SetEraseRefId(key);
    auto ret = receiveMap_.emplace(key, indexer);
    if (!ret->second) {
        return SMS_RESULT_UNKOWN_ERROR;
    }
    CombineMessagePart(indexer);
    return SMS_RESULT_PROCESSED;
}

void GsmSmsReceiveHandler::ReplySmsToSmsc(int result, const shared_ptr<SmsBaseMessage> &response)
{
    if (rilManager_ != nullptr) {
        int32_t ackResult = AckIncomeCause::SMS_ACK_UNKOWN_ERROR;
        switch (result) {
            case SMS_RESULT_OK:
                return;
            case SMS_RESULT_PROCESSED:
                ackResult = AckIncomeCause::SMS_ACK_RESULT_OK;
                break;
            case SMS_RESULT_OUT_OF_MEMORY:
                ackResult = AckIncomeCause::SMS_ACK_OUT_OF_MEMORY;
                break;
            case SMS_RESULT_UNKOWN_ERROR:
            default:
                ackResult = AckIncomeCause::SMS_ACK_UNKOWN_ERROR;
                break;
        }
        auto reply = AppExecFwk::InnerEvent::Get(SMS_EVENT_NEW_SMS_REPLY, response);
        reply->SetOwner(shared_from_this());
        HILOG_DEBUG("GsmSmsReceiveHandler::ReplySmsToSmsc ackResult %{public}d", ackResult);
        rilManager_->SendSmsAck(result == SMS_RESULT_PROCESSED, ackResult, reply);
    }
}

shared_ptr<SmsBaseMessage> GsmSmsReceiveHandler::TransformMessageInfo(const shared_ptr<SmsMessageInfo> &info)
{
    std::shared_ptr<SmsBaseMessage> baseMessage = nullptr;
    if (info == nullptr) {
        return baseMessage;
    }
    std::string pdu = StringUtils::StringToHex(info->pdu);
    baseMessage = GsmSmsMessage::CreateMessage(pdu);
    return baseMessage;
}
} // namespace SMS
} // namespace OHOS