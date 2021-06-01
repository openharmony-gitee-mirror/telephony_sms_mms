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
#include "sms_hilog_wrapper.h"
#include "sms_receive_manager.h"
namespace OHOS {
namespace SMS {
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
    rilManager_ = PhoneManager ::GetInstance().phone_[slotId_]->rilManager_;
    if (rilManager_ == nullptr) {
        HILOG_INFO("SmsSendManager::RegisterHandler Get RilManager Fail.");
        return;
    }

    HILOG_INFO("SmsSendManager::RegisterHandler Ok.");
    rilManager_->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_ON, nullptr);
    rilManager_->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_NETWORK_STATE, nullptr);
    rilManager_->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_IMS_NETWORK_STATE_CHANGED, nullptr);
    HandlerRadioState();
}

void SmsSendManager::UnRegisterHandler()
{
    if (rilManager_ != nullptr) {
        rilManager_->UnRegisterPhoneNotify(ObserverHandler::RADIO_ON);
        rilManager_->UnRegisterPhoneNotify(ObserverHandler::RADIO_NETWORK_STATE);
        rilManager_->UnRegisterPhoneNotify(ObserverHandler::RADIO_IMS_NETWORK_STATE_CHANGED);
    }
}

void SmsSendManager::Init()
{
    gsmSmsSendRunner_ = AppExecFwk::EventRunner::Create("gsmSmsSenderLoop" + to_string(slotId_));
    if (gsmSmsSendRunner_ == nullptr) {
        HILOG_ERROR("failed to create GsmSenderEventRunner");
        return;
    }
    gsmSmsSender_ = std::make_shared<GsmSmsSender>(
        gsmSmsSendRunner_, slotId_, bind(&SmsSendManager::RetriedSmsDelivery, this, placeholders::_1));
    if (gsmSmsSender_ == nullptr) {
        HILOG_ERROR("failed to create GsmSmsSender");
        return;
    }
    gsmSmsSender_->Init();
    gsmSmsSendRunner_->Run();

    cdmaSmsSendRunner_ = AppExecFwk::EventRunner::Create("cdmaSmsSenderLoop" + to_string(slotId_));
    if (cdmaSmsSendRunner_ == nullptr) {
        HILOG_ERROR("failed to create CdmaSenderEventRunner");
        return;
    }
    cdmaSmsSender_ = std::make_shared<CdmaSmsSender>(
        cdmaSmsSendRunner_, slotId_, bind(&SmsSendManager::RetriedSmsDelivery, this, placeholders::_1));
    if (cdmaSmsSender_ == nullptr) {
        HILOG_ERROR("failed to create CdmaSmsSender");
        return;
    }
    cdmaSmsSendRunner_->Run();
    RegisterHandler();
    HILOG_INFO("Init SmsSenderHandler start successfully.");
}

void SmsSendManager::TextBasedSmsDelivery(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback, const sptr<IDeliveryShortMessageCallback> &deliverCallback)
{
    NetWorkType netWorkType = GetNetWorkType();
    if (netWorkType == NetWorkType::NET_TYPE_GSM) {
        gsmSmsSender_->TextBasedSmsDelivery(desAddr, scAddr, text, sendCallback, deliverCallback);
    } else if (netWorkType == NetWorkType::NET_TYPE_CDMA) {
        cdmaSmsSender_->TextBasedSmsDelivery(desAddr, scAddr, text, sendCallback, deliverCallback);
    }
}

void SmsSendManager::DataBasedSmsDelivery(const string &desAddr, const string &scAddr, uint16_t port,
    const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliverCallback)
{
    NetWorkType netWorkType = GetNetWorkType();
    if (netWorkType == NetWorkType::NET_TYPE_GSM) {
        gsmSmsSender_->DataBasedSmsDelivery(desAddr, scAddr, port, data, dataLen, sendCallback, deliverCallback);
    } else if (netWorkType == NetWorkType::NET_TYPE_CDMA) {
        cdmaSmsSender_->DataBasedSmsDelivery(desAddr, scAddr, port, data, dataLen, sendCallback, deliverCallback);
    }
}

void SmsSendManager::RetriedSmsDelivery(const shared_ptr<SmsSendIndexer> smsIndexer)
{
    if (smsIndexer == nullptr) {
        return;
    }
    NetWorkType oldNetWorkType = smsIndexer->GetNetWorkType();
    NetWorkType newNetWorkType = GetNetWorkType();
    if (oldNetWorkType != newNetWorkType) {
        switch (newNetWorkType) {
            case NetWorkType::NET_TYPE_CDMA:
                if (smsIndexer->GetIsText()) {
                    cdmaSmsSender_->TextBasedSmsDelivery(smsIndexer->GetDestAddr(), smsIndexer->GetSmcaAddr(),
                        smsIndexer->GetText(), smsIndexer->GetSendCallback(), smsIndexer->GetDeliveryCallback());
                } else {
                    const uint8_t *data = smsIndexer->GetData().data();
                    cdmaSmsSender_->DataBasedSmsDelivery(smsIndexer->GetDestAddr(), smsIndexer->GetSmcaAddr(),
                        smsIndexer->GetDestPort(), data, smsIndexer->GetData().size(),
                        smsIndexer->GetSendCallback(), smsIndexer->GetDeliveryCallback());
                }
                break;
            case NetWorkType::NET_TYPE_GSM:
                if (smsIndexer->GetIsText()) {
                    gsmSmsSender_->TextBasedSmsDelivery(smsIndexer->GetDestAddr(), smsIndexer->GetSmcaAddr(),
                        smsIndexer->GetText(), smsIndexer->GetSendCallback(), smsIndexer->GetDeliveryCallback());
                } else {
                    const uint8_t *data = smsIndexer->GetData().data();
                    gsmSmsSender_->DataBasedSmsDelivery(smsIndexer->GetDestAddr(), smsIndexer->GetSmcaAddr(),
                        smsIndexer->GetDestPort(), data, smsIndexer->GetData().size(),
                        smsIndexer->GetSendCallback(), smsIndexer->GetDeliveryCallback());
                }
                break;
            default:
                HILOG_INFO("Network Unknown.");
                if (smsIndexer->GetSendCallback() != nullptr) {
                    smsIndexer->GetSendCallback()->OnSmsSendResult(
                        ISendShortMessageCallback::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE);
                }
                break;
        }
        return;
    }

    smsIndexer->SetNetWorkType(newNetWorkType);
    NetWorkType netWorkType = GetNetWorkType();
    if (netWorkType == NetWorkType::NET_TYPE_GSM) {
        gsmSmsSender_->SendSmsToRil(smsIndexer);
    } else if (netWorkType == NetWorkType::NET_TYPE_CDMA) {
        cdmaSmsSender_->SendSmsToRil(smsIndexer);
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

void SmsSendManager::HandlerRadioState()
{
    if (rilManager_ != nullptr) {
        OHOS::ModemPowerState radioState = rilManager_->GetRadioState();
        switch (radioState) {
            case OHOS::ModemPowerState::CORE_SERVICE_POWER_ON:
                HILOG_INFO("SmsSendManager::HandlerRadioState RADIO_ON");
                netDomainType_ = NetDomainType::NET_DOMAIN_CS;
                csNetWorkType_ = NetWorkType::NET_TYPE_GSM;
                break;
            case OHOS::ModemPowerState::CORE_SERVICE_POWER_NOT_AVAILABLE:
            case OHOS::ModemPowerState::CORE_SERVICE_POWER_OFF:
            default: {
                HILOG_INFO("SmsSendManager::HandlerRadioState RADIO_OFF");
                netDomainType_ = NetDomainType::NET_DOMAIN_UNKOWN;
                csNetWorkType_ = NetWorkType::NET_TYPE_UNKOWN;
                break;
            }
        }
    }
}

void SmsSendManager::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    int eventId = 0;
    eventId = event->GetInnerEventId();
    HILOG_INFO("SmsSendManager::ProcessEvent Handler Rec%{pulbic}d", eventId);
    switch (eventId) {
        case ObserverHandler::RADIO_ON:
        case ObserverHandler::RADIO_NETWORK_STATE:
        case ObserverHandler::RADIO_IMS_NETWORK_STATE_CHANGED:
            HandlerRadioState();
            break;
        case ObserverHandler::RADIO_RIL_IMS_REGISTRATION_STATE:
            break;
        default:
            break;
    }
}
} // namespace SMS
} // namespace OHOS