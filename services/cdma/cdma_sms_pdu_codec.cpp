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

#include "cdma_sms_pdu_codec.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>

#include "securec.h"

#include "msg_text_convert.h"
#include "sms_common_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void ShiftNBit(unsigned char *src, unsigned int nBytes, unsigned int nShiftBit)
{
    unsigned char temp;
    if (src == nullptr) {
        return;
    }

    for (unsigned int index = 1; index < nBytes; index++) {
        temp = src[index] >> (BYTE_BITS - nShiftBit);
        src[index - 1] |= temp;
        src[index] = src[index] << nShiftBit;
    }
}

void ShiftNBitForDecode(unsigned char *src, unsigned int nBytes, unsigned int nShiftBit)
{
    if (src == nullptr) {
        return;
    }

    for (unsigned int index = 0; index < nBytes; index++) {
        src[index] <<= nShiftBit;
        src[index] |= (src[index + 1] >> (BYTE_BITS - nShiftBit));
    }
}

unsigned char CdmaSmsPduCodec::ConvertToBCD(const unsigned char val)
{
    return static_cast<unsigned char>((val / DECIMAL_NUM) << SHIFT_4BITS) | (val % DECIMAL_NUM);
}

unsigned char CdmaSmsPduCodec::DecodeDigitModeNumberType(const unsigned char val, bool isNumber)
{
    unsigned char temp = (val >> SHIFT_5BITS);
    if (isNumber) {
        if (temp >= SMS_TRANS_DNET_UNKNOWN && temp <= SMS_TRANS_DNET_RESERVED) {
            return temp;
        } else {
            return SMS_TRANS_DNET_RESERVED;
        }
    } else {
        if (temp >= SMS_NUMBER_TYPE_UNKNOWN && temp <= SMS_NUMBER_TYPE_RESERVED_7) {
            return temp;
        }
        return SMS_NUMBER_TYPE_UNKNOWN;
    }
}

unsigned char CdmaSmsPduCodec::DecodeDigitModeNumberPlan(const unsigned char val)
{
    unsigned char numberPlan = 0;
    switch (val) {
        case SMS_NPI_UNKNOWN:
            numberPlan = SMS_NPI_UNKNOWN;
            break;
        case SMS_NPI_ISDN:
            numberPlan = SMS_NPI_ISDN;
            break;
        case SMS_NPI_DATA:
            numberPlan = SMS_NPI_DATA;
            break;
        case SMS_NPI_TELEX:
            numberPlan = SMS_NPI_TELEX;
            break;
        case SMS_NPI_PRIVATE:
            numberPlan = SMS_NPI_PRIVATE;
            break;
        case SMS_NPI_RESERVED:
        default:
            numberPlan = SMS_NPI_RESERVED;
            break;
    }
    return numberPlan;
}

bool CdmaSmsPduCodec::CheckInvalidPDU(const std::vector<unsigned char> &pduStr)
{
    std::size_t offset = 0;
    if (pduStr.size() < MIN_PDU_LEN) {
        TELEPHONY_LOGD("Invalid PDU : pdu size less than 2 is invalid");
        return false;
    }
    if (!(pduStr[offset] == SMS_TRANS_P2P_MSG || pduStr[offset] == SMS_TRANS_BROADCAST_MSG ||
        pduStr[offset] == SMS_TRANS_ACK_MSG)) {
        TELEPHONY_LOGD("Invalid PDU : Message Type [%{public}2x]", pduStr[offset]);
        return false;
    }
    offset++;
    while (offset < pduStr.size()) {
        if (pduStr[offset] >= 0 && pduStr[offset] <= 0x08) {
            offset += (pduStr[offset + 1] + HEX_BYTE_STEP);
        } else {
            TELEPHONY_LOGD("Invalid PDU : Parameter ID [%{public}2x]", pduStr[offset]);
            return false;
        }
    }
    if (offset != pduStr.size()) {
        return false;
    }
    return true;
}

int CdmaSmsPduCodec::EncodeMsg(const struct SmsTransMsg &transMsg, unsigned char *pduStr)
{
    int encodeSize = 0;
    switch (transMsg.type) {
        case SMS_TRANS_P2P_MSG:
            encodeSize = EncodeP2PMsg(transMsg.data.p2pMsg, pduStr);
            break;
        case SMS_TRANS_BROADCAST_MSG:
            encodeSize = EncodeCBMsg(transMsg.data.cbMsg, pduStr);
            break;
        case SMS_TRANS_ACK_MSG:
            encodeSize = EncodeAckMsg(transMsg.data.ackMsg, pduStr);
            break;
        default:
            break;
    }
    return encodeSize;
}

int CdmaSmsPduCodec::EncodeP2PMsg(const struct SmsTransP2PMsg &p2pMsg, unsigned char *pduStr)
{
    int offset = 0;
    int encodeSize = 0;
    int index = 0;
    if (pduStr == nullptr) {
        return offset;
    }

    pduStr[offset++] = SMS_TRANS_P2P_MSG;
    /* 1. teleservice id */
    pduStr[offset++] = SMS_TRANS_PARAM_TELESVC_IDENTIFIER;
    pduStr[offset++] = 0x02;
    pduStr[offset++] = (0xff00 & p2pMsg.transTelesvcId) >> SHIFT_8BITS;
    pduStr[offset++] = (0x00ff & p2pMsg.transTelesvcId);
    /* 2. Service category */
    if (p2pMsg.transSvcCtg < SMS_TRANS_SVC_CTG_UNDEFINED) {
        pduStr[offset++] = SMS_TRANS_PARAM_SERVICE_CATEGORY;
        pduStr[offset++] = 0x02;
        pduStr[offset++] = (0xff00 & p2pMsg.transSvcCtg) >> SHIFT_8BITS;
        pduStr[offset++] = (0x00ff & p2pMsg.transSvcCtg);
    }
    /* 3. Address */
    offset += EncodeAddress(p2pMsg.address, &pduStr[offset]);
    /* 4. Sub address (optional) */
    offset += EncodeSubAddress(p2pMsg.subAddress, &pduStr[offset]);
    /* 5. Bearer reply option (optional) */
    if (p2pMsg.transReplySeq > 0) {
        pduStr[offset++] = SMS_TRANS_PARAM_BEARER_REPLY_OPTION;
        pduStr[offset++] = 0x01;
        pduStr[offset++] = (p2pMsg.transReplySeq << SHIFT_2BITS);
        TELEPHONY_LOGD("Reply sequnce number = [%{public}d]", p2pMsg.transReplySeq);
    }
    /* 6. Bearer data */
    pduStr[offset++] = SMS_TRANS_PARAM_BEARER_DATA;
    /* PARAMETER_LEN field should be filled at the last part. */
    index = offset++;
    encodeSize = EncodeTelesvcMsg(p2pMsg.telesvcMsg, &pduStr[offset]);
    /* PARAMETER_LEN */
    pduStr[index] = encodeSize;
    offset += encodeSize;
    return offset;
}

int CdmaSmsPduCodec::EncodeCBMsg(const struct SmsTransBroadCastMsg &cbMsg, unsigned char *pduStr)
{
    int offset = 0;
    int encodeSize = 0;
    int lenIndex = 0;

    if (pduStr == nullptr) {
        return offset;
    }
    /* 1. Service Category(Mandatory) */
    pduStr[offset++] = SMS_TRANS_PARAM_SERVICE_CATEGORY;
    pduStr[offset++] = 0x02;

    pduStr[offset++] = (0xff00 & cbMsg.transSvcCtg) >> SHIFT_8BITS;
    pduStr[offset++] = (0x00ff & cbMsg.transSvcCtg);

    /* 2. Bearer Data(Optional) */
    pduStr[offset++] = SMS_TRANS_PARAM_BEARER_DATA;
    /* PARAMETER_LEN field should be filled at the last part. */
    lenIndex = offset++;
    encodeSize = EncodeTelesvcMsg(cbMsg.telesvcMsg, &pduStr[offset]);
    /* PARAMETER_LEN */
    pduStr[lenIndex] = encodeSize;
    offset += encodeSize;
    return offset;
}

int CdmaSmsPduCodec::EncodeAckMsg(const struct SmsTransAckMsg &ackMsg, unsigned char *pduStr)
{
    int offset = 0;
    int index = 0;
    if (pduStr == nullptr) {
        return offset;
    }
    /* 1. Address */
    offset += EncodeAddress(ackMsg.address, &pduStr[offset]);
    /* 2. Sub address */
    offset += EncodeSubAddress(ackMsg.subAddress, &pduStr[offset]);
    /* 3. Cause code */
    pduStr[offset++] = SMS_TRANS_PARAM_CAUSE_CODES;
    index = offset++;
    pduStr[offset] |=
        static_cast<unsigned char>(static_cast<uint32_t>(ackMsg.causeCode.transReplySeq) << SHIFT_2BITS);
    pduStr[offset] |= ackMsg.causeCode.errorClass;
    if (ackMsg.causeCode.errorClass != 0x0) {
        pduStr[++offset] = ackMsg.causeCode.causeCode;
    }
    pduStr[index] = offset - index;
    return offset;
}

int CdmaSmsPduCodec::EncodeTelesvcMsg(const struct SmsTeleSvcMsg &svcMsg, unsigned char *pduStr)
{
    int encodeSize = 0;
    TELEPHONY_LOGD("Teleservice msg type = [%{public}d]", svcMsg.type);
    switch (svcMsg.type) {
        case SMS_TYPE_SUBMIT:
            encodeSize = EncodeTelesvcSubmitMsg(svcMsg.data.submit, pduStr);
            break;
        case SMS_TYPE_CANCEL:
            encodeSize = EncodeTelesvcCancelMsg(svcMsg.data.cancel, pduStr);
            break;
        case SMS_TYPE_DELIVER_REPORT:
            encodeSize = EncodeTelesvcDeliverReportMsg(svcMsg.data.report, pduStr);
            break;
        case SMS_TYPE_DELIVER:
        case SMS_TYPE_USER_ACK:
        case SMS_TYPE_READ_ACK:
        default:
            TELEPHONY_LOGD("No matching type for [%{public}d]", svcMsg.type);
            break;
    }
    return encodeSize;
}

int CdmaSmsPduCodec::EncodeTelesvcDeliverReportMsg(
    const struct SmsTeleSvcDeliverReport &dRMsg, unsigned char *pduStr)
{
    int offset = 0;
    if (pduStr == nullptr) {
        return offset;
    }
    /* 1. Message Identifier (Mandatory) */
    offset += EncodeMsgId(dRMsg.msgId, SMS_TYPE_DELIVER_REPORT, pduStr);
    /* 2. TP-Failure Cause (Conditional) */
    if (dRMsg.tpFailCause >= 0x80) {
        pduStr[offset++] = SMS_BEARER_TP_FAILURE_CAUSE;
        pduStr[offset++] = 0x01;
        pduStr[offset++] = dRMsg.tpFailCause;
    }
    /* 3. User Data (Optional) */
    offset += EncodeBearerUserData(dRMsg.userData, &pduStr[offset]);
    return offset;
}

int CdmaSmsPduCodec::EncodeTelesvcCancelMsg(const struct SmsTeleSvcCancel &cancelMsg, unsigned char *pduStr)
{
    return EncodeMsgId(cancelMsg.msgId, SMS_TYPE_CANCEL, pduStr);
}

int CdmaSmsPduCodec::EncodeTelesvcSubmitMsg(const struct SmsTeleSvcSubmit &sbMsg, unsigned char *pduStr)
{
    int offset = 0;
    if (pduStr == nullptr) {
        return offset;
    }

    /* 1. Message Identifier */
    offset += EncodeMsgId(sbMsg.msgId, SMS_TYPE_SUBMIT, &pduStr[offset]);
    /* 2. User Data */
    offset += EncodeBearerUserData(sbMsg.userData, &pduStr[offset]);
    /* 3. Deferred Delivery Time */
    if (sbMsg.deferValPeriod.format == SMS_TIME_RELATIVE) {
        pduStr[offset++] = SMS_BEARER_DEFERRED_DELIVERY_TIME_RELATIVE;
        pduStr[offset++] = 0x01;
        pduStr[offset++] = sbMsg.deferValPeriod.time.relTime.time;
    } else if (sbMsg.deferValPeriod.format == SMS_TIME_ABSOLUTE) {
        std::vector<unsigned char> temp;
        pduStr[offset++] = SMS_BEARER_DEFERRED_DELIVERY_TIME_ABSOLUTE;
        pduStr[offset++] = EncodeAbsTime(sbMsg.deferValPeriod.time.absTime, temp);
        for (std::size_t i = 0; i < temp.size(); i++) {
            pduStr[offset++] = temp[i];
        }
    }
    /* 5. Priority Indicator */
    if (sbMsg.priority >= SMS_PRIORITY_NORMAL && sbMsg.priority <= SMS_PRIORITY_EMERGENCY) {
        pduStr[offset++] = SMS_BEARER_PRIORITY_INDICATOR;
        pduStr[offset++] = 0x01;
        pduStr[offset++] = static_cast<unsigned char>(static_cast<uint32_t>(sbMsg.priority) << SHIFT_6BITS);
    }
    /* 7. Reply Option */
    if (sbMsg.replyOpt.userAckReq | sbMsg.replyOpt.deliverAckReq | sbMsg.replyOpt.readAckReq |
        sbMsg.replyOpt.reportReq) {
        pduStr[offset++] = SMS_BEARER_REPLY_OPTION;
        pduStr[offset++] = 0x01;
        pduStr[offset] |= sbMsg.replyOpt.userAckReq << SHIFT_7BITS;
        pduStr[offset] |= sbMsg.replyOpt.deliverAckReq << SHIFT_6BITS;
        pduStr[offset] |= sbMsg.replyOpt.readAckReq << SHIFT_5BITS;
        pduStr[offset++] |= sbMsg.replyOpt.reportReq << SHIFT_4BITS;
    }

    if (sbMsg.callbackNumber.addrLen > 0) {
        std::vector<unsigned char> temp;
        pduStr[offset++] = SMS_BEARER_CALLBACK_NUMBER;
        pduStr[offset++] = EncodeCbNumber(sbMsg.callbackNumber, temp);
        for (std::size_t i = 0; i < temp.size(); i++) {
            pduStr[offset++] = temp[i];
        }
    }

    return offset;
}

int CdmaSmsPduCodec::EncodeCbNumber(const SmsTeleSvcAddr &cbNumber, std::vector<unsigned char> &pdustr)
{
    unsigned char cbData[SMS_TRANS_ADDRESS_MAX_LEN] = {0};
    unsigned char temp = static_cast<unsigned char>(static_cast<uint32_t>(cbNumber.digitMode) << SHIFT_7BITS);
    if (cbNumber.digitMode == false) {
        temp |= (cbNumber.addrLen & 0xfe) >> SHIFT_1BITS;
        pdustr.push_back(temp);

        int offset = 0;
        cbData[offset] |= (cbNumber.addrLen & 0x01) << SHIFT_7BITS;
        int addrLen =
            SmsCommonUtils::ConvertDigitToDTMF(cbNumber.szData, cbNumber.addrLen, SHIFT_1BITS, cbData + offset);
        pdustr.insert(pdustr.end(), cbData, cbData + addrLen);
    } else if (cbNumber.digitMode == true) {
        temp |= cbNumber.numberType << SHIFT_6BITS;
        temp |= cbNumber.numberPlan;
        pdustr.push_back(temp);
        pdustr.push_back(cbNumber.addrLen);
        pdustr.insert(pdustr.end(), cbNumber.szData, cbNumber.szData + cbNumber.addrLen);
    }
    return static_cast<int>(pdustr.size());
}

int CdmaSmsPduCodec::EncodeAbsTime(const SmsTimeAbs &absTime, std::vector<unsigned char> &pdustr)
{
    pdustr.push_back(ConvertToBCD(absTime.year));
    pdustr.push_back(ConvertToBCD(absTime.month));
    pdustr.push_back(ConvertToBCD(absTime.day));
    pdustr.push_back(ConvertToBCD(absTime.hour));
    pdustr.push_back(ConvertToBCD(absTime.minute));
    pdustr.push_back(ConvertToBCD(absTime.second));
    return static_cast<int>(pdustr.size());
}

int CdmaSmsPduCodec::EncodeMsgId(const SmsTransMsgId &msgId, const SmsMessageType &type, unsigned char *pduStr)
{
    int offset = 0;
    if (pduStr == nullptr) {
        return offset;
    }
    pduStr[offset++] = SMS_BEARER_MESSAGE_IDENTIFIER;
    pduStr[offset++] = 0x03;
    pduStr[offset++] = static_cast<unsigned char>(
        (static_cast<uint32_t>(type) << SHIFT_4BITS) | ((msgId.msgId & 0xf000) >> (BYTE_BITS + SHIFT_4BITS)));
    pduStr[offset++] = (msgId.msgId & 0x0ff0) >> (SHIFT_4BITS);
    pduStr[offset++] = ((msgId.msgId & 0x000f) << (SHIFT_4BITS)) | (msgId.headerInd ? 0x08 : 0x00);
    return offset;
}

int CdmaSmsPduCodec::EncodeSubAddress(const struct SmsTransSubAddr &address, unsigned char *pduStr)
{
    unsigned int index = 0;
    unsigned int offset = 0;
    if (pduStr == nullptr || address.addrLen == 0) {
        return offset;
    }
    pduStr[offset++] = SMS_TRANS_PARAM_ORG_SUB_ADDRESS;
    pduStr[offset] = address.addrLen + 0x02;
    index = offset++;
    pduStr[offset] |= static_cast<unsigned char>(static_cast<uint32_t>(address.type) << SHIFT_5BITS);
    pduStr[offset++] |= (address.odd ? 0x10 : 0x00);
    pduStr[offset++] = address.addrLen;
    if (memcpy_s(pduStr + offset, address.addrLen, address.szData, address.addrLen) != EOK) {
        TELEPHONY_LOGE("EncodeSubAddress memcpy_s err.");
        return offset;
    }
    offset += address.addrLen;
    ShiftNBit(&pduStr[index], offset - index + 1, SHIFT_4BITS);
    return offset;
}

int CdmaSmsPduCodec::EncodeAddress(const SmsTransAddr &address, unsigned char *pduStr)
{
    unsigned int index = 0;
    unsigned int offset = 0;
    unsigned int lenIndex = 0;
    if (pduStr == nullptr) {
        return offset;
    }

    pduStr[offset++] = SMS_TRANS_PARAM_DEST_ADDRESS; /* Will be set to param length */
    lenIndex = offset++; // then lenIndex 1

    pduStr[offset] = (address.digitMode ? 0x80 : 0x00);
    pduStr[offset] |= (address.numberMode ? 0x40 : 0x00);
    if (address.digitMode) {
        pduStr[offset] |= address.numberType << SHIFT_3BITS;
    }

    if (address.digitMode) {
        if (address.numberMode) {
            index = offset++;
            pduStr[offset++] = address.addrLen;
            if (memcpy_s(pduStr + offset, address.addrLen, address.szData, address.addrLen) != EOK) {
                TELEPHONY_LOGE("EncodeAddress numberMode memcpy_s fail.");
                return offset;
            }
            offset += address.addrLen;
            ShiftNBit(&pduStr[index], offset - index + 1, SHIFT_3BITS);
        } else {
            pduStr[offset++] |= address.numberPlan >> SHIFT_1BITS;
            pduStr[offset++] |= address.numberPlan << SHIFT_7BITS;
            index = offset - 1;
            pduStr[offset++] = address.addrLen;
            if (memcpy_s(pduStr + offset, address.addrLen, address.szData, address.addrLen) != EOK) {
                TELEPHONY_LOGE("EncodeAddress mecpy_s fail.");
                return offset;
            }
            offset += address.addrLen;
            ShiftNBit(&pduStr[index], offset - index + 1, SHIFT_7BITS);
        }
    } else {
        index = offset++;
        pduStr[offset++] = address.addrLen;
        int addrLen = SmsCommonUtils::ConvertDigitToDTMF(address.szData, address.addrLen, 0, pduStr + offset);
        for (int j = 0; j < addrLen; j++) {
            TELEPHONY_LOGD(
                "ADDRESS 4BIT DTMF [%{public}d] = [%{public}02x]", j, static_cast<uint32_t>(pduStr[offset + j]));
        }
        offset += addrLen;
        ShiftNBit(&pduStr[index], offset - index + 1, SHIFT_6BITS);
    }
    pduStr[lenIndex] = offset - lenIndex - 1;
    TELEPHONY_LOGD("Address subparam length field = [%{public}d]", pduStr[lenIndex]);
    return offset;
}

int CdmaSmsPduCodec::EncodeBearerUserData(const struct SmsTeleSvcUserData &userData, unsigned char *pduStr)
{
    int offset = 0;
    int lenIndex = 0;
    int encodeSize = 0;
    bool delReservedBit = false;
    if (pduStr == nullptr || userData.dataLen == 0) {
        return offset;
    }

    pduStr[offset++] = SMS_BEARER_USER_DATA;
    lenIndex = offset;
    offset++;
    if (userData.encodeType == SMS_ENCODE_EPM || userData.encodeType == SMS_ENCODE_GSMDCS) {
        pduStr[offset++] = userData.msgType;
    }
    pduStr[offset++] = static_cast<unsigned char>(static_cast<uint32_t>(userData.encodeType) << SHIFT_3BITS);
    pduStr[offset++] = userData.dataLen;

    switch (userData.encodeType) {
        case SMS_ENCODE_7BIT_ASCII:
            encodeSize = EncodeUserData(userData.userData, &pduStr[offset], userData.dataLen);
            offset += encodeSize;
            if (userData.dataLen % BYTE_BITS > HALF_BYTE) {
                delReservedBit = true;
            }
            break;
        case SMS_ENCODE_GSM7BIT:
            encodeSize = SmsCommonUtils::Pack7bitChar(userData.userData, userData.dataLen, 0x00, &pduStr[offset]);
            offset += encodeSize;
            if (userData.dataLen % BYTE_BITS > HALF_BYTE) {
                delReservedBit = true;
            }
            break;
        case SMS_ENCODE_UNICODE: {
            MsgTextConvert *textCvt = MsgTextConvert::Instance();
            encodeSize = textCvt->ConvertUTF8ToUCS2(
                &pduStr[offset], SMS_MAX_USER_DATA_LEN, userData.userData, userData.dataLen);
            pduStr[offset - 1] = encodeSize / HEX_BYTE_STEP;
            offset += encodeSize;
            break;
        }
        default:
            if (memcpy_s(pduStr + offset, userData.dataLen, userData.userData, userData.dataLen) == EOK) {
                offset += userData.dataLen;
            }
            break;
    }

    ShiftNBit(&pduStr[lenIndex + 1], offset - lenIndex - 1, SHIFT_3BITS);
    if (delReservedBit == true) {
        offset--;
    }
    pduStr[lenIndex] = offset - lenIndex - 1;
    return offset;
}

int CdmaSmsPduCodec::DecodeMsg(const unsigned char *pduStr, int pduLen, struct SmsTransMsg &transMsg)
{
    int decodelen = 0;
    int offset = 0;
    if (pduStr == nullptr) {
        return decodelen;
    }

    char mti = pduStr[offset++] & 0xff;
    switch (mti) {
        case SMS_TRANS_P2P_MSG:
            transMsg.type = SMS_TRANS_P2P_MSG;
            decodelen = DecodeP2PMsg(&pduStr[offset], pduLen - offset, transMsg.data.p2pMsg);
            break;
        case SMS_TRANS_BROADCAST_MSG:
            transMsg.type = SMS_TRANS_BROADCAST_MSG;
            decodelen = DecodeCBMsg(&pduStr[offset], pduLen - offset, transMsg.data.cbMsg);
            break;
        case SMS_TRANS_ACK_MSG:
            transMsg.type = SMS_TRANS_ACK_MSG;
            decodelen = DecodeAckMsg(&pduStr[offset], pduLen - offset, transMsg.data.ackMsg);
            break;
        default:
            transMsg.type = SMS_TRANS_TYPE_RESERVED;
            break;
    }
    return decodelen + offset;
}

int CdmaSmsPduCodec::DecodeP2PMsg(const unsigned char *pduStr, int pduLen, struct SmsTransP2PMsg &p2pMsg)
{
    int offset = 0;
    int tmpLen = 0;
    if (pduStr == nullptr) {
        return offset;
    }

    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_TRANS_PARAM_TELESVC_IDENTIFIER:
                offset += DecodeTeleId(&pduStr[offset], pduLen - offset, p2pMsg.transTelesvcId);
                break;
            case SMS_TRANS_PARAM_SERVICE_CATEGORY:
                offset += DecodeSvcCtg(&pduStr[offset], pduLen - offset, p2pMsg.transSvcCtg);
                break;
            case SMS_TRANS_PARAM_ORG_ADDRESS:
            case SMS_TRANS_PARAM_DEST_ADDRESS:
                offset += DecodeAddress(&pduStr[offset], pduLen - offset, p2pMsg.address);
                break;
            case SMS_TRANS_PARAM_ORG_SUB_ADDRESS:
            case SMS_TRANS_PARAM_DEST_SUB_ADDRESS:
                offset += DecodeSubAddress(&pduStr[offset], pduLen - offset, p2pMsg.subAddress);
                break;
            case SMS_TRANS_PARAM_BEARER_REPLY_OPTION:
                offset += HEX_BYTE_STEP;
                p2pMsg.transReplySeq = (pduStr[offset] >> SHIFT_2BITS);
                offset++;
                break;
            case SMS_TRANS_PARAM_BEARER_DATA:
                offset++;
                tmpLen = pduStr[offset++];
                DecodeP2PTelesvcMsg(&pduStr[offset], tmpLen, p2pMsg.telesvcMsg);
                offset += (tmpLen + 1);
                break;
            default:
                return offset;
        }
    }
    return offset;
}

int CdmaSmsPduCodec::DecodeCBMsg(const unsigned char *pduStr, int pduLen, struct SmsTransBroadCastMsg &cbMsg)
{
    int offset = 0;
    int tempLen = 0;
    if (pduStr == nullptr) {
        return offset;
    }

    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_TRANS_PARAM_SERVICE_CATEGORY:
                offset += DecodeTeleId(pduStr + offset, pduLen, cbMsg.transSvcCtg);
                break;
            case SMS_TRANS_PARAM_BEARER_DATA:
                tempLen = pduStr[++offset];
                offset++;
                if (cbMsg.transSvcCtg >= SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL &&
                    cbMsg.transSvcCtg <= SMS_TRANS_SVC_CTG_CMAS_TEST) {
                    DecodeCBBearerData(pduStr + offset, tempLen, cbMsg.telesvcMsg, true);
                } else {
                    DecodeCBBearerData(pduStr + offset, tempLen, cbMsg.telesvcMsg, false);
                }
                offset += (tempLen + 1);
                break;
            default:
                return offset;
        }
    }
    return offset;
}

int CdmaSmsPduCodec::DecodeAckMsg(const unsigned char *pduStr, int pduLen, struct SmsTransAckMsg &ackMsg)
{
    int offset = 0;
    if (pduStr == nullptr) {
        return offset;
    }

    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_TRANS_PARAM_DEST_ADDRESS:
                offset += DecodeAddress(pduStr + offset, pduLen, ackMsg.address);
                break;
            case SMS_TRANS_PARAM_DEST_SUB_ADDRESS:
                offset += DecodeSubAddress(pduStr + offset, pduLen, ackMsg.subAddress);
                break;
            case SMS_TRANS_PARAM_CAUSE_CODES:
                offset += HEX_BYTE_STEP;
                ackMsg.causeCode.transReplySeq = pduStr[offset] >> SHIFT_2BITS;
                if ((pduStr[offset++] & 0x03) == 0x00) {
                    ackMsg.causeCode.errorClass = SMS_TRANS_ERR_CLASS_NONE;
                } else {
                    ackMsg.causeCode.causeCode = static_cast<enum SmsTransCauseCodeType>(pduStr[offset++]);
                }
                break;
            default:
                return offset;
        }
    }
    return offset;
}

void CdmaSmsPduCodec::DecodeP2PTelesvcMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcMsg &svcMsg)
{
    if (pduStr == nullptr) {
        return;
    }

    std::vector<unsigned char> tempPdu(pduStr, pduStr + pduLen);
    svcMsg.type = FindMsgType(tempPdu);
    TELEPHONY_LOGD("Msg Type = [%{public}d]", svcMsg.type);
    switch (svcMsg.type) {
        case SMS_TYPE_DELIVER:
            DecodeP2PDeliverMsg(pduStr, pduLen, svcMsg.data.deliver);
            break;
        case SMS_TYPE_SUBMIT:
            DecodeP2PSubmitMsg(pduStr, pduLen, svcMsg.data.submit);
            break;
        case SMS_TYPE_DELIVERY_ACK:
            DecodeP2PDeliveryAckMsg(pduStr, pduLen, svcMsg.data.deliveryAck);
            break;
        case SMS_TYPE_USER_ACK:
            DecodeP2PUserAckMsg(pduStr, pduLen, svcMsg.data.userAck);
            break;
        case SMS_TYPE_READ_ACK:
            DecodeP2PReadAckMsg(pduStr, pduLen, svcMsg.data.readAck);
            break;
        case SMS_TYPE_SUBMIT_REPORT:
        default:
            break;
    }
}

void CdmaSmsPduCodec::DecodeP2PDeliverMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcDeliver &delMsg)
{
    int offset = 0;
    int tempLen;
    unsigned char tempStr[pduLen + 1];
    if (pduStr == nullptr) {
        return;
    }

    while (offset < pduLen) {
        TELEPHONY_LOGD("current offset = [%{public}d] [%{public}x]", offset, pduStr[offset]);
        switch (pduStr[offset]) {
            case SMS_BEARER_MESSAGE_IDENTIFIER:
                offset += DecodeMsgId(pduStr + offset, MAX_MSG_ID_LEN, delMsg.msgId);
                break;
            case SMS_BEARER_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
                if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) == EOK) {
                    DecodeUserData(tempStr, tempLen, delMsg.userData);
                    offset += tempLen;
                }
                break;
            case SMS_BEARER_MSG_CENTER_TIME_STAMP:
                offset += HEX_BYTE_STEP;
                offset += DecodeAbsTime(pduStr + offset, delMsg.timeStamp);
                break;
            case SMS_BEARER_VALIDITY_PERIOD_ABSOLUTE:
                offset += HEX_BYTE_STEP;
                delMsg.valPeriod.format = SMS_TIME_ABSOLUTE;
                offset += DecodeAbsTime(pduStr + offset, delMsg.valPeriod.time.absTime);
                break;
            case SMS_BEARER_VALIDITY_PERIOD_RELATIVE:
                offset += HEX_BYTE_STEP;
                delMsg.valPeriod.format = SMS_TIME_RELATIVE;
                delMsg.valPeriod.time.relTime.time = static_cast<enum SmsRelativeTime>(pduStr[offset++]);
                break;
            case SMS_BEARER_PRIORITY_INDICATOR:
                offset += HEX_BYTE_STEP;
                delMsg.priority = static_cast<enum SmsPriorityIndicator>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_PRIVACY_INDICATOR:
                offset += HEX_BYTE_STEP;
                delMsg.privacy = static_cast<enum SmsPrivacyIndicator>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_REPLY_OPTION:
                offset += HEX_BYTE_STEP;
                delMsg.replyOpt.userAckReq = (pduStr[offset] & 0x80) ? true : false;
                delMsg.replyOpt.deliverAckReq = (pduStr[offset] & 0x40) ? true : false;
                delMsg.replyOpt.readAckReq = (pduStr[offset] & 0x20) ? true : false;
                delMsg.replyOpt.reportReq = (pduStr[offset] & 0x10) ? true : false;
                offset++;
                break;
            case SMS_BEARER_NUMBER_OF_MESSAGES:
                offset += HEX_BYTE_STEP;
                delMsg.numMsg = (((pduStr[offset] & 0xf0) >> SHIFT_4BITS) * DECIMAL_NUM) + (pduStr[offset] & 0x0f);
                offset++;
                break;
            case SMS_BEARER_ALERT_ON_MSG_DELIVERY:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                if (tempLen > 0) {
                    delMsg.alertPriority =
                        static_cast<enum SmsAlertPriority>((pduStr[offset] & 0xc0) >> SHIFT_6BITS);
                }
                offset += tempLen;
                break;
            case SMS_BEARER_LANGUAGE_INDICATOR:
                offset += HEX_BYTE_STEP;
                delMsg.language = static_cast<enum SmsLanguageType>(pduStr[offset++]);
                break;
            case SMS_BEARER_CALLBACK_NUMBER:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                DecodeCallBackNum(&pduStr[offset], tempLen, delMsg.callbackNumber);
                offset += tempLen;
                break;
            case SMS_BEARER_MSG_DISPLAY_MODE:
                offset += HEX_BYTE_STEP;
                delMsg.displayMode = static_cast<enum SmsDisplayMode>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_MULTI_ENCODING_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                offset += tempLen;
                break;
            case SMS_BEARER_MSG_DEPOSIT_INDEX:
                offset += HEX_BYTE_STEP;
                delMsg.depositId = pduStr[offset++];
                delMsg.depositId = (delMsg.depositId << SHIFT_8BITS) + pduStr[offset++];
                break;
            case SMS_BEARER_ENHANCED_VMN:
                DecodeP2PEnhancedVmn(pduStr, pduLen, delMsg.enhancedVmn);
                break;
            case SMS_BEARER_ENHANCED_VMN_ACK:
                DecodeP2PDeliverVmnAck(pduStr, pduLen, delMsg.enhancedVmnAck);
                break;
            default: /* skip unrecognized sub parameters */
                offset++;
                tempLen = pduStr[offset++];
                offset += tempLen;
                break;
        }
    }
}

void CdmaSmsPduCodec::DecodeP2PEnhancedVmn(
    const unsigned char *pduStr, int pduLen, struct SmsEnhancedVmn &enhancedVmn)
{
    int ret = 0;
    int offset = 1;
    int tempOff = 0;
    int tempLen = pduStr[offset++];
    unsigned char tempStr[pduLen + 1];
    offset++;
    tempLen = pduStr[offset++];
    (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
    ret = memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen);
    if (ret != EOK) {
        TELEPHONY_LOGE("DecodeP2PEnhancedVmn memcpy_s");
        return;
    }
    enhancedVmn.priority = (enum SmsPriorityIndicator)(pduStr[offset] >> SHIFT_6BITS);
    enhancedVmn.passwordReq = (pduStr[offset] & 0x20) ? true : false;
    enhancedVmn.setupReq = (pduStr[offset] & 0x10) ? true : false;
    enhancedVmn.pwChangeReq = (pduStr[offset] & 0x08) ? true : false;
    ShiftNBitForDecode(tempStr, tempLen, SHIFT_5BITS);
    if (enhancedVmn.setupReq || enhancedVmn.pwChangeReq) {
        enhancedVmn.minPwLen = tempStr[tempOff] >> SHIFT_4BITS;
        enhancedVmn.maxPwLen = tempStr[tempOff++] & 0x0f;
    }

    enhancedVmn.vmNumUnheardMsg = tempStr[tempOff++];
    enhancedVmn.vmMailboxAlmFull = (tempStr[tempOff] & 0x80) ? true : false;
    enhancedVmn.vmMailboxFull = (tempStr[tempOff] & 0x40) ? true : false;
    enhancedVmn.replyAllowed = (tempStr[tempOff] & 0x20) ? true : false;
    enhancedVmn.faxIncluded = (tempStr[tempOff] & 0x10) ? true : false;
    enhancedVmn.vmLen = static_cast<unsigned short>(
        ((static_cast<uint32_t>(tempStr[tempOff])) << SHIFT_8BITS) | tempStr[tempOff + 1]);
    tempOff += HEX_BYTE_STEP;
    enhancedVmn.vmRetDay = tempStr[tempOff] >> SHIFT_1BITS;
    ShiftNBitForDecode(tempStr, tempLen, SHIFT_7BITS);
    enhancedVmn.vmMsgId = (tempStr[tempOff] << SHIFT_8BITS) | tempStr[tempOff + 1];
    tempOff += HEX_BYTE_STEP;
    enhancedVmn.vmMailboxId = (tempStr[tempOff] << SHIFT_8BITS) | tempStr[tempOff + 1];
    tempOff += HEX_BYTE_STEP;

    enhancedVmn.anDigitMode = (tempStr[tempOff] & 0x80) ? true : false;
    enhancedVmn.anNumberType = (tempStr[tempOff] & 0x70) >> SHIFT_4BITS;
    if (enhancedVmn.anDigitMode) {
        enhancedVmn.anNumberPlan = tempStr[tempOff++] & 0x0f;
        enhancedVmn.anNumField = tempStr[tempOff++];
        SmsCommonUtils::BcdToDigit(&(tempStr[tempOff]), enhancedVmn.anNumField, (char *)enhancedVmn.anChar);
    } else {
        ShiftNBitForDecode(tempStr, tempLen, SHIFT_4BITS);
        enhancedVmn.anNumField = tempStr[tempOff++];
        ret = memset_s(enhancedVmn.anChar, sizeof(enhancedVmn.anChar), 0x00, sizeof(enhancedVmn.anChar));
        if (ret != EOK) {
            TELEPHONY_LOGE("enhancedVmn memset_s err.");
            return;
        }
        ret = memcpy_s(enhancedVmn.anChar, sizeof(enhancedVmn.anChar), tempStr + tempOff, enhancedVmn.anNumField);
        if (ret != EOK) {
            TELEPHONY_LOGE("enhancedVmn memcpy_s err.");
            return;
        }
        tempOff += enhancedVmn.anNumField;
    }

    enhancedVmn.cliDigitMode = (tempStr[offset] & 0x80) ? true : false;
    enhancedVmn.cliNumberType = (tempStr[offset] & 0x70) >> SHIFT_4BITS;
    if (enhancedVmn.cliDigitMode) {
        enhancedVmn.cliNumberPlan = tempStr[tempOff++] & 0x0f;
        enhancedVmn.cliNumField = tempStr[tempOff++];
        SmsCommonUtils::BcdToDigit(&(tempStr[tempOff]), enhancedVmn.cliNumField, (char *)enhancedVmn.cliChar);
    } else {
        ShiftNBitForDecode(tempStr, tempLen, SHIFT_4BITS);
        enhancedVmn.cliNumField = tempStr[tempOff++];
        (void)memset_s(enhancedVmn.cliChar, sizeof(enhancedVmn.cliChar), 0x00, sizeof(enhancedVmn.cliChar));
        ret =
            memcpy_s(enhancedVmn.cliChar, sizeof(enhancedVmn.cliChar), tempStr + tempOff, enhancedVmn.cliNumField);
        if (ret != EOK) {
            TELEPHONY_LOGE("enhancedVmn cliChar memcpy_s fail.");
            return;
        }
        tempOff += enhancedVmn.cliNumField;
    }
    offset += tempLen;
}

void CdmaSmsPduCodec::DecodeP2PDeliverVmnAck(
    const unsigned char *pduStr, int pduLen, struct SmsEnhancedVmnAck &enhancedVmnAck)
{
    int offset = 1;
    if (pduStr == nullptr) {
        return;
    }

    int tempLen = pduStr[offset++];
    unsigned char tempStr[pduLen + 1];
    (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
    if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) != EOK) {
        TELEPHONY_LOGE("DecodeP2PDeliverVmnAck memcpy_s error.");
        return;
    }
    enhancedVmnAck.vmMailboxId = (tempStr[offset] << SHIFT_8BITS) | tempStr[offset + 1];
    offset += HEX_BYTE_STEP;
    enhancedVmnAck.vmNumUnheardMsg = tempStr[offset++];
    enhancedVmnAck.numDeleteAck = tempStr[offset] >> SHIFT_5BITS;
    enhancedVmnAck.numPlayAck = (tempStr[offset] & 0x1c) >> SHIFT_2BITS;
    ShiftNBitForDecode(tempStr, tempLen, SHIFT_6BITS);
    for (int i = 0; i < enhancedVmnAck.numDeleteAck; i++) {
        enhancedVmnAck.daVmMsgId[i] = (tempStr[offset] << SHIFT_8BITS) | tempStr[offset + 1];
        offset += HEX_BYTE_STEP;
    }
    for (int i = 0; i < enhancedVmnAck.numPlayAck; i++) {
        enhancedVmnAck.paVmMsgId[i] = (tempStr[offset] << SHIFT_8BITS) | tempStr[offset + 1];
        offset += HEX_BYTE_STEP;
    }
}

void CdmaSmsPduCodec::DecodeP2PSubmitMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcSubmit &subMsg)
{
    int offset = 0;
    int tempLen;
    unsigned char tempStr[pduLen + 1];
    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_BEARER_MESSAGE_IDENTIFIER:
                offset += DecodeMsgId(pduStr + offset, MAX_MSG_ID_LEN, subMsg.msgId);
                break;
            case SMS_BEARER_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
                if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) == EOK) {
                    DecodeUserData(tempStr, tempLen, subMsg.userData);
                    offset += tempLen;
                }
                break;
            case SMS_BEARER_VALIDITY_PERIOD_ABSOLUTE:
                offset += HEX_BYTE_STEP;
                subMsg.valPeriod.format = SMS_TIME_ABSOLUTE;
                offset += DecodeAbsTime(pduStr + offset, subMsg.valPeriod.time.absTime);
                break;
            case SMS_BEARER_VALIDITY_PERIOD_RELATIVE:
                offset += HEX_BYTE_STEP;
                subMsg.valPeriod.format = SMS_TIME_RELATIVE;
                subMsg.valPeriod.time.relTime.time = static_cast<enum SmsRelativeTime>(pduStr[offset++]);
                break;
            case SMS_BEARER_DEFERRED_DELIVERY_TIME_ABSOLUTE:
                offset += HEX_BYTE_STEP;
                subMsg.deferValPeriod.format = SMS_TIME_ABSOLUTE;
                offset += DecodeAbsTime(pduStr + offset, subMsg.deferValPeriod.time.absTime);
                break;
            case SMS_BEARER_DEFERRED_DELIVERY_TIME_RELATIVE:
                offset += HEX_BYTE_STEP;
                subMsg.deferValPeriod.format = SMS_TIME_RELATIVE;
                subMsg.deferValPeriod.time.relTime.time = static_cast<enum SmsRelativeTime>(pduStr[offset++]);
                break;
            case SMS_BEARER_PRIORITY_INDICATOR:
                offset += HEX_BYTE_STEP;
                subMsg.priority = static_cast<enum SmsPriorityIndicator>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_PRIVACY_INDICATOR:
                offset += HEX_BYTE_STEP;
                subMsg.privacy = static_cast<enum SmsPrivacyIndicator>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_REPLY_OPTION:
                offset += HEX_BYTE_STEP;
                subMsg.replyOpt.userAckReq = (pduStr[offset] & 0x80) ? true : false;
                subMsg.replyOpt.deliverAckReq = (pduStr[offset] & 0x40) ? true : false;
                subMsg.replyOpt.readAckReq = (pduStr[offset] & 0x20) ? true : false;
                subMsg.replyOpt.reportReq = (pduStr[offset] & 0x10) ? true : false;
                offset++;
                break;
            case SMS_BEARER_ALERT_ON_MSG_DELIVERY:
                offset += HEX_BYTE_STEP;
                subMsg.alertPriority = static_cast<enum SmsAlertPriority>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_LANGUAGE_INDICATOR:
                offset += HEX_BYTE_STEP;
                subMsg.language = static_cast<enum SmsLanguageType>(pduStr[offset++]);
                break;
            case SMS_BEARER_CALLBACK_NUMBER:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                DecodeCallBackNum(&pduStr[offset], tempLen, subMsg.callbackNumber);
                offset += tempLen;
                break;
            case SMS_BEARER_MULTI_ENCODING_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                offset += tempLen;
                break;
            case SMS_BEARER_MSG_DEPOSIT_INDEX:
                offset += HEX_BYTE_STEP;
                subMsg.depositId = pduStr[offset++];
                subMsg.depositId = (subMsg.depositId << SHIFT_8BITS) + pduStr[offset++];
                break;
            default:
                return;
        }
    }
}

void CdmaSmsPduCodec::DecodeP2PUserAckMsg(
    const unsigned char *pduStr, int pduLen, struct SmsTeleSvcUserAck &userAck)
{
    int offset = 0;
    int tempLen;
    unsigned char tempStr[pduLen + 1];
    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_BEARER_MESSAGE_IDENTIFIER:
                offset += DecodeMsgId(pduStr + offset, MAX_MSG_ID_LEN, userAck.msgId);
                break;
            case SMS_BEARER_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
                if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) == EOK) {
                    DecodeUserData(tempStr, tempLen, userAck.userData);
                    offset += tempLen;
                }
                break;
            case SMS_BEARER_USER_RESPONSE_CODE:
                offset += HEX_BYTE_STEP;
                userAck.respCode = pduStr[offset++];
                break;
            case SMS_BEARER_MSG_CENTER_TIME_STAMP:
                offset += HEX_BYTE_STEP;
                offset += DecodeAbsTime(pduStr + offset, userAck.timeStamp);
                break;
            case SMS_BEARER_MULTI_ENCODING_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                offset += tempLen;
                break;
            case SMS_BEARER_MSG_DEPOSIT_INDEX:
                offset += HEX_BYTE_STEP;
                userAck.depositId = pduStr[++offset]; // the hight byte
                offset++;
                break;
            default:
                return;
        }
    }
}

void CdmaSmsPduCodec::DecodeP2PReadAckMsg(
    const unsigned char *pduStr, int pduLen, struct SmsTeleSvcReadAck &readAck)
{
    int offset = 0;
    int tempLen;
    unsigned char tempStr[pduLen + 1];
    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_BEARER_MESSAGE_IDENTIFIER:
                offset += DecodeMsgId(pduStr + offset, MAX_MSG_ID_LEN, readAck.msgId);
                break;
            case SMS_BEARER_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
                if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) == EOK) {
                    DecodeUserData(tempStr, tempLen, readAck.userData);
                    offset += tempLen;
                }
                break;
            case SMS_BEARER_MSG_CENTER_TIME_STAMP:
                offset += HEX_BYTE_STEP;
                offset += DecodeAbsTime(pduStr + offset, readAck.timeStamp);
                break;
            case SMS_BEARER_MULTI_ENCODING_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                offset += tempLen;
                break;
            case SMS_BEARER_MSG_DEPOSIT_INDEX:
                /* Message Deposit Index */
                offset += HEX_BYTE_STEP;
                readAck.depositId = pduStr[++offset]; // the hight byte
                offset++;
                break;
            default:
                return;
        }
    }
}

void CdmaSmsPduCodec::DecodeP2PSubmitReportMsg(
    const unsigned char *pduStr, int pduLen, struct SmsTeleSvcDeliverReport &subReport)
{
    int offset = 0;
    int tempLen;
    unsigned char tempStr[pduLen + 1];
    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_BEARER_MESSAGE_IDENTIFIER:
                offset += DecodeMsgId(&pduStr[offset], MAX_MSG_ID_LEN, subReport.msgId);
                break;
            case SMS_BEARER_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
                if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) == EOK) {
                    DecodeUserData(tempStr, tempLen, subReport.userData);
                    offset += tempLen;
                }
                break;
            case SMS_BEARER_LANGUAGE_INDICATOR:
                offset += HEX_BYTE_STEP;
                subReport.language = static_cast<enum SmsLanguageType>(pduStr[offset++]);
                break;
            case SMS_BEARER_MULTI_ENCODING_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                offset += tempLen;
                break;
            case SMS_BEARER_TP_FAILURE_CAUSE:
                offset += HEX_BYTE_STEP;
                subReport.tpFailCause = pduStr[offset++];
                break;
            default:
                return;
        }
    }
}

void CdmaSmsPduCodec::DecodeP2PDeliveryAckMsg(
    const unsigned char *pduStr, int pduLen, struct SmsTeleSvcDeliverAck &delAckMsg)
{
    int offset = 0;
    int tmpLen;
    unsigned char tempStr[pduLen + 1];
    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_BEARER_MESSAGE_IDENTIFIER:
                offset += DecodeMsgId(pduStr + offset, MAX_MSG_ID_LEN, delAckMsg.msgId);
                break;
            case SMS_BEARER_USER_DATA:
                offset += HEX_BYTE_STEP;
                tmpLen = pduStr[offset - 1];
                (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
                if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tmpLen) == EOK) {
                    DecodeUserData(tempStr, tmpLen, delAckMsg.userData);
                    offset += tmpLen;
                }
                break;
            case SMS_BEARER_MSG_CENTER_TIME_STAMP:
                offset += HEX_BYTE_STEP;
                offset += DecodeAbsTime(pduStr + offset, delAckMsg.timeStamp);
                break;
            case SMS_BEARER_MULTI_ENCODING_USER_DATA:
                offset += HEX_BYTE_STEP;
                tmpLen = pduStr[offset - 1];
                offset += tmpLen;
                break;
            case SMS_BEARER_MESSAGE_STATUS:
                offset += HEX_BYTE_STEP;
                delAckMsg.msgStatus = static_cast<enum SmsStatusCode>(pduStr[offset++]);
                break;
            default:
                return;
        }
    }
}

void CdmaSmsPduCodec::DecodeCBBearerData(
    const unsigned char *pduStr, int pduLen, struct SmsTeleSvcMsg &telesvc, bool isCMAS)
{
    int offset = 0;
    int tempLen;
    unsigned char tempStr[pduLen + 1];
    while (offset < pduLen) {
        switch (pduStr[offset]) {
            case SMS_BEARER_MESSAGE_IDENTIFIER:
                telesvc.type = SMS_TYPE_DELIVER;
                offset += DecodeMsgId(pduStr + offset, MAX_MSG_ID_LEN, telesvc.data.deliver.msgId);
                break;
            case SMS_BEARER_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
                if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) != EOK) {
                    TELEPHONY_LOGE("DecodeCBBearerData memcpy_s fail.");
                    break;
                }
                if (isCMAS) {
                    DecodeCMASData(tempStr, tempLen, telesvc.data.deliver.cmasData);
                } else {
                    DecodeUserData(tempStr, tempLen, telesvc.data.deliver.userData);
                }
                offset += tempLen;
                break;
            case SMS_BEARER_MSG_CENTER_TIME_STAMP:
                offset += HEX_BYTE_STEP;
                offset += DecodeAbsTime(pduStr + offset, telesvc.data.deliver.timeStamp);
                break;
            case SMS_BEARER_VALIDITY_PERIOD_ABSOLUTE:
                offset += HEX_BYTE_STEP;
                telesvc.data.deliver.valPeriod.format = SMS_TIME_ABSOLUTE;
                offset += DecodeAbsTime(pduStr + offset, telesvc.data.deliver.valPeriod.time.absTime);
                break;
            case SMS_BEARER_VALIDITY_PERIOD_RELATIVE:
                offset += HEX_BYTE_STEP;
                telesvc.data.deliver.valPeriod.format = SMS_TIME_RELATIVE;
                telesvc.data.deliver.valPeriod.time.relTime.time =
                    static_cast<enum SmsRelativeTime>(pduStr[offset++]);
                break;
            case SMS_BEARER_PRIORITY_INDICATOR:
                offset += HEX_BYTE_STEP;
                telesvc.data.deliver.priority =
                    static_cast<enum SmsPriorityIndicator>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_ALERT_ON_MSG_DELIVERY:
                offset += HEX_BYTE_STEP;
                telesvc.data.deliver.alertPriority =
                    static_cast<enum SmsAlertPriority>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_LANGUAGE_INDICATOR:
                offset += HEX_BYTE_STEP;
                telesvc.data.deliver.language = static_cast<enum SmsLanguageType>(pduStr[offset++]);
                break;
            case SMS_BEARER_CALLBACK_NUMBER:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                DecodeCallBackNum(&pduStr[offset], tempLen, telesvc.data.deliver.callbackNumber);
                offset += tempLen;
                break;
            case SMS_BEARER_MSG_DISPLAY_MODE:
                offset += HEX_BYTE_STEP;
                telesvc.data.deliver.displayMode =
                    static_cast<enum SmsDisplayMode>((pduStr[offset++] & 0xc0) >> SHIFT_6BITS);
                break;
            case SMS_BEARER_MULTI_ENCODING_USER_DATA:
                offset += HEX_BYTE_STEP;
                tempLen = pduStr[offset - 1];
                offset += tempLen;
                break;
            default:
                return;
        }
    }
}

int CdmaSmsPduCodec::DecodeTeleId(const unsigned char *pduStr, int pduLen, unsigned short &teleSvcId)
{
    int offset = 0;
    unsigned short tempParam;
    offset += HEX_BYTE_STEP;
    tempParam = pduStr[offset++];
    tempParam = (tempParam << SHIFT_8BITS) + pduStr[offset++];
    switch (tempParam) {
        case SMS_TRANS_TELESVC_CMT_91:
            teleSvcId = SMS_TRANS_TELESVC_CMT_91;
            break;
        case SMS_TRANS_TELESVC_CPT_95:
            teleSvcId = SMS_TRANS_TELESVC_CPT_95;
            break;
        case SMS_TRANS_TELESVC_CMT_95:
            teleSvcId = SMS_TRANS_TELESVC_CMT_95;
            break;
        case SMS_TRANS_TELESVC_VMN_95:
            teleSvcId = SMS_TRANS_TELESVC_VMN_95;
            break;
        case SMS_TRANS_TELESVC_WAP:
            teleSvcId = SMS_TRANS_TELESVC_WAP;
            break;
        case SMS_TRANS_TELESVC_WEMT:
            teleSvcId = SMS_TRANS_TELESVC_WEMT;
            break;
        case SMS_TRANS_TELESVC_SCPT:
            teleSvcId = SMS_TRANS_TELESVC_SCPT;
            break;
        case SMS_TRANS_TELESVC_CATPT:
            teleSvcId = SMS_TRANS_TELESVC_CATPT;
            break;
        default:
            teleSvcId = SMS_TRANS_TELESVC_RESERVED;
            break;
    }

    return offset;
}

int CdmaSmsPduCodec::DecodeSvcCtg(const unsigned char *pduStr, int pduLen, unsigned short &transSvcCtg)
{
    int offset = 0;
    unsigned short tempParam;
    offset += HEX_BYTE_STEP;
    tempParam = pduStr[offset++];
    tempParam = (tempParam << SHIFT_8BITS) + pduStr[offset++];
    if ((tempParam >= SMS_TRANS_SVC_CTG_UNKNOWN && tempParam <= SMS_TRANS_SVC_CTG_KDDI_CORP_MAX1) ||
        (tempParam >= SMS_TRANS_SVC_CTG_KDDI_CORP_MIN2 && tempParam <= SMS_TRANS_SVC_CTG_KDDI_CORP_MAX2) ||
        (tempParam >= SMS_TRANS_SVC_CTG_KDDI_CORP_MIN3 && tempParam <= SMS_TRANS_SVC_CTG_KDDI_CORP_MAX3)) {
        transSvcCtg = tempParam;
    } else {
        transSvcCtg = SMS_TRANS_SVC_CTG_RESERVED;
    }
    return offset;
}

int CdmaSmsPduCodec::DecodeAddress(const unsigned char *pduStr, int pduLen, struct SmsTransAddr &transAddr)
{
    int offset = 1;
    unsigned char tempStr[pduLen + 1];
    int tempLen = pduStr[offset++];
    (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
    if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) != EOK) {
        return offset;
    }

    if (memset_s(&transAddr, sizeof(SmsTransAddr), 0x00, sizeof(SmsTransAddr)) != EOK) {
        return offset;
    }

    offset += tempLen;
    transAddr.digitMode = (tempStr[0] & 0x80) ? true : false;
    transAddr.numberMode = (tempStr[0] & 0x40) ? true : false;
    ShiftNBitForDecode(tempStr, tempLen, SHIFT_2BITS);
    if (transAddr.digitMode) {
        if (transAddr.numberMode) {
            /* digit mode 1, number mode 1 */
            transAddr.numberType = DecodeDigitModeNumberType(tempStr[0] & 0xe0, true);
            ShiftNBitForDecode(tempStr, tempLen, SHIFT_3BITS);
        } else {
            /* digit mode 1, number mode 0 */
            transAddr.numberType = DecodeDigitModeNumberType(tempStr[0] & 0xe0, false);
            ShiftNBitForDecode(tempStr, tempLen, SHIFT_3BITS);
            transAddr.numberPlan = DecodeDigitModeNumberPlan((tempStr[0] >> SHIFT_4BITS) & 0x0f);
            ShiftNBitForDecode(tempStr, tempLen, SHIFT_4BITS);
        }
        transAddr.addrLen = tempStr[0];
        if (memcpy_s(transAddr.szData, sizeof(transAddr.szData), &tempStr[1], transAddr.addrLen) != EOK) {
            TELEPHONY_LOGE("DecodeAddress memcpy_s error.");
        }
    } else {
        transAddr.addrLen = SmsCommonUtils::BcdToDigit(&(tempStr[1]), tempStr[0], transAddr.szData);
    }
    return offset;
}

int CdmaSmsPduCodec::DecodeSubAddress(const unsigned char *pduStr, int pduLen, struct SmsTransSubAddr &subAddr)
{
    int offset = 0;
    int tempOffset = 0;
    int tempLen = 0;
    unsigned char tempStr[pduLen + 1];
    if (pduStr == nullptr) {
        return offset;
    }

    offset++;
    tempLen = pduStr[offset++];
    (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
    if (memcpy_s(tempStr, sizeof(tempStr), &pduStr[offset], tempLen) != EOK) {
        TELEPHONY_LOGE("DecodeSubAddress memcpy_s err.");
        return offset;
    }

    offset += tempLen;
    switch (tempStr[tempOffset] & 0xe0) {
        case 0x00:
            subAddr.type = SMS_TRANS_SUB_ADDR_NSAP;
            break;
        case 0x20:
            subAddr.type = SMS_TRANS_SUB_ADDR_USER;
            break;
        default:
            subAddr.type = SMS_TRANS_SUB_ADDR_RESERVED;
            break;
    }
    subAddr.odd = (tempStr[tempOffset] & 0x10) ? true : false;
    ShiftNBitForDecode(tempStr, tempLen, SHIFT_4BITS);

    int numfeilds = tempStr[tempOffset++];
    (void)memset_s(subAddr.szData, sizeof(subAddr.szData), 0x00, sizeof(subAddr.szData));
    if (memcpy_s(subAddr.szData, sizeof(subAddr.szData), tempStr + tempOffset, numfeilds) != EOK) {
        TELEPHONY_LOGE("DecodeSubAddress memcpy_s szdata error.");
    }
    return offset;
}

int CdmaSmsPduCodec::DecodeMsgId(const unsigned char *pduStr, int pduLen, struct SmsTransMsgId &smgId)
{
    int ret;
    int offset = 0;
    unsigned char tempStr[pduLen + 1];

    ret = memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
    if (ret != EOK) {
        TELEPHONY_LOGE("DecodeMsgId memset_s fail.");
        return offset;
    }
    ret = memcpy_s(tempStr, sizeof(tempStr), &pduStr[offset + HEX_BYTE_STEP], MSG_UDID_PARAM_LEN);
    if (ret != EOK) {
        TELEPHONY_LOGE("DecodeMsgId memcpy_s fail.");
        return offset;
    }

    ShiftNBitForDecode(tempStr, MSG_UDID_PARAM_LEN, SHIFT_4BITS);
    smgId.msgId = tempStr[offset++];
    smgId.msgId = (smgId.msgId << SHIFT_8BITS) + tempStr[offset++];
    smgId.headerInd = (tempStr[offset++] & 0x80) ? true : false;
    offset += HEX_BYTE_STEP;
    return offset;
}

void CdmaSmsPduCodec::DecodeCallBackNum(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcAddr &svcAddr)
{
    int offset = 0;
    unsigned char tempStr[pduLen + 1];
    (void)memset_s(&svcAddr, sizeof(SmsTeleSvcAddr), 0x00, sizeof(SmsTeleSvcAddr));

    if (pduStr[offset] & 0x80) {
        svcAddr.digitMode = true;
        svcAddr.numberType = DecodeDigitModeNumberType((pduStr[offset] << SHIFT_1BITS) & 0x70, false);
        svcAddr.numberPlan = DecodeDigitModeNumberPlan(pduStr[offset++] & 0x0f);
        svcAddr.addrLen = pduStr[offset++];
        if (svcAddr.numberType == SMS_NUMBER_TYPE_INTERNATIONAL) {
            if (memcpy_s(&(svcAddr.szData[1]), sizeof(svcAddr.szData) - 1, pduStr + offset, svcAddr.addrLen) ==
                EOK) {
                if (svcAddr.szData[1] != '\0') {
                    svcAddr.szData[0] = '+';
                }
            }
        } else {
            if (memcpy_s(svcAddr.szData, sizeof(svcAddr.szData), pduStr + offset, svcAddr.addrLen) != EOK) {
                TELEPHONY_LOGE("DecodeCallBackNum memcpy_s fail.");
            }
        }
    } else {
        svcAddr.digitMode = false;
        (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
        if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, pduLen) == EOK) {
            ShiftNBitForDecode(tempStr, pduLen, SHIFT_1BITS);
            svcAddr.addrLen = tempStr[0];
            SmsCommonUtils::BcdToDigit(&(tempStr[1]), svcAddr.addrLen, svcAddr.szData);
        }
    }
}

int CdmaSmsPduCodec::DecodeAbsTime(const unsigned char *pduStr, struct SmsTimeAbs &timeAbs)
{
    int offset = 0;
    if (pduStr == nullptr) {
        return offset;
    }

    timeAbs.year = (((pduStr[offset] & 0xf0) >> SHIFT_4BITS) * DECIMAL_NUM) + (pduStr[offset] & 0x0f);
    offset++;
    timeAbs.month = (((pduStr[offset] & 0xf0) >> SHIFT_4BITS) * DECIMAL_NUM) + (pduStr[offset] & 0x0f);
    offset++;
    timeAbs.day = (((pduStr[offset] & 0xf0) >> SHIFT_4BITS) * DECIMAL_NUM) + (pduStr[offset] & 0x0f);
    offset++;
    timeAbs.hour = (((pduStr[offset] & 0xf0) >> SHIFT_4BITS) * DECIMAL_NUM) + (pduStr[offset] & 0x0f);
    offset++;
    timeAbs.minute = (((pduStr[offset] & 0xf0) >> SHIFT_4BITS) * DECIMAL_NUM) + (pduStr[offset] & 0x0f);
    offset++;
    timeAbs.second = (((pduStr[offset] & 0xf0) >> SHIFT_4BITS) * DECIMAL_NUM) + (pduStr[offset] & 0x0f);
    offset++;
    return offset;
}

int CdmaSmsPduCodec::EncodeUserData(const unsigned char *src, unsigned char *dest, int srcSize)
{
    int shift = 0;
    if (src == nullptr) {
        return shift;
    }

    unsigned char *temp = (unsigned char *)calloc(BYTE_STEP, srcSize + 0x01);
    if (temp == nullptr) {
        return shift;
    }
    for (int i = 0; i < srcSize; i++) {
        temp[i] = src[i] << SHIFT_1BITS;
    }
    int j = 0;
    for (int i = 0; i < srcSize; i++) {
        shift = j % ENCODE_GSM_BIT;
        dest[j++] = static_cast<unsigned char>((static_cast<uint32_t>(temp[i]) << shift) +
            (static_cast<uint32_t>(temp[i + 0x01]) >> (ENCODE_GSM_BIT - shift)));
        if (shift == 0x06) {
            i++;
        }
    }
    if (temp) {
        free(temp);
        temp = nullptr;
    }
    return j;
}

void CdmaSmsPduCodec::DecodeCMASData(unsigned char *pduStr, int pduLen, struct SmsTeleSvcCmasData &cmasData)
{
    int offset = 0;
    int tempLen = 0;
    if (memset_s(&cmasData, sizeof(SmsTeleSvcCmasData), 0x00, sizeof(SmsTeleSvcCmasData)) != EOK) {
        TELEPHONY_LOGE("DecodeCMASData memset_s fail.");
        return;
    }
    if ((pduStr[offset] & 0xf8) != 0x00) {
        TELEPHONY_LOGE("Wrong Encode Type = [%{public}d]!! must be 0", (pduStr[offset] & 0xf8) >> SHIFT_3BITS);
        return;
    }
    ShiftNBitForDecode(pduStr, pduLen, SHIFT_5BITS);
    offset++;
    if (pduStr[offset++] != 0x00) {
        TELEPHONY_LOGD("Wrong protocol version = [%{public}d]!! must be 0", pduStr[offset - 1]);
        cmasData.isWrongRecodeType = true;
        return;
    }
    while (offset < pduLen - 1) {
        if (pduStr[offset] == 0x00) {
            offset++;
            offset += DecodeCMASType0Data(&pduStr[offset], pduLen - offset, cmasData);
        } else if (pduStr[offset] == 0x01) {
            offset += HEX_BYTE_STEP;
            tempLen = pduStr[offset - 1];
            TELEPHONY_LOGD("Type 1 length = [%{public}d]", tempLen);
            cmasData.category = static_cast<enum SmsCmaeCategory>(pduStr[offset++]);
            cmasData.responseType = static_cast<enum SmsCmaeResponseType>(pduStr[offset++]);
            cmasData.severity = static_cast<enum SmsCmaeSeverity>(pduStr[offset] >> SHIFT_4BITS);
            cmasData.urgency = static_cast<enum SmsCmaeUrgency>(pduStr[offset++] & 0x0f);
            cmasData.certainty = static_cast<enum SmsCmaeCertainty>(pduStr[offset++] >> SHIFT_4BITS);
        } else if (pduStr[offset] == 0x02) {
            offset += HEX_BYTE_STEP;
            tempLen = pduStr[offset - 1];
            TELEPHONY_LOGD("Type 2 length = [%{public}d]", tempLen);
            cmasData.id = pduStr[offset++];
            cmasData.id = (cmasData.id << SHIFT_8BITS) + pduStr[offset++];
            cmasData.alertHandle = static_cast<enum SmsCmaeAlertHandle>(pduStr[offset++]);
            offset += DecodeAbsTime(pduStr + offset, cmasData.expires);
            cmasData.language = static_cast<enum SmsLanguageType>(pduStr[offset++]);
        } else {
            offset++;
        }
        TELEPHONY_LOGD("offset = [%d], pduLen = [%{public}d]", offset, pduLen);
    }
}

int CdmaSmsPduCodec::DecodeCMASType0Data(unsigned char *pduStr, int pduLen, struct SmsTeleSvcCmasData &cmasData)
{
    int offset = 0;
    int tempLen = 0;
    unsigned char tempStr[pduLen + 1];
    tempLen = pduStr[offset++];
    TELEPHONY_LOGD("Type 0 length = [%{public}d]", tempLen);
    (void)memset_s(tempStr, sizeof(tempStr), 0x00, sizeof(tempStr));
    if (memcpy_s(tempStr, sizeof(tempStr), pduStr + offset, tempLen) != EOK) {
        cmasData.encodeType = FindMsgEncodeType(tempStr[0] & 0xf8);
        ShiftNBitForDecode(tempStr, tempLen, SHIFT_5BITS);
        switch (cmasData.encodeType) {
            case SMS_ENCODE_7BIT_ASCII:
            case SMS_ENCODE_IA5:
            case SMS_ENCODE_GSM7BIT:
                cmasData.dataLen = (tempLen * BYTE_BITS - SHIFT_5BITS) / ENCODE_GSM_BIT;
                for (unsigned int i = 0; i < cmasData.dataLen; i++) {
                    cmasData.alertText[i] = tempStr[0] >> SHIFT_1BITS;
                    ShiftNBitForDecode(tempStr, tempLen, SHIFT_7BITS);
                }
                break;
            case SMS_ENCODE_EPM:
            case SMS_ENCODE_GSMDCS:
                break;
            default:
                cmasData.dataLen = tempLen - 1;
                if (memcpy_s(cmasData.alertText, sizeof(cmasData.alertText), tempStr + offset, tempLen - 1) != EOK) {
                    TELEPHONY_LOGE("cmasData alertText memcpy_s fail.");
                }
                break;
        }
    }
    offset += tempLen;
    return offset;
}

void CdmaSmsPduCodec::DecodeUserData(unsigned char *pduStr, int pduLen, struct SmsTeleSvcUserData &userData)
{
    int offset = 0;
    if (pduStr == nullptr) {
        return;
    }

    userData.encodeType = FindMsgEncodeType(pduStr[offset] & 0xf8);
    ShiftNBitForDecode(pduStr, pduLen, SHIFT_5BITS);
    if (userData.encodeType == SMS_ENCODE_EPM || userData.encodeType == SMS_ENCODE_GSMDCS) {
        userData.msgType = pduStr[offset];
        ShiftNBitForDecode(pduStr, pduLen, SHIFT_8BITS);
    }

    userData.dataLen = pduStr[offset++];
    if (memset_s(userData.userData, sizeof(userData.userData), 0x00, sizeof(userData.userData)) != EOK) {
        TELEPHONY_LOGE("DecodeUserData memset_s err.");
        return;
    }

    switch (userData.encodeType) {
        case SMS_ENCODE_7BIT_ASCII:
        case SMS_ENCODE_IA5:
            for (unsigned int i = 0; i < userData.dataLen; i++) {
                userData.userData[i] = pduStr[offset] >> SHIFT_1BITS;
                ShiftNBitForDecode(&pduStr[offset], pduLen, SHIFT_7BITS);
                offset++;
            }
            break;
        case SMS_ENCODE_GSM7BIT:
            SmsCommonUtils::Unpack7bitChar(&(pduStr[offset]), userData.dataLen, 0x00, userData.userData);
            break;
        case SMS_ENCODE_EPM:
        case SMS_ENCODE_GSMDCS:
            break;
        case SMS_ENCODE_UNICODE:
            userData.dataLen *= HEX_BYTE_STEP;
            if (memcpy_s(userData.userData, sizeof(userData.userData), pduStr + offset, userData.dataLen) != EOK) {
                TELEPHONY_LOGE("SMS_ENCODE_UNICODE memcpy_s err.");
            }
            break;
        default:
            if (memcpy_s(userData.userData, sizeof(userData.userData), pduStr + offset, userData.dataLen) != EOK) {
                TELEPHONY_LOGE("unkown encodeType memcpy_s err.");
            }
            break;
    }
}

enum SmsMessageType CdmaSmsPduCodec::FindMsgType(const std::vector<unsigned char> &pduStr)
{
    std::size_t offset = 0;
    while (offset < pduStr.size()) {
        if (pduStr[offset] == 0x00) {
            return static_cast<enum SmsMessageType>((pduStr[offset + HEX_BYTE_STEP] & 0xf0) >> SHIFT_4BITS);
        }
        offset += (pduStr[offset + 1] + HEX_BYTE_STEP);
    }
    return SMS_TYPE_MAX_VALUE;
}

enum SmsEncodingType CdmaSmsPduCodec::FindMsgEncodeType(const unsigned char value)
{
    enum SmsEncodingType encodeType = SMS_ENCODE_RESERVED;
    unsigned char temp = (value >> SHIFT_3BITS);
    if (temp >= SMS_ENCODE_OCTET && temp <= SMS_ENCODE_GSMDCS) {
        encodeType = static_cast<SmsEncodingType>(temp);
    }
    if (temp == SMS_ENCODE_EUCKR) {
        return SMS_ENCODE_EUCKR;
    }
    return encodeType;
}
} // namespace Telephony
} // namespace OHOS