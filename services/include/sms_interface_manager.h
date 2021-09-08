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

#ifndef SMS_INTERFACE_MANAGER_H
#define SMS_INTERFACE_MANAGER_H

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "i_sms_service_interface.h"
#include "sms_receive_manager.h"
#include "sms_send_manager.h"
#include "sms_misc_manager.h"

namespace OHOS {
namespace Telephony {
class SmsInterfaceManager {
public:
    SmsInterfaceManager(int32_t slotId);
    virtual ~SmsInterfaceManager();
    void InitInterfaceManager();
    void TextBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, const std::string &text,
        const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveryCallback);
    void DataBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, const uint16_t port,
        const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveryCallback);
    bool AddSimMessage(
        const std::string &smsc, const std::string &pdu, ISmsServiceInterface::SimMessageStatus status);
    bool DelSimMessage(uint32_t msgIndex);
    bool UpdateSimMessage(uint32_t msgIndex, ISmsServiceInterface::SimMessageStatus newStatus,
        const std::string &pdu, const std::string &smsc);
    std::vector<ShortMessage> GetAllSimMessages();
    bool SetSmscAddr(const std::string &scAddr);
    std::string GetSmscAddr();
    bool SetCBConfig(bool enable, uint32_t fromMsgId, uint32_t toMsgId, uint8_t netType);
    bool SetDefaultSmsSlotId(int32_t slotId);
    int32_t GetDefaultSmsSlotId();

private:
    int32_t slotId_;
    std::shared_ptr<SmsSendManager> smsSendManager_;
    std::unique_ptr<SmsReceiveManager> smsReceiveManager_;
    std::shared_ptr<AppExecFwk::EventRunner> smsSendManagerRunner_;
    std::shared_ptr<SmsMiscManager> smsMiscManager_;
};
} // namespace Telephony
} // namespace OHOS
#endif