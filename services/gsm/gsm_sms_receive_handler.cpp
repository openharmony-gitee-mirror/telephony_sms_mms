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
#include "sms_receive_indexer.h"
#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
GsmSmsReceiveHandler::GsmSmsReceiveHandler(const shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : SmsReceiveHandler(runner, slotId)
{}

GsmSmsReceiveHandler::~GsmSmsReceiveHandler()
{
    UnRegisterHandler();
}

void GsmSmsReceiveHandler::Init()
{
    if (!RegisterHandler()) {
        TELEPHONY_LOGI("GsmSmsSender::Init Register RADIO_SMS_STATUS fail.");
    }
    smsCbRunner_ = AppExecFwk::EventRunner::Create("GsmSmsCbHandler" + to_string(slotId_));
    if (smsCbRunner_ == nullptr) {
        TELEPHONY_LOGE("failed to create GsmSmsCbHandler");
        return;
    }
    smsCbHandler_ = std::make_shared<GsmSmsCbHandler>(smsCbRunner_, slotId_);
    if (smsCbHandler_ == nullptr) {
        TELEPHONY_LOGE("failed to create GsmSmsCbHandler");
        return;
    }
    smsCbHandler_->Init();
    smsCbRunner_->Run();
    TELEPHONY_LOGI("smsCbHandler_->Run().");
}

bool GsmSmsReceiveHandler::RegisterHandler()
{
    bool ret = false;
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        TELEPHONY_LOGI("GsmSmsReceiveHandler::RegisteHandler Register RADIO_GSM_SMS ok.");
        core->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_GSM_SMS, nullptr);
        ret = true;
    }
    return ret;
}

void GsmSmsReceiveHandler::UnRegisterHandler()
{
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        TELEPHONY_LOGD("SmsReceiveHandler::UnRegisterHandler::slotId= %{public}d", slotId_);
        core->UnRegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_GSM_SMS);
    }
    if (smsCbHandler_ != nullptr) {
        smsCbRunner_->Stop();
    }
}

int32_t GsmSmsReceiveHandler::HandleSmsByType(const shared_ptr<SmsBaseMessage> &smsBaseMessage)
{
    TELEPHONY_LOGD("GsmSmsReceiveHandler:: HandleSmsByType");
    if (smsBaseMessage == nullptr) {
        return AckIncomeCause::SMS_ACK_UNKNOWN_ERROR;
    }
    GsmSmsMessage *message = (GsmSmsMessage *)smsBaseMessage.get();
    if (message->IsSpecialMessage()) {
        TELEPHONY_LOGD("GsmSmsReceiveHandler:: IsSpecialMessage");
        return AckIncomeCause::SMS_ACK_RESULT_OK;
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
            return AckIncomeCause::SMS_ACK_UNKNOWN_ERROR;
        }
        indexer = make_shared<SmsReceiveIndexer>(message->GetRawPdu(), message->GetScTimestamp(),
            message->GetDestPort(), message->GetGsm(), message->GetOriginatingAddress(),
            message->GetVisibleOriginatingAddress(), smsConcat->msgRef, smsConcat->seqNum, smsConcat->totalSeg,
            false, message->GetVisibleMessageBody());
    }
    // add messages to the database
    if (indexer == nullptr) {
        return AckIncomeCause::SMS_ACK_UNKNOWN_ERROR;
    }

    if (indexer->GetIsText() && IsRepeatedMessagePart(indexer)) {
        return AckIncomeCause::SMS_ACK_REPEATED_ERROR;
    }
    string key =
        indexer->GetOriginatingAddress() + to_string(indexer->GetMsgRefId()) + to_string(indexer->GetMsgCount());
    indexer->SetEraseRefId(key);
    auto ret = receiveMap_.emplace(key, indexer);
    if (!ret->second) {
        return AckIncomeCause::SMS_ACK_UNKNOWN_ERROR;
    }
    CombineMessagePart(indexer);
    return AckIncomeCause::SMS_ACK_RESULT_OK;
}

void GsmSmsReceiveHandler::ReplySmsToSmsc(int result, const shared_ptr<SmsBaseMessage> &response)
{
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        auto reply = AppExecFwk::InnerEvent::Get(SMS_EVENT_NEW_SMS_REPLY, response);
        reply->SetOwner(shared_from_this());
        TELEPHONY_LOGD("GsmSmsReceiveHandler::ReplySmsToSmsc ackResult %{public}d", result);
        core->SendSmsAck(result == AckIncomeCause::SMS_ACK_RESULT_OK, result, reply);
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
} // namespace Telephony
} // namespace OHOS