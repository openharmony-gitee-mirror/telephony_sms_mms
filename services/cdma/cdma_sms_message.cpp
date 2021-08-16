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

#include "cdma_sms_message.h"

#include <cmath>

#include "securec.h"
#include "sms_common_utils.h"
#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
std::shared_ptr<CdmaSmsMessage> CdmaSmsMessage::CreateMessage(const std::string &pdu)
{
    std::shared_ptr<CdmaSmsMessage> message = std::make_shared<CdmaSmsMessage>();
    if (message == nullptr) {
        TELEPHONY_LOGE("CreateMessage message nullptr");
        return nullptr;
    }
    message->transMsg_ = std::make_unique<struct SmsTransMsg>();
    if (message->transMsg_ == nullptr) {
        TELEPHONY_LOGE("CreateMessage message transMsg_ nullptr");
        return nullptr;
    }

    (void)memset_s(message->transMsg_.get(), sizeof(struct SmsTransMsg), 0x00, sizeof(struct SmsTransMsg));
    if (message->PduAnalysis(pdu)) {
        return message;
    }
    return nullptr;
}

bool CdmaSmsMessage::PduAnalysis(const std::string &pduHex)
{
    if (transMsg_ == nullptr || pduHex.empty()) {
        TELEPHONY_LOGE("PduAnalysis is unInvalid param!");
        return false;
    }

    rawPdu_ = StringUtils::HexToByteVector(pduHex);
    if (!CdmaSmsPduCodec::CheckInvalidPDU(rawPdu_)) {
        TELEPHONY_LOGE("PduAnalysis is unInvalid pdu data!");
        return false;
    }

    int decodeLen = 0;
    std::string pdu = StringUtils::HexToString(pduHex);
    decodeLen = CdmaSmsPduCodec::DecodeMsg((const unsigned char *)pdu.c_str(), pdu.length(), *transMsg_);
    if (decodeLen <= 0) {
        TELEPHONY_LOGE("Pdu DecodeMsg has failure.");
        return false;
    }

    if (transMsg_->type == SMS_TRANS_BROADCAST_MSG) {
        if (transMsg_->data.cbMsg.telesvcMsg.data.deliver.cmasData.isWrongRecodeType) {
            TELEPHONY_LOGE("Invalid CMAS Record Type");
            return false;
        }
        SmsEncodingType encodeType = transMsg_->data.cbMsg.telesvcMsg.data.deliver.cmasData.encodeType;
        if ((encodeType == SMS_ENCODE_KOREAN) || (encodeType == SMS_ENCODE_GSMDCS)) {
            TELEPHONY_LOGE("This encode type is not supported [%{public}d]", encodeType);
            return false;
        }
    }

    switch (transMsg_->type) {
        case SMS_TRANS_P2P_MSG:
            AnalysisP2pMsg(transMsg_->data.p2pMsg);
            break;
        case SMS_TRANS_BROADCAST_MSG:
            AnalysisCbMsg(transMsg_->data.cbMsg);
            break;
        case SMS_TRANS_ACK_MSG:
            AnalsisAckMsg(transMsg_->data.ackMsg);
            break;
        default:
            return false;
    }
    return true;
}

void CdmaSmsMessage::AnalysisP2pMsg(const SmsTransP2PMsg &p2pMsg)
{
    if (p2pMsg.transTelesvcId == SMS_TRANS_TELESVC_RESERVED) {
        TELEPHONY_LOGE("this Incoming Message has Unknown Teleservice ID");
        return;
    }

    address_ = std::make_unique<struct SmsTransAddr>();
    if (address_ == nullptr) {
        TELEPHONY_LOGE("AnalysisP2pMsg make address == nullptr");
        return;
    }

    if (memcpy_s(address_.get(), sizeof(SmsTransAddr), &p2pMsg.address, sizeof(SmsTransAddr)) != EOK) {
        TELEPHONY_LOGE("AnalysisP2pMsg address memcpy_s error.");
        return;
    }

    switch (p2pMsg.telesvcMsg.type) {
        case SmsMessageType::SMS_TYPE_DELIVER:
            AnalsisDeliverMwi(p2pMsg);
            AnalsisDeliverMsg(p2pMsg.telesvcMsg.data.deliver);
            break;
        case SmsMessageType::SMS_TYPE_DELIVERY_ACK:
            AnalsisDeliverAck(p2pMsg.telesvcMsg.data.deliveryAck);
            break;
        case SmsMessageType::SMS_TYPE_USER_ACK:
        case SmsMessageType::SMS_TYPE_READ_ACK:
            break;
        case SmsMessageType::SMS_TYPE_SUBMIT_REPORT:
            AnalsisSubmitReport(p2pMsg.telesvcMsg.data.report);
            break;
        default:
            break;
    }
}

void CdmaSmsMessage::AnalsisDeliverMwi(const SmsTransP2PMsg &p2pMsg)
{
    specialSmsInd_ = nullptr;
    if (p2pMsg.transTelesvcId == SMS_TRANS_TELESVC_VMN_95) {
        specialSmsInd_ = std::make_shared<SpecialSmsIndication>();
        if (specialSmsInd_ == nullptr) {
            return;
        }

        specialSmsInd_->msgInd = SMS_VOICE_INDICATOR;
        if (p2pMsg.telesvcMsg.data.deliver.enhancedVmn.faxIncluded) {
            specialSmsInd_->msgInd = SMS_FAX_INDICATOR;
        }

        if (p2pMsg.telesvcMsg.data.deliver.numMsg < 0) {
            specialSmsInd_->waitMsgNum = 0;
            bMwiClear_ = true;
            bMwiSet_ = false;
        } else {
            specialSmsInd_->waitMsgNum = p2pMsg.telesvcMsg.data.deliver.numMsg;
            bMwiClear_ = false;
            bMwiSet_ = true;
        }

        bMwiNotStore_ = false;
        if (bMwiSet_ && (p2pMsg.telesvcMsg.data.deliver.userData.dataLen == 0)) {
            bMwiNotStore_ = true;
        }
    }
}

void CdmaSmsMessage::AnalsisDeliverMsg(const SmsTeleSvcDeliver &deliver)
{
    msgClass_ = SMS_CLASS_UNKNOWN;
    if (deliver.displayMode == SMS_DISPLAY_IMMEDIATE) {
        msgClass_ = SMS_INSTANT_MESSAGE;
    }

    msgRef_ = deliver.msgId.msgId;
    scTimestamp_ = SmsCommonUtils::ConvertTime(deliver.timeStamp);
    AnalsisUserData(deliver.userData);
}

void CdmaSmsMessage::AnalsisDeliverAck(const SmsTeleSvcDeliverAck &deliverAck)
{
    bStatusReportMessage_ = true;
    scTimestamp_ = SmsCommonUtils::ConvertTime(deliverAck.timeStamp);
    AnalsisUserData(deliverAck.userData);
}

void CdmaSmsMessage::AnalsisSubmitReport(const SmsTeleSvcDeliverReport &report)
{
    AnalsisUserData(report.userData);
}

void CdmaSmsMessage::AnalsisUserData(const SmsTeleSvcUserData &userData)
{
    int dataSize = 0;
    MsgLangInfo langinfo = {
        0,
    };

    MsgTextConvert *textCvt = MsgTextConvert::Instance();
    if (textCvt == nullptr) {
        return;
    }

    unsigned char buff[MAX_MSG_TEXT_LEN + 1] = {0};
    switch (userData.encodeType) {
        case SMS_ENCODE_GSM7BIT: {
            dataSize = textCvt->ConvertGSM7bitToUTF8(
                buff, MAX_MSG_TEXT_LEN, (unsigned char *)&userData.userData, userData.dataLen, &langinfo);
            break;
        }
        case SMS_ENCODE_KOREAN:
        case SMS_ENCODE_EUCKR: {
            dataSize = textCvt->ConvertEUCKRToUTF8(
                buff, MAX_MSG_TEXT_LEN, (unsigned char *)&userData.userData, userData.dataLen);
            break;
        }
        case SMS_ENCODE_IA5:
        case SMS_ENCODE_7BIT_ASCII:
        case SMS_ENCODE_LATIN_HEBREW:
        case SMS_ENCODE_LATIN:
        case SMS_ENCODE_OCTET: {
            if (memcpy_s(buff, sizeof(buff), userData.userData, userData.dataLen) != EOK) {
                TELEPHONY_LOGE("AnalsisDeliverMsg memcpy_s fail.");
                return;
            }
            dataSize = userData.dataLen;
            buff[dataSize] = '\0';
            break;
        }
        case SMS_ENCODE_SHIFT_JIS: {
            dataSize = textCvt->ConvertSHIFTJISToUTF8(
                buff, MAX_MSG_TEXT_LEN, (unsigned char *)&userData.userData, userData.dataLen);
            break;
        }
        default: {
            dataSize = textCvt->ConvertUCS2ToUTF8(
                buff, MAX_MSG_TEXT_LEN, (unsigned char *)&userData.userData, userData.dataLen);
            break;
        }
    }
    visibleMessageBody_.insert(0, (char *)buff, dataSize);
}

