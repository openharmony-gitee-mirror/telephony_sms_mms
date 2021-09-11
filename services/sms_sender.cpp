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

#include "sms_sender.h"

#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
using namespace std::chrono;
SmsSender::SmsSender(const shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId,
    function<void(shared_ptr<SmsSendIndexer>)> &sendRetryFun)
    : AppExecFwk::EventHandler(runner), slotId_(slotId), sendRetryFun_(sendRetryFun)
{}

SmsSender::~SmsSender() {}

void SmsSender::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is nullptr");
        return;
    }
    TELEPHONY_LOGD("SmsSender::ProcessEvent");
    shared_ptr<SmsSendIndexer> smsIndexer = nullptr;
    int eventId = event->GetInnerEventId();
    switch (eventId) {
        case ObserverHandler::RADIO_SEND_SMS:
        case ObserverHandler::RADIO_SEND_IMS_GSM_SMS:
        case ObserverHandler::RADIO_SEND_SMS_EXPECT_MORE: {
            smsIndexer = FindCacheMapAndTransform(event);
            HandleMessageResponse(smsIndexer);
            break;
        }
        case MSG_SMS_RETRY_DELIVERY: {
            smsIndexer = event->GetSharedObject<SmsSendIndexer>();
            sendRetryFun_(smsIndexer);
            break;
        }
        case ObserverHandler::RADIO_SMS_STATUS: {
            StatusReportAnalysis(event);
            break;
        }
        default:
            break;
    }
}

void SmsSender::HandleMessageResponse(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        TELEPHONY_LOGE("smsIndexer is nullptr");
        return;
    }
    if (!SendCacheMapEraseItem(smsIndexer->GetMsgRefId64Bit())) {
        TELEPHONY_LOGE("SendCacheMapEraseItem fail !!!!!");
    }
    SendCacheMapTimeoutCheck();
    if (!smsIndexer->GetIsFailure()) {
        if (smsIndexer->GetDeliveryCallback() != nullptr) {
            // Expecting a status report.  Add it to the list.
            if (reportList_.size() > MAX_REPORT_LIST_LIMIT) {
                reportList_.pop_front();
            }
            reportList_.push_back(smsIndexer);
        }
        SendMessageSucceed(smsIndexer);
    } else {
        if (smsIndexer->GetPsResendCount() > 0) {
            smsIndexer->SetCsResendCount(MAX_SEND_RETRIES);
        }
        HandleResend(smsIndexer);
    }
}

void SmsSender::SendMessageSucceed(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        TELEPHONY_LOGE("SendMessageSucceed but smsIndexer drop!");
        return;
    }
    bool isLastPart = false;
    std::shared_ptr<uint8_t> unSentCellCount = smsIndexer->GetUnSentCellCount();
    if (unSentCellCount != nullptr) {
        (*unSentCellCount) = (*unSentCellCount) - 1;
        if ((*unSentCellCount) == 0) {
            isLastPart = true;
        }
    }
    if (isLastPart) {
        ISendShortMessageCallback::SmsSendResult messageType = ISendShortMessageCallback::SEND_SMS_SUCCESS;
        if (smsIndexer->GetHasCellFailed() != nullptr) {
            if (*smsIndexer->GetHasCellFailed()) {
                messageType = ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN;
            }
        }
        sptr<ISendShortMessageCallback> sendCallback = smsIndexer->GetSendCallback();
        if (sendCallback != nullptr) {
            sendCallback->OnSmsSendResult(messageType);
        }
    }
}

void SmsSender::SendMessageFailed(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        TELEPHONY_LOGE("smsIndexer is nullptr");
        return;
    }
    shared_ptr<bool> hasCellFailed = smsIndexer->GetHasCellFailed();
    if (hasCellFailed != nullptr) {
        *hasCellFailed = true;
    }
    bool isLastPart = false;
    std::shared_ptr<uint8_t> unSentCellCount = smsIndexer->GetUnSentCellCount();
    if (unSentCellCount == nullptr) {
        TELEPHONY_LOGE("unSentCellCount is nullptr");
        return;
    }
    (*unSentCellCount) = (*unSentCellCount) - 1;
    if ((*unSentCellCount) == 0) {
        isLastPart = true;
    }
    if (isLastPart) {
        // save to db and update state
        sptr<ISendShortMessageCallback> sendCallback = smsIndexer->GetSendCallback();
        if (sendCallback != nullptr) {
            sendCallback->OnSmsSendResult(ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        }
    }
}

bool SmsSender::GetImsDomain() const
{
    return (netDomainType_ == NetDomainType::NET_DOMAIN_IMS) ? true : false;
}

NetWorkType SmsSender::GetNetWorkType() const
{
    return netWorkType_;
}

NetDomainType SmsSender::GetNetDomainType() const
{
    return netDomainType_;
}

void SmsSender::SendResultCallBack(
    const std::shared_ptr<SmsSendIndexer> &indexer, ISendShortMessageCallback::SmsSendResult result)
{
    if (indexer != nullptr && indexer->GetSendCallback() != nullptr) {
        indexer->GetSendCallback()->OnSmsSendResult(result);
    }
}

