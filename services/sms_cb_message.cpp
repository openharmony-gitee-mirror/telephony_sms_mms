/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Copyright (C) 2014 Samsung Electronics Co., Ltd. All rights reserved
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

#include "sms_cb_message.h"

#include <string>

#include "securec.h"
#include "sms_common_utils.h"
#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
bool SmsCbMessage::operator==(const SmsCbMessage &other) const
{
    if (cbHeader_ == nullptr || other.cbHeader_ == nullptr) {
        return false;
    }

    return cbHeader_->serialNum.geoScope == other.cbHeader_->serialNum.geoScope &&
        cbHeader_->serialNum.msgCode == other.cbHeader_->serialNum.msgCode &&
        cbHeader_->msgId == other.cbHeader_->msgId;
}

std::shared_ptr<SmsCbMessage> SmsCbMessage::CreateCbMessage(const std::string &pdu)
{
    bool result = false;
    std::shared_ptr<SmsCbMessage> message = std::make_shared<SmsCbMessage>();
    if (message != nullptr) {
        result = message->PduAnalysis(StringUtils::HexToByteVector(pdu));
    }

    return (result ? message : nullptr);
}

std::shared_ptr<SmsCbMessage> SmsCbMessage::CreateCbMessage(const std::vector<unsigned char> &pdu)
{
    bool result = false;
    std::shared_ptr<SmsCbMessage> message = std::make_shared<SmsCbMessage>();
    if (message != nullptr) {
        result = message->PduAnalysis(pdu);
    }

    return (result ? message : nullptr);
}

std::shared_ptr<SmsCbMessageHeader> SmsCbMessage::GetCbHeader() const
{
    return cbHeader_;
}

std::string SmsCbMessage::GetCbMessageRaw() const
{
    return messageRaw_;
}

bool SmsCbMessage::IsSinglePageMsg() const
{
    return cbHeader_ != nullptr && (cbHeader_->totalPages == 1);
}

bool SmsCbMessage::PduAnalysis(const std::vector<unsigned char> &pdu)
{
    cbHeader_ = std::make_shared<SmsCbMessageHeader>();
    if (cbHeader_ == nullptr) {
        return false;
    }

    int offset = 0;
    // from 3GPP TS 23.041 V4.1.0 (2001-06) 9.1.1 9.1.2 section Protocols and Protocol Architecture
    if (pdu.size() <= (MAX_CBMSG_PAGE_SIZE + SMS_CB_HEADER_SIZE)) {
        cbHeader_->cbMsgType = SMS_CBMSG_TYPE_CBS;
        cbHeader_->cbNetType = SMS_NETTEXT_CB_MSG_GSM;
        offset = Decode2gHeader(pdu);
    } else {
        cbHeader_->cbMsgType = pdu[offset];
        cbHeader_->cbNetType = SMS_NETTEXT_CB_MSG_UMTS;
        offset = Decode3gHeader(pdu);
    }
    if (offset == 0 || pdu.size() <= (std::size_t)offset) {
        return false;
    }

    std::vector<unsigned char> bodyPdu(pdu.begin() + offset, pdu.end());
    if (cbHeader_->bEtwsMessage) {
        DecodeEtwsMsg(bodyPdu);
        return true;
    }

    switch (cbHeader_->cbNetType) {
        case SMS_NETTEXT_CB_MSG_GSM:
            Decode2gCbMsg(bodyPdu);
            break;
        case SMS_NETTEXT_CB_MSG_UMTS:
            Decode3gCbMsg(pdu);
            break;
        default:
            break;
    }
    return true;
}

