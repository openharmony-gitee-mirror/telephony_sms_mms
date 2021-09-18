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

#include "short_message_manager.h"

#include <iostream>

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "telephony_log_wrapper.h"
#include "system_ability_definition.h"
#include "telephony_napi_common_error.h"

namespace OHOS {
namespace Telephony {
ShortMessageManager::ShortMessageManager()
{
    int32_t connectState = ConnectService();
    if (connectState == SERVICE_CONNECTED) {
        TELEPHONY_LOGD("ShortMessageManager::ShortMessageManager() sms_service connected\n");
    } else {
        TELEPHONY_LOGD("ShortMessageManager::ShortMessageManager() sms_service connect failed\n");
    }
}

ShortMessageManager::~ShortMessageManager() {}

bool ShortMessageManager::IsConnect()
{
    return smsServiceInterface_ != nullptr;
}

bool ShortMessageManager::SetDefaultSmsSlotId(int32_t slotId)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->SetDefaultSmsSlotId(slotId);
    }
    return false;
}
int32_t ShortMessageManager::GetDefaultSmsSlotId()
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->GetDefaultSmsSlotId();
    }
    return ERROR_SERVICE_UNAVAILABLE;
}

void ShortMessageManager::SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr,
    const std::u16string text, const sptr<ISendShortMessageCallback> &callback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    if (smsServiceInterface_ != nullptr) {
        smsServiceInterface_->SendMessage(slotId, desAddr, scAddr, text, callback, deliveryCallback);
        TELEPHONY_LOGD("execd smsServiceInterface_->SendMessage\n");
    }
}

void ShortMessageManager::SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr,
    uint16_t port, const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &callback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    if (smsServiceInterface_ != nullptr) {
        smsServiceInterface_->SendMessage(slotId, desAddr, scAddr, port, data, dataLen, callback, deliveryCallback);
    }
}

bool ShortMessageManager::SetScAddress(int32_t slotId, const std::u16string &scAddr)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->SetSmscAddr(slotId, scAddr);
    }
    return false;
}

std::u16string ShortMessageManager::GetScAddress(int32_t slotId)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->GetSmscAddr(slotId);
    }
    return u"";
}

bool ShortMessageManager::AddSimMessage(int32_t slotId, const std::u16string &smsc, const std::u16string &pdu,
    ISmsServiceInterface::SimMessageStatus status)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->AddSimMessage(slotId, smsc, pdu, status);
    }
    return false;
}

bool ShortMessageManager::DelSimMessage(int32_t slotId, uint32_t msgIndex)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->DelSimMessage(slotId, msgIndex);
    }
    return false;
}

bool ShortMessageManager::UpdateSimMessage(int32_t slotId, uint32_t msgIndex,
    ISmsServiceInterface::SimMessageStatus newStatus, const std::u16string &pdu, const std::u16string &smsc)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->UpdateSimMessage(slotId, msgIndex, newStatus, pdu, smsc);
    }
    return false;
}

std::vector<ShortMessage> ShortMessageManager::GetAllSimMessages(int32_t slotId)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->GetAllSimMessages(slotId);
    }
    std::vector<ShortMessage> ret;
    return ret;
}

bool ShortMessageManager::SetCBConfig(
    int32_t slotId, bool enable, uint32_t startMessageId, uint32_t endMessageId, uint8_t ranType)
{
    if (smsServiceInterface_ != nullptr) {
        return smsServiceInterface_->SetCBConfig(slotId, enable, startMessageId, endMessageId, ranType);
    }
    return false;
}

int32_t ShortMessageManager::ConnectService()
{
    sptr<OHOS::ISystemAbilityManager> smsManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smsManager == nullptr) {
        return ERROR_SERVICE_NOT_AVAILABLE;
    }
    sptr<IRemoteObject> object = smsManager->GetSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    if (object != nullptr) {
        smsServiceInterface_ = iface_cast<ISmsServiceInterface>(object);
    } else {
        return ERROR_SERVICE_NOT_AVAILABLE;
    }
    if (smsServiceInterface_ == nullptr) {
        return ERROR_SERVICE_NOT_AVAILABLE;
    }
    return SERVICE_CONNECTED;
}
} // namespace Telephony
} // namespace OHOS