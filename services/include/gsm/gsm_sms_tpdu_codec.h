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
#ifndef GSM_SMS_TPDU_CODEC_H
#define GSM_SMS_TPDU_CODEC_H
#include "gsm_sms_udata_codec.h"
#include "msg_text_convert.h"
namespace OHOS {
namespace SMS {
#define HEX_BYTE_STEP 2
enum Sms_MTI_TYPE { SMS_MTI_DELIVER = 0, SMS_MTI_SUBMIT, SMS_MTI_STATUS_REP };

enum DecodeType {
    DECODE_SUBMIT_TYPE,
    DECODE_DELIVER_TYPE,
    DECODE_STATUS_REP_TYPE,
};

class GsmSmsTpduCodec {
public:
    GsmSmsTpduCodec() = default;
    virtual ~GsmSmsTpduCodec() = default;
    static int EncodeTpdu(const struct SmsTpdu *pSmsTpdu, char *pTpdu, int pduLen);
    static int DecodeTpdu(const unsigned char *pTpdu, int TpduLen, struct SmsTpdu *pSmsTpdu);

private:
    static int EncodeSubmit(const struct SmsSubmit *pSubmit, char *pTpdu);
    static int EncodeDeliver(const struct SmsDeliver *pDeliver, char *pTpdu);
    static int EncodeDeliverReport(const struct SmsDeliverReport *pDeliverRep, char *pTpdu);
    static int EncodeStatusReport(const struct SmsStatusReport *pStatusRep, char *pTpdu);
    static int DecodeSubmit(const unsigned char *pTpdu, int TpduLen, struct SmsSubmit *pSubmit);
    static int DecodeDeliver(const unsigned char *pTpdu, int TpduLen, struct SmsDeliver *pDeliver);
    static int DecodeStatusReport(const unsigned char *pTpdu, int TpduLen, struct SmsStatusReport *pStatusRep);
    static enum SMS_PID_E ParsePid(const unsigned char pid);
    static int EncodeSubmitTpduType(const struct SmsSubmit &pSubmit, unsigned char *pTpdu);
    static void DebugTpdu(const unsigned char *pTpdu, int TpduLen, const enum DecodeType type);
};
} // namespace SMS
} // namespace OHOS
#endif