void SmsSender::SendResultCallBack(
    const sptr<ISendShortMessageCallback> &sendCallback, ISendShortMessageCallback::SmsSendResult result)
{
    if (sendCallback != nullptr) {
        sendCallback->OnSmsSendResult(result);
    }
}

void SmsSender::SendCacheMapTimeoutCheck()
{
    std::lock_guard<std::mutex> guard(sendCacheMapMutex_);
    system_clock::duration timePoint = system_clock::now().time_since_epoch();
    seconds sec = duration_cast<seconds>(timePoint);
    long timeStamp = sec.count();
    auto item = sendCacheMap_.begin();
    while (item != sendCacheMap_.end()) {
        auto iter = item++;
        shared_ptr<SmsSendIndexer> &indexer = iter->second;
        if (indexer == nullptr || (timeStamp - indexer->GetTimeStamp()) > EXPIRED_TIME) {
            sendCacheMap_.erase(iter);
        }
    }
}

bool SmsSender::SendCacheMapLimitCheck(const sptr<ISendShortMessageCallback> &sendCallback)
{
    std::lock_guard<std::mutex> guard(sendCacheMapMutex_);
    if (sendCacheMap_.size() > MSG_QUEUE_LIMIT) {
        if (sendCallback != nullptr) {
            sendCallback->OnSmsSendResult(ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
            return true;
        }
    }
    return false;
}

bool SmsSender::SendCacheMapAddItem(int64_t id, const std::shared_ptr<SmsSendIndexer> &smsIndexer)
{
    std::lock_guard<std::mutex> guard(sendCacheMapMutex_);
    if (smsIndexer != nullptr) {
        auto result = sendCacheMap_.emplace(id, smsIndexer);
        return result.second;
    }
    return false;
}

bool SmsSender::SendCacheMapEraseItem(int64_t id)
{
    std::lock_guard<std::mutex> guard(sendCacheMapMutex_);
    return (sendCacheMap_.erase(id) != 0);
}

std::shared_ptr<SmsSendIndexer> SmsSender::FindCacheMapAndTransform(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is nullptr");
        return nullptr;
    }
    std::shared_ptr<SmsSendIndexer> smsIndexer = nullptr;
    std::lock_guard<std::mutex> guard(sendCacheMapMutex_);
    std::shared_ptr<HRilRadioResponseInfo> res = event->GetSharedObject<HRilRadioResponseInfo>();
    if (res != nullptr) {
        auto iter = sendCacheMap_.find(res->flag);
        if (iter != sendCacheMap_.end()) {
            smsIndexer = iter->second;
            if (smsIndexer == nullptr) {
                TELEPHONY_LOGE("smsIndexer is nullptr");
                return nullptr;
            }
            smsIndexer->SetErrorCode(ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
            smsIndexer->SetMsgRefId64Bit(res->flag);
            smsIndexer->SetIsFailure(true);
        }
    } else {
        std::shared_ptr<SendSmsResultInfo> info = event->GetSharedObject<SendSmsResultInfo>();
        if (info == nullptr) {
            return nullptr;
        }
        auto iter = sendCacheMap_.find(info->flag);
        if (iter != sendCacheMap_.end()) {
            TELEPHONY_LOGD("msgRef = %{public}d", info->msgRef);
            smsIndexer = iter->second;
            if (smsIndexer == nullptr) {
                TELEPHONY_LOGE("smsIndexer is nullptr");
                return nullptr;
            }
            smsIndexer->SetMsgRefId((uint8_t)info->msgRef);
            smsIndexer->SetAckPdu(std::move(StringUtils::HexToByteVector(info->pdu)));
            if (info->errCode != 0) {
                smsIndexer->SetIsFailure(true);
            }
            smsIndexer->SetErrorCode(info->errCode);
            smsIndexer->SetMsgRefId64Bit(info->flag);
        }
    }
    return smsIndexer;
}

void SmsSender::HandleResend(const std::shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        TELEPHONY_LOGE("smsIndexer is nullptr");
        return;
    }
    if (!GetImsDomain()) {
        sptr<ISendShortMessageCallback> sendShortMessageCallback = smsIndexer->GetSendCallback();
        if (sendShortMessageCallback != nullptr) {
            sendShortMessageCallback->OnSmsSendResult(
                (ISendShortMessageCallback::SmsSendResult)(smsIndexer->GetErrorCode()));
        }
        // resending mechanism
    } else if ((smsIndexer->GetErrorCode() == HRIL_ERR_CMD_SEND_FAILURE) &&
        smsIndexer->GetCsResendCount() < MAX_SEND_RETRIES) {
        smsIndexer->SetCsResendCount(smsIndexer->GetCsResendCount() + 1);
        SendEvent(MSG_SMS_RETRY_DELIVERY, smsIndexer, DELAY_MAX_TIME_MSCE);
    } else {
        SendMessageFailed(smsIndexer);
    }
}

uint8_t SmsSender::GetMsgRef8Bit()
{
    return ++msgRef8bit_;
}

int64_t SmsSender::GetMsgRef64Bit()
{
    return ++msgRef64bit_;
}

std::shared_ptr<Core> SmsSender::GetCore() const
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        return core;
    }
    return nullptr;
}
} // namespace Telephony
} // namespace OHOS