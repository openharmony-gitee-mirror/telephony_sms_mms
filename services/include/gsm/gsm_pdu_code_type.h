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
#ifndef GSM_PDU_CODE_TYPE_H
#define GSM_PDU_CODE_TYPE_H

#define MAX_UD_HEADER_NUM 7
#define MAX_USER_DATA_LEN 160
#define MAX_ADDRESS_LEN 21
#define MAX_SEGMENT_NUM 15

using SMS_TON_T = unsigned char;
using SMS_NPI_T = unsigned char;

// from 3GPP TS 23.040 V5.1.0 9.1.2.5 Address fields
enum SMS_TON_E {
    SMS_TON_UNKNOWN = 0, /* unknown */
    SMS_TON_INTERNATIONAL = 1, /* international number */
    SMS_TON_NATIONAL = 2, /* national number */
    SMS_TON_NETWORK_SPECIFIC = 3, /* network specific number */
    SMS_TON_DEDICATED_ACCESS = 4, /* subscriber number */
    SMS_TON_ALPHA_NUMERIC = 5, /* alphanumeric, GSM 7-bit default */
    SMS_TON_ABBREVIATED_NUMBER = 6, /* abbreviated number */
    SMS_TON_RESERVED_FOR_EXT = 7 /* reserved for extension */
};

// from 3GPP TS 23.040 V5.1.0 9.1.2.5 Address fields
enum SMS_NPI_E {
    SMS_NPI_UNKNOWN = 0, // Unknown
    SMS_NPI_ISDN = 1, // ISDN/telephone numbering plan
    SMS_NPI_DATA = 3, // Data numbering plan
    SMS_NPI_TELEX = 4, // Telex numbering plan
    SMS_NPI_PRIVATE = 9, // Private numbering plan
    SMS_NPI_RESERVED = 15, // Reserved for extension
};

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.2 Special SMS Message Indication
enum SMS_INDICATOR_TYPE_E {
    SMS_VOICE_INDICATOR = 0, // Voice Message Waiting
    SMS_VOICE2_INDICATOR, /* Only for CPSH */
    SMS_FAX_INDICATOR, // Fax Message Waiting
    SMS_EMAIL_INDICATOR, // Electronic Mail Message Waiting
    SMS_OTHER_INDICATOR, // Other Message Waiting
};

// from 3GPP TS 23.038 V4.3.0 5 CBS Data Coding Scheme
enum SMS_MSG_CLASS_E {
    /** class0 Indicates an instant message, which is displayed immediately after being received. */
    SMS_INSTANT_MESSAGE = 0,
    /** class1 Indicates an SMS message that can be stored on the device or SIM card based on the storage status. */
    SMS_OPTIONAL_MESSAGE,
    /** class2 Indicates an SMS message containing SIM card information, which is to be stored in a SIM card. */
    SMS_SIM_MESSAGE,
    /** class3 Indicates an SMS message to be forwarded to another device. */
    SMS_FORWARD_MESSAGE,
    /** Indicates an unknown type. */
    SMS_CLASS_UNKNOWN,
};

enum SMS_TIME_FORMAT_E { SMS_TIME_RELATIVE = 0, SMS_TIME_ABSOLUTE };

// from 3GPP TS 23.038 V4.3.0 4 SMS Data Coding Scheme
enum SMS_CODING_GROUP_E {
    SMS_GENERAL_GROUP = 0, // General Data Coding indication
    SMS_CODING_CLASS_GROUP, // Data coding/message class
    SMS_DELETION_GROUP, // Message Marked for Automatic Deletion Group
    SMS_DISCARD_GROUP, // Message Waiting Indication Group: Discard Message
    SMS_STORE_GROUP, // Message Waiting Indication Group: Store Message
    SMS_UNKNOWN_GROUP, // Reserved coding groups
};

// from 3GPP TS 23.040 V5.1.0 3.2.3 Protocol Identifier
enum SMS_PID_E {
    SMS_NORMAL_PID =
        0x00, // implicit device type is specific to this SC, or can be concluded on the basis of the address

    SMS_TELEX_PID = 0x21, // telex (or teletex reduced to telex format)
    SMS_GROUP3_TELEX_PID = 0x22, // group 3 telefax
    SMS_GROUP4_TELEX_PID = 0x23, // group 4 telefax
    SMS_VOICE_PID = 0x24, // voice telephone (i.e. conversion to speech)
    SMS_ERMES_PID = 0x25, // ERMES (European Radio Messaging System)
    SMS_NPS_PID = 0x26, // National Paging system (known to the SC)
    SMS_VIDEO_PID = 0x27, // Videotex (T.100 [20] /T.101 [21])
    SMS_TELETEX_UNSPEC_PID = 0x28, // teletex, carrier unspecified
    SMS_TELETEX_PSPDN_PID = 0x29, // teletex, in PSPDN
    SMS_TELETEX_CSPDN_PID = 0x2A, // teletex, in CSPDN
    SMS_TELETEX_PSTN_PID = 0x2B, // teletex, in analog PSTN
    SMS_TELETEX_ISDN_PID = 0x2C, // teletex, in digital ISDN
    SMS_UCI_PID = 0x2D, // UCI (Universal Computer Interface, ETSI DE/PS 3 01 3)
    SMS_x400_PID = 0x31, // any public X.400 based message handling system
    SMS_EMAIL_PID = 0x32, // Internet Electronic Mail

    SMS_TYPE0_PID = 0x40, // Short Message Type 0
    SMS_REPLACE_TYPE1_PID = 0x41, // Replace Short Message Type 1
    SMS_REPLACE_TYPE2_PID = 0x42, // Replace Short Message Type 2
    SMS_REPLACE_TYPE3_PID = 0x43, // Replace Short Message Type 3
    SMS_REPLACE_TYPE4_PID = 0x44, // Replace Short Message Type 4
    SMS_REPLACE_TYPE5_PID = 0x45, // Replace Short Message Type 5
    SMS_REPLACE_TYPE6_PID = 0x46, // Replace Short Message Type 6
    SMS_REPLACE_TYPE7_PID = 0x47, // Replace Short Message Type 7

    SMS_RETURN_CALL_PID = 0x5F,
};

// from 3GPP TS 23.040 V5.1.0 9.2.3.10 TP Data Coding Scheme (TP DCS)
typedef enum _SMS_CODING_SCHEME_E {
    SMS_CODING_7BIT = 0, // GSM 7 bit default alphabet
    SMS_CODING_8BIT, // 8 bit data
    SMS_CODING_UCS2, // UCS2 (16bit) [10]
    SMS_CODING_AUTO,
    SMS_CODING_EUCKR,
} SMS_CODING_SCHEME_E;

typedef struct SmsAddress {
    SMS_TON_T ton;
    SMS_NPI_T npi;
    char address[MAX_ADDRESS_LEN + 1]; /* < null terminated string */
} SmsAddress_S;

struct SmsTimeRel {
    unsigned char time;
};

// from 3GPP TS 23.040 V5.1.0 9.2.3.11 TP Service Centre Time Stamp (TP SCTS)
struct SmsTimeAbs {
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    int timeZone;
};

struct SmsTimeStamp {
    enum SMS_TIME_FORMAT_E format;
    union {
        struct SmsTimeRel relative;
        struct SmsTimeAbs absolute;
    } time;
};

// 3GPP TS 23.038 V4.3.0 4	SMS Data Coding Scheme
typedef struct SmsDcs {
    // indicates the text is compressed using the  compression algorithm defined in 3GPP TS 23.042 [13]
    bool bCompressed;
    // Message Waiting Indication Group: Discard Message
    bool bMWI;
    bool bIndActive;
    enum SMS_MSG_CLASS_E msgClass;
    enum _SMS_CODING_SCHEME_E codingScheme;
    enum SMS_CODING_GROUP_E codingGroup;
    enum SMS_INDICATOR_TYPE_E indType;
} SmsDcs_;

// from 3GPP TS 23.040 V5.1.0 9.2.3.3 TP Validity Period Format (TP VPF)
enum SMS_VPF_E {
    SMS_VPF_NOT_PRESENT = 0,
    SMS_VPF_ENHANCED,
    SMS_VPF_RELATIVE,
    SMS_VPF_ABSOLUTE,
};

enum _SMS_REPORT_TYPE_E { SMS_REPORT_POSITIVE = 0, SMS_REPORT_NEGATIVE };

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.1 Concatenated Short Messages
typedef struct SmsConcat8Bit {
    unsigned char msgRef;
    unsigned char totalSeg;
    unsigned char seqNum;
} SmsConcat8Bit_;

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.8 Concatenated short messages, 16-bit reference number
typedef struct SmsConcat16Bit {
    unsigned short msgRef;
    unsigned char totalSeg;
    unsigned char seqNum;
} SmsConcat16Bit_;

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.3 Application Port Addressing 8 bit address
typedef struct SmsAppPort8Bits {
    unsigned char destPort;
    unsigned char originPort;
} SmsAppPort8Bits_;

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.4 Application Port Addressing 16 bit address
typedef struct SmsAppPort16Bit {
    unsigned short destPort;
    unsigned short originPort;
} SmsAppPort16Bit_;

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.2 Special SMS Message Indication
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

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.6 UDH Source Indicator
typedef struct SmsUDH {
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
} SmsUDH_;

// from 3GPP TS 23.040 V5.1.0 9.2.3.24	TP User Data (TP UD)
typedef struct SmsUserData {
    int headerCnt;
    struct SmsUDH header[MAX_UD_HEADER_NUM];
    int length;
    char data[MAX_USER_DATA_LEN + 1];
} SmsUserData_;

// from 3GPP TS 23.040 V5.1.0 9.2.2.2 SMS SUBMIT type
typedef struct SmsSubmit {
    bool bRejectDup; // TP Reject Duplicates
    bool bStatusReport; // TP Status report capabilities
    bool bHeaderInd; // TP User Data Header Indicator (TP UDHI)
    bool bReplyPath; // TP Reply Path
    unsigned char msgRef; // TP Concatenated Short Messages
    enum SMS_VPF_E vpf; // TP Validity Period Format (TP VPF)
    struct SmsAddress destAddress; // TP Destination Address (TP DA)
    enum SMS_PID_E pid; // TP Protocol Identifier (TP PID)
    struct SmsDcs dcs; // TP Data Coding Scheme (TP DCS)
    struct SmsTimeStamp validityPeriod; // TP Validity Period Format
    struct SmsUserData userData; // TP User Data (TP UD)
} SmsSubmit_;

typedef struct SmsTpud {
    int udl;
    char ud[MAX_USER_DATA_LEN + 1];
} SmsTpud_;

// from 3GPP TS 23.040 V5.1.0 9.2.2.1 SMS DELIVER type
typedef struct SmsDeliver {
    bool bMoreMsg; // TP More Messages to Send
    bool bStatusReport; // TP Status report capabilities
    bool bHeaderInd; // TP User Data Header Indicator (TP UDHI)
    bool bReplyPath; // TP Reply Path
    struct SmsAddress originAddress; // TP Originating Address (TP OA)
    enum SMS_PID_E pid; // TP Protocol Identifier (TP PID)
    struct SmsDcs dcs; // TP Data Coding Scheme (TP DCS)
    struct SmsTimeStamp timeStamp; // TP Service Centre Time Stamp
    struct SmsUserData userData; // TP User Data (TP UD)
    struct SmsTpud udData; // TP User Data (TP UD)
} SmsDeliver_;

using SMS_REPORT_TYPE_T = unsigned char;
using SMS_FAIL_CAUSE_T = unsigned char;

// from 3GPP TS 23.040 V5.1.0 9.2.2.1a SMS DELIVER REPORT type
typedef struct SmsDeliverReport {
    SMS_REPORT_TYPE_T reportType; // TP Message Type Indicator
    bool bHeaderInd; // TP User Data Header Indicator (TP UDHI)
    SMS_FAIL_CAUSE_T failCause; // TP Failure Cause
    unsigned char paramInd; // TP Parameter Indicator
    enum SMS_PID_E pid; // TP Protocol Identifier (TP PID)
    struct SmsDcs dcs; // TP Data Coding Scheme (TP DCS)
    struct SmsUserData userData; // TP User Data (TP UD)
} SmsDeliverReport_;

using SMS_STATUS_T = unsigned char;

// from 3GPP TS 23.040 V5.1.0 9.2.2.3 SMS STATUS REPORT type
typedef struct SmsStatusReport {
    bool bMoreMsg; // More Messages to Send
    bool bStatusReport; // TP Status Report Qualifier
    bool bHeaderInd; // TP-User-Data-Header-Indication
    unsigned char msgRef; // TP Message Reference
    struct SmsAddress recipAddress; // TP Recipient Address
    struct SmsTimeStamp timeStamp; // TP Service Centre Time Stamp
    struct SmsTimeStamp dischargeTime; // TP Discharge Time
    SMS_STATUS_T status; // TP Status
    unsigned char paramInd; // TP-Parameter-Indicator
    enum SMS_PID_E pid; // TP-Protocol-Identifier
    struct SmsDcs dcs; // TP-Data-Coding-Scheme
    struct SmsUserData userData; // TP-User-Data
} SmsStatusReport_;

// from 3GPP TS 23.040 V5.1.0 9.2.3.23 TP User Data Header Indicator (TP UDHI)
enum SMS_TPDU_TYPE_E {
    SMS_TPDU_SUBMIT,
    SMS_TPDU_DELIVER,
    SMS_TPDU_DELIVER_REP,
    SMS_TPDU_STATUS_REP,
};

typedef struct SmsTpdu {
    enum SMS_TPDU_TYPE_E tpduType;
    union {
        struct SmsSubmit submit;
        struct SmsDeliver deliver;
        struct SmsDeliverReport deliverRep;
        struct SmsStatusReport statusRep;
    } data;
} SmsTpdu_S;

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.6 UDH Source Indicator
enum _SMS_UDH_TYPE_E {
    SMS_UDH_CONCAT_8BIT = 0x00,
    SMS_UDH_SPECIAL_SMS = 0x01,
    /* 0x02, 0x03 - Reserved */
    SMS_UDH_APP_PORT_8BIT = 0x04,
    SMS_UDH_APP_PORT_16BIT = 0x05,
    SMS_UDH_SC_CONTROL = 0x06,
    SMS_UDH_SRC_IND = 0x07,
    SMS_UDH_CONCAT_16BIT = 0x08,
    SMS_UDH_WCMP = 0x09,
    SMS_UDH_EMS_FIRST = 0x0a,
    SMS_UDH_EMS_LAST = 0x1f,
    SMS_UDH_ALTERNATE_REPLY_ADDRESS = 0x22,
    SMS_UDH_SINGLE_SHIFT = 0x24,
    SMS_UDH_LOCKING_SHIFT = 0x25,
    SMS_UDH_NONE = 0xFF,
};
#endif