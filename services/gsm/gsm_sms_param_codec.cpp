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
#include "sms_hilog_wrapper.h"
namespace OHOS {
namespace SMS {
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
        HILOG_ERROR("textData memset_s error!");
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
    HILOG_INFO("Address length is %{public}d.", tempParam[0]);
    length = ConvertDigitToBcd(temp, strlen(temp), (unsigned char *)&(tempParam[offset]));
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
        HILOG_ERROR("EncodeSMSC error!");
        return 0;
    }
    /* Set Address */
    int encodeLen = ConvertDigitToBcd(newAddr, strlen(newAddr), pEncodeAddr);
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
        HILOG_ERROR("EncodeSMSC memory copy error!");
    }
    addrLen = strlen(newAddr);
    if (addrLen % HEX_BYTE_STEP == 0) {
        dataSize = HEX_BYTE_STEP + (addrLen / HEX_BYTE_STEP);
    } else {
        dataSize = HEX_BYTE_STEP + (addrLen / HEX_BYTE_STEP) + 1;
    }
    if (dataSize > MAX_SMSC_LEN || dataSize > smscLen) {
        HILOG_INFO("addrLen is too long [%{public}d]", addrLen);
        HILOG_INFO("dataSize is too long [%{public}d]", dataSize);
        return 0;
    }
    /* Set Address Length Check IPC 4.0 -> addrLen/2 */
    pSMSC[0] = dataSize - 1;
    /* Set TON, NPI */
    pSMSC[1] = 0x80 + ((unsigned char)pAddress->ton << 0x04) + pAddress->npi;
    /* Set Address */
    ConvertDigitToBcd(newAddr, addrLen, &(pSMSC[0x02]));
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
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.year % numberTen_) << 0x04) +
            (pTimeStamp->time.absolute.year / numberTen_);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.month % numberTen_) << 0x04) +
            (pTimeStamp->time.absolute.month / numberTen_);
        (*ppParam)[offset++] =
            ((pTimeStamp->time.absolute.day % numberTen_) << 0x04) + (pTimeStamp->time.absolute.day / numberTen_);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.hour % numberTen_) << 0x04) +
            (pTimeStamp->time.absolute.hour / numberTen_);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.minute % numberTen_) << 0x04) +
            (pTimeStamp->time.absolute.minute / numberTen_);
        (*ppParam)[offset++] = ((pTimeStamp->time.absolute.second % numberTen_) << 0x04) +
            (pTimeStamp->time.absolute.second / numberTen_);

        if (timeZone < 0) {
            timeZone *= -1;
            (*ppParam)[offset] = 0x08;
        }
        (*ppParam)[offset++] += ((unsigned int)(pTimeStamp->time.absolute.timeZone % numberTen_) << 0x04) +
            (pTimeStamp->time.absolute.timeZone / numberTen_);

        return offset;
    } else if (pTimeStamp->format == SMS_TIME_RELATIVE) {
        *ppParam = new (std::nothrow) char[MAX_REL_TIME_PARAM_LEN + 1];
        if (*ppParam == nullptr) {
            return offset;
        }
        ret = memcpy_s(
            *ppParam, MAX_REL_TIME_PARAM_LEN + 1, &(pTimeStamp->time.relative.time), MAX_REL_TIME_PARAM_LEN);
        if (ret != EOK) {
            HILOG_ERROR("EncodeTime memcpy_s error!");
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
        HILOG_ERROR("MsgTextConvert Instance is nullptr");
        return offset;
    }
    if (memset_s(pAddress->address, sizeof(pAddress->address), 0x00, sizeof(pAddress->address)) != EOK) {
        HILOG_ERROR("pAddress memset_s error!");
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
            HILOG_ERROR("pAddress memset_s error!");
        }
        int tmplength = 0;
        tmplength = GsmSmsUDataCodec::Unpack7bitChar(&(pTpdu[offset]), (addrLen * 0x04) / 0x07, 0x00, tmpAddress);
        MSG_LANG_INFO_S langInfo = {0};
        langInfo.bSingleShift = false;
        langInfo.bLockingShift = false;
        textCvt->ConvertGSM7bitToUTF8((unsigned char *)pAddress->address, MAX_ADDRESS_LEN,
            (unsigned char *)tmpAddress, tmplength, &langInfo);
        if (tmpAddress) {
            delete[] tmpAddress;
        }
    } else if (pAddress->ton == SMS_TON_INTERNATIONAL) {
        ConvertBcdToDigit(&(pTpdu[offset]), bcdLen, &((pAddress->address)[1]));
        if (pAddress->address[1] != '\0') {
            pAddress->address[0] = '+';
        }
    } else {
        ConvertBcdToDigit(&(pTpdu[offset]), bcdLen, &((pAddress->address)[0]));
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

    pTimeStamp->time.absolute.year = (pTpdu[offset] & 0x0F) * numberTen_ + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.month = (pTpdu[offset] & 0x0F) * numberTen_ + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.day = (pTpdu[offset] & 0x0F) * numberTen_ + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.hour = (pTpdu[offset] & 0x0F) * numberTen_ + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.minute = (pTpdu[offset] & 0x0F) * numberTen_ + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.second = (pTpdu[offset] & 0x0F) * numberTen_ + ((pTpdu[offset] & 0xF0) >> 0x04);
    offset++;

    pTimeStamp->time.absolute.timeZone = (pTpdu[offset] & 0x07) * numberTen_ + ((pTpdu[offset] & 0xF0) >> 0x04);

    if (pTpdu[offset] & 0x08) {
        pTimeStamp->time.absolute.timeZone *= (-1);
    }
    offset++;
    return offset;
}

static enum SMS_MSG_CLASS_E ParseMsgClasse(unsigned char dcs)
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

static SMS_CODING_SCHEME_E ParseMsgCodingScheme(unsigned char dcs)
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

static SMS_INDICATOR_TYPE_E ParseMsgIndicatorType(const unsigned char dcs)
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
        pDCS->codingScheme = ParseMsgCodingScheme((dcs & 0x0C) >> 2);

        if (((dcs & 0x10) >> 0x04) == 0) {
            pDCS->msgClass = SMS_CLASS_UNKNOWN;
        } else {
            pDCS->msgClass = ParseMsgClasse(dcs & 0x03);
        }
    } else if (((dcs & 0xF0) >> 0x04) == 0x0F) {
        pDCS->codingGroup = SMS_CODING_CLASS_GROUP;
        pDCS->bCompressed = false;
        pDCS->codingScheme = ParseMsgCodingScheme((dcs & 0x0C) >> 2);
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

void GsmSmsParamCodec::DecodeSMSC(unsigned char *pAddress, int AddrLen, enum SMS_TON_E ton, char *pDecodeAddr)
{
    if (pAddress == nullptr || AddrLen == 0) {
        return;
    }
    if (pDecodeAddr == nullptr) {
        return;
    }
    if (ton == SMS_TON_INTERNATIONAL) {
        pDecodeAddr[0] = '+';
        ConvertBcdToDigit(pAddress, AddrLen, &(pDecodeAddr[1]));
    } else {
        ConvertBcdToDigit(pAddress, AddrLen, pDecodeAddr);
    }
}

int GsmSmsParamCodec::DecodeSMSC(const unsigned char *pTpdu, struct SmsAddress &pAddress)
{
    int offset = 0;
    int addrLen = 0;
    if (pTpdu == nullptr) {
        return offset;
    }
    if (memset_s(pAddress.address, sizeof(pAddress.address), 0x00, sizeof(pAddress.address)) != EOK) {
        HILOG_ERROR("textData memset_s error!");
    }
    addrLen = (int)pTpdu[offset++];
    pAddress.ton = (pTpdu[offset] & 0x70) >> 0x04;
    pAddress.npi = pTpdu[offset++] & 0x0F;

    if (pAddress.ton == SMS_TON_INTERNATIONAL) {
        ConvertBcdToDigit(&(pTpdu[offset]), addrLen, &((pAddress.address)[1]));
        if (pAddress.address[1] != '\0') {
            pAddress.address[0] = '+';
        }
    } else {
        ConvertBcdToDigit(&(pTpdu[offset]), addrLen, &((pAddress.address)[0]));
    }

    offset += (addrLen - 1);
    return offset;
}

int GsmSmsParamCodec::ConvertDigitToBcd(const char *pDigit, int DigitLen, unsigned char *pBcd)
{
    int offset = 0;
    unsigned char temp;
    if (pDigit == nullptr) {
        return offset;
    }
    for (int i = 0; i < DigitLen; i++) {
        switch (pDigit[i]) {
            case '*':
                temp = 0x0A;
                break;
            case '#':
                temp = 0x0B;
                break;
            case 'P':
            case 'p':
                temp = 0x0C;
                break;
            default:
                temp = pDigit[i] - '0';
                break;
        }
        if ((i % HEX_BYTE_STEP) == 0) {
            pBcd[offset] = temp & 0x0F;
        } else {
            pBcd[offset++] |= ((temp & 0x0F) << 0x04);
        }
    }

    if ((DigitLen % HEX_BYTE_STEP) == 1) {
        pBcd[offset++] |= 0xF0;
    }
    return offset;
}

char GsmSmsParamCodec::ConvertBcdToChar(const unsigned char c)
{
    char temp = 0;
    switch (c) {
        case 0x0A:
            temp = '*';
            break;
        case 0x0B:
            temp = '#';
            break;
        case 0x0C:
            temp = 'P';
            break;
        default:
            temp = c + '0';
            break;
    }
    return temp;
}

int GsmSmsParamCodec::ConvertBcdToDigit(const unsigned char *pBcd, int BcdLen, char *pDigit)
{
    int offset = 0;
    unsigned char temp;
    if (pBcd == nullptr || pDigit == nullptr) {
        return offset;
    }
    for (int i = 0; i < BcdLen; i++) {
        temp = pBcd[i] & 0x0F;
        pDigit[offset++] = ConvertBcdToChar(temp);
        temp = (pBcd[i] & 0xF0) >> 0x04;
        if (temp == 0x0F) {
            pDigit[offset] = '\0';
            return offset;
        }
        pDigit[offset++] = ConvertBcdToChar(temp);
    }
    pDigit[offset] = '\0';
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
            HILOG_INFO("####### VMI msg ######");
            *setType = (int)(pTpdu[offset] & 0x01); /* 0 : clear, 1 : set */
            *indType = (int)(pTpdu[offset + 1] & 0x01); /* 0 : indicator 1, 1 : indicator 2 */
            ret = true;
        }
    }
    return ret;
}

using MTime = struct {
    unsigned int year0;
    unsigned int mon0;
    unsigned int day;
    unsigned int hour;
    unsigned int min;
    unsigned int sec;
};

long long MkTime(const MTime &time)
{
    unsigned int mon = time.mon0;
    unsigned int year = time.year0;
    /* 1..12 -> 11,12,1..10 */
    if (static_cast<int>(mon -= 0x02) <= 0) {
        mon += YEARY_OF_MON; /* Puts Feb last since it has leap day */
        year -= 1;
    }
    auto days = static_cast<long long>(year / 0x04 - year / 0x64 + year / 0x190 + 0x16F * mon / 0x12 + time.day);
    return (((days + year * 0x16D - 0x0AFA8B) * 0x18 + time.hour) * 0x3C + time.min) * 0x3C + time.sec;
}

long GsmSmsParamCodec::ConvertTime(const struct SmsTimeStamp *time_stamp)
{
    time_t rawtime;
    if (time_stamp == nullptr) {
        return 0;
    }
    if (time_stamp->format == SMS_TIME_ABSOLUTE) {
        DebugTimeStamp(*time_stamp);
        char displayTime[MAX_TIME_LEN];
        struct tm timeTM;
        (void)memset_s(&timeTM, sizeof(struct tm), 0x00, sizeof(tm));
        struct tm timeinfo;
        (void)memset_s(&timeinfo, sizeof(struct tm), 0x00, sizeof(tm));

        timeinfo.tm_year = (time_stamp->time.absolute.year + 0x64);
        timeinfo.tm_mon = (time_stamp->time.absolute.month - 0x01);
        timeinfo.tm_mday = time_stamp->time.absolute.day;
        timeinfo.tm_hour = time_stamp->time.absolute.hour;
        timeinfo.tm_min = time_stamp->time.absolute.minute;
        timeinfo.tm_sec = time_stamp->time.absolute.second;
        timeinfo.tm_isdst = 0;
        MTime mTime;
        mTime.year0 = timeinfo.tm_year;
        mTime.mon0 = timeinfo.tm_mon;
        mTime.day = timeinfo.tm_mday;
        mTime.hour = timeinfo.tm_hour;
        mTime.min = timeinfo.tm_min;
        mTime.sec = timeinfo.tm_sec;
        rawtime = MkTime(mTime);
        (void)memset_s(displayTime, sizeof(displayTime), 0x00, sizeof(displayTime));
        strftime(displayTime, MAX_TIME_LEN, "%Y-%02m-%02d %T %z", &timeinfo);
        HILOG_INFO("displayTime [%{public}s]", displayTime);

        rawtime -= (time_stamp->time.absolute.timeZone * (0x0E10 / 0x04));

        localtime_r(&rawtime, &timeTM);
        (void)memset_s(displayTime, sizeof(displayTime), 0x00, sizeof(displayTime));
        strftime(displayTime, MAX_TIME_LEN, "%Y-%02m-%02d %T %z", &timeTM);
        HILOG_INFO("displayTime [%{public}s]", displayTime);

/* timezone value is tiemzone + daylight. So should not add daylight */
#ifdef __MSG_DAYLIGHT_APPLIED__
        rawtime -= (timezone - daylight * 0x0E10);
#else
        rawtime -= timezone;
#endif
        (void)memset_s(&timeTM, sizeof(timeTM), 0x00, sizeof(tm));
        localtime_r(&rawtime, &timeTM);
        (void)memset_s(displayTime, sizeof(displayTime), 0x00, sizeof(displayTime));
        strftime(displayTime, MAX_TIME_LEN, "%Y-%02m-%02d %T %z", &timeTM);
        HILOG_INFO("displayTime [%{public}s]", displayTime);
    } else {
        rawtime = time(NULL);
    }
    return rawtime;
}

void GsmSmsParamCodec::DebugTimeStamp(const struct SmsTimeStamp &time_stamp)
{
    HILOG_INFO("year : %{public}d", time_stamp.time.absolute.year);
    HILOG_INFO("month : %{public}d", time_stamp.time.absolute.month);
    HILOG_INFO("day : %{public}d", time_stamp.time.absolute.day);
    HILOG_INFO("hour : %{public}d", time_stamp.time.absolute.hour);
    HILOG_INFO("minute : %{public}d", time_stamp.time.absolute.minute);
    HILOG_INFO("second : %{public}d", time_stamp.time.absolute.second);
    HILOG_INFO("timezone : %{public}d", time_stamp.time.absolute.timeZone);
}
} // namespace SMS
} // namespace OHOS