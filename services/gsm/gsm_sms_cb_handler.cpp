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

#include "gsm_sms_cb_handler.h"

#include "common_event.h"
#include "common_event_manager.h"
#include "securec.h"
#include "want.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace EventFwk;
GsmSmsCbHandler::GsmSmsCbHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : AppExecFwk::EventHandler(runner), slotId_(slotId)
{
    TELEPHONY_LOGI("GsmSmsCbHandler Create slotId (%{public}d)", slotId_);
}

GsmSmsCbHandler::~GsmSmsCbHandler()
{
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        core->UnRegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_CELL_BROADCAST);
    }
}

void GsmSmsCbHandler::Init()
{
    cbMsgList_.clear();
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        core->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_CELL_BROADCAST, nullptr);
        TELEPHONY_LOGD("GsmSmsCbHandler::RegisterHandler::slotId= %{public}d", slotId_);
    }
}

bool GsmSmsCbHandler::CheckCbActive(const std::shared_ptr<SmsCbMessage> &cbMessage)
{
    if (cbMessage == nullptr) {
        return false;
    }
    return true;
}

unsigned char GsmSmsCbHandler::CheckCbMessage(const std::shared_ptr<SmsCbMessage> &cbMessage)
{
    unsigned char currPageCnt = 0;
    bool bFind = false;
    if (cbMessage == nullptr) {
        TELEPHONY_LOGE("CheckCbMessage cbMessage nullptr err.");
        return currPageCnt;
    }

    std::shared_ptr<SmsCbMessageHeader> cbHeader = cbMessage->GetCbHeader();
    if (cbHeader == nullptr || cbHeader->totalPages == 0) {
        TELEPHONY_LOGE("CheckCbMessage GetCbHeader err.");
        return currPageCnt;
    }

    for (std::size_t i = 0; i < cbMsgList_.size(); i++) {
        SmsCbInfo &cbInfo = cbMsgList_[i];
        if (*cbInfo.header.get() == *cbHeader.get()) {
            int updateNum = cbHeader->serialNum.updateNum - cbInfo.header->serialNum.updateNum;
            if (updateNum == 0) {
                if (cbInfo.cbMsgs.count(cbHeader->page)) {
                    currPageCnt = 0x01;
                    return currPageCnt;
                }
                cbInfo.cbMsgs.insert(std::make_pair(cbHeader->page, cbMessage));
                currPageCnt = cbInfo.cbMsgs.size();
                bFind = true;
            }
            break;
        }
    }

    if (bFind == false || cbHeader->totalPages == 0x01) {
        AddCbMessageToList(cbMessage);
        currPageCnt = 0x01;
    }

    return currPageCnt;
}

std::unique_ptr<SmsCbInfo> GsmSmsCbHandler::FindCbMessage(const std::shared_ptr<SmsCbMessage> &cbMessage)
{
    std::unique_ptr<SmsCbInfo> cbInfo = nullptr;
    if (cbMessage == nullptr) {
        TELEPHONY_LOGE("FindCbMessage cbMessage nullptr err.");
        return cbInfo;
    }

    std::shared_ptr<SmsCbMessageHeader> cbHeader = cbMessage->GetCbHeader();
    if (cbHeader == nullptr) {
        TELEPHONY_LOGE("FindCbMessage header err.");
        return cbInfo;
    }

    for (std::size_t i = 0; i < cbMsgList_.size(); i++) {
        SmsCbInfo &info = cbMsgList_[i];
        if (*info.header.get() == *cbHeader.get()) {
            cbInfo = std::make_unique<SmsCbInfo>(info.header, info.cbMsgs);
            return cbInfo;
        }
    }
    return cbInfo;
}

bool GsmSmsCbHandler::AddCbMessageToList(const std::shared_ptr<SmsCbMessage> &cbMessage)
{
    if (cbMessage == nullptr) {
        TELEPHONY_LOGE("AddCbMessageToList cbMessage nullptr err.");
        return false;
    }

    std::shared_ptr<SmsCbMessageHeader> cbHeader = cbMessage->GetCbHeader();
    if (cbHeader == nullptr) {
        TELEPHONY_LOGE("AddCbMessageToList header err.");
        return false;
    }

    std::unique_ptr<SmsCbInfo> cbInfo = FindCbMessage(cbMessage);
    if (cbInfo == nullptr) {
        SmsCbInfo info;
        info.header = cbHeader;
        info.cbMsgs.insert(std::make_pair(cbHeader->page, cbMessage));
        cbMsgList_.push_back(info);
        return true;
    }
    return false;
}

bool GsmSmsCbHandler::RemoveCbMessageFromList(const std::shared_ptr<SmsCbMessage> &cbMessage)
{
    if (cbMessage == nullptr) {
        TELEPHONY_LOGE("RemoveCbMessageFromList cbMessage nullptr err.");
        return false;
    }

    std::shared_ptr<SmsCbMessageHeader> header = cbMessage->GetCbHeader();
    if (header == nullptr) {
        TELEPHONY_LOGE("RemoveCbMessageFromList header err.");
        return false;
    }
    auto it = cbMsgList_.begin();
    while (it != cbMsgList_.end()) {
        auto oldIt = it++;
        SmsCbInfo &info = *oldIt;
        if (*info.header.get() == *header.get()) {
            cbMsgList_.erase(oldIt);
            return true;
        }
    }
    return false;
}

void GsmSmsCbHandler::HandleCbMessage(std::shared_ptr<CellBroadcastReportInfo> &message)
{
    if (message == nullptr) {
        TELEPHONY_LOGE("SmsCellBroadcastHandler HandleCbMessage message == nullptr");
        return;
    }

    std::string pdu(message->pdu);
    std::shared_ptr<SmsCbMessage> cbMessage = SmsCbMessage::CreateCbMessage(pdu);
    if (cbMessage == nullptr) {
        TELEPHONY_LOGE("create Sms CbMessage fail, pdu %{public}s", pdu.c_str());
        return;
    }
    std::shared_ptr<SmsCbMessageHeader> header = cbMessage->GetCbHeader();
    if (header == nullptr) {
        TELEPHONY_LOGE("HandleCbMessage header is null.");
        return;
    }

    if (!CheckCbActive(cbMessage)) {
        TELEPHONY_LOGD("The Cell Broadcast msg is not active");
        return;
    }

    unsigned char pageCnt = CheckCbMessage(cbMessage);
    if (header->totalPages == pageCnt) {
        SendCbMessageBroadcast(cbMessage);
        RemoveCbMessageFromList(cbMessage);
    }
}

bool GsmSmsCbHandler::SendCbMessageBroadcast(const std::shared_ptr<SmsCbMessage> &cbMessage)
{
    if (cbMessage == nullptr) {
        TELEPHONY_LOGE("SendCbMessageBroadcast cbMessage nullptr err.");
        return false;
    }

    std::unique_ptr<SmsCbInfo> info = FindCbMessage(cbMessage);
    std::shared_ptr<SmsCbMessageHeader> header = cbMessage->GetCbHeader();
    if (header == nullptr || info == nullptr) {
        TELEPHONY_LOGE("SendCbMessageBroadcast FindCbMessage header err.");
        return false;
    }

    std::string rawMsgBody;
    std::string msgBody;
    for (auto it = info->cbMsgs.begin(); it != info->cbMsgs.end(); it++) {
        rawMsgBody.append(it->second->GetCbMessageRaw());
    }

    cbMessage->ConvertToUTF8(rawMsgBody, msgBody);
    TELEPHONY_LOGI("cell broadcast msgBody--> %{public}s", msgBody.c_str());
    Want want;
    CommonEventData data;
    unsigned short serialNum = cbMessage->EncodeCbSerialNum(header->serialNum);
    want.SetParam("serialNum", serialNum);
    want.SetParam("msgBody", msgBody);
    want.SetParam("recvTime", header->recvTime);
    data.SetCode(msgReceiveCode_);
    if (!header->bEtwsMessage) {
        want.SetAction("ohos.action.telephonySmsETWSCBReceiveFinished");
        want.SetParam("warningType", header->warningType);
        data.SetData("ETWS CB MSG RECEIVE");
    } else {
        want.SetAction("ohos.action.telephonySmsCBReceiveFinished");
        want.SetParam("cbMsgType", header->cbMsgType);
        want.SetParam("msgId", header->msgId);
        want.SetParam("langType", header->langType);
        want.SetParam("dcs", header->dcs.rawData);
        data.SetData("CB MSG RECEIVE");
    }
    data.SetWant(want);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    if (!publishResult) {
        TELEPHONY_LOGE("SendBroadcast PublishBroadcastEvent result fail");
        return false;
    }
    return true;
}

void GsmSmsCbHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SmsCellBroadcastHandler event == nullptr");
        return;
    }

    int eventId = 0;
    eventId = event->GetInnerEventId();
    switch (eventId) {
        case ObserverHandler::RADIO_CELL_BROADCAST: {
            std::shared_ptr<CellBroadcastReportInfo> cbMessage = nullptr;
            cbMessage = event->GetSharedObject<CellBroadcastReportInfo>();
            HandleCbMessage(cbMessage);
            break;
        }
        default:
            break;
    }
}

std::shared_ptr<Core> GsmSmsCbHandler::GetCore() const
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        return core;
    }
    return nullptr;
}
} // namespace Telephony
} // namespace OHOS