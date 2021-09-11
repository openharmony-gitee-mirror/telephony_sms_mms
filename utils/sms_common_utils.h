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

#ifndef SMS_COMMON_UTILS_H
#define SMS_COMMON_UTILS_H

#include <cstdint>
#include <ctime>

#include "sms_pdu_code_type.h"

namespace OHOS {
namespace Telephony {
class SmsCommonUtils {
public:
    static int Pack7bitChar(const unsigned char *userData, int dataLen, int fillBits, unsigned char *packData);
    static int Unpack7bitChar(
        const unsigned char *tpdu, unsigned char dataLen, int fillBits, unsigned char *unpackData);
    static int DigitToBcd(const char *digit, int digitLen, unsigned char *bcd);
    static int BcdToDigit(const unsigned char *bcd, int bcdLen, char *digit);
    static int ConvertDigitToDTMF(const char *digit, int digitLen, int startBit, unsigned char *dtmf);
    static long ConvertTime(const struct SmsTimeAbs &timeAbs);

private:
    static char BcdToChar(const unsigned char c);
    static unsigned char DigitToDtmfChar(const unsigned char c);
    static void DisplayTime(const time_t &rawtime);

    static constexpr uint8_t SMS_HEX_BYTE_STEP = 2;
    static constexpr uint8_t SMS_ENCODE_GSM_BIT = 7;
    static constexpr uint8_t SMS_BYTE_BIT = 8;
    static constexpr uint16_t SEC_PER_HOUR = 3600;

    static constexpr uint8_t BASE_GSM_YEAR = 100;
    static constexpr uint16_t BASE_CDMA_YEAR = 1900;
    static constexpr uint16_t BASE_CDMA_YEAR_V2 = 2000;
};
} // namespace Telephony
} // namespace OHOS
#endif // SMS_COMMON_UTILS_H