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

#include "sms_common_utils.h"

#include <ctime>
#include "securec.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
int SmsCommonUtils::Pack7bitChar(const unsigned char *userData, int dataLen, int fillBits, unsigned char *packData)
{
    int srcIdx = 0;
    int dstIdx = 0;
    auto shift = static_cast<unsigned int>(fillBits);

    if (userData == nullptr || packData == nullptr) {
        return dstIdx;
    }

    if (shift > 0) {
        dstIdx = 1;
    }
    while (srcIdx < dataLen) {
        if (shift == 0) {
            packData[dstIdx] = userData[srcIdx];
            shift = SmsCommonUtils::SMS_ENCODE_GSM_BIT;
            srcIdx++;
            dstIdx++;
            if (srcIdx >= dataLen) {
                break;
            }
        }
        if (shift > 1) {
            packData[dstIdx - 1] |= userData[srcIdx] << shift;
            packData[dstIdx] = userData[srcIdx] >> (SmsCommonUtils::SMS_BYTE_BIT - shift);
            shift--;
            srcIdx++;
            dstIdx++;
        } else if (shift == 1) {
            packData[dstIdx - 1] |= userData[srcIdx] << shift;
            srcIdx++;
            shift--;
        }
    }
    return dstIdx;
}

int SmsCommonUtils::Unpack7bitChar(
    const unsigned char *tpdu, unsigned char dataLen, int fillBits, unsigned char *unpackData)
{
    int srcIdx = 0;
    int dstIdx = 0;
    auto shift = static_cast<unsigned int>(fillBits);
    if (unpackData == nullptr || tpdu == nullptr) {
        return dstIdx;
    }
    if (shift > 0) {
        srcIdx = 1;
    }
    for (; dstIdx < dataLen; dstIdx++) {
        if (shift == 0) {
            unpackData[dstIdx] = tpdu[srcIdx] & 0x7F;
            shift = SmsCommonUtils::SMS_ENCODE_GSM_BIT;
            srcIdx++;
            dstIdx++;
            if (dstIdx >= dataLen) {
                break;
            }
        }
        if (shift > 0) {
            unpackData[dstIdx] =
                (tpdu[srcIdx - 1] >> shift) + (tpdu[srcIdx] << (SmsCommonUtils::SMS_BYTE_BIT - shift));
            unpackData[dstIdx] &= 0x7F;
            shift--;
            if (shift > 0) {
                srcIdx++;
            }
        }
    }
    return dstIdx;
}

int SmsCommonUtils::DigitToBcd(const char *digit, int digitLen, unsigned char *bcd)
{
    int offset = 0;
    unsigned char temp;
    if (digit == nullptr || bcd == nullptr) {
        return offset;
    }
    for (int i = 0; i < digitLen; i++) {
        switch (digit[i]) {
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
                temp = digit[i] - '0';
                break;
        }
        if ((i % SmsCommonUtils::SMS_HEX_BYTE_STEP) == 0) {
            bcd[offset] = temp & 0x0F;
        } else {
            bcd[offset++] |= ((temp & 0x0F) << 0x04);
        }
    }

    if ((digitLen % SmsCommonUtils::SMS_HEX_BYTE_STEP) == 1) {
        bcd[offset++] |= 0xF0;
    }
    return offset;
}

int SmsCommonUtils::BcdToDigit(const unsigned char *bcd, int bcdLen, char *digit)
{
    int offset = 0;
    unsigned char temp;
    if (bcd == nullptr || digit == nullptr) {
        return offset;
    }
    for (int i = 0; i < bcdLen; i++) {
        temp = bcd[i] & 0x0F;
        digit[offset++] = BcdToChar(temp);
        temp = (bcd[i] & 0xF0) >> 0x04;
        if (temp == 0x0F) {
            digit[offset] = '\0';
            return offset;
        }
        digit[offset++] = BcdToChar(temp);
    }
    digit[offset] = '\0';
    return offset;
}

char SmsCommonUtils::BcdToChar(const unsigned char c)
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

int SmsCommonUtils::ConvertDigitToDTMF(const char *digit, int digitLen, int startBit, unsigned char *dtmf)
{
    int shift = startBit;
    int offset = 0;
    int srcIdx = 0;
    uint8_t smsMaxShift = 4;
    unsigned char temp;
    if (digit == nullptr || dtmf == nullptr) {
        TELEPHONY_LOGE("digit or dtmf is nullptr.");
        return offset;
    }
    /* shift 1 and shift 2 are supported in this spec. */
    if (shift >= smsMaxShift) {
        TELEPHONY_LOGE("Invalid Param value shift : %d", shift);
        return offset;
    }

    for (int i = 0; i < digitLen; i++) {
        temp = DigitToDtmfChar(digit[srcIdx]);
        temp &= 0x0F;
        if (shift == 0) {
            if (i % SmsCommonUtils::SMS_HEX_BYTE_STEP == 0x01) {
                dtmf[offset] |= temp;
                offset++;
            } else {
                dtmf[offset] |= temp << 0x04;
            }
        } else if (shift >= 0x01 && shift < smsMaxShift) {
            if (i % SmsCommonUtils::SMS_HEX_BYTE_STEP == 0x01) {
                dtmf[offset] |= (temp >> shift);
                dtmf[offset + 0x01] = temp << (SmsCommonUtils::SMS_BYTE_BIT - shift);
                offset++;
            } else {
                dtmf[offset] |= (temp << (SmsCommonUtils::SMS_BYTE_BIT - shift - smsMaxShift));
            }
        }
        srcIdx++;
    }
    return offset;
}

unsigned char SmsCommonUtils::DigitToDtmfChar(const unsigned char c)
{
    switch (c) {
        case '*':
            return 0x0B;
        case '#':
            return 0x0C;
        case '0':
            return 0x0A;
        default:
            return (c - '0');
    }
}

long SmsCommonUtils::ConvertTime(const struct SmsTimeAbs &timeAbs)
{
    time_t rawtime;
    struct tm tmInfo;
    if (memset_s(&tmInfo, sizeof(struct tm), 0x00, sizeof(tm)) != EOK) {
        return time(nullptr);
    }
    tmInfo.tm_year = (timeAbs.year + BASE_GSM_YEAR);
    tmInfo.tm_mon = (timeAbs.month - 0x01);
    tmInfo.tm_mday = timeAbs.day;
    tmInfo.tm_hour = timeAbs.hour;
    tmInfo.tm_min = timeAbs.minute;
    tmInfo.tm_sec = timeAbs.second;
    tmInfo.tm_isdst = 0;

    rawtime = mktime(&tmInfo);
    DisplayTime(rawtime);

    rawtime -= (timeAbs.timeZone * (SEC_PER_HOUR / 0x04));
    DisplayTime(rawtime);
/* timezone value is tiemzone + daylight. So should not add daylight */
#ifdef __MSG_DAYLIGHT_APPLIED__
    rawtime -= (timezone - daylight * SEC_PER_HOUR);
#else
    rawtime -= timezone;
#endif

    DisplayTime(rawtime);
    return rawtime;
}

void SmsCommonUtils::DisplayTime(const time_t &rawtime)
{
    struct tm tmInfo;
    const uint8_t maxAbsTimeLen = 32;
    char displayTime[maxAbsTimeLen];
    if (memset_s(&tmInfo, sizeof(struct tm), 0x00, sizeof(tm)) != EOK) {
        TELEPHONY_LOGE("DisplayTime memset fail.");
        return;
    }

    if (memset_s(displayTime, sizeof(displayTime), 0x00, sizeof(displayTime)) != EOK) {
        TELEPHONY_LOGE("DisplayTime memset fail.");
        return;
    }

    localtime_r(&rawtime, &tmInfo);
    if (strftime(displayTime, maxAbsTimeLen, "%Y-%02m-%02d %T %z", &tmInfo) <= 0) {
        TELEPHONY_LOGE("strftime error.");
        return;
    }
    TELEPHONY_LOGI("displayTime [%{public}s]", displayTime);
}
} // namespace Telephony
} // namespace OHOS