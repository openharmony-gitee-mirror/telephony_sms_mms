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
#include "sms_interface_stub.h"
#include <codecvt>
#include <locale>
#include <map>
#include "sms_hilog_wrapper.h"
#include "sms_interface_manager.h"
#include "string_utils.h"
namespace OHOS {
namespace SMS {
using namespace std;
SmsInterfaceStub::SmsInterfaceStub() {}

SmsInterfaceStub::~SmsInterfaceStub()
{
    slotSmsInterfaceManagerMap_.clear();
}

void SmsInterfaceStub::InitModule()
{
    static bool bInitModule = false;
    if (!bInitModule) {
        bInitModule = true;
        HILOG_INFO("SmsInterfaceStub = ############### InitModule Start");
        slotSmsInterfaceManagerMap_[1] = std::make_shared<SmsInterfaceManager>(1);
        slotSmsInterfaceManagerMap_[1]->InitInterfaceManager();
        HILOG_INFO("SmsInterfaceStub = ############### InitModule end");
    }
}

std::shared_ptr<SmsInterfaceManager> SmsInterfaceStub::GetSmsInterfaceManager(int32_t subId)
{
    std::map<uint32_t, std::shared_ptr<SmsInterfaceManager>>::iterator iter =
        slotSmsInterfaceManagerMap_.find(subId);
    if (iter != slotSmsInterfaceManagerMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

void SmsInterfaceStub::SendMessage(int32_t slotId, const u16string desAddr, const u16string scAddr,
    const u16string text, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliverCallback)
{
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        HILOG_ERROR("SmsInterfaceStub::SendMessage interfaceManager== nullptr");
        return;
    }
    interfaceManager->TextBasedSmsDelivery(StringUtils::ToUtf8(desAddr), StringUtils::ToUtf8(scAddr),
        StringUtils::ToUtf8(text), sendCallback, deliverCallback);
}

void SmsInterfaceStub::SendMessage(int32_t slotId, const u16string desAddr, const u16string scAddr, uint16_t port,
    const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliverCallback)
{
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        HILOG_ERROR("SmsInterfaceStub::SendMessage interfaceManager== nullptr");
        return;
    }
    interfaceManager->DataBasedSmsDelivery(StringUtils::ToUtf8(desAddr), StringUtils::ToUtf8(scAddr), port, data,
        dataLen, sendCallback, deliverCallback);
}

void SmsInterfaceStub::OnSendSmsTextRequest(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int32_t result = 0;
    sptr<ISendShortMessageCallback> sendCallback = nullptr;
    sptr<IDeliveryShortMessageCallback> deliveryCallback = nullptr;
    int32_t slotId = data.ReadInt32();
    u16string desAddr = data.ReadString16();
    u16string scAddr = data.ReadString16();
    u16string text = data.ReadString16();
    sptr<IRemoteObject> remoteSendCallback = data.ReadRemoteObject();
    sptr<IRemoteObject> remoteDeliveryCallback = data.ReadRemoteObject();
    if (remoteSendCallback != nullptr) {
        sendCallback = iface_cast<ISendShortMessageCallback>(remoteSendCallback);
    }
    if (remoteDeliveryCallback != nullptr) {
        deliveryCallback = iface_cast<IDeliveryShortMessageCallback>(remoteDeliveryCallback);
    }
    HILOG_INFO("MessageID::TEXT_BASED_SMS_DELIVERY %{public}d", slotId);
    SendMessage(slotId, desAddr, scAddr, text, sendCallback, deliveryCallback);
    reply.WriteInt32(result);
}

void SmsInterfaceStub::OnSendSmsDataRequest(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int32_t result = 0;
    sptr<ISendShortMessageCallback> sendCallback = nullptr;
    sptr<IDeliveryShortMessageCallback> deliveryCallback = nullptr;
    int32_t slotId = data.ReadInt32();
    u16string desAddr = data.ReadString16();
    u16string scAddr = data.ReadString16();
    int16_t port = data.ReadInt16();
    sptr<IRemoteObject> remoteSendCallback = data.ReadRemoteObject();
    sptr<IRemoteObject> remoteDeliveryCallback = data.ReadRemoteObject();
    if (remoteSendCallback != nullptr) {
        sendCallback = iface_cast<ISendShortMessageCallback>(remoteSendCallback);
    }
    if (remoteDeliveryCallback != nullptr) {
        deliveryCallback = iface_cast<IDeliveryShortMessageCallback>(remoteDeliveryCallback);
    }
    int16_t dataLen = data.ReadInt16();
    const uint8_t *buffer = reinterpret_cast<const uint8_t *>(data.ReadRawData(dataLen));
    if (buffer == nullptr) {
        result = -1;
        reply.WriteInt32(result);
        return;
    }
    HILOG_DEBUG("##################%{public}s##################", buffer);
    SendMessage(slotId, desAddr, scAddr, port, buffer, dataLen, sendCallback, deliveryCallback);
    reply.WriteInt32(result);
}

int SmsInterfaceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int32_t result = 0;
    HILOG_INFO("###################################### OnRemoteRequest");
    std::u16string myDescripter = SmsInterfaceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter == remoteDescripter) {
        switch (code) {
            case MessageID::TEXT_BASED_SMS_DELIVERY: {
                OnSendSmsTextRequest(data, reply, option);
                break;
            }
            case MessageID::DATA_BASED_SMS_DELIVERY: {
                OnSendSmsDataRequest(data, reply, option);
                break;
            }
            default:
                HILOG_ERROR("sms remote code=%d\r\n", code);
                break;
        }
    } else {
        HILOG_ERROR("descriptor checked fail");
        return result;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace SMS
} // namespace OHOS