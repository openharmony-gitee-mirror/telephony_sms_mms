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

#include "sms_interface_manager.h"
#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
SmsInterfaceStub::SmsInterfaceStub()
{
    memberFuncMap_[TEXT_BASED_SMS_DELIVERY] = &SmsInterfaceStub::OnSendSmsTextRequest;
    memberFuncMap_[DATA_BASED_SMS_DELIVERY] = &SmsInterfaceStub::OnSendSmsDataRequest;
    memberFuncMap_[SET_SMSC_ADDRESS] = &SmsInterfaceStub::OnSetSmscAddr;
    memberFuncMap_[GET_SMSC_ADDRESS] = &SmsInterfaceStub::OnGetSmscAddr;
    memberFuncMap_[ADD_SIM_MESSAGE] = &SmsInterfaceStub::OnAddSimMessage;
    memberFuncMap_[DEL_SIM_MESSAGE] = &SmsInterfaceStub::OnDelSimMessage;
    memberFuncMap_[UPDATE_SIM_MESSAGE] = &SmsInterfaceStub::OnUpdateSimMessage;
    memberFuncMap_[GET_ALL_SIM_MESSAGE] = &SmsInterfaceStub::OnGetAllSimMessages;
    memberFuncMap_[SET_CB_CONFIG] = &SmsInterfaceStub::OnSetCBConfig;
    memberFuncMap_[SET_DEFAULT_SMS_SLOT_ID] = &SmsInterfaceStub::OnSetDefaultSmsSlotId;
    memberFuncMap_[GET_DEFAULT_SMS_SLOT_ID] = &SmsInterfaceStub::OnGetDefaultSmsSlotId;
}

SmsInterfaceStub::~SmsInterfaceStub()
{
    slotSmsInterfaceManagerMap_.clear();
    memberFuncMap_.clear();
}

void SmsInterfaceStub::InitModule()
{
    static bool bInitModule = false;
    if (!bInitModule) {
        bInitModule = true;
        TELEPHONY_LOGI("SmsInterfaceStub = ############### InitModule Start");
        slotSmsInterfaceManagerMap_[CoreManager::DEFAULT_SLOT_ID] =
            std::make_shared<SmsInterfaceManager>(CoreManager::DEFAULT_SLOT_ID);
        slotSmsInterfaceManagerMap_[CoreManager::DEFAULT_SLOT_ID]->InitInterfaceManager();
        TELEPHONY_LOGI("SmsInterfaceStub = ############### InitModule end");
    }
}

std::shared_ptr<SmsInterfaceManager> SmsInterfaceStub::GetSmsInterfaceManager(int32_t slotId)
{
    TELEPHONY_LOGI("GetSmsInterfaceManager = %{public}d", slotId);
    std::map<uint32_t, std::shared_ptr<SmsInterfaceManager>>::iterator iter =
        slotSmsInterfaceManagerMap_.find(slotId);
    if (iter != slotSmsInterfaceManagerMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<SmsInterfaceManager> SmsInterfaceStub::GetSmsInterfaceManager()
{
    for (auto &iter : slotSmsInterfaceManagerMap_) {
        return iter.second;
    }
    return nullptr;
}

void SmsInterfaceStub::SendMessage(int32_t slotId, const u16string desAddr, const u16string scAddr,
    const u16string text, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::SendMessage interfaceManager== nullptr");
        return;
    }
    interfaceManager->TextBasedSmsDelivery(StringUtils::ToUtf8(desAddr), StringUtils::ToUtf8(scAddr),
        StringUtils::ToUtf8(text), sendCallback, deliveryCallback);
}

void SmsInterfaceStub::SendMessage(int32_t slotId, const u16string desAddr, const u16string scAddr, uint16_t port,
    const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::SendMessage interfaceManager== nullptr");
        return;
    }
    interfaceManager->DataBasedSmsDelivery(StringUtils::ToUtf8(desAddr), StringUtils::ToUtf8(scAddr), port, data,
        dataLen, sendCallback, deliveryCallback);
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
    TELEPHONY_LOGI("MessageID::TEXT_BASED_SMS_DELIVERY %{public}d", slotId);
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
    SendMessage(slotId, desAddr, scAddr, port, buffer, dataLen, sendCallback, deliveryCallback);
    reply.WriteInt32(result);
}

void SmsInterfaceStub::OnSetSmscAddr(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool result = false;
    int32_t slotId = data.ReadInt32();
    std::u16string scAddr = data.ReadString16();
    result = SetSmscAddr(slotId, scAddr);
    TELEPHONY_LOGI("SetSmscAddr result %{public}d", result);
    reply.WriteBool(result);
}

void SmsInterfaceStub::OnGetSmscAddr(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string result;
    int32_t slotId = data.ReadInt32();
    result = GetSmscAddr(slotId);
    TELEPHONY_LOGI("GetSmscAddr result size %{public}zu", result.size());
    reply.WriteString16(result);
}

void SmsInterfaceStub::OnAddSimMessage(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool result = false;
    int32_t slotId = data.ReadInt32();
    std::u16string smsc = data.ReadString16();
    std::u16string pdu = data.ReadString16();
    uint32_t status = data.ReadUint32();
    result = AddSimMessage(slotId, smsc, pdu, static_cast<SimMessageStatus>(status));
    TELEPHONY_LOGI("AddSimMessage result %{public}d", result);
    reply.WriteBool(result);
}

void SmsInterfaceStub::OnDelSimMessage(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool result = false;
    int32_t slotId = data.ReadInt32();
    uint32_t msgIndex = data.ReadUint32();
    result = DelSimMessage(slotId, msgIndex);
    TELEPHONY_LOGI("DelSimMessage result %{public}d", result);
    reply.WriteBool(result);
}

void SmsInterfaceStub::OnUpdateSimMessage(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool result = false;
    int32_t slotId = data.ReadInt32();
    uint32_t msgIndex = data.ReadUint32();
    uint32_t newStatus = data.ReadUint32();
    std::u16string pdu = data.ReadString16();
    std::u16string smsc = data.ReadString16();
    result = UpdateSimMessage(slotId, msgIndex, static_cast<SimMessageStatus>(newStatus), pdu, smsc);
    TELEPHONY_LOGI("UpdateSimMessage result %{public}d", result);
    reply.WriteBool(result);
}

void SmsInterfaceStub::OnGetAllSimMessages(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::vector<ShortMessage> result;
    int32_t slotId = data.ReadInt32();
    result = GetAllSimMessages(slotId);
    TELEPHONY_LOGI("GetAllSimMessages size %{public}zu", result.size());

    int32_t resultLen = static_cast<int32_t>(result.size());
    reply.WriteInt32(resultLen);
    for (const auto &v : result) {
        v.Marshalling(reply);
    }
}

void SmsInterfaceStub::OnSetCBConfig(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool result = false;
    int32_t slotId = data.ReadInt32();
    bool enable = data.ReadBool();
    uint32_t fromMsgId = data.ReadUint32();
    uint32_t toMsgId = data.ReadUint32();
    uint8_t ranType = data.ReadUint8();
    result = SetCBConfig(slotId, enable, fromMsgId, toMsgId, ranType);
    TELEPHONY_LOGI("OnSetCBConfig result %{public}d", result);
    reply.WriteBool(result);
}

void SmsInterfaceStub::OnSetDefaultSmsSlotId(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool result = false;
    int32_t slotId = data.ReadInt32();
    result = SetDefaultSmsSlotId(slotId);
    TELEPHONY_LOGI("SetDefaultSmsSlotId result %{public}d", result);
    reply.WriteBool(result);
}

void SmsInterfaceStub::OnGetDefaultSmsSlotId(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int32_t result = 0;
    result = GetDefaultSmsSlotId();
    TELEPHONY_LOGI("SetDefaultSmsSlotId result %{public}d", result);
    reply.WriteInt32(result);
}

int SmsInterfaceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int32_t result = 0;
    TELEPHONY_LOGI("###################################### OnRemoteRequest");
    std::u16string myDescripter = SmsInterfaceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter == remoteDescripter) {
        auto itFunc = memberFuncMap_.find(code);
        if (itFunc != memberFuncMap_.end()) {
            auto memberFunc = itFunc->second;
            if (memberFunc != nullptr) {
                (this->*memberFunc)(data, reply, option);
            } else {
                TELEPHONY_LOGE("memberFunc is nullptr");
            }
        } else {
            TELEPHONY_LOGE("itFunc was not found");
        }
    } else {
        TELEPHONY_LOGE("descriptor checked fail");
        return result;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool SmsInterfaceStub::SetSmscAddr(int32_t slotId, const std::u16string &scAddr)
{
    bool result = false;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::SetSmscAddr interfaceManager== nullptr");
        return result;
    }
    string sca = StringUtils::ToUtf8(scAddr);
    return interfaceManager->SetSmscAddr(sca);
}

std::u16string SmsInterfaceStub::GetSmscAddr(int32_t slotId)
{
    std::u16string result;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::GetSmscAddr interfaceManager== nullptr");
        return result;
    }
    return StringUtils::ToUtf16(interfaceManager->GetSmscAddr());
}

bool SmsInterfaceStub::AddSimMessage(
    int32_t slotId, const std::u16string &smsc, const std::u16string &pdu, SimMessageStatus status)
{
    bool result = false;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::AddSimMessage interfaceManager== nullptr");
        return result;
    }
    std::string smscData = StringUtils::ToUtf8(smsc);
    std::string pduData = StringUtils::ToUtf8(pdu);
    return interfaceManager->AddSimMessage(smscData, pduData, status);
}

bool SmsInterfaceStub::DelSimMessage(int32_t slotId, uint32_t msgIndex)
{
    bool result = false;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::DelSimMessage interfaceManager== nullptr");
        return result;
    }
    return interfaceManager->DelSimMessage(msgIndex);
}

bool SmsInterfaceStub::UpdateSimMessage(int32_t slotId, uint32_t msgIndex, SimMessageStatus newStatus,
    const std::u16string &pdu, const std::u16string &smsc)
{
    bool result = false;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::UpdateSimMessage interfaceManager== nullptr");
        return result;
    }
    std::string pduData = StringUtils::ToUtf8(pdu);
    std::string smscData = StringUtils::ToUtf8(smsc);
    return interfaceManager->UpdateSimMessage(msgIndex, newStatus, pduData, smscData);
}

std::vector<ShortMessage> SmsInterfaceStub::GetAllSimMessages(int32_t slotId)
{
    std::vector<ShortMessage> result;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::GetAllSimMessages interfaceManager== nullptr");
        return result;
    }
    return interfaceManager->GetAllSimMessages();
}

bool SmsInterfaceStub::SetCBConfig(
    int32_t slotId, bool enable, uint32_t fromMsgId, uint32_t toMsgId, uint8_t netType)
{
    bool result = false;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager(slotId);
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::SetCBConfig interfaceManager== nullptr");
        return result;
    }
    return interfaceManager->SetCBConfig(enable, fromMsgId, toMsgId, netType);
}

bool SmsInterfaceStub::SetDefaultSmsSlotId(int32_t slotId)
{
    bool result = false;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager();
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::SetCBConfig interfaceManager== nullptr");
        return result;
    }
    return interfaceManager->SetDefaultSmsSlotId(slotId);
}

int32_t SmsInterfaceStub::GetDefaultSmsSlotId()
{
    int32_t result = -1;
    std::shared_ptr<SmsInterfaceManager> interfaceManager = GetSmsInterfaceManager();
    if (interfaceManager == nullptr) {
        TELEPHONY_LOGE("SmsInterfaceStub::SetCBConfig interfaceManager== nullptr");
        return result;
    }
    return interfaceManager->GetDefaultSmsSlotId();
}
} // namespace Telephony
} // namespace OHOS
