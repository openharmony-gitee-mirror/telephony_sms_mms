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

#include "gsm_sms_param_codec.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>

#include "gsm_sms_udata_codec.h"
#include "msg_text_convert.h"
#include "securec.h"
#include "sms_common_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
int GsmSmsParamCodec::EncodeAddress(const struct SmsAddress *pAddress, char **ppParam)
{
    int offset = 0;
    int length = 0;
    if (pAddress == nullptr) {
        return offset;
    }
    const char *temp = static_cast<const char *>(pAddress->address);
    unsigned char ton = pAddress->ton;

    char *tempParam = new (std::nothrow) char[MAX_ADD_PARAM_LEN];
    if (tempParam == nullptr) {
        return offset;
    }

    if (memset_s(tempParam, sizeof(char) * MAX_ADD_PARAM_LEN, 0x00, sizeof(char) * MAX_ADD_PARAM_LEN) != EOK) {
        TELEPHONY_LOGE("textData memset_s error!");
    }
    if (temp[0] == '+') {
        tempParam[offset++] = strlen(temp) - 1;
        temp++;
        ton = SMS_TON_INTERNATIONAL;
    } else {
        tempParam[offset++] = strlen(temp);
        ton = pAddress->ton;
    }

    /* Set TON, NPI */
    tempParam[offset++] = 0x80 + (ton << 0x04) + pAddress->npi;
    TELEPHONY_LOGI("Address length is %{public}d.", tempParam[0]);
    length = SmsCommonUtils::DigitToBcd(temp, strlen(temp), (unsigned char *)&(tempParam[offset]));
    if (length >= 0) {
        offset += length;
    }
    *ppParam = tempParam;
    return offset;
}

int GsmSmsParamCodec::EncodeSMSC(const char *pAddress, unsigned char *pEncodeAddr)
{
    int ret = 0;
    char newAddr[MAX_SMSC_LEN + 1];
    if (pAddress == nullptr || pEncodeAddr == nullptr) {
        return 0;
    }
    (void)memset_s(newAddr, sizeof(newAddr), 0x00, sizeof(newAddr));
    if (pAddress[0] == '+') {
        ret = strncpy_s(newAddr, sizeof(newAddr), pAddress + 1, MAX_SMSC_LEN);
    } else {
        ret = strncpy_s(newAddr, sizeof(newAddr), pAddress, MAX_SMSC_LEN);
    }
    if (ret != EOK) {
        TELEPHONY_LOGE("EncodeSMSC error!");
        return 0;
    }
    /* Set Address */
    int encodeLen = SmsCommonUtils::DigitToBcd(newAddr, strlen(newAddr), pEncodeAddr);
    if (encodeLen < 0) {
        return 0;
    }
    pEncodeAddr[encodeLen] = '\0';
    return encodeLen;
}

int GsmSmsParamCodec::EncodeSMSC(const struct SmsAddress *pAddress, unsigned char *pSMSC, int smscLen)
{
    int ret = 0;
    int dataSize = 0;
    int addrLen = 0;
    char newAddr[MAX_SMSC_LEN + 1];
    (void)memset_s(newAddr, sizeof(newAddr), 0x00, sizeof(newAddr));
    if (pAddress == nullptr || pSMSC == nullptr) {
        return dataSize;
    }
    if (pAddress->address[0] == '+') {
        ret = memcpy_s(newAddr, sizeof(newAddr), pAddress->address + 1, strlen(pAddress->address) - 1);
    } else {
        ret = memcpy_s(newAddr, sizeof(newAddr), pAddress->address, strlen(pAddress->address));
    }
    if (ret != EOK) {
        TELEPHONY_LOGE("EncodeSMSC memory copy error!");
    }
    addrLen = strlen(newAddr);
    if (addrLen % HEX_BYTE_STEP == 0) {
        dataSize = HEX_BYTE_STEP + (addrLen / HEX_BYTE_STEP);
    } else {
        dataSize = HEX_BYTE_STEP + (addrLen / HEX_BYTE_STEP) + 1;
    }
    if (dataSize > MAX_SMSC_LEN || dataSize > smscLen) {
        TELEPHONY_LOGI("addrLen is too long [%{public}d]", addrLen);
        TELEPHONY_LOGI("dataSize is too long [%{public}d]", dataSize);
        return 0;
    }
    /* Set Address Length Check IPC 4.0 -> addrLen/2 */
    pSMSC[0] = dataSize - 1;
    /* Set TON, NPI */
    pSMSC[1] = 0x80 + ((unsigned char)pAddress->ton << 0x04) + pAddress->npi;
    /* Set Address */
    SmsCommonUtils::DigitToBcd(newAddr, addrLen, &(pSMSC[0x02]));
    pSMSC[dataSize] = '\0';
    return dataSize;
}

int GsmSmsParamCodec::EncodeTime(const struct SmsTimeStamp *pTimeStamp, char **ppParam)
{
    int ret = 0;
    int offset = 0;
    if (pTimeStamp == nullptr) {
        return offset;
    }
    if (pTimeStamp->format == SMS_TIME_ABSOLUTE) {
        int timeZone = pTimeStamp->time.absolute.timeZone;
        *ppParam = new (std::nothrow) char[MAX_ABS_TIME_PARAM_LEN];
        if (*ppParam == nullptr) {
            return offset;
        }
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.year % NUMBER_TEN) << 0x04) +
            (pTimeStamp->time.absolute.year / NUMBER_TEN);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.month % NUMBER_TEN) << 0x04) +
            (pTimeStamp->time.absolute.month / NUMBER_TEN);
        (*ppParam)[offset++] =
            ((pTimeStamp->time.absolute.day % NUMBER_TEN) << 0x04) + (pTimeStamp->time.absolute.day / NUMBER_TEN);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.hour % NUMBER_TEN) << 0x04) +
            (pTimeStamp->time.absolute.hour / NUMBER_TEN);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.minute % NUMBER_TEN) << 0x04) +
            (pTimeStamp->time.absolute.minute / NUMBER_TEN);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.second % NUMBER_TEN) << 0x04) +
            (pTimeStamp->time.absolute.second / NUMBER_TEN);

        if (timeZone < 0) {
            timeZone *= -1;
            (*ppParam)[offset] = 0x08;
        }
        (*ppParam)[offset++] += ((unsigned int)(pTimeStamp->time.absolute.timeZone % NUMBER_TEN) << 0x04) +
            (pTimeStamp->time.absolute.timeZone / NUMBER_TEN);

        return offset;
    } else if (pTimeStamp->format == SMS_TIME_RELATIVE) {
        *ppParam = new (std::nothrow) char[MAX_REL_TIME_PARAM_LEN + 1];
        if (*ppParam == nullptr) {
            return offset;
        }
        ret = memcpy_s(
            *ppParam, MAX_REL_TIME_PARAM_LEN + 1, &(pTimeStamp->time.relative.time), MAX_REL_TIME_PARAM_LEN);
        if (ret != EOK) {
            TELEPHONY_LOGE("EncodeTime memcpy_s error!");
        }
        return MAX_REL_TIME_PARAM_LEN;
    }

    return offset;
}

int GsmSmsParamCodec::EncodeDCS(const struct SmsDcs *pDCS, char **ppParam)
{
    if (pDCS == nullptr) {
        return 0;
    }
    *ppParam = new (std::nothrow) char[MAX_DCS_PARAM_LEN];
    if (*ppParam == nullptr) {
        return 0;
    }
    unsigned char *temp = (unsigned char *)*ppParam;
    *temp = 0x00;
    switch (pDCS->codingGroup) {
        case SMS_DELETION_GROUP:
        case SMS_DISCARD_GROUP:
        case SMS_STORE_GROUP:
            /* not supported */
            break;
        case SMS_GENERAL_GROUP:
            if (pDCS->msgClass != SMS_CLASS_UNKNOWN) {
                *temp = 0x10 + pDCS->msgClass;
            }
            if (pDCS->bCompressed) {
                *temp |= 0x20;
            }
            break;
        case SMS_CODING_CLASS_GROUP:
            *temp = 0xF0 + pDCS->msgClass;
            break;
        default:
            return 0;
    }

    switch (pDCS->codingScheme) {
        case SMS_CODING_7BIT:
            break;
        case SMS_CODING_UCS2:
            *temp |= 0x08;
            break;
        case SMS_CODING_8BIT:
            *temp |= 0x04;
            break;
        default:
            return 0;
    }

    return MAX_DCS_PARAM_LEN;
}

int GsmSmsParamCodec::DecodeAddress(const unsigned char *pTpdu, struct SmsAddress *pAddress)
{
    int offset = 0;
    int addrLen = 0;
    int bcdLen = 0;
    if (pTpdu == nullptr || pAddress == nullptr) {
        return offset;
    }
    MsgTextConvert *textCvt = MsgTextConvert::Instance();
    if (textCvt == nullptr) {
        TELEPHONY_LOGE("MsgTextConvert Instance is nullptr");
        return offset;
    }
    if (memset_s(pAddress->address, sizeof(pAddress->address), 0x00, sizeof(pAddress->address)) != EOK) {
        TELEPHONY_LOGE("pAddress memset_s error!");
    }
    addrLen = (int)pTpdu[offset++];
    if (addrLen % HEX_BYTE_STEP == 0) {
        bcdLen = addrLen / HEX_BYTE_STEP;
    } else {
        bcdLen = addrLen / HEX_BYTE_STEP + 1;
    }
    pAddress->ton = (pTpdu[offset] & 0x70) >> 0x04;
    pAddress->npi = pTpdu[offset++] & 0x0F;
    if (pAddress->ton == SMS_TON_ALPHA_NUMERIC) {
        char *tmpAddress = new (std::nothrow) char[MAX_ADDRESS_LEN];
        if (tmpAddress == nullptr) {
            return offset;
        }
        if (memset_s(tmpAddress, MAX_ADDRESS_LEN, 0x00, MAX_ADDRESS_LEN) != EOK) {
            TELEPHONY_LOGE("pAddress memset_s error!");
        }
        int tmplength = 0;
        tmplength = SmsCommonUtils::Unpack7bitChar(
            &(pTpdu[offset]), (addrLen * 0x04) / 0x07, 0x00, (unsigned char *)tmpAddress);
        MsgLangInfo langInfo = {0};
        langInfo.bSingleShift = false;
        langInfo.bLockingShift = false;
        textCvt->ConvertGSM7bitToUTF8((unsigned char *)pAddress->address, MAX_ADDRESS_LEN,
            (unsigned char *)tmpAddress, tmplength, &langInfo);
        if (tmpAddress) {
            delete[] tmpAddress;
        }
    } else if (pAddress->ton == SMS_TON_INTERNATIONAL) {
        SmsCommonUtils::BcdToDigit(&(pTpdu[offset]), bcdLen, &((pAddress->address)[1]));
        if (pAddress->address[1] != '\0') {
            pAddress->address[0] = '+';
        }
    } else {
        SmsCommonUtils::BcdToDigit(&(pTpdu[offset]), bcdLen, &((pAddress->address)[0]));
    }

    offset += bcdLen;
    return offset;
}

int GsmSmsParamCodec::DecodeTime(const unsigned char *pTpdu, struct SmsTimeStamp *pTimeStamp)
{
    int offset = 0;
    if (pTpdu == nullptr || pTimeStamp == nullptr) {
        return offset;
    }
    /* decode in ABSOLUTE time type. */
    pTimeStamp->format = SMS_TIME_ABSOLUTE;

    pTimeStamp->time.absolute.year = (pTpdu[offset] & 0x0F) * NUMBER_TEN + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.month = (pTpdu[offset] & 0x0F) * NUMBER_TEN + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.day = (pTpdu[offset] & 0x0F) * NUMBER_TEN + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.hour = (pTpdu[offset] & 0x0F) * NUMBER_TEN + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.minute = (pTpdu[offset] & 0x0F) * NUMBER_TEN + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.second = (pTpdu[offset] & 0x0F) * NUMBER_TEN + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.timeZone = (pTpdu[offset] & 0x07) * NUMBER_TEN + ((pTpdu[offset] & 0xF0) >> 0x04);

    if (pTpdu[offset] & 0x08) {
        pTimeStamp->time.absolute.timeZone *= (-1);
    }
    offset++;
    return offset;
}

static enum SmsMessageClass ParseMsgClasse(unsigned char dcs)
{
    switch (dcs & 0x03) {
        case SMS_INSTANT_MESSAGE:
            return SMS_INSTANT_MESSAGE;
        case SMS_OPTIONAL_MESSAGE:
            return SMS_OPTIONAL_MESSAGE;
        case SMS_SIM_MESSAGE:
            return SMS_SIM_MESSAGE;
        case SMS_FORWARD_MESSAGE:
            return SMS_FORWARD_MESSAGE;
        default:
            return SMS_CLASS_UNKNOWN;
    }
}

static SmsCodingScheme ParseMsgCodingScheme(unsigned char dcs)
{
    switch (dcs & 0x03) {
        case SMS_CODING_7BIT:
            return SMS_CODING_7BIT;
        case SMS_CODING_8BIT:
            return SMS_CODING_8BIT;
        case SMS_CODING_UCS2:
            return SMS_CODING_UCS2;
        case SMS_CODING_AUTO:
            return SMS_CODING_AUTO;
        default:
            return SMS_CODING_AUTO;
    }
}

static SmsIndicatorType ParseMsgIndicatorType(const unsigned char dcs)
{
    switch (dcs & 0x03) {
        case SMS_VOICE_INDICATOR:
            return SMS_VOICE_INDICATOR;
        case SMS_VOICE2_INDICATOR:
            return SMS_VOICE2_INDICATOR;
        case SMS_FAX_INDICATOR:
            return SMS_FAX_INDICATOR;
        case SMS_EMAIL_INDICATOR:
            return SMS_EMAIL_INDICATOR;
        default:
            return SMS_EMAIL_INDICATOR;
    }
}

static void DecodeMWIType(const unsigned char dcs, struct SmsDcs &pDCS)
{
    pDCS.bCompressed = false;
    pDCS.msgClass = SMS_CLASS_UNKNOWN;
    pDCS.bMWI = true;
    pDCS.bIndActive = (((dcs & 0x08) >> 0x03) == 0x01) ? true : false;
    pDCS.indType = ParseMsgIndicatorType(dcs & 0x03);
}

int GsmSmsParamCodec::DecodeDCS(const unsigned char *pTpdu, struct SmsDcs *pDCS)
{
    int offset = 0;
    if (pTpdu == nullptr || pDCS == nullptr) {
        return offset;
    }
    unsigned char dcs = pTpdu[offset++];
    pDCS->bMWI = false;
    pDCS->bIndActive = false;
    pDCS->indType = SMS_OTHER_INDICATOR;
    if (((dcs & 0xC0) >> 0x06) == 0) {
        pDCS->codingGroup = SMS_GENERAL_GROUP;
        pDCS->bCompressed = (dcs & 0x20) >> 0x05;
        pDCS->codingScheme = ParseMsgCodingScheme((dcs & 0x0C) >> 0x02);

        if (((dcs & 0x10) >> 0x04) == 0) {
            pDCS->msgClass = SMS_CLASS_UNKNOWN;
        } else {
            pDCS->msgClass = ParseMsgClasse(dcs & 0x03);
        }
    } else if (((dcs & 0xF0) >> 0x04) == 0x0F) {
        pDCS->codingGroup = SMS_CODING_CLASS_GROUP;
        pDCS->bCompressed = false;
        pDCS->codingScheme = ParseMsgCodingScheme((dcs & 0x0C) >> 0x02);
        pDCS->msgClass = ParseMsgClasse(dcs & 0x03);
    } else if (((dcs & 0xC0) >> 0x06) == 1) {
        pDCS->codingGroup = SMS_DELETION_GROUP;
        pDCS->bCompressed = false;
        pDCS->msgClass = SMS_CLASS_UNKNOWN;
    } else if (((dcs & 0xF0) >> 0x04) == 0x0C) {
        pDCS->codingGroup = SMS_DISCARD_GROUP;
        DecodeMWIType(dcs, *pDCS);
    } else if (((dcs & 0xF0) >> 0x04) == 0x0D) {
        pDCS->codingGroup = SMS_STORE_GROUP;
        pDCS->codingScheme = SMS_CODING_7BIT;
        DecodeMWIType(dcs, *pDCS);
    } else if (((dcs & 0xF0) >> 0x04) == 0x0E) {
        pDCS->codingGroup = SMS_STORE_GROUP;
        pDCS->codingScheme = SMS_CODING_UCS2;
        DecodeMWIType(dcs, *pDCS);
    } else {
        pDCS->codingGroup = SMS_UNKNOWN_GROUP;
        pDCS->bCompressed = (dcs & 0x20) >> 0x05;
        pDCS->codingScheme = ParseMsgCodingScheme((dcs & 0x0C) >> 0x02);
        pDCS->msgClass = SMS_CLASS_UNKNOWN;
    }

    return offset;
}

void GsmSmsParamCodec::DecodeSMSC(unsigned char *pAddress, int AddrLen, enum SmsTon ton, char *pDecodeAddr)
{
    if (pAddress == nullptr || AddrLen == 0) {
        return;
    }
    if (pDecodeAddr == nullptr) {
        return;
    }
    if (ton == SMS_TON_INTERNATIONAL) {
        pDecodeAddr[0] = '+';
        SmsCommonUtils::BcdToDigit(pAddress, AddrLen, &(pDecodeAddr[1]));
    } else {
        SmsCommonUtils::BcdToDigit(pAddress, AddrLen, pDecodeAddr);
    }
}

int GsmSmsParamCodec::DecodeSMSC(const unsigned char *pTpdu, int pduLen, struct SmsAddress &pAddress)
{
    int offset = 0;
    int addrLen = 0;
    if (pTpdu == nullptr) {
        return offset;
    }
    if (memset_s(pAddress.address, sizeof(pAddress.address), 0x00, sizeof(pAddress.address)) != EOK) {
        TELEPHONY_LOGE("textData memset_s error!");
    }
    addrLen = (int)pTpdu[offset++];
    if ((addrLen == 0) || (addrLen >= pduLen)) {
        return offset;
    }

    pAddress.ton = (pTpdu[offset] & 0x70) >> 0x04;
    pAddress.npi = pTpdu[offset++] & 0x0F;
    if (pAddress.ton == SMS_TON_INTERNATIONAL) {
        if (addrLen > (SMS_MAX_ADDRESS_LEN - 1)) {
            return 0;
        }
        SmsCommonUtils::BcdToDigit(&(pTpdu[offset]), addrLen, &((pAddress.address)[1]));
        if (pAddress.address[1] != '\0') {
            pAddress.address[0] = '+';
        }
    } else {
        if (addrLen > SMS_MAX_ADDRESS_LEN) {
            return 0;
        }
        SmsCommonUtils::BcdToDigit(&(pTpdu[offset]), addrLen, &((pAddress.address)[0]));
    }

    offset += (addrLen - 1);
    return offset;
}

bool GsmSmsParamCodec::CheckCphsVmiMsg(const unsigned char *pTpdu, int *setType, int *indType)
{
    bool ret = false;
    int offset = 0;
    int addrLen = 0;
    if (pTpdu == nullptr) {
        return ret;
    }
    addrLen = (int)pTpdu[offset++];
    if (addrLen == 0x04 && pTpdu[offset++] == 0xD0) {
        if (pTpdu[offset] == 0x11 || pTpdu[offset] == 0x10) {
            TELEPHONY_LOGI("####### VMI msg ######");
            *setType = (int)(pTpdu[offset] & 0x01); /* 0 : clear, 1 : set */
            *indType = (int)(pTpdu[offset + 1] & 0x01); /* 0 : indicator 1, 1 : indicator 2 */
            ret = true;
        }
    }
    return ret;
}
} // namespace Telephony
} // namespace OHOS