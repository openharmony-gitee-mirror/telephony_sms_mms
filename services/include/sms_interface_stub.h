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

#ifndef SMS_INTERFACE_STUB_H
#define SMS_INTERFACE_STUB_H

#include <list>
#include <memory>
#include <string>

#include "i_sms_service_interface.h"
#include "iremote_stub.h"
#include "sms_interface_manager.h"
#include "sms_receive_manager.h"
#include "sms_send_manager.h"

namespace OHOS {
namespace Telephony {
class SmsInterfaceStub : public IRemoteStub<ISmsServiceInterface> {
public:
    SmsInterfaceStub();
    ~SmsInterfaceStub();
    void SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr,
        const std::u16string text, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveryCallback) override;
    void SendMessage(int32_t slotId, const std::u16string desAddr, const std::u16string scAddr, uint16_t port,
        const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveryCallback) override;
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    std::shared_ptr<SmsInterfaceManager> GetSmsInterfaceManager(int32_t slotId);
    std::shared_ptr<SmsInterfaceManager> GetSmsInterfaceManager();
    bool SetSmscAddr(int32_t slotId, const std::u16string &scAddr) override;
    std::u16string GetSmscAddr(int32_t slotId) override;
    bool AddSimMessage(
        int32_t slotId, const std::u16string &smsc, const std::u16string &pdu, SimMessageStatus status) override;
    bool DelSimMessage(int32_t slotId, uint32_t msgIndex) override;
    bool UpdateSimMessage(int32_t slotId, uint32_t msgIndex, SimMessageStatus newStatus, const std::u16string &pdu,
        const std::u16string &smsc) override;
    std::vector<ShortMessage> GetAllSimMessages(int32_t slotId) override;
    bool SetCBConfig(int32_t slotId, bool enable, uint32_t fromMsgId, uint32_t toMsgId, uint8_t netType) override;
    bool SetDefaultSmsSlotId(int32_t slotId) override;
    int32_t GetDefaultSmsSlotId() override;

protected:
    void InitModule();

private:
    void OnSendSmsTextRequest(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnSendSmsDataRequest(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnSetSmscAddr(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetSmscAddr(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnAddSimMessage(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnDelSimMessage(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnUpdateSimMessage(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetAllSimMessages(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnSetCBConfig(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnSetDefaultSmsSlotId(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetDefaultSmsSlotId(MessageParcel &data, MessageParcel &reply, MessageOption &option);

    std::map<uint32_t, std::shared_ptr<SmsInterfaceManager>> slotSmsInterfaceManagerMap_;
    using SmsServiceFunc = void (SmsInterfaceStub::*)(
        MessageParcel &data, MessageParcel &reply, MessageOption &option);
    std::map<uint32_t, SmsServiceFunc> memberFuncMap_;
};
} // namespace Telephony
} // namespace OHOS
#endif