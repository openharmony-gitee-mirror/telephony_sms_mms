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

#include "gsm_sms_sender.h"

#include <chrono>
#include <list>

#include "securec.h"
#include "string_utils.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
GsmSmsSender::GsmSmsSender(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId,
    function<void(std::shared_ptr<SmsSendIndexer>)> sendRetryFun)
    : SmsSender(runner, slotId, sendRetryFun)
{}

GsmSmsSender::~GsmSmsSender() {}

void GsmSmsSender::Init()
{
    if (!RegisterHandler()) {
        TELEPHONY_LOGI("GsmSmsSender::Init Register RADIO_SMS_STATUS fail.");
    }
}

void GsmSmsSender::TextBasedSmsDelivery(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    bool isMore = false;
    bool isStatusReport = false;
    int ret = 0;
    int headerCnt;
    int cellsInfosSize;
    unsigned char msgRef8bit;
    SmsCodingScheme codingType;
    GsmSmsMessage gsmSmsMessage;
    std::vector<struct SplitInfo> cellsInfos;
    gsmSmsMessage.SplitMessage(cellsInfos, text, false, codingType);
    isStatusReport = (deliveryCallback == nullptr) ? false : true;
    std::shared_ptr<struct SmsTpdu> tpdu =
        gsmSmsMessage.CreateDefaultSubmitSmsTpdu(desAddr, scAddr, text, isStatusReport, codingType);
    if (tpdu == nullptr) {
        SendResultCallBack(sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        TELEPHONY_LOGE("TextBasedSmsDelivery tpdu nullptr error.");
        return;
    }
    cellsInfosSize = cellsInfos.size();
    msgRef8bit = GetMsgRef8Bit();
    isStatusReport = tpdu->data.submit.bStatusReport;

    TELEPHONY_LOGI("TextBasedSmsDelivery isStatusReport= %{public}d", isStatusReport);
    shared_ptr<uint8_t> unSentCellCount = make_shared<uint8_t>(cellsInfosSize);
    shared_ptr<bool> hasCellFailed = make_shared<bool>(false);
    chrono::system_clock::duration timePoint = chrono::system_clock::now().time_since_epoch();
    chrono::seconds sec = chrono::duration_cast<chrono::seconds>(timePoint);
    long timeStamp = sec.count();
    std::unique_lock<std::mutex> lock(mutex_);
    for (int i = 0; i < cellsInfosSize; i++) {
        headerCnt = 0;
        (void)memset_s(tpdu->data.submit.userData.data, MAX_USER_DATA_LEN + 1, 0x00, MAX_USER_DATA_LEN + 1);
        ret = memcpy_s(tpdu->data.submit.userData.data, MAX_USER_DATA_LEN + 1, &cellsInfos[i].encodeData[0],
            cellsInfos[i].encodeData.size());
        if (ret != EOK) {
            SendResultCallBack(sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
            TELEPHONY_LOGE("TextBasedSmsDelivery memcpy_s error.");
            return;
        }
        tpdu->data.submit.userData.length = cellsInfos[i].encodeData.size();
        tpdu->data.submit.userData.data[cellsInfos[i].encodeData.size()] = 0;
        tpdu->data.submit.msgRef = msgRef8bit;
        if (cellsInfosSize > 1) {
            SmsConcat concat;
            concat.is8Bits = true;
            concat.msgRef = msgRef8bit;
            concat.totalSeg = cellsInfosSize;
            concat.seqNum = i + 1;
            headerCnt += gsmSmsMessage.SetHeaderConcat(headerCnt, concat);
        }
        /* Set User Data Header for Alternate Reply Address */
        headerCnt += gsmSmsMessage.SetHeaderReply(headerCnt);
        /* Set User Data Header for National Language Single Shift */
        headerCnt += gsmSmsMessage.SetHeaderLang(headerCnt, codingType, cellsInfos[i].langId);
        tpdu->data.submit.userData.headerCnt = headerCnt;
        tpdu->data.submit.bHeaderInd = (headerCnt > 0) ? true : false;
        if (cellsInfosSize > 1 && i < (cellsInfosSize - 1)) {
            tpdu->data.submit.bStatusReport = false;
            isMore = true;
        } else {
            tpdu->data.submit.bStatusReport = isStatusReport;
            isMore = false;
        }
        std::shared_ptr<struct EncodeInfo> encodeInfo = gsmSmsMessage.GetSubmitEncodeInfo(scAddr, isMore);
        std::shared_ptr<SmsSendIndexer> indexer = nullptr;
        indexer = make_shared<SmsSendIndexer>(desAddr, scAddr, text, sendCallback, deliveryCallback);
        if (encodeInfo == nullptr || indexer == nullptr) {
            SendResultCallBack(sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
            TELEPHONY_LOGE("create encodeInfo indexer == nullptr\r\n");
            continue;
        }
        SetSendIndexerInfo(indexer, encodeInfo, msgRef8bit);
        indexer->SetUnSentCellCount(unSentCellCount);
        indexer->SetHasCellFailed(hasCellFailed);
        indexer->SetTimeStamp(timeStamp);
        SendSmsToRil(indexer);
    }
}

void GsmSmsSender::DataBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, int32_t port,
    const uint8_t *data, uint32_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveryCallback)
{
    uint8_t msgRef8bit = GetMsgRef8Bit();
    GsmSmsMessage gsmSmsMessage;
    std::shared_ptr<struct SmsTpdu> tpdu = gsmSmsMessage.CreateDataSubmitSmsTpdu(
        desAddr, scAddr, port, data, dataLen, msgRef8bit, (deliveryCallback == nullptr) ? false : true);
    std::shared_ptr<struct EncodeInfo> encodeInfo = gsmSmsMessage.GetSubmitEncodeInfo(scAddr, false);
    if (encodeInfo == nullptr || tpdu == nullptr) {
        SendResultCallBack(sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        TELEPHONY_LOGE("DataBasedSmsDelivery encodeInfo or tpdu nullptr error.");
        return;
    }
    shared_ptr<SmsSendIndexer> indexer = nullptr;
    indexer = make_shared<SmsSendIndexer>(desAddr, scAddr, port, data, dataLen, sendCallback, deliveryCallback);
    if (indexer == nullptr) {
        SendResultCallBack(sendCallback, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        TELEPHONY_LOGE("DataBasedSmsDelivery create SmsSendIndexer nullptr");
        return;
    }
    std::vector<uint8_t> smca(encodeInfo->smcaData_, encodeInfo->smcaData_ + encodeInfo->smcaLen);
    std::vector<uint8_t> pdu(encodeInfo->tpduData_, encodeInfo->tpduData_ + encodeInfo->tpduLen);
    std::shared_ptr<uint8_t> unSentCellCount = make_shared<uint8_t>(1);
    std::shared_ptr<bool> hasCellFailed = make_shared<bool>(false);
    chrono::system_clock::duration timePoint = chrono::system_clock::now().time_since_epoch();
    chrono::seconds sec = chrono::duration_cast<chrono::seconds>(timePoint);
    long timeStamp = sec.count();
    indexer->SetUnSentCellCount(unSentCellCount);
    indexer->SetHasCellFailed(hasCellFailed);
    indexer->SetEncodeSmca(std::move(smca));
    indexer->SetEncodePdu(std::move(pdu));
    indexer->SetHasMore(encodeInfo->isMore_);
    indexer->SetMsgRefId(msgRef8bit);
    indexer->SetNetWorkType(GetNetWorkType());
    indexer->SetTimeStamp(timeStamp);
    std::unique_lock<std::mutex> lock(mutex_);
    SendSmsToRil(indexer);
}

void GsmSmsSender::SendSmsToRil(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        SendResultCallBack(smsIndexer, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        TELEPHONY_LOGE("gsm_sms_sender: SendSms smsIndexer nullptr");
        return;
    }
    std::shared_ptr<Core> core = GetCore();
    if (core == nullptr) {
        SendResultCallBack(smsIndexer, ISendShortMessageCallback::SEND_SMS_FAILURE_UNKNOWN);
        TELEPHONY_LOGE("gsm_sms_sender: SendSms core nullptr");
        return;
    }
    int64_t refId = GetMsgRef64Bit();
    if (!SendCacheMapAddItem(refId, smsIndexer)) {
        TELEPHONY_LOGE("SendCacheMapAddItem Error!!");
    }
    if (netDomainType_ != NET_DOMAIN_IMS && (smsIndexer->GetPsResendCount() == 0)) {
        uint8_t tryCount = smsIndexer->GetCsResendCount();
        if (tryCount > 0) {
            smsIndexer->UpdatePduForResend();
        }
        if (tryCount == 0 && smsIndexer->GetHasMore()) {
            auto reply = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SEND_SMS_EXPECT_MORE, refId);
            reply->SetOwner(shared_from_this());
            TELEPHONY_LOGI("SendSmsMoreMode pdu len = %{public}zu", smsIndexer->GetEncodePdu().size());
            core->SendSmsMoreMode(StringUtils::StringToHex(smsIndexer->GetEncodeSmca()),
                StringUtils::StringToHex(smsIndexer->GetEncodePdu()), reply);
        } else {
            auto reply = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SEND_SMS, refId);
            reply->SetOwner(shared_from_this());
            TELEPHONY_LOGI("SendSms pdu len = %{public}zu", smsIndexer->GetEncodePdu().size());
            core->SendSms(StringUtils::StringToHex(smsIndexer->GetEncodeSmca()),
                StringUtils::StringToHex(smsIndexer->GetEncodePdu()), reply);
        }
    } else {
        TELEPHONY_LOGD("ims network domain not support!");
        smsIndexer->SetPsResendCount(smsIndexer->GetPsResendCount() + 1);
    }
}

void GsmSmsSender::StatusReportAnalysis(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<SmsMessageInfo> statusInfo = event->GetSharedObject<SmsMessageInfo>();
    if (statusInfo == nullptr) {
        return;
    }
    if (statusInfo->pdu.size() == 0) {
        TELEPHONY_LOGE("gsm_sms_sender: StatusReportAnalysis statusInfo->pdu == 0");
        return;
    }
    std::shared_ptr<Core> core = GetCore();
    if (core == nullptr) {
        TELEPHONY_LOGE("gsm_sms_sender: StatusReportAnalysis core nullptr");
        return;
    }
    std::string pdu = StringUtils::StringToHex(statusInfo->pdu);
    std::shared_ptr<GsmSmsMessage> message = GsmSmsMessage::CreateMessage(pdu);
    if (message == nullptr) {
        TELEPHONY_LOGE("gsm_sms_sender: StatusReportAnalysis message nullptr");
        return;
    }
    sptr<IDeliveryShortMessageCallback> deliveryCallback = nullptr;
    auto oldIndexer = reportList_.begin();
    while (oldIndexer != reportList_.end()) {
        auto iter = oldIndexer++;
        TELEPHONY_LOGI("StatusReport %{public}d %{public}d", message->GetMsgRef(), (*iter)->GetMsgRefId());
        if (*iter != nullptr && (message->GetMsgRef() == (*iter)->GetMsgRefId())) {
            // save the message to db, or updata to db msg state(success or failed)
            deliveryCallback = (*iter)->GetDeliveryCallback();
            reportList_.erase(iter);
        }
    }
    core->SendSmsAck(true, AckIncomeCause::SMS_ACK_PROCESSED, AppExecFwk::InnerEvent::Pointer(nullptr, nullptr));
    if (deliveryCallback != nullptr) {
        std::string ackpdu = StringUtils::StringToHex(message->GetRawPdu());
        deliveryCallback->OnSmsDeliveryResult(StringUtils::ToUtf16(ackpdu));
        TELEPHONY_LOGI("gsm_sms_sender: StatusReportAnalysis %{public}s", pdu.c_str());
    }
}

void GsmSmsSender::SetSendIndexerInfo(std::shared_ptr<SmsSendIndexer> &indexer,
    const std::shared_ptr<struct EncodeInfo> &encodeInfo, unsigned char msgRef8bit)
{
    if (encodeInfo == nullptr || indexer == nullptr) {
        TELEPHONY_LOGE("CreateSendIndexer encodeInfo nullptr");
        return;
    }

    std::vector<uint8_t> smca(encodeInfo->smcaData_, encodeInfo->smcaData_ + encodeInfo->smcaLen);
    std::vector<uint8_t> pdu(encodeInfo->tpduData_, encodeInfo->tpduData_ + encodeInfo->tpduLen);
    indexer->SetEncodeSmca(std::move(smca));
    indexer->SetEncodePdu(std::move(pdu));
    indexer->SetHasMore(encodeInfo->isMore_);
    indexer->SetMsgRefId(msgRef8bit);
    indexer->SetNetWorkType(GetNetWorkType());
}

bool GsmSmsSender::RegisterHandler()
{
    bool ret = false;
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        TELEPHONY_LOGI("GsmSmsSender::RegisteHandler Register RADIO_SMS_STATUS ok.");
        core->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_SMS_STATUS, nullptr);
        ret = true;
    }
    return ret;
}
} // namespace Telephony
} // namespace OHOS
