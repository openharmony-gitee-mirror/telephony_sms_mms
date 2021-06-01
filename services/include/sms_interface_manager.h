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
namespace OHOS {
namespace SMS {
class SmsInterfaceManager {
public:
    explicit SmsInterfaceManager(int32_t slotId);
    virtual ~SmsInterfaceManager();
    void InitInterfaceManager();
    void TextBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, const std::string &text,
        const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliverCallback);
    void DataBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, const uint16_t port,
        const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliverCallback);
    bool AddSIMMessage(const std::vector<uint8_t> &smsc, const std::vector<uint8_t> &pdu, int status);
    bool DelSIMMessage(uint32_t msgIndex);
    bool UpdateSIMMessage(uint32_t msgIndex, uint32_t newStatus, const std::vector<uint8_t> &pdu);
    std::list<SmsBaseMessage> GetAllSIMMessages();
    bool EnableCB(uint32_t msgId, uint8_t netType);
    bool DisableCB(uint32_t msgId, uint8_t netType);
    bool EnableCBRange(uint32_t startMsgId, uint32_t endMsgId, uint8_t netType);
    bool DisableCBRange(uint32_t startMsgId, uint32_t endMsgId, uint8_t netType);

private:
    int32_t slotId_;
    std::shared_ptr<SmsSendManager> smsSendManager_;
    std::unique_ptr<SmsReceiveManager> smsReceiveManager_;
    std::shared_ptr<AppExecFwk::EventRunner> smsSendManagerRunner_;
};
} // namespace SMS
} // namespace OHOS
#endif