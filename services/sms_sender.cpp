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
#include "sms_hilog_wrapper.h"
#include "string_utils.h"
namespace OHOS {
namespace SMS {
using namespace std;
using namespace std::chrono;
SmsSender::SmsSender(const shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId,
    function<void(shared_ptr<SmsSendIndexer>)> &sendRetryFun)
    : AppExecFwk::EventHandler(runner), slotId_(slotId), sendRetryFun_(sendRetryFun)
{}

SmsSender::~SmsSender() {}

void SmsSender::Init()
{
    HILOG_INFO("SmsSender::Init.");
}

void SmsSender::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    shared_ptr<SmsSendIndexer> smsIndexer = nullptr;
    int eventId = event->GetInnerEventId();
    switch (eventId) {
        case ObserverHandler::RADIO_SEND_SMS:
        case ObserverHandler::RADIO_SEND_IMS_GSM_SMS:
        case ObserverHandler::RADIO_SEND_SMS_EXPECT_MORE: {
            smsIndexer = FindCacheMapAndTransform(event->GetSharedObject<SendSmsResultInfo>());
            HandleMessageResponse(smsIndexer);
            break;
        }
        case MSG_SMS_RETRY_DELIVERY: {
            smsIndexer = event->GetSharedObject<SmsSendIndexer>();
            sendRetryFun_(smsIndexer);
            break;
        }
        case ObserverHandler::RADIO_SMS_STATUS: {
            smsIndexer = FindCacheMapAndTransform(event->GetSharedObject<SendSmsResultInfo>());
            StatusReportAnalysis(smsIndexer);
            break;
        }
        default:
            break;
    }
}

void SmsSender::HandleMessageResponse(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        return;
    }
    if (!SendCacheMapEraseItem(smsIndexer->GetMsgRefId64Bit())) {
        HILOG_ERROR("SendCacheMapEraseItem fail !!!!!");
    }
    SendCacheMapTimeoutCheck();
    if (!smsIndexer->GetIsFailure()) {
        if (smsIndexer->GetDeliveryCallback() != nullptr) {
            // Expecting a status report.  Add it to the list.
            if (reportList_.size() > maxReportListLimit_) {
                reportList_.pop_front();
            }
            reportList_.push_back(smsIndexer);
        }
        SendMessageSucceed(smsIndexer);
    } else {
        if (smsIndexer->GetPsResendCount() > 0) {
            smsIndexer->SetCsResendCount(maxSendRetries_);
        }
        HandleResend(smsIndexer);
    }
}

void SmsSender::SendMessageSucceed(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        HILOG_ERROR("SendMessageSucceed but smsIndexer drop!");
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
        return;
    }
    shared_ptr<bool> hasCellFailed = smsIndexer->GetHasCellFailed();
    if (hasCellFailed != nullptr) {
        *hasCellFailed = true;
    }
    bool isLastPart = false;
    std::shared_ptr<uint8_t> unSentCellCount = smsIndexer->GetUnSentCellCount();
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
        if (indexer == nullptr || (timeStamp - indexer->GetTimeStamp()) > expiredTime_) {
            sendCacheMap_.erase(iter);
        }
    }
}

bool SmsSender::SendCacheMapLimitCheck(const sptr<ISendShortMessageCallback> &sendCallback)
{
    std::lock_guard<std::mutex> guard(sendCacheMapMutex_);
    if (sendCacheMap_.size() > msgQueueLimit_) {
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

std::shared_ptr<SmsSendIndexer> SmsSender::FindCacheMapAndTransform(const std::shared_ptr<SendSmsResultInfo> &info)
{
    if (info == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::mutex> guard(sendCacheMapMutex_);
    auto iter = sendCacheMap_.find(info->flag);
    if (iter != sendCacheMap_.end()) {
        std::shared_ptr<SmsSendIndexer> sendIndexer = iter->second;
        sendIndexer->SetMsgRefId((uint8_t)info->msgRef);
        sendIndexer->SetAckPdu(std::move(StringUtils::HexToByteVector(info->pdu)));
        if (info->errCode != 0) {
            sendIndexer->SetIsFailure(true);
        }
        sendIndexer->SetErrorCode(info->errCode);
        sendIndexer->SetMsgRefId64Bit(info->flag);
        return sendIndexer;
    }
    return nullptr;
}

void SmsSender::HandleResend(const std::shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        return;
    }
    if (!GetImsDomain()) {
        sptr<ISendShortMessageCallback> sendShortMessageCallback = smsIndexer->GetSendCallback();
        if (sendShortMessageCallback != nullptr) {
            sendShortMessageCallback->OnSmsSendResult(
                (ISendShortMessageCallback::SmsSendResult)(smsIndexer->GetErrorCode()));
        }
        // resending mechanism
    } else if ((smsIndexer->GetErrorCode() == SMS_FAIL_RETRY) && smsIndexer->GetCsResendCount() < maxSendRetries_) {
        smsIndexer->SetCsResendCount(smsIndexer->GetCsResendCount() + 1);
        SendEvent(MSG_SMS_RETRY_DELIVERY, smsIndexer, delayMaxTimeMsce_);
    } else {
        SendMessageFailed(smsIndexer);
    }
}
} // namespace SMS
} // namespace OHOS