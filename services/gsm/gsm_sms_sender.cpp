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
#include "sms_hilog_wrapper.h"
#include "string_utils.h"
using namespace std;
namespace OHOS {
namespace SMS {
GsmSmsSender::GsmSmsSender(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t subId,
    function<void(std::shared_ptr<SmsSendIndexer>)> sendRetryFun)
    : SmsSender(runner, subId, sendRetryFun)
{}

GsmSmsSender::~GsmSmsSender() {}

void GsmSmsSender::Init()
{
    rilManager_ = PhoneManager ::GetInstance().phone_[slotId_]->rilManager_;
    if (rilManager_ != nullptr) {
        HILOG_INFO("GsmSmsSender::Init OnRequesSmsStatus.");
        rilManager_->RegisterPhoneNotify(shared_from_this(), ObserverHandler::RADIO_SMS_STATUS, nullptr);
    }
}

void GsmSmsSender::TextBasedSmsDelivery(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback, const sptr<IDeliveryShortMessageCallback> &deliveCallback)
{
    bool bMore = false;
    bool bStatusReport;
    int ret = 0;
    int headerCnt;
    int cellsInfosSize;
    unsigned char msgRef8bit;
    SMS_CODING_SCHEME_E codingType;
    GsmSmsMessage gsmSmsMessage;
    std::vector<struct SpiltInfo> cellsInfos;
    gsmSmsMessage.SplitMessage(cellsInfos, text, false, codingType);
    std::shared_ptr<struct SmsTpdu> tpdu = gsmSmsMessage.CreateDefaultSubmitSmsTpdu(
        desAddr, scAddr, text, (deliveCallback == nullptr) ? false : true, codingType);
    if (tpdu == nullptr) {
        return;
    }
    cellsInfosSize = cellsInfos.size();
    msgRef8bit = GetMsgRef8Bit();
    bStatusReport = tpdu->data.submit.bStatusReport;

    HILOG_INFO("TextBasedSmsDelivery bStatusReport= %{public}d", bStatusReport);
    shared_ptr<uint8_t> unSentCellCount = make_shared<uint8_t>(cellsInfosSize);
    shared_ptr<bool> hasCellFailed = make_shared<bool>(false);
    chrono::system_clock::duration timePoint = chrono::system_clock::now().time_since_epoch();
    chrono::seconds sec = chrono::duration_cast<chrono::seconds>(timePoint);
    long timeStamp = sec.count();

    for (int i = 0; i < cellsInfosSize; i++) {
        headerCnt = 0;
        (void)memset_s(tpdu->data.submit.userData.data, MAX_USER_DATA_LEN + 1, 0x00, MAX_USER_DATA_LEN + 1);
        ret = memcpy_s(tpdu->data.submit.userData.data, MAX_USER_DATA_LEN + 1, &cellsInfos[i].encodeData[0],
            cellsInfos[i].encodeData.size());
        if (ret != EOK) {
            HILOG_ERROR("TextBasedSmsDelivery memcpy_s error.");
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
        headerCnt += gsmSmsMessage.SetHeaderReplay(headerCnt);
        /* Set User Data Header for National Language Single Shift */
        headerCnt += gsmSmsMessage.SetHeaderLang(headerCnt, codingType, cellsInfos[i].langId);
        tpdu->data.submit.userData.headerCnt = headerCnt;
        if (headerCnt > 0) {
            tpdu->data.submit.bHeaderInd = true;
        } else {
            tpdu->data.submit.bHeaderInd = false;
        }
        if (cellsInfosSize > 1 && i < (cellsInfosSize - 1)) {
            tpdu->data.submit.bStatusReport = false;
            bMore = true;
        } else {
            tpdu->data.submit.bStatusReport = bStatusReport;
            bMore = false;
        }
        std::shared_ptr<struct EncodeInfo> encodeInfo = gsmSmsMessage.GetSubmitEncodeInfo(scAddr, bMore);
        std::shared_ptr<SmsSendIndexer> indexer = nullptr;
        indexer = make_shared<SmsSendIndexer>(desAddr, scAddr, text, sendCallback, deliveCallback);
        if (encodeInfo == nullptr || indexer == nullptr) {
            HILOG_ERROR("create encodeInfo indexer == nullptr\r\n");
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
    const sptr<IDeliveryShortMessageCallback> &deliveCallback)
{
    uint8_t msgRef8bit = GetMsgRef8Bit();
    GsmSmsMessage gsmSmsMessage;
    std::shared_ptr<struct SmsTpdu> tpdu = gsmSmsMessage.CreateDataSubmitSmsTpdu(
        desAddr, scAddr, port, data, dataLen, msgRef8bit, (deliveCallback == nullptr) ? false : true);
    std::shared_ptr<struct EncodeInfo> encodeInfo = gsmSmsMessage.GetSubmitEncodeInfo(scAddr, false);
    if (encodeInfo == nullptr) {
        HILOG_ERROR("DataBasedSmsDelivery encodeInfo nullptr");
        return;
    }
    shared_ptr<SmsSendIndexer> indexer = nullptr;
    indexer = make_shared<SmsSendIndexer>(desAddr, scAddr, port, data, dataLen, sendCallback, deliveCallback);
    if (indexer == nullptr) {
        HILOG_ERROR("DataBasedSmsDelivery create SmsSendIndexer nullptr");
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
    SendSmsToRil(indexer);
}

void GsmSmsSender::SendSmsToRil(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        HILOG_ERROR("gsm_sms_sender: SendSms smsIndexer nullptr");
        return;
    }
    if (rilManager_ == nullptr) {
        HILOG_ERROR("gsm_sms_sender: SendSms rilManager nullptr");
        return;
    }
    int64_t refId = GetMsgRef64Bit();
    if (!SendCacheMapAddItem(refId, smsIndexer)) {
        HILOG_ERROR("SendCacheMapAddItem Error!!");
    }
    if (netDomainType_ != NET_DOMAIN_IMS && (smsIndexer->GetPsResendCount() == 0)) {
        uint8_t tryCount = smsIndexer->GetCsResendCount();
        if (tryCount > 0) {
            smsIndexer->UpdatePduForResend();
        }
        if (tryCount == 0 && smsIndexer->GetHasMore()) {
            auto reply = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SEND_SMS_EXPECT_MORE, refId);
            reply->SetOwner(shared_from_this());
            rilManager_->SendSmsMoreMode(StringUtils::StringToHex(smsIndexer->GetEncodeSmca()),
                StringUtils::StringToHex(smsIndexer->GetEncodePdu()), reply);
        } else {
            auto reply = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SEND_SMS, refId);
            reply->SetOwner(shared_from_this());
            rilManager_->SendSms(StringUtils::StringToHex(smsIndexer->GetEncodeSmca()),
                StringUtils::StringToHex(smsIndexer->GetEncodePdu()), reply);
        }
    } else {
        HILOG_DEBUG("ims network domain not support!");
    }
}

void GsmSmsSender::StatusReportAnalysis(const shared_ptr<SmsSendIndexer> &smsIndexer)
{
    if (smsIndexer == nullptr) {
        return;
    }
    if (rilManager_ == nullptr) {
        HILOG_ERROR("gsm_sms_sender: StatusReportAnalysis rilManager nullptr");
        return;
    }
    auto oldIndexer = reportList_.begin();
    while (oldIndexer != reportList_.end()) {
        auto iter = oldIndexer++;
        if (*iter != nullptr) {
            if (smsIndexer->GetMsgRefId() == (*iter)->GetMsgRefId()) {
                reportList_.erase(iter);
                // save the message to db, or updata to db msg state(success or failed)
                rilManager_->SendSmsAck(
                    true, AckIncomeCause::SMS_ACK_PROCESSED, AppExecFwk::InnerEvent::Pointer(nullptr, nullptr));
                sptr<IDeliveryShortMessageCallback> deliverCallback = smsIndexer->GetDeliveryCallback();
                if (deliverCallback != nullptr) {
                    std::string ackpdu = StringUtils::StringToHex(smsIndexer->GetAckPdu());
                    deliverCallback->OnSmsDeliveryResult(StringUtils::ToUtf16(ackpdu));
                }
            }
        }
    }
}

void GsmSmsSender::SetSendIndexerInfo(std::shared_ptr<SmsSendIndexer> &indexer,
    const std::shared_ptr<struct EncodeInfo> &encodeInfo, unsigned char msgRef8bit)
{
    if (encodeInfo == nullptr || indexer == nullptr) {
        HILOG_ERROR("CreateSendIndexer encodeInfo nullptr");
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

uint8_t GsmSmsSender::GetMsgRef8Bit()
{
    return ++msgRef8bit_;
}

int64_t GsmSmsSender::GetMsgRef64Bit()
{
    return ++msgRef64bit_;
}
} // namespace SMS
} // namespace OHOS