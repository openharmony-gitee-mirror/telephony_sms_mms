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
#include "kit_short_message_hilog_wrap.h"
#include "system_ability_definition.h"
namespace OHOS {
ShortMessageManager::ShortMessageManager()
{
    int32_t connectState = ConnectService();
    if (connectState == 0) {
        HILOG_DEBUG("ShortMessageManager::ShortMessageManager() sms_service connected\n");
    } else {
        HILOG_DEBUG("ShortMessageManager::ShortMessageManager() sms_service connect failed\n");
    }
}

ShortMessageManager::~ShortMessageManager() {}

void ShortMessageManager::SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr,
    const std::u16string text, const sptr<SMS::ISendShortMessageCallback> &callback,
    const sptr<SMS::IDeliveryShortMessageCallback> &deliverCallback)
{
    HILOG_DEBUG("enter ShortMessageManager::SendMessage()\n");
    if (smsServiceInterface_ != nullptr) {
        smsServiceInterface_->SendMessage(slotId, desAddr, scAddr, text, callback, deliverCallback);
        HILOG_DEBUG("execd smsServiceInterface_->SendMessage\n");
    }
    HILOG_DEBUG("end ShortMessageManager::SendMessage()\n");
}

void ShortMessageManager::SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr,
    uint16_t port, const uint8_t *data, uint16_t dataLen, const sptr<SMS::ISendShortMessageCallback> &callback,
    const sptr<SMS::IDeliveryShortMessageCallback> &deliverCallback)
{
    if (smsServiceInterface_ != nullptr) {
        smsServiceInterface_->SendMessage(slotId, desAddr, scAddr, port, data, dataLen, callback, deliverCallback);
    }
}

int32_t ShortMessageManager::ConnectService()
{
    sptr<OHOS::ISystemAbilityManager> smsManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smsManager == nullptr) {
        HILOG_ERROR("ShortMessageManager::ConnectService() sm null\n");
        return -1;
    }
    sptr<IRemoteObject> object = smsManager->GetSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    if (object != nullptr) {
        smsServiceInterface_ = iface_cast<SMS::ISmsServiceInterface>(object);
    } else {
        HILOG_ERROR("ShortMessageManager::ConnectService() object null\n");
        return -1;
    }
    if (smsServiceInterface_ == nullptr) {
        HILOG_ERROR("ShortMessageManager::ConnectService() smsServiceInterface_ null\n");
        return -1;
    }
    HILOG_ERROR("ShortMessageManager::ConnectService() success\n");
    return 0;
}
} // namespace OHOS