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
#include "sms_interface_manager.h"
#include <codecvt>
#include <locale>
#include "sms_hilog_wrapper.h"
#include "string_utils.h"
namespace OHOS {
namespace SMS {
using namespace std;
SmsInterfaceManager::SmsInterfaceManager(int32_t slotId) : slotId_(slotId) {}

SmsInterfaceManager::~SmsInterfaceManager() {}

void SmsInterfaceManager::InitInterfaceManager()
{
    smsSendManagerRunner_ = AppExecFwk::EventRunner::Create("SmsSMEventLoop" + to_string(slotId_));
    if (smsSendManagerRunner_ == nullptr) {
        HILOG_ERROR("failed to create SmsSendManagerRunner_");
        return;
    }
    smsSendManager_ = make_shared<SmsSendManager>(smsSendManagerRunner_, slotId_);
    if (smsSendManager_ == nullptr) {
        HILOG_ERROR("failed to create SmsSendManager");
        return;
    }
    smsSendManager_->Init();
    smsSendManagerRunner_->Run();

    smsReceiveManager_ = make_unique<SmsReceiveManager>(slotId_);
    if (smsReceiveManager_ == nullptr) {
        HILOG_ERROR("failed to create SmsReceiveManager");
        return;
    }
    smsReceiveManager_->Init();
    HILOG_INFO("SmsInterfaceManager::InitInterfaceManager success, %{public}d", slotId_);
}

void SmsInterfaceManager::TextBasedSmsDelivery(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback, const sptr<IDeliveryShortMessageCallback> &deliverCallback)
{
    if (desAddr.empty() || text.empty()) {
        HILOG_ERROR("TextBasedSmsDelivery Text Parameter invalid.");
        return;
    }
    if (smsSendManager_ == nullptr) {
        HILOG_ERROR("TextBasedSmsDelivery Text smsSendManager_ nullptr.");
        return;
    }
    smsSendManager_->TextBasedSmsDelivery(desAddr, scAddr, text, sendCallback, deliverCallback);
}

void SmsInterfaceManager::DataBasedSmsDelivery(const string &desAddr, const string &scAddr, const uint16_t port,
    const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliverCallback)
{
    if (desAddr.empty()) {
        HILOG_ERROR("DataBasedSmsDelivery Data Parameter invalid.");
        return;
    }
    if (smsSendManager_ == nullptr) {
        HILOG_ERROR("TextBasedSmsDelivery Data smsSendManager_ nullptr.");
        return;
    }
    smsSendManager_->DataBasedSmsDelivery(desAddr, scAddr, port, data, dataLen, sendCallback, deliverCallback);
}

bool SmsInterfaceManager::AddSIMMessage(
    const std::vector<uint8_t> &smsc, const std::vector<uint8_t> &pdu, int status)
{
    HILOG_INFO("smscLen = %{public}d pudLen = %{public}d status = %{public}d", smsc.size(), pdu.size(), status);
    return false;
}

bool SmsInterfaceManager::DelSIMMessage(uint32_t msgIndex)
{
    HILOG_INFO("messageIndex = %{pulbic}d", msgIndex);
    return false;
}

bool SmsInterfaceManager::UpdateSIMMessage(uint32_t msgIndex, uint32_t newStatus, const std::vector<uint8_t> &pdu)
{
    HILOG_INFO("messageIndex = %{pulbic}d", msgIndex);
    HILOG_INFO("newStatus = %{pulbic}d", newStatus);
    HILOG_INFO("pduLen = %{pulbic}d", pdu.size());
    return false;
}

list<SmsBaseMessage> SmsInterfaceManager::GetAllSIMMessages()
{
    list<SmsBaseMessage> messages;
    return messages;
}

bool SmsInterfaceManager::EnableCB(uint32_t msgId, uint8_t netType)
{
    HILOG_INFO("msgId = %{pulbic}d", msgId);
    HILOG_INFO("netType = %{pulbic}d", netType);
    return false;
}

bool SmsInterfaceManager::DisableCB(uint32_t msgId, uint8_t netType)
{
    HILOG_INFO("msgId = %{pulbic}d", msgId);
    HILOG_INFO("netType = %{pulbic}d", netType);
    return false;
}

bool SmsInterfaceManager::EnableCBRange(uint32_t startMsgId, uint32_t endMsgId, uint8_t netType)
{
    HILOG_INFO("startMsgId = %{pulbic}d", startMsgId);
    HILOG_INFO("endMsgId = %{pulbic}d", endMsgId);
    HILOG_INFO("netType = %{pulbic}d", netType);
    return false;
}

bool SmsInterfaceManager::DisableCBRange(uint32_t startMsgId, uint32_t endMsgId, uint8_t netType)
{
    HILOG_INFO("startMsgId = %{pulbic}d", startMsgId);
    HILOG_INFO("endMsgId = %{pulbic}d", endMsgId);
    HILOG_INFO("netType = %{pulbic}d", netType);
    return false;
}
} // namespace SMS
} // namespace OHOS