int SmsCbMessage::Decode2gHeader(const std::vector<unsigned char> &pdu)
{
    int offset = 0;
    unsigned char temp;
    if (pdu.size() < SMS_CB_HEADER_SIZE) {
        return offset;
    }
    if (cbHeader_ == nullptr) {
        return offset;
    }

    cbHeader_->bEtwsMessage = false;
    temp = pdu[offset++];
    cbHeader_->serialNum.geoScope = (temp & 0xC0) >> 0x06;
    cbHeader_->serialNum.msgCode = (temp & 0x3F) << 0x04;

    temp = pdu[offset++];
    cbHeader_->serialNum.msgCode |= (temp & 0xF0) >> 0x04;
    cbHeader_->serialNum.updateNum = temp & 0x0F;

    temp = pdu[offset++];
    cbHeader_->msgId = (temp << 0x08) | pdu[offset++];

    if ((cbHeader_->msgId & 0xFFF8) == SMS_ETWS_BASE_MASK && pdu.size() <= MAX_ETWS_SIZE) {
        cbHeader_->bEtwsMessage = true;
        temp = pdu[offset++];
        cbHeader_->warningType = (temp << 0x08) | pdu[offset++];
    } else {
        unsigned char dcs = pdu[offset++];

        temp = pdu[offset++];
        cbHeader_->totalPages = temp & 0x0f;
        cbHeader_->page = (temp & 0xF0) >> 0x04;

        unsigned short iosTemp = pdu[offset++];
        iosTemp |= (pdu[offset++] << BYTE_BIT);
        DecodeCbMsgDCS(dcs, iosTemp, cbHeader_->dcs);

        cbHeader_->langType = cbHeader_->dcs.langType;
        cbHeader_->recvTime = GetRecvTime();
        if (cbHeader_->totalPages > MAX_CBMSG_PAGE_NUM) {
            TELEPHONY_LOGE("CB Page Count is over MAX[%{public}d]", cbHeader_->totalPages);
            offset = 0;
        }
        if ((cbHeader_->msgId & 0xFFF8) == SMS_ETWS_BASE_MASK) {
            cbHeader_->bEtwsMessage = true;
            cbHeader_->warningType = cbHeader_->msgId - SMS_ETWS_BASE_MASK;
        }
    }
    return offset;
}

int SmsCbMessage::Decode3gHeader(const std::vector<unsigned char> &pdu)
{
    int offset = 0;
    unsigned char temp;
    if (pdu.size() < SMS_CB_HEADER_SIZE) {
        return offset;
    }
    if (cbHeader_ == nullptr) {
        return offset;
    }

    offset++;
    temp = pdu[offset++];
    cbHeader_->msgId = (temp << 0x08) | pdu[offset++];

    temp = pdu[offset++];
    cbHeader_->serialNum.geoScope = (temp & 0xC0) >> 0x06;
    cbHeader_->serialNum.msgCode = (temp & 0x3F) << 0x04;

    temp = pdu[offset++];
    cbHeader_->serialNum.msgCode |= (temp & 0xF0) >> 0x04;
    cbHeader_->serialNum.updateNum = temp & 0x0F;

    unsigned char dcs = pdu[offset++];
    cbHeader_->totalPages = pdu[offset++];

    unsigned short iosTemp = pdu[offset++];
    iosTemp |= (pdu[offset++] << BYTE_BIT);

    DecodeCbMsgDCS(dcs, iosTemp, cbHeader_->dcs);
    cbHeader_->langType = cbHeader_->dcs.langType;
    cbHeader_->recvTime = GetRecvTime();
    return offset;
}

void SmsCbMessage::Decode2gCbMsg(const std::vector<unsigned char> &pdu)
{
    int offset = 0;
    int dataLen = static_cast<int>(pdu.size());
    if (cbHeader_ == nullptr) {
        TELEPHONY_LOGE("decode2gCbMsg null header");
    }
    switch (cbHeader_->dcs.codingScheme) {
        case SMS_CODING_7BIT: {
            dataLen = (dataLen * BYTE_BIT) / ENCODE_GSM_BIT;
            unsigned char pageData[MAX_CBMSG_PAGE_SIZE * BYTE_BIT / ENCODE_GSM_BIT] = {0};
            int unpackLen = SmsCommonUtils::Unpack7bitChar(pdu.data(), dataLen, 0x00, pageData);

            if (cbHeader_->dcs.iso639Lang[0]) {
                unpackLen = unpackLen - SMS_CB_IOS639LANG_SIZE;
                offset = SMS_CB_IOS639LANG_SIZE;
            }
            messageRaw_.insert(0, (const char *)&pageData[offset], unpackLen);
            break;
        }
        case SMS_CODING_8BIT:
        case SMS_CODING_UCS2: {
            if (cbHeader_->dcs.iso639Lang[0]) {
                offset = 0x02;
                dataLen -= offset;
            }

            if (dataLen > 0) {
                messageRaw_.insert(0, (const char *)(pdu.data() + offset), dataLen);
            }
            break;
        }
        default:
            break;
    }
}

void SmsCbMessage::Decode3gCbMsg(const std::vector<unsigned char> &pdu)
{
    int dataLen = 0;
    int offset = 0;
    if (cbHeader_ == nullptr) {
        TELEPHONY_LOGE("Decode3gCbMsg cbHeader null err.");
        return;
    }
    const int pduLen = static_cast<int>(pdu.size());
    const unsigned char *tpdu = pdu.data();
    switch (cbHeader_->dcs.codingScheme) {
        case SMS_CODING_7BIT: {
            TELEPHONY_LOGD("SMS_CODING_7BIT %{public}d", cbHeader_->totalPages);
            for (int i = 0; i < cbHeader_->totalPages; ++i) {
                unsigned char pageLenOffset = (ENCODE_GSM_BIT + (i + 1) * MAX_CBMSG_PAGE_SIZE + i);
                if (pduLen < pageLenOffset) {
                    TELEPHONY_LOGE("CB Msg Size err [%{pulbic}d]", pduLen);
                    messageRaw_.clear();
                    return;
                }

                dataLen = tpdu[pageLenOffset];
                offset = ENCODE_GSM_BIT + (i * MAX_CBMSG_PAGE_SIZE) + i;
                if (dataLen > MAX_CBMSG_PAGE_SIZE) {
                    TELEPHONY_LOGE("CB Msg Size is over MAX [%{pulbic}d]", dataLen);
                    messageRaw_.clear();
                    return;
                }

                int unpackLen = 0;
                dataLen = (dataLen * BYTE_BIT) / ENCODE_GSM_BIT;
                unsigned char pageData[MAX_CBMSG_PAGE_SIZE * BYTE_BIT / ENCODE_GSM_BIT] = {0};
                unpackLen = SmsCommonUtils::Unpack7bitChar(&tpdu[offset], dataLen, 0x00, pageData);
                messageRaw_.insert(messageRaw_.size(), (const char *)pageData, unpackLen);
            }
            break;
        }

        case SMS_CODING_8BIT:
        case SMS_CODING_UCS2: {
            for (int i = 0; i < cbHeader_->totalPages; ++i) {
                unsigned char pageLenOffset = (ENCODE_GSM_BIT + (i + 1) * MAX_CBMSG_PAGE_SIZE + i);
                if (pduLen < pageLenOffset) {
                    TELEPHONY_LOGD("CB Msg Size err [%{pulbic}d]", pduLen);
                    messageRaw_.clear();
                    return;
                }

                if (cbHeader_->dcs.iso639Lang[0]) {
                    dataLen = tpdu[pageLenOffset] - 0x02;
                    offset = ENCODE_GSM_BIT + (i * MAX_CBMSG_PAGE_SIZE) + i + 0x02;
                } else {
                    dataLen = tpdu[pageLenOffset];
                    offset = ENCODE_GSM_BIT + (i * MAX_CBMSG_PAGE_SIZE) + i;
                }

                if (dataLen > 0 && dataLen <= MAX_CBMSG_PAGE_SIZE) {
                    messageRaw_.insert(messageRaw_.size(), (const char *)&tpdu[offset], dataLen);
                }
            }
            break;
        }
        default:
            break;
    }
    cbHeader_->totalPages = 1;
}

void SmsCbMessage::DecodeEtwsMsg(const std::vector<unsigned char> &pdu)
{
    int offset = 0;
    if (pdu.size() > MAX_ETWS_SIZE) {
        TELEPHONY_LOGE("ETWS Msg Size is over MAX [%{public}zu]", pdu.size());
        return;
    }

    messageRaw_.insert(0, (const char *)(pdu.data() + offset), pdu.size() - offset);
}

void SmsCbMessage::ConvertToUTF8(const std::string &raw, std::string &message) const
{
    if (cbHeader_ == nullptr) {
        TELEPHONY_LOGE("ConvertToUTF8 cbHeader null err.");
        return;
    }
    if (cbHeader_->bEtwsMessage) {
        message.assign(raw);
    } else {
        MsgTextConvert *textCvt = MsgTextConvert::Instance();
        if (textCvt == nullptr) {
            TELEPHONY_LOGE("MsgTextConvert Instance nullptr");
            return;
        }

        int codeSize = 0;
        MsgLangInfo langInfo = {
            0,
        };
        unsigned char outBuf[MAX_CB_MSG_TEXT_LEN + 1] = {0};
        if (cbHeader_->dcs.codingScheme == SMS_CODING_7BIT) {
            codeSize = textCvt->ConvertGSM7bitToUTF8(
                outBuf, sizeof(outBuf), (unsigned char *)raw.data(), raw.length(), &langInfo);
        } else if (cbHeader_->dcs.codingScheme == SMS_CODING_UCS2) {
            codeSize =
                textCvt->ConvertUCS2ToUTF8(outBuf, sizeof(outBuf), (unsigned char *)raw.data(), raw.length());
        }

        outBuf[codeSize] = '\0';
        message.insert(0, (const char *)outBuf, codeSize);
    }
}

unsigned short SmsCbMessage::EncodeCbSerialNum(const SmsCBMessageSerialNum &snFields)
{
    unsigned short serialNum = 0;
    serialNum = ((snFields.geoScope & 0x03) << 0x0E) | ((snFields.msgCode & 0x03FF) << 0x04) |
        (snFields.updateNum & 0x0F);
    return serialNum;
}

int SmsCbMessage::CMASClass(const unsigned short messageId) const
{
    int ret = 0;
    switch (messageId) {
        case CMAS_PRESIDENTIAL_DEFUALT:
        case CMAS_PRESIDENTIAL_SPANISH:
            ret = MSG_CMAS_PRESIDENTIAL;
            break;
        case CMAS_EXTREME_OBSERVED_DEFUALT:
        case CMAS_EXTREME_OBSERVED_SPANISH:
        case CMAS_EXTREME_LIKELY_DEFUALT:
        case CMAS_EXTREME_LIKELY_SPANISH:
            ret = MSG_CMAS_EXTREME;
            break;
        case CMAS_SERVER_EXTERME_OBSERVED_DEFUALT:
        case CMAS_SERVER_EXTREME_OBSERVED_SPANISH:
        case CMAS_SERVER_EXTERME_LIKELY_DEFUALT:
        case CMAS_SERVER_EXTERME_LIKELY_SPANISH:
        case CMAS_SERVER_SERVER_OBSERVED_DEFUALT:
        case CMAS_SERVER_SERVER_OBSERVED_SPANISH:
        case CMAS_SERVER_SERVER_LIKELY_DEFUALT:
        case CMAS_SERVER_SERVER_LIKELY_SPANISH:
        case CMAS_SERVER_SERVER_EXPECTED_OBSERVED_DEFUALT:
        case CMAS_SERVER_SERVER_EXPECTED_OBSERVED_SPANISH:
        case CMAS_SERVER_SERVER_EXPECTED_LIKELY_DEFUALT:
        case CMAS_SERVER_SERVER_EXPECTED_LIKELY_SPANISH:
            ret = MSG_CMAS_SEVERE;
            break;
        case CMAS_AMBER_ALERT_DEFUALT:
        case CMAS_AMBER_ALERT_SPANISH:
            ret = MSG_CMAS_AMBER;
            break;
        case CMAS_RMT_ALERT_DEFUALT:
        case CMAS_RMT_ALERT_SPANISH:
            ret = MSG_CMAS_TEST;
            break;
        case CMAS_EXERCISE_ALERT_DEFUALT:
        case CMAS_EXERCISE_ALERT_SPANISH:
            ret = MSG_CMAS_EXERCISE;
            break;
        case CMAS_OPERATOR_ALERT_DEFUALT:
        case CMAS_OPERATOR_ALERT_SPANISH:
            ret = MSG_CMAS_OPERATOR_DEFINED;
            break;
        default:
            break;
    }
    return ret;
}

/**
 * @brief DecodeIos639Dcs
 *  from 3GPP TS 23.038 V4.3.0 (2001-09)  6.1.1 section
 *  Control characters
 * @param dcsData
 * @param iosData
 * @param pDcs
 */
void SmsCbMessage::DecodeIos639Dcs(
    const unsigned char dcsData, const unsigned short iosData, SmsCbMessageDcs &pDcs) const
{
    unsigned char dcsLow = (dcsData & 0x0F);
    switch (dcsLow) {
        case 0x00:
        case 0x01: {
            pDcs.codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
            pDcs.codingScheme = (dcsData & 0x01) ? SMS_CODING_UCS2 : SMS_CODING_7BIT;
            pDcs.langType = SMS_CBMSG_LANG_ISO639;
            unsigned char hight = (iosData >> BYTE_BIT);
            unsigned char low = iosData;
            // from 3GPP TS 23.038 V4.3.0 (2001-09)  6.1.1 section Control characters
            if (hight && low) {
                pDcs.iso639Lang[0x00] = hight & 0x7F;
                pDcs.iso639Lang[0x01] = (hight & 0X80) >> 0x07;
                pDcs.iso639Lang[0x01] |= (low & 0X3F) << 0x01;
                pDcs.iso639Lang[0x02] = 0x13; /* CR */
            } else {
                pDcs.iso639Lang[0x00] = 0x45; /* E */
                pDcs.iso639Lang[0x01] = 0x4E; /* N */
                pDcs.iso639Lang[0x02] = 0x13; /* CR */
            }
            break;
        }
        default:
            break;
    }
}

/**
 * @brief DecodeGeneralDcs
 *  from 3GPP TS 23.038 V4.3.0 (2001-09) 6 section
 *  CBS Data Coding Scheme
 * @param dcsData
 * @param iosData
 * @param pDcs
 */
void SmsCbMessage::DecodeGeneralDcs(const unsigned char dcsData, SmsCbMessageDcs &pDcs) const
{
    pDcs.codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
    pDcs.bCompressed = (dcsData & 0x20) ? true : false;
    if (dcsData & 0x10) {
        pDcs.classType = (dcsData & 0x03);
    }
    unsigned char tmpScheme = (dcsData & 0x0C) >> 0x02;
    switch (tmpScheme) {
        case 0x00:
            pDcs.codingScheme = SMS_CODING_7BIT;
            break;
        case 0x01:
            pDcs.codingScheme = SMS_CODING_8BIT;
            break;
        case 0x02:
            pDcs.codingScheme = SMS_CODING_UCS2;
            break;
        default:
            break;
    }
}

/**
 * @brief DecodeCbMsgDCS
 *  from 3GPP TS 23.038 V4.3.0 (2001-09) 6 section
 *  CBS Data Coding Scheme
 * @param dcsData
 * @param iosData
 * @param pDcs
 */
void SmsCbMessage::DecodeCbMsgDCS(
    const unsigned char dcsData, const unsigned short iosData, SmsCbMessageDcs &pDcs) const
{
    pDcs.codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
    pDcs.classType = static_cast<unsigned char>(SmsMessageClass::SMS_CLASS_UNKNOWN);
    pDcs.bCompressed = false;
    pDcs.codingScheme = SMS_CODING_7BIT;
    pDcs.langType = SMS_CBMSG_LANG_UNSPECIFIED;
    (void)memset_s(pDcs.iso639Lang, sizeof(pDcs.iso639Lang), 0x00, sizeof(pDcs.iso639Lang));
    pDcs.bUDH = false;
    pDcs.rawData = dcsData;
    unsigned char codingGroup = (dcsData & 0xF0) >> 0x04;

    switch (codingGroup) {
        case 0x00:
        case 0x02:
        case 0x03:
            pDcs.codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
            pDcs.langType = dcsData;
            break;
        case 0x01:
            DecodeIos639Dcs(dcsData, iosData, pDcs);
            break;
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            DecodeGeneralDcs(dcsData, pDcs);
            break;
        case 0x09:
            pDcs.bUDH = true;
            pDcs.classType = dcsData & 0x03;
            pDcs.codingScheme = (dcsData & 0x0C) >> 0x02;
            break;
        case 0x0E:
            pDcs.codingGroup = SMS_CBMSG_CODGRP_WAP;
            break;
        case 0x0F:
            pDcs.codingGroup = SMS_CBMSG_CODGRP_CLASS_CODING;
            pDcs.codingScheme = (dcsData & 0x04) ? SMS_CODING_8BIT : SMS_CODING_7BIT;
            pDcs.classType = dcsData & 0x03;
            break;
        default:
            TELEPHONY_LOGE("codingGrp: [0x%{public}x]", codingGroup);
            break;
    }
}

unsigned long SmsCbMessage::GetRecvTime() const
{
    time_t recvTime = time(NULL);
    return (unsigned long)recvTime;
}

std::string SmsCbMessage::ToString() const
{
    if (cbHeader_ == nullptr) {
        return "SmsCbMessage Header nullptr";
    }

    std::string msgId("msgId:" + std::to_string(cbHeader_->msgId));
    std::string langType("\nlangType:" + std::to_string(cbHeader_->langType));
    std::string isEtwsMessage("\nisEtws:" + std::to_string(cbHeader_->bEtwsMessage));
    std::string cbMsgType("\ncbMsgType:" + std::to_string(cbHeader_->cbMsgType));
    std::string warningType("\nwarningType:" + std::to_string(cbHeader_->warningType));
    std::string serialNum("\nserialNum: geoScope " + std::to_string(cbHeader_->serialNum.geoScope) +
        "| updateNum " + std::to_string(cbHeader_->serialNum.updateNum) + "| msgCode " +
        std::to_string(cbHeader_->serialNum.msgCode));
    std::string page(
        "\ntotalpage: " + std::to_string(cbHeader_->totalPages) + " page:" + std::to_string(cbHeader_->page));
    std::string recvTime("\nrecvTime: " + std::to_string(cbHeader_->recvTime));
    return msgId.append(langType)
        .append(isEtwsMessage)
        .append(cbMsgType)
        .append(warningType)
        .append(serialNum)
        .append(page)
        .append(recvTime);
}
} // namespace Telephony
} // namespace OHOS
