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

#ifndef SMS_PDU_CODE_TYPE_H
#define SMS_PDU_CODE_TYPE_H

namespace OHOS {
namespace Telephony {
#define SMS_MAX_ADDRESS_LEN 21

enum SmsNumberPlanType : unsigned char {
    SMS_NPI_UNKNOWN = 0,
    SMS_NPI_ISDN = 1,
    SMS_NPI_DATA = 3,
    SMS_NPI_TELEX = 4,
    SMS_NPI_PRIVATE = 9,
    SMS_NPI_RESERVED = 15,
};

/**
 * @brief SmsIndicatorType
 * from 3GPP TS 23.040 V5.1.0 9.2.3.24.2 Special SMS Message Indication
 */
enum SmsIndicatorType {
    SMS_VOICE_INDICATOR = 0, // Voice Message Waiting
    SMS_VOICE2_INDICATOR, /* Only for CPSH */
    SMS_FAX_INDICATOR, // Fax Message Waiting
    SMS_EMAIL_INDICATOR, // Electronic Mail Message Waiting
    SMS_OTHER_INDICATOR, // Other Message Waiting
};

enum SmsTimeFormat : unsigned char { SMS_TIME_EMPTY = 0, SMS_TIME_RELATIVE, SMS_TIME_ABSOLUTE };

// cdma
typedef enum SmsRelativeTime : unsigned char {
    SMS_REL_TIME_5_MINS = 0,
    SMS_REL_TIME_12_HOURS = 143,
    SMS_REL_TIME_1_DAY = 167,
    SMS_REL_TIME_2_DAYS = 168,
    SMS_REL_TIME_3_DAYS = 169,
    SMS_REL_TIME_1_WEEK = 173,
    SMS_REL_TIME_INDEFINITE = 245,
    SMS_REL_TIME_IMMEDIATE = 246,
    SMS_REL_TIME_ACTIVE = 247,
    SMS_REL_TIME_REGISTRATION = 248,
    SMS_REL_TIME_RESERVED
} SmsRelativeTimeEnum;

struct SmsTimeRel {
    unsigned char time;
};

struct SmsTimeAbs {
    unsigned char year; /* range 00-99 (96~99 : 19xx, 00~95 : 20xx) */
    unsigned char month; /* range 1-12 */
    unsigned char day;
    unsigned char hour; /* range 0-23 */
    unsigned char minute; /* range 0-59 */
    unsigned char second; /* range 0-59 */
    int timeZone; // gsm
};

/**
 * @brief SmsConcat8Bit
 * from 3GPP TS 23.040 V5.1.0 9.2.3.24.1 Concatenated Short Messages
 */
typedef struct SmsConcat8Bit {
    unsigned char msgRef;
    unsigned char totalSeg;
    unsigned char seqNum;
} SmsConcat8Bit_;

/**
 * @brief SmsConcat16Bit
 * from 3GPP TS 23.040 V5.1.0 9.2.3.24.8 Concatenated short messages, 16-bit reference number
 */
typedef struct SmsConcat16Bit {
    unsigned short msgRef;
    unsigned char totalSeg;
    unsigned char seqNum;
} SmsConcat16Bit_;

/**
 * @brief SmsAppPort8Bits
 * from 3GPP TS 23.040 V5.1.0 9.2.3.24.3 Application Port Addressing 8 bit address
 */
typedef struct SmsAppPort8Bits {
    unsigned char destPort;
    unsigned char originPort;
} SmsAppPort8Bits_;

/**
 * @brief SmsAppPort16Bit
 * from 3GPP TS 23.040 V5.1.0 9.2.3.24.4 Application Port Addressing 16 bit address
 */
typedef struct SmsAppPort16Bit {
    unsigned short destPort;
    unsigned short originPort;
} SmsAppPort16Bit_;

/**
 * @brief SmsSpecialIndication
 * from 3GPP TS 23.040 V5.1.0 9.2.3.24.2 Special SMS Message Indication
 */
typedef struct SmsSpecialIndication {
    bool bStore;
    unsigned short msgInd;
    unsigned short waitMsgNum;
} SmsSpecialIndication_;

typedef struct MsgSingleShift {
    unsigned char langId;
} MsgSingleShift_;

typedef struct MsgLockingShifi {
    unsigned char langId;
} MsgLockingShifi_;

typedef struct SmsAddress {
    unsigned char ton;
    unsigned char npi;
    char address[SMS_MAX_ADDRESS_LEN + 1]; /* < null terminated string */
} SmsAddress_S;

/**
 * @brief Sms User Data Header
 * from 3GPP TS 23.040 V5.1.0 9.2.3.24.6 UDH Source Indicator
 */
struct SmsUDH {
    unsigned char udhType;
    union {
        struct SmsConcat8Bit concat8bit;
        struct SmsConcat16Bit concat16bit;
        struct SmsAppPort8Bits appPort8bit;
        struct SmsAppPort16Bit appPort16bit;
        struct SmsSpecialIndication specialInd;
        struct MsgSingleShift singleShift;
        struct MsgLockingShifi lockingShift;
        struct SmsAddress alternateAddress;
    } udh;
};
} // namespace Telephony
} // namespace OHOS
#endif