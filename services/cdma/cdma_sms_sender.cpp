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

#include "cdma_sms_sender.h"

#include <numeric>

#include "cdma_sms_message.h"
#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
CdmaSmsSender::CdmaSmsSender(const shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId,
    function<void(shared_ptr<SmsSendIndexer>)> sendRetryFun)
    : SmsSender(runner, slotId, sendRetryFun)
{}

CdmaSmsSender::~CdmaSmsSender() {}

void CdmaSmsSender::TextBasedSmsDelivery(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{}

void CdmaSmsSender::DataBasedSmsDelivery(const string &desAddr, const string &scAddr, int32_t port,
    const uint8_t *data, uint32_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{}

void CdmaSmsSender::StatusReportAnalysis(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is nullptr");
        return;
    }
    std::shared_ptr<SmsReceiveIndexer> statusInfo = event->GetSharedObject<SmsReceiveIndexer>();
    if (statusInfo == nullptr) {
        return;
    }
    if (statusInfo->GetPdu().empty()) {
        TELEPHONY_LOGE("gsm_sms_sender: StatusReportAnalysis statusInfo->pdu == 0");
        return;
    }
    std::shared_ptr<Core> core = GetCore();
    if (core == nullptr) {
        TELEPHONY_LOGE("gsm_sms_sender: StatusReportAnalysis core nullptr");
        return;
    }
    std::string pdu = StringUtils::StringToHex(statusInfo->GetPdu());
    std::shared_ptr<CdmaSmsMessage> message = CdmaSmsMessage::CreateMessage(pdu);
    if (message == nullptr) {
        TELEPHONY_LOGE("gsm_sms_sender: StatusReportAnalysis message nullptr");
        return;
    }
    bool isSent = false;
    auto oldIndexer = reportList_.begin();
    while (oldIndexer != reportList_.end()) {
        auto iter = oldIndexer++;
        if (*iter != nullptr) {
            TELEPHONY_LOGE(
                "StatusReportAnalysis %{public}d %{public}d", message->GetMsgRef(), (*iter)->GetMsgRefId());
            if (message->GetMsgRef() == (*iter)->GetMsgRefId()) {
                // save the message to db, or updata to db msg state(success or failed)
                if (!isSent) {
                    core->SendSmsAck(true, AckIncomeCause::SMS_ACK_PROCESSED,
                        AppExecFwk::InnerEvent::Pointer(nullptr, nullptr));
                    sptr<IDeliveryShortMessageCallback> deliveryCallback = (*iter)->GetDeliveryCallback();
                    if (deliveryCallback != nullptr) {
                        std::string ackpdu = StringUtils::StringToHex(message->GetRawPdu());
                        deliveryCallback->OnSmsDeliveryResult(StringUtils::ToUtf16(ackpdu));
                        TELEPHONY_LOGE("gsm_sms_sender: StatusReportAnalysis %{public}s", pdu.c_str());
                    }
                    isSent = true;
                }
                reportList_.erase(iter);
            }
        }
    }
}

void CdmaSmsSender::SendSmsToRil(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        TELEPHONY_LOGE("cdma_sms_sender: SendSms smsIndexer nullptr");
        return;
    }
    std::shared_ptr<Core> core = GetCore();
    if (core == nullptr) {
        TELEPHONY_LOGE("cdma_sms_sender: SendSms core nullptr");
        return;
    }
    int64_t refId = GetMsgRef64Bit();
    if (!SendCacheMapAddItem(refId, smsIndexer)) {
        TELEPHONY_LOGE("SendCacheMapAddItem Error!!");
    }
    if (netDomainType_ != NET_DOMAIN_IMS && (smsIndexer->GetPsResendCount() == 0)) {
        smsIndexer->SetCsResendCount(smsIndexer->GetCsResendCount() + 1);
        auto reply = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SEND_SMS, refId);
        reply->SetOwner(shared_from_this());
    } else {
        TELEPHONY_LOGD("ims network domain not support!");
        smsIndexer->SetPsResendCount(smsIndexer->GetPsResendCount() + 1);
    }
}

void CdmaSmsSender::Init() {}

void CdmaSmsSender::ReceiveStatusReport(const std::shared_ptr<SmsReceiveIndexer> &smsIndexer)
{
    SendEvent(ObserverHandler::RADIO_SMS_STATUS, smsIndexer);
}
} // namespace Telephony
} // namespace OHOS