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

#include "sms_send_manager.h"

#include <functional>
#include <memory.h>

#include "gsm_sms_tpdu_codec.h"
#include "i_sms_service_interface.h"
#include "sms_receive_manager.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
SmsSendManager::SmsSendManager(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : AppExecFwk::EventHandler(runner), slotId_(slotId)
{}

SmsSendManager::~SmsSendManager()
{
    UnRegisterHandler();
}

void SmsSendManager::RegisterHandler()
{
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        TELEPHONY_LOGI("SmsSendManager::RegisterHandler Ok.");
        core->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_ON, nullptr);
        core->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_NETWORK_STATE, nullptr);
        core->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_IMS_NETWORK_STATE_CHANGED, nullptr);
        GetRadioState();
    }
}

void SmsSendManager::UnRegisterHandler()
{
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        core->UnRegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_ON);
        core->UnRegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_NETWORK_STATE);
        core->UnRegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_IMS_NETWORK_STATE_CHANGED);
    }
}

void SmsSendManager::Init()
{
    gsmSmsSendRunner_ = AppExecFwk::EventRunner::Create("gsmSmsSenderLoop" + to_string(slotId_));
    if (gsmSmsSendRunner_ == nullptr) {
        TELEPHONY_LOGE("failed to create GsmSenderEventRunner");
        return;
    }
    gsmSmsSender_ = std::make_shared<GsmSmsSender>(
        gsmSmsSendRunner_, slotId_, bind(&SmsSendManager::RetriedSmsDelivery, this, placeholders::_1));
    if (gsmSmsSender_ == nullptr) {
        TELEPHONY_LOGE("failed to create GsmSmsSender");
        return;
    }
    gsmSmsSender_->Init();
    gsmSmsSendRunner_->Run();

    cdmaSmsSendRunner_ = AppExecFwk::EventRunner::Create("cdmaSmsSenderLoop" + to_string(slotId_));
    if (cdmaSmsSendRunner_ == nullptr) {
        TELEPHONY_LOGE("failed to create CdmaSenderEventRunner");
        return;
    }
    cdmaSmsSender_ = std::make_shared<CdmaSmsSender>(
        cdmaSmsSendRunner_, slotId_, bind(&SmsSendManager::RetriedSmsDelivery, this, placeholders::_1));
    if (cdmaSmsSender_ == nullptr) {
        TELEPHONY_LOGE("failed to create CdmaSmsSender");
        return;
    }
    cdmaSmsSendRunner_->Run();
    RegisterHandler();
    TELEPHONY_LOGI("Init SmsSenderHandler start successfully.");
}

void SmsSendManager::TextBasedSmsDelivery(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    if (gsmSmsSender_ == nullptr || cdmaSmsSender_ == nullptr) {
        SmsSender::SendResultCallBack(sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        TELEPHONY_LOGE("gsmSmsSender or cdmaSmsSender nullptr error.");
        return;
    }

    NetWorkType netWorkType = GetNetWorkType();
    if (netWorkType == NetWorkType::NET_TYPE_GSM) {
        gsmSmsSender_->TextBasedSmsDelivery(desAddr, scAddr, text, sendCallback, deliveryCallback);
    } else if (netWorkType == NetWorkType::NET_TYPE_CDMA) {
        cdmaSmsSender_->TextBasedSmsDelivery(desAddr, scAddr, text, sendCallback, deliveryCallback);
    } else {
        SmsSender::SendResultCallBack(
            sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE);
        TELEPHONY_LOGI("network unknown send error.");
    }
}

void SmsSendManager::DataBasedSmsDelivery(const string &desAddr, const string &scAddr, uint16_t port,
    const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    if (gsmSmsSender_ == nullptr || cdmaSmsSender_ == nullptr) {
        SmsSender::SendResultCallBack(sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        TELEPHONY_LOGE("gsmSmsSender or cdmaSmsSender nullptr error.");
        return;
    }

    NetWorkType netWorkType = GetNetWorkType();
    if (netWorkType == NetWorkType::NET_TYPE_GSM) {
        gsmSmsSender_->DataBasedSmsDelivery(desAddr, scAddr, port, data, dataLen, sendCallback, deliveryCallback);
    } else if (netWorkType == NetWorkType::NET_TYPE_CDMA) {
        cdmaSmsSender_->DataBasedSmsDelivery(desAddr, scAddr, port, data, dataLen, sendCallback, deliveryCallback);
    } else {
        SmsSender::SendResultCallBack(
            sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE);
        TELEPHONY_LOGI("network unknown send error.");
    }
}

void SmsSendManager::RetriedSmsDelivery(const shared_ptr<SmsSendIndexer> smsIndexer)
{
    if (smsIndexer == nullptr) {
        TELEPHONY_LOGE("smsIndexer is nullptr");
        return;
    }
    if (gsmSmsSender_ == nullptr || cdmaSmsSender_ == nullptr) {
        TELEPHONY_LOGE("gsmSmsSender or cdmaSmsSender nullptr error.");
        return;
    }

    NetWorkType oldNetWorkType = smsIndexer->GetNetWorkType();
    NetWorkType newNetWorkType = GetNetWorkType();
    TELEPHONY_LOGE("oldNetWorkType = %{public}d newNetWorkType = %{public}d", oldNetWorkType, newNetWorkType);
    if (oldNetWorkType != newNetWorkType) {
        smsIndexer->SetNetWorkType(newNetWorkType);
        shared_ptr<SmsSendIndexer> indexer = smsIndexer;
        switch (newNetWorkType) {
            case NetWorkType::NET_TYPE_CDMA:
                if (smsIndexer->GetIsText()) {
                    cdmaSmsSender_->ResendTextDelivery(indexer);
                } else {
                    cdmaSmsSender_->ResendDataDelivery(indexer);
                }
                break;
            case NetWorkType::NET_TYPE_GSM:
                if (smsIndexer->GetIsText()) {
                    gsmSmsSender_->ResendTextDelivery(indexer);
                } else {
                    gsmSmsSender_->ResendDataDelivery(indexer);
                }
                break;
            default:
                SmsSender::SendResultCallBack(
                    smsIndexer, ISendShortMessageCallback::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE);
                TELEPHONY_LOGI("network unknown send error.");
                break;
        }
        return;
    }

    if (newNetWorkType == NetWorkType::NET_TYPE_GSM) {
        gsmSmsSender_->SendSmsToRil(smsIndexer);
    } else if (newNetWorkType == NetWorkType::NET_TYPE_CDMA) {
        cdmaSmsSender_->SendSmsToRil(smsIndexer);
    } else {
        SmsSender::SendResultCallBack(smsIndexer, ISendShortMessageCallback::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE);
        TELEPHONY_LOGI("Network Unknown.");
    }
}

NetWorkType SmsSendManager::GetNetWorkType()
{
    NetWorkType netWorkType = NetWorkType::NET_TYPE_GSM;
    if (netDomainType_ == NetDomainType::NET_DOMAIN_CS) {
        netWorkType = csNetWorkType_;
    } else if (netDomainType_ == NetDomainType::NET_DOMAIN_IMS) {
        netWorkType = imsNetWorkType_;
    }
    return netWorkType;
}

void SmsSendManager::HandlerRadioState(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("HandlerRadioState event == nullptr");
        return;
    }
    int64_t radioState = event->GetParam();
    switch (radioState) {
        case OHOS::Telephony::ModemPowerState::CORE_SERVICE_POWER_ON:
            TELEPHONY_LOGI("SmsSendManager::HandlerRadioState RADIO_ON");
            netDomainType_ = NetDomainType::NET_DOMAIN_CS;
            csNetWorkType_ = NetWorkType::NET_TYPE_GSM;
            break;
        case OHOS::Telephony::ModemPowerState::CORE_SERVICE_POWER_NOT_AVAILABLE:
        case OHOS::Telephony::ModemPowerState::CORE_SERVICE_POWER_OFF:
        default:
            TELEPHONY_LOGI("SmsSendManager::HandlerRadioState RADIO_OFF");
            netDomainType_ = NetDomainType::NET_DOMAIN_UNKNOWN;
            csNetWorkType_ = NetWorkType::NET_TYPE_UNKNOWN;
            break;
    }
}

void SmsSendManager::GetRadioState()
{
    std::shared_ptr<Core> core = GetCore();
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_GET_STATUS);
    if (event != nullptr && core != nullptr) {
        event->SetOwner(shared_from_this());
        core->GetRadioStatus(event);
    }
}

void SmsSendManager::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SmsSendManager::ProcessEvent event == nullptr");
        return;
    }

    int eventId = 0;
    eventId = event->GetInnerEventId();
    TELEPHONY_LOGI("SmsSendManager::ProcessEvent Handler Rec%{public}d", eventId);
    switch (eventId) {
        case ObserverHandler::RADIO_ON:
        case ObserverHandler::RADIO_NETWORK_STATE:
        case ObserverHandler::RADIO_IMS_NETWORK_STATE_CHANGED:
            GetRadioState();
            break;
        case ObserverHandler::RADIO_RIL_IMS_REGISTRATION_STATE:
            break;
        case ObserverHandler::RADIO_GET_STATUS:
            HandlerRadioState(event);
            break;
        default:
            break;
    }
}

std::shared_ptr<Core> SmsSendManager::GetCore() const
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        return core;
    }
    return nullptr;
}
} // namespace Telephony
} // namespace OHOS