void CdmaSmsMessage::AnalsisCMASMsg(const SmsTeleSvcDeliver &deliver)
{
    msgClass_ = SMS_CLASS_UNKNOWN;
    scTimestamp_ = SmsCommonUtils::ConvertTime(deliver.timeStamp);
    SmsTeleSvcUserData userData;
    (void)memset_s(&userData, sizeof(SmsTeleSvcUserData), 0x00, sizeof(SmsTeleSvcUserData));
    userData.dataLen = deliver.cmasData.dataLen;
    userData.encodeType = deliver.cmasData.encodeType;
    if (memcpy_s(userData.userData, sizeof(userData.userData), deliver.cmasData.alertText,
        deliver.cmasData.dataLen) == EOK) {
        AnalsisUserData(userData);
    }
}

void CdmaSmsMessage::AnalysisCbMsg(const SmsTransBroadCastMsg &cbMsg)
{
    if (cbMsg.telesvcMsg.type != SmsMessageType::SMS_TYPE_DELIVER) {
        TELEPHONY_LOGE("No matching type = [%{public}d]", cbMsg.telesvcMsg.type);
        return;
    }

    TELEPHONY_LOGI("analysisCbMsg transSvcCtg %{public}d", cbMsg.transSvcCtg);
    if ((cbMsg.transSvcCtg >= SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL) &&
        (cbMsg.transSvcCtg <= SMS_TRANS_SVC_CTG_CMAS_TEST)) {
        AnalsisCMASMsg(cbMsg.telesvcMsg.data.deliver);
    } else {
        AnalsisDeliverMsg(cbMsg.telesvcMsg.data.deliver);
    }
}

void CdmaSmsMessage::AnalsisAckMsg(const SmsTransAckMsg &ackMsg)
{
    originatingAddress_ = ackMsg.address.szData;
}

bool CdmaSmsMessage::AddUserDataHeader(const struct SmsUDH &header)
{
    if (userHeaders_.size() >= CDMA_MAX_UD_HEADER_NUM) {
        return false;
    }

    userHeaders_.push_back(header);
    return true;
}

/**
 * @brief GetTransMsgType
 * 0x00 is point to point message
 * 0x01 is broadcast message
 * 0x02 is ack message
 * 0x03 is unkown message
 * @return int
 */
int CdmaSmsMessage::GetTransMsgType() const
{
    if (transMsg_ == nullptr) {
        return SMS_TRANS_TYPE_RESERVED;
    }

    return transMsg_->type;
}

/**
 * @brief Get the Trans Tele Service object
 * 0x1000 IS-91 Extended Protocol Enhanced Services
 * 0x1001 Wireless Paging Teleservice
 * 0x1002 Wireless Messaging Teleservice
 * 0x1003 Voice Mail Notification
 * 0x1004 Wireless Application Protocol
 * 0x1005 Wireless Enhanced Messaging Teleservice
 * 0x1006 Service Category Programming Teleservice
 * 0x1007 Card Application Toolkit Protocol Teleservice
 * 0xffff
 * @return int
 */
int CdmaSmsMessage::GetTransTeleService() const
{
    if ((transMsg_ == nullptr) || (transMsg_->type != SMS_TRANS_P2P_MSG)) {
        return SMS_TRANS_TELESVC_RESERVED;
    }

    return transMsg_->data.p2pMsg.transTelesvcId;
}

int CdmaSmsMessage::GetProtocolId() const
{
    return 0;
}

bool CdmaSmsMessage::IsReplaceMessage()
{
    return false;
}

bool CdmaSmsMessage::IsCphsMwi() const
{
    return false;
}

bool CdmaSmsMessage::IsWapPushMsg()
{
    if (transMsg_ == nullptr) {
        return false;
    }

    if (transMsg_->type == SMS_TRANS_P2P_MSG) {
        return (transMsg_->data.p2pMsg.transTelesvcId == SMS_TRANS_TELESVC_WAP);
    }
    return false;
}

std::shared_ptr<SpecialSmsIndication> CdmaSmsMessage::GetSpecialSmsInd()
{
    return specialSmsInd_;
}
} // namespace Telephony
} // namespace OHOS