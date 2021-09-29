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

#ifndef SMS_CB_MESSAGE_H
#define SMS_CB_MESSAGE_H

#include <ctime>
#include <memory>
#include <string>

#include "gsm_pdu_code_type.h"
#include "gsm_sms_tpdu_codec.h"

#define BYTE_BIT 8
#define ENCODE_GSM_BIT 7
#define HEX_BYTE_STEP 2

#define SMS_CB_IOS639LANG_SIZE 3
#define SMS_CB_HEADER_SIZE 6
#define SMS_CBMI_LIST_SIZE_MAX 50 // Maximum SMS CBMI list size
#define MAX_CB_MSG_TEXT_LEN 4200

#define MAX_CBMSG_PAGE_SIZE 82
#define MAX_CBMSG_TOTAL_PAGE_SIZE 1252
#define MAX_CBMSG_PAGE_BEAR_SIZE (MAX_CBMSG_TOTAL_PAGE_SIZE * 8 / 7)
#define MAX_ETWS_SIZE 56
#define MAX_CBMSG_PAGE_NUM 15

#define SMS_DCS_ALPHABET_GROUP 0x01
#define SMS_ETWS_BASE_MASK 0x1100

/**
 * @brief SmsCbMsgNetType
 * from 3GPP TS 23.041 V4.1.0 (2001-06) 5 section Network Architecture
 * GSM Cell broadcast message 0x01
 * UMTSCell broadcast message 0x02
 * CDMA broadcast message 0x03
 */
enum SmsCbMsgNetType { SMS_NETTEXT_CB_MSG_GSM = 1, SMS_NETTEXT_CB_MSG_UMTS, SMS_NETTEXT_CB_MSG_CDMA };

/**
 * @brief SmsEtwsMsgType
 * Primary ETWS message
 * GSM Secondary ETWS message
 * UMTS Secondary ETWS message
 * CDMA Seconday ETWS message
 */
enum SmsEtwsMsgType {
    SMS_NETTEXT_ETWS_PRIMARY = 0,
    SMS_NETTEXT_ETWS_SECONDARY_GSM,
    SMS_NETTEXT_ETWS_SECONDARY_UMTS,
    SMS_NETTEXT_ETWS_SECONDARY_CDMA
};

/**
 * @brief SmsCbMsgType
 * CBS Message
 * Schedule Message
 * CBS41 Message
 * JAVA-CB Message
 */
enum SmsCbMsgType {
    SMS_CBMSG_TYPE_CBS = 1,
    SMS_CBMSG_TYPE_SCHEDULE,
    SMS_CBMSG_TYPE_CBS41,
    SMS_CBMSG_TYPE_JAVACBS,
    SMS_CBMSG_TYPE_ETWS,
};

/**
 * @brief SmsCbMessageLangType
 * from 3GPP TS 23.038 V4.3.0 (2001-09) 5 section
 * CBS Data Coding Scheme
 */
enum SmsCbMessageLangType {
    SMS_CBMSG_LANG_GERMAN = 0x00,
    SMS_CBMSG_LANG_ENGLISH = 0x01,
    SMS_CBMSG_LANG_ITALIAN = 0x02,
    SMS_CBMSG_LANG_FRENCH = 0x03,
    SMS_CBMSG_LANG_SPANISH = 0x04,
    SMS_CBMSG_LANG_DUTCH = 0x05,
    SMS_CBMSG_LANG_SWEDISH = 0x06,
    SMS_CBMSG_LANG_DANISH = 0x07,
    SMS_CBMSG_LANG_PORTUGUESE = 0x08,
    SMS_CBMSG_LANG_FINNISH = 0x09,
    SMS_CBMSG_LANG_NORWEGIAN = 0x0a,
    SMS_CBMSG_LANG_GREEK = 0x0b,
    SMS_CBMSG_LANG_TURKISH = 0x0c,
    SMS_CBMSG_LANG_HUNGARIAN = 0x0d,
    SMS_CBMSG_LANG_POLISH = 0x0e,
    SMS_CBMSG_LANG_UNSPECIFIED = 0x0f,

    SMS_CBMSG_LANG_ISO639 = 0x10,

    SMS_CBMSG_LANG_CZECH = 0x20,
    SMS_CBMSG_LANG_HEBREW = 0x21,
    SMS_CBMSG_LANG_ARABIC = 0x22,
    SMS_CBMSG_LANG_RUSSIAN = 0x23,
    SMS_CBMSG_LANG_ICELANDIC = 0x24,
    SMS_CBMSG_LANG_RESERVED_25 = 0x25,
    SMS_CBMSG_LANG_RESERVED_26 = 0x26,
    SMS_CBMSG_LANG_RESERVED_27 = 0x27,
    SMS_CBMSG_LANG_RESERVED_28 = 0x28,
    SMS_CBMSG_LANG_RESERVED_29 = 0x29,
    SMS_CBMSG_LANG_RESERVED_2A = 0x2a,
    SMS_CBMSG_LANG_RESERVED_2B = 0x2b,
    SMS_CBMSG_LANG_RESERVED_2C = 0x2c,
    SMS_CBMSG_LANG_RESERVED_2D = 0x2d,
    SMS_CBMSG_LANG_RESERVED_2E = 0x2e,
    SMS_CBMSG_LANG_RESERVED_2F = 0x2f,

    SMS_CBMSG_LANG_DUMMY = 0xFF
};

enum SmsMessageSubType {
    /* SMS Specific Message Type */
    MSG_NORMAL_SMS = 0, /** Text SMS message */
    MSG_CB_SMS, /** Cell Broadcasting  message */
    MSG_JAVACB_SMS, /** JAVA Cell Broadcasting  message */

    MSG_CMAS_PRESIDENTIAL, /** CMAS CLASS */
    MSG_CMAS_EXTREME,
    MSG_CMAS_SEVERE,
    MSG_CMAS_AMBER,
    MSG_CMAS_TEST,
    MSG_CMAS_EXERCISE,
    MSG_CMAS_OPERATOR_DEFINED,
};

/**
 * @brief
 * CMAS(Commercial Mobile Alert System) 4370-4399 (0x1112-0x112F)
 */
enum SmsCmasMessageType : unsigned short {
    CMAS_PRESIDENTIAL_DEFUALT = 4370,
    CMAS_PRESIDENTIAL_SPANISH = 4383,

    CMAS_EXTREME_OBSERVED_DEFUALT = 4371,
    CMAS_EXTREME_OBSERVED_SPANISH = 4384,
    CMAS_EXTREME_LIKELY_DEFUALT = 4372,
    CMAS_EXTREME_LIKELY_SPANISH = 4385,

    CMAS_SERVER_EXTERME_OBSERVED_DEFUALT = 4373,
    CMAS_SERVER_EXTREME_OBSERVED_SPANISH = 4386,
    CMAS_SERVER_EXTERME_LIKELY_DEFUALT = 4374,
    CMAS_SERVER_EXTERME_LIKELY_SPANISH = 4387,
    CMAS_SERVER_SERVER_OBSERVED_DEFUALT = 4375,
    CMAS_SERVER_SERVER_OBSERVED_SPANISH = 4388,
    CMAS_SERVER_SERVER_LIKELY_DEFUALT = 4376,
    CMAS_SERVER_SERVER_LIKELY_SPANISH = 4389,
    CMAS_SERVER_SERVER_EXPECTED_OBSERVED_DEFUALT = 4377,
    CMAS_SERVER_SERVER_EXPECTED_OBSERVED_SPANISH = 4390,
    CMAS_SERVER_SERVER_EXPECTED_LIKELY_DEFUALT = 4378,
    CMAS_SERVER_SERVER_EXPECTED_LIKELY_SPANISH = 4391,

    CMAS_AMBER_ALERT_DEFUALT = 4379,
    CMAS_AMBER_ALERT_SPANISH = 4392,
    CMAS_RMT_ALERT_DEFUALT = 4380,
    CMAS_RMT_ALERT_SPANISH = 4393,
    CMAS_EXERCISE_ALERT_DEFUALT = 4381,
    CMAS_EXERCISE_ALERT_SPANISH = 4394,

    CMAS_OPERATOR_ALERT_DEFUALT = 4382,
    CMAS_OPERATOR_ALERT_SPANISH = 4395,
};

enum SmsCbCodingGroupType {
    SMS_CBMSG_CODGRP_GENERAL_DCS, /** Bits 7..4 00xx */
    SMS_CBMSG_CODGRP_WAP, /** 1110 Cell Broadcast */
    SMS_CBMSG_CODGRP_CLASS_CODING, /** 1111 Cell Broadcast */
};

enum CbLanguageType : unsigned short {
    SMS_LANGUAGE_UNKNOWN = 0x00,
    SMS_LANGUAGE_ENGLISH = 0x01,
    SMS_LANGUAGE_FRENCH = 0x02,
    SMS_LANGUAGE_SPANISH = 0x03,
    SMS_LANGUAGE_JAPANESE = 0x04,
    SMS_LANGUAGE_KOREAN = 0x05,
    SMS_LANGUAGE_CHINESE = 0x06,
    SMS_LANGUAGE_HEBREW = 0x07,
};

/**
 * @brief SmsCBMessageSerialNum
 * from 3GPP TS 23.041 V4.1.0 (2001-06) 9.4.1.2.1 section
 * Serial Number
 */
struct SmsCBMessageSerialNum {
    unsigned char geoScope;
    unsigned char updateNum;
    unsigned short msgCode;
};

/**
 * @brief SmsCbMessageDcs
 * from 3GPP TS 23.038 V4.3.0 (2001-09) 5 section
 * CBS Data Coding Scheme
 */
struct SmsCbMessageDcs {
    unsigned char codingGroup; /**  Coding group, GSM 03.38 */
    unsigned char classType; /** The message class */
    bool bCompressed; /** if text is compressed this is TRUE */
    unsigned char codingScheme; /** How to encode a message. */
    unsigned char langType;
    unsigned char iso639Lang[SMS_CB_IOS639LANG_SIZE]; /* 2 GSM chars and a CR char */
    bool bUDH;
    unsigned char rawData;
};

/**
 * @brief SmsCbMessageHeader
 * from 3GPP TS 23.041 V4.1.0 (2001-06) 9.4.1.2 section
 * Message Parameter
 */
struct SmsCbMessageHeader {
    bool bEtwsMessage;
    SmsCbMsgNetType cbNetType;
    unsigned char cbMsgType;
    unsigned short warningType;
    SmsCBMessageSerialNum serialNum;
    unsigned short msgId;
    unsigned char langType;
    SmsCbMessageDcs dcs;
    unsigned char page;
    unsigned char totalPages;
    time_t recvTime;

    bool operator==(const SmsCbMessageHeader &other) const
    {
        return serialNum.geoScope == other.serialNum.geoScope && serialNum.msgCode == other.serialNum.msgCode &&
            msgId == other.msgId;
    }
};

namespace OHOS {
namespace Telephony {
class SmsCbMessage {
public:
    SmsCbMessage() = default;
    ~SmsCbMessage() = default;
    bool operator==(const SmsCbMessage &other) const;
    static std::shared_ptr<SmsCbMessage> CreateCbMessage(const std::string &pdu);
    static std::shared_ptr<SmsCbMessage> CreateCbMessage(const std::vector<unsigned char> &pdu);
    std::shared_ptr<SmsCbMessageHeader> GetCbHeader() const;
    void ConvertToUTF8(const std::string &raw, std::string &message) const;
    std::string GetCbMessageRaw() const;
    bool IsSinglePageMsg() const;
    std::string ToString() const;
    unsigned short EncodeCbSerialNum(const SmsCBMessageSerialNum &snFields);

private:
    bool PduAnalysis(const std::vector<unsigned char> &pdu);
    int Decode2gHeader(const std::vector<unsigned char> &pdu);
    int Decode3gHeader(const std::vector<unsigned char> &pdu);
    void Decode2gCbMsg(const std::vector<unsigned char> &pdu);
    void Decode3gCbMsg(const std::vector<unsigned char> &pdu);
    void DecodeEtwsMsg(const std::vector<unsigned char> &pdu);
    void DecodeCbMsgDCS(const unsigned char dcsData, const unsigned short iosData, SmsCbMessageDcs &pDcs) const;
    void DecodeIos639Dcs(const unsigned char dcsData, const unsigned short iosData, SmsCbMessageDcs &pDcs) const;
    void DecodeGeneralDcs(const unsigned char dcsData, SmsCbMessageDcs &pDcs) const;
    unsigned long GetRecvTime() const;
    int CMASClass(const unsigned short messageId) const;

    std::shared_ptr<SmsCbMessageHeader> cbHeader_;
    std::string messageRaw_;
};
} // namespace Telephony
} // namespace OHOS
#endif // SMS_CB_MESSAGE_H
