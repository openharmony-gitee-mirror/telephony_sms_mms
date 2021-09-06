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

#ifndef CDMA_SMS_PDU_CODEC_H
#define CDMA_SMS_PDU_CODEC_H

#include <vector>

#include "cdma_sms_types.h"

#define MSG_UDID_PARAM_LEN 3

#define BYTE_BITS 8
#define SHIFT_1BITS 1
#define SHIFT_2BITS 2
#define SHIFT_3BITS 3
#define SHIFT_4BITS 4
#define SHIFT_5BITS 5
#define SHIFT_6BITS 6
#define SHIFT_7BITS 7
#define SHIFT_8BITS 8

#define BYTE_STEP 1
#define HALF_BYTE 4
#define MIN_PDU_LEN 2
#define HEX_BYTE_STEP 2
#define MAX_MSG_ID_LEN 5
#define DECIMAL_NUM 10
#define ENCODE_GSM_BIT 7

namespace OHOS {
namespace Telephony {
class CdmaSmsPduCodec {
public:
    CdmaSmsPduCodec() = default;
    virtual ~CdmaSmsPduCodec() = default;

    static bool CheckInvalidPDU(const std::vector<unsigned char> &pduStr);
    static int EncodeMsg(const struct SmsTransMsg &transMsg, unsigned char *pduStr);
    static int DecodeMsg(const unsigned char *pduStr, int pduLen, struct SmsTransMsg &transMsg);

private:
    static int EncodeP2PMsg(const struct SmsTransP2PMsg &p2pMsg, unsigned char *pduStr);
    static int EncodeAckMsg(const struct SmsTransAckMsg &ackMsg, unsigned char *pduStr);
    static int EncodeCBMsg(const struct SmsTransBroadCastMsg &cbMsg, unsigned char *pduStr);

    static int EncodeTelesvcMsg(const struct SmsTeleSvcMsg &svcMsg, unsigned char *pduStr);

    static int EncodeTelesvcCancelMsg(const struct SmsTeleSvcCancel &cancelMsg, unsigned char *pduStr);
    static int EncodeTelesvcSubmitMsg(const struct SmsTeleSvcSubmit &sbMsg, unsigned char *pduStr);
    static int EncodeTelesvcDeliverReportMsg(const struct SmsTeleSvcDeliverReport &dRMsg, unsigned char *pduStr);
    static int EncodeAddress(const struct SmsTransAddr &address, unsigned char *pduStr);
    static int EncodeSubAddress(const struct SmsTransSubAddr &address, unsigned char *pduStr);

    static int DecodeP2PMsg(const unsigned char *pduStr, int pduLen, struct SmsTransP2PMsg &p2pMsg);
    static int DecodeCBMsg(const unsigned char *pduStr, int pduLen, struct SmsTransBroadCastMsg &cbMsg);
    static int DecodeAckMsg(const unsigned char *pduStr, int pduLen, struct SmsTransAckMsg &ackMsg);

    static void DecodeP2PTelesvcMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcMsg &svcMsg);
    static void DecodeP2PDeliveryAckMsg(
        const unsigned char *pduStr, int pduLen, struct SmsTeleSvcDeliverAck &delAckMsg);
    static void DecodeP2PSubmitReportMsg(
        const unsigned char *pduStr, int pduLen, struct SmsTeleSvcDeliverReport &subReport);
    static void DecodeP2PUserAckMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcUserAck &userAck);
    static void DecodeP2PReadAckMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcReadAck &readAck);
    static void DecodeP2PDeliverMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcDeliver &delMsg);
    static void DecodeP2PSubmitMsg(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcSubmit &subMsg);
    static void DecodeCBBearerData(
        const unsigned char *pduStr, int pduLen, struct SmsTeleSvcMsg &telesvc, bool isCMAS);

    static int EncodeUserData(const unsigned char *src, unsigned char *dest, int srcSize);
    static void DecodeUserData(unsigned char *pduStr, int pduLen, struct SmsTeleSvcUserData &userData);
    static void DecodeCMASData(unsigned char *pduStr, int pduLen, struct SmsTeleSvcCmasData &cmasData);
    static int DecodeCMASType0Data(unsigned char *pduStr, int pduLen, struct SmsTeleSvcCmasData &cmasData);

    static int DecodeTeleId(const unsigned char *pduStr, int pduLen, unsigned short &teleSvcId);
    static int DecodeSvcCtg(const unsigned char *pduStr, int pduLen, unsigned short &transSvcCtg);
    static int DecodeAddress(const unsigned char *pduStr, int pduLen, struct SmsTransAddr &transAddr);
    static int DecodeSubAddress(const unsigned char *pduStr, int pduLen, struct SmsTransSubAddr &subAddr);

    static int DecodeMsgId(const unsigned char *pduStr, int pduLen, struct SmsTransMsgId &smgId);
    static void DecodeCallBackNum(const unsigned char *pduStr, int pduLen, struct SmsTeleSvcAddr &svcAddr);
    static int DecodeAbsTime(const unsigned char *pduStr, struct SmsTimeAbs &timeAbs);
    static enum SmsMessageType FindMsgType(const std::vector<unsigned char> &pduStr);
    static enum SmsEncodingType FindMsgEncodeType(const unsigned char value);
    static void DecodeP2PDeliverVmnAck(
        const unsigned char *pduStr, int pduLen, struct SmsEnhancedVmnAck &enhancedVmnAck);
    static void DecodeP2PEnhancedVmn(const unsigned char *pduStr, int pduLen, struct SmsEnhancedVmn &enhancedVmn);

    static unsigned char ConvertToBCD(const unsigned char val);
    static unsigned char DecodeDigitModeNumberType(const unsigned char val, bool isNumber);
    static unsigned char DecodeDigitModeNumberPlan(const unsigned char val);

    static int EncodeCbNumber(const SmsTeleSvcAddr &cbNumber, std::vector<unsigned char> &pdustr);
    static int EncodeAbsTime(const SmsTimeAbs &absTime, std::vector<unsigned char> &pdustr);
    static int EncodeMsgId(const SmsTransMsgId &msgId, const SmsMessageType &type, unsigned char *pduStr);
    static int EncodeBearerUserData(const struct SmsTeleSvcUserData &userData, unsigned char *pduStr);
};
} // namespace Telephony
} // namespace OHOS
#endif