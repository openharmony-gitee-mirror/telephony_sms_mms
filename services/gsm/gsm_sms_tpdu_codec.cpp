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

#include "gsm_sms_tpdu_codec.h"

#include <memory>
#include <string>

#include "gsm_sms_param_codec.h"
#include "gsm_sms_udata_codec.h"
#include "securec.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
template<typename T>
inline void UniquePtrDeleterOneDimension(T **(&ptr))
{
    if (ptr && *ptr) {
        delete[] *ptr;
        *ptr = nullptr;
    }
}

int GsmSmsTpduCodec::EncodeTpdu(const struct SmsTpdu *pSmsTpdu, char *pTpdu, int pduLen)
{
    int tpduLen = 0;
    TELEPHONY_LOGI("pduLen%{public}d", pduLen);
    if (pSmsTpdu == nullptr) {
        return tpduLen;
    }
    switch (pSmsTpdu->tpduType) {
        case SMS_TPDU_SUBMIT:
            tpduLen = EncodeSubmit(&(pSmsTpdu->data.submit), pTpdu);
            break;
        case SMS_TPDU_DELIVER:
            tpduLen = EncodeDeliver(&(pSmsTpdu->data.deliver), pTpdu);
            break;
        case SMS_TPDU_DELIVER_REP:
            tpduLen = EncodeDeliverReport(&(pSmsTpdu->data.deliverRep), pTpdu);
            break;
        case SMS_TPDU_STATUS_REP:
            tpduLen = EncodeStatusReport(&(pSmsTpdu->data.statusRep), pTpdu);
            break;
        default:
            break;
    }
    return tpduLen;
}

int GsmSmsTpduCodec::DecodeTpdu(const unsigned char *pTpdu, int TpduLen, struct SmsTpdu *pSmsTpdu)
{
    int decodeLen = 0;
    if (pTpdu == nullptr || pSmsTpdu == nullptr) {
        return decodeLen;
    }
    const char mti = pTpdu[0] & 0x03;
    switch (mti) {
        case SMS_MTI_DELIVER:
            pSmsTpdu->tpduType = SMS_TPDU_DELIVER;
            decodeLen = DecodeDeliver(pTpdu, TpduLen, &(pSmsTpdu->data.deliver));
            break;
        case SMS_MTI_SUBMIT:
            pSmsTpdu->tpduType = SMS_TPDU_SUBMIT;
            decodeLen = DecodeSubmit(pTpdu, TpduLen, &(pSmsTpdu->data.submit));
            break;
        case SMS_MTI_STATUS_REP:
            pSmsTpdu->tpduType = SMS_TPDU_STATUS_REP;
            decodeLen = DecodeStatusReport(pTpdu, TpduLen, &(pSmsTpdu->data.statusRep));
            break;
        default:
            break;
    }
    return decodeLen;
}

int GsmSmsTpduCodec::EncodeSubmit(const struct SmsSubmit *pSubmit, char *pTpdu)
{
    int ret = 0;
    int offset = 0;
    int length = 0;
    int encodeSize = 0;
    char *address = nullptr;
    char *dcs = nullptr;
    char *vpTime = nullptr;
    unique_ptr<char *, void (*)(char **(&))> addressBuf(&address, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> dcsBuf(&dcs, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> vpBuf(&vpTime, UniquePtrDeleterOneDimension);
    if (pSubmit == nullptr || pTpdu == nullptr) {
        return offset;
    }
    offset += EncodeSubmitTpduType(*pSubmit, (unsigned char *)pTpdu);
    /* TP-MR */
    pTpdu[offset++] = pSubmit->msgRef;
    /* TP-DA */
    length = GsmSmsParamCodec::EncodeAddress(&pSubmit->destAddress, &address);
    ret = memcpy_s(&(pTpdu[offset]), length, address, length);
    if (ret != EOK) {
        return offset;
    }
    offset += length;
    TELEPHONY_LOGI("TP-DA len:=%{public}d,TP-MR msgRef:=%{public}d", length, pSubmit->msgRef);
    /* TP-PID */
    pTpdu[offset++] = pSubmit->pid;
    TELEPHONY_LOGI("TP-PID pSubmit->pid : =%{public}d", pSubmit->pid);
    /* TP-DCS */
    length = GsmSmsParamCodec::EncodeDCS(&pSubmit->dcs, &dcs);
    ret = memcpy_s(&(pTpdu[offset]), length, dcs, length);
    if (ret != EOK) {
        return offset;
    }
    offset += length;
    TELEPHONY_LOGI("TP-DCS length : =%{public}d", length);
    /* TP-VP */
    if (pSubmit->vpf != SMS_VPF_NOT_PRESENT) {
        length = GsmSmsParamCodec::EncodeTime(&pSubmit->validityPeriod, &vpTime);
        if (length > 0) {
            ret = memcpy_s(&(pTpdu[offset]), length, vpTime, length);
            if (ret != EOK) {
                TELEPHONY_LOGE("EncodeSubmit vpTime memcpy_s error.");
                return offset;
            }
            offset += length;
        }
    }
    const struct SmsUserData *pUserData = &(pSubmit->userData);
    char *pEncodeData = &(pTpdu[offset]);
    encodeSize = GsmSmsUDataCodec::EncodeUserData(pUserData, pSubmit->dcs.codingScheme, pEncodeData);
    TELEPHONY_LOGI("encodeSize : =%{public}d", encodeSize);
    offset += encodeSize;
    return offset;
}

int GsmSmsTpduCodec::EncodeSubmitTpduType(const struct SmsSubmit &pSubmit, unsigned char *pTpdu)
{
    int offset = 0;
    pTpdu[offset] = 0x01;
    /* TP-RD */
    if (pSubmit.bRejectDup == true) {
        pTpdu[offset] |= 0x04;
    }
    /* TP-VPF */
    switch (pSubmit.vpf) {
        case SMS_VPF_NOT_PRESENT:
            break;
        case SMS_VPF_ENHANCED:
            pTpdu[offset] |= 0x08;
            break;
        case SMS_VPF_RELATIVE:
            pTpdu[offset] |= 0x10;
            break;
        case SMS_VPF_ABSOLUTE:
            pTpdu[offset] |= 0x18;
            break;
        default:
            break;
    }
    /* TP-SRR */
    if (pSubmit.bStatusReport == true) {
        pTpdu[offset] |= 0x20;
    }
    TELEPHONY_LOGI("TP-SRR pSubmit->bStatusReport: =%{public}d", pSubmit.bStatusReport);
    /* TP-UDHI */
    if (pSubmit.bHeaderInd == true) {
        pTpdu[offset] |= 0x40;
    }
    /* TP-RP */
    if (pSubmit.bReplyPath == true) {
        pTpdu[offset] |= 0x80;
    }
    offset++;
    return offset;
}

int GsmSmsTpduCodec::EncodeDeliver(const struct SmsDeliver *pDeliver, char *pTpdu)
{
    int offset = 0;
    int length = 0;
    int encodeSize = 0;
    char *address = nullptr;
    char *dcs = nullptr;
    char *scts = nullptr;
    unique_ptr<char *, void (*)(char **(&))> addressBuf(&address, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> dcsBuf(&dcs, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> timeBuf(&scts, UniquePtrDeleterOneDimension);
    if (pDeliver == nullptr || pTpdu == nullptr) {
        return offset;
    }
    /* TP-MTI : 00 */
    pTpdu[offset] = 0x00;
    /* TP-MMS */
    if (pDeliver->bMoreMsg == false) {
        pTpdu[offset] |= 0x04;
    }
    /* TP-SRI */
    if (pDeliver->bStatusReport == true) {
        pTpdu[offset] |= 0x20;
    }
    /* TP-UDHI */
    if (pDeliver->bHeaderInd == true) {
        pTpdu[offset] |= 0x40;
    }
    /* TP-RP */
    if (pDeliver->bReplyPath == true) {
        pTpdu[offset] |= 0x80;
    }
    offset++;
    /* TP-OA */
    length = GsmSmsParamCodec::EncodeAddress(&pDeliver->originAddress, &address);
    if (memcpy_s(&(pTpdu[offset]), length, address, length) != EOK) {
        TELEPHONY_LOGE("EncodeDeliver originAddress memory error.");
        return offset;
    }
    offset += length;
    /* TP-PID */
    pTpdu[offset++] = pDeliver->pid;
    /* TP-DCS */
    length = GsmSmsParamCodec::EncodeDCS(&pDeliver->dcs, &dcs);
    if (memcpy_s(&(pTpdu[offset]), length, dcs, length) != EOK) {
        TELEPHONY_LOGE("EncodeDeliver dcs memory error.");
        return offset;
    }
    offset += length;
    /* TP-SCTS */
    length = GsmSmsParamCodec::EncodeTime(&pDeliver->timeStamp, &scts);
    if (memcpy_s(&(pTpdu[offset]), length, scts, length) != EOK) {
        return offset;
    }
    offset += length;
    /* TP-UDL & TP-UD */
    const struct SmsUserData *pUserData = &(pDeliver->userData);
    char *pEncodeData = &(pTpdu[offset]);
    encodeSize = GsmSmsUDataCodec::EncodeUserData(pUserData, pDeliver->dcs.codingScheme, pEncodeData);
    TELEPHONY_LOGI("encodeSize : %{public}d", encodeSize);
    offset += encodeSize;
    return offset;
}

int GsmSmsTpduCodec::EncodeDeliverReport(const struct SmsDeliverReport *pDeliverRep, char *pTpdu)
{
    int offset = 0;
    if (pDeliverRep == nullptr || pTpdu == nullptr) {
        return offset;
    }
    /* TP-MTI : 00 */
    pTpdu[offset] = 0x00;
    /* TP-UDHI */
    if (pDeliverRep->bHeaderInd == true) {
        pTpdu[offset] |= 0x40;
    }
    offset++;
    /* TP-FCS */
    if (pDeliverRep->reportType == SMS_REPORT_NEGATIVE) {
        pTpdu[offset++] = pDeliverRep->failCause;
        TELEPHONY_LOGI("Delivery report : fail cause = [%{public}02x]", pDeliverRep->failCause);
    }
    /* TP-PI */
    pTpdu[offset++] = pDeliverRep->paramInd;
    /* TP-PID */
    if (pDeliverRep->paramInd & 0x01) {
        pTpdu[offset++] = pDeliverRep->pid;
    }
    /* TP-DCS */
    if (pDeliverRep->paramInd & 0x02) {
        int length = 0;
        char *dcs = nullptr;
        unique_ptr<char *, void (*)(char **(&))> dcsBuf(&dcs, UniquePtrDeleterOneDimension);
        length = GsmSmsParamCodec::EncodeDCS(&pDeliverRep->dcs, &dcs);
        if (memcpy_s(&(pTpdu[offset]), length, dcs, length) != EOK) {
            return offset;
        }
        offset += length;
    }
    /* TP-UDL & TP-UD */
    if (pDeliverRep->paramInd & 0x04) {
        int encodeSize = 0;
        const struct SmsUserData *pUserData = &(pDeliverRep->userData);
        char *pEncodeData = &(pTpdu[offset]);
        encodeSize = GsmSmsUDataCodec::EncodeUserData(pUserData, pDeliverRep->dcs.codingScheme, pEncodeData);
        TELEPHONY_LOGI("encodeSize : %{public}d", encodeSize);
        offset += encodeSize;
    }
    pTpdu[offset] = '\0';
    return offset;
}

int GsmSmsTpduCodec::EncodeStatusReport(const struct SmsStatusReport *pStatusRep, char *pTpdu)
{
    int offset = 0;
    int length = 0;
    char *address = nullptr;
    char *scts = nullptr;
    char *dt = nullptr;
    unique_ptr<char *, void (*)(char **(&))> addressBuf(&address, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> sctsBuf(&scts, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> dtBuf(&dt, UniquePtrDeleterOneDimension);
    if (pStatusRep == nullptr || pTpdu == nullptr) {
        return offset;
    }
    /* TP-MTI : 10 */
    pTpdu[offset] = 0x02;
    /* TP-MMS */
    if (pStatusRep->bMoreMsg == true) {
        pTpdu[offset] |= 0x04;
    }
    /* TP-SRQ */
    if (pStatusRep->bStatusReport == true) {
        pTpdu[offset] |= 0x20;
    }
    /* TP-UDHI */
    if (pStatusRep->bHeaderInd == true) {
        pTpdu[offset] |= 0x40;
    }
    offset++;
    /* TP-MR */
    pTpdu[offset++] = pStatusRep->msgRef;
    /* TP-RA */
    length = GsmSmsParamCodec::EncodeAddress(&pStatusRep->recipAddress, &address);
    if (memcpy_s(&(pTpdu[offset]), length, address, length) != EOK) {
        TELEPHONY_LOGE("EncodeStatusReport memory EncodeAddress error");
        return offset;
    }
    offset += length;
    /* TP-SCTS */
    length = GsmSmsParamCodec::EncodeTime(&pStatusRep->timeStamp, &scts);
    if (memcpy_s(&(pTpdu[offset]), length, scts, length) != EOK) {
        TELEPHONY_LOGE("EncodeStatusReport memory EncodeTime error");
        return offset;
    }
    offset += length;
    /* TP-DT */
    length = GsmSmsParamCodec::EncodeTime(&pStatusRep->dischargeTime, &dt);
    if (memcpy_s(&(pTpdu[offset]), length, dt, length) != EOK) {
        TELEPHONY_LOGE("EncodeStatusReport memory EncodeTime dt error");
        return offset;
    }
    offset += length;
    /* TP-Status */
    pTpdu[offset++] = pStatusRep->status;
    /* TP-PI */
    pTpdu[offset++] = pStatusRep->paramInd;
    /* TP-PID */
    if (pStatusRep->paramInd & 0x01) {
        pTpdu[offset++] = pStatusRep->pid;
    }
    /* TP-DCS */
    if (pStatusRep->paramInd & 0x02) {
        int len = 0;
        char *dcs = nullptr;
        unique_ptr<char *, void (*)(char **(&))> dcsBuf(&dcs, UniquePtrDeleterOneDimension);
        len = GsmSmsParamCodec::EncodeDCS(&pStatusRep->dcs, &dcs);
        if (memcpy_s(&(pTpdu[offset]), len, dcs, len) != EOK) {
            TELEPHONY_LOGE("EncodeStatusReport memory EncodeDCS error");
            return offset;
        }
        offset += len;
    }
    /* TP-UDL & TP-UD */
    if (pStatusRep->paramInd & 0x04) {
        int encodeSize = 0;
        const struct SmsUserData *pUserData = &(pStatusRep->userData);
        char *pEncodeData = &(pTpdu[offset]);
        encodeSize = GsmSmsUDataCodec::EncodeUserData(pUserData, pStatusRep->dcs.codingScheme, pEncodeData);
        offset += encodeSize;
    }
    pTpdu[offset] = '\0';
    return offset;
}

int GsmSmsTpduCodec::DecodeSubmit(const unsigned char *pSubpdu, int pduLen, struct SmsSubmit *pSmsSub)
{
    int offset = 0;
    int udLen = 0;
    if (pSubpdu == nullptr || pSmsSub == nullptr) {
        return offset;
    }
    DebugTpdu(pSubpdu, pduLen, DECODE_SUBMIT_TYPE);
    // TP-RD
    if (pSubpdu[offset] & 0x04) {
        pSmsSub->bRejectDup = false;
    } else {
        pSmsSub->bRejectDup = true;
    }
    // TP-VPF
    pSmsSub->vpf = (enum SmsVpf)(pSubpdu[offset] & 0x18);
    // TP-SRR
    if (pSubpdu[offset] & 0x20) {
        pSmsSub->bStatusReport = true;
    } else {
        pSmsSub->bStatusReport = false;
    }
    // TP-UDHI
    if (pSubpdu[offset] & 0x40) {
        pSmsSub->bHeaderInd = true;
    } else {
        pSmsSub->bHeaderInd = false;
    }
    // TP-RP
    if (pSubpdu[offset] & 0x80) {
        pSmsSub->bReplyPath = true;
    } else {
        pSmsSub->bReplyPath = false;
    }
    offset++;
    // TP-MR
    pSmsSub->msgRef = pSubpdu[offset++];
    // TP-DA
    offset += GsmSmsParamCodec::DecodeAddress(pSubpdu + offset, &(pSmsSub->destAddress));
    // TP-PID
    pSmsSub->pid = ParsePid(pSubpdu[offset++]);
    // TP-DCS
    offset += GsmSmsParamCodec::DecodeDCS(pSubpdu + offset, &(pSmsSub->dcs));
    // TP-VP
    if (pSmsSub->vpf != SMS_VPF_NOT_PRESENT) {
        // Decode VP
    }
    // TP-UDL & TP-UD
    udLen = GsmSmsUDataCodec::DecodeUserData(
        pSubpdu + offset, pduLen, pSmsSub->bHeaderInd, pSmsSub->dcs.codingScheme, &(pSmsSub->userData));
    return udLen + offset;
}

void DecodePartData(const unsigned char &pTpdu, struct SmsDeliver &pDeliver)
{
    /* TP-MMS */
    if (pTpdu & 0x04) {
        pDeliver.bMoreMsg = false;
    } else {
        pDeliver.bMoreMsg = true;
    }
    /* TP-SRI */
    if (pTpdu & 0x20) {
        pDeliver.bStatusReport = true;
    } else {
        pDeliver.bStatusReport = false;
    }
    /* TP-UDHI */
    if (pTpdu & 0x40) {
        pDeliver.bHeaderInd = true;
    } else {
        pDeliver.bHeaderInd = false;
    }
    /* TP-RP */
    if (pTpdu & 0x80) {
        pDeliver.bReplyPath = true;
    } else {
        pDeliver.bReplyPath = false;
    }
}

int GsmSmsTpduCodec::DecodeDeliver(const unsigned char *pTpdu, int TpduLen, struct SmsDeliver *pDeliver)
{
    int offset = 0;
    int udLen = 0;
    int tmpOffset = 0;
    if (pTpdu == nullptr || pDeliver == nullptr) {
        return offset;
    }
    DebugTpdu(pTpdu, TpduLen, DECODE_DELIVER_TYPE);
    DecodePartData(pTpdu[offset], *pDeliver);
    offset++;
    tmpOffset = offset;
    /* TP-OA */
    offset += GsmSmsParamCodec::DecodeAddress(&pTpdu[offset], &(pDeliver->originAddress));
    /* TP-PID */
    pDeliver->pid = ParsePid(pTpdu[offset++]);
    /* TP-DCS */
    offset += GsmSmsParamCodec::DecodeDCS(&pTpdu[offset], &(pDeliver->dcs));
    /* Support KSC5601 :: Coding group bits == 0x84 */
    if (pTpdu[offset - 1] == 0x84) {
        pDeliver->dcs.codingScheme = SMS_CODING_EUCKR;
    }
    if (pDeliver->pid == 0x20 && pDeliver->originAddress.ton == SMS_TON_ALPHA_NUMERIC) {
        int setType = -1;
        int indType = -1;
        bool bVmi = GsmSmsParamCodec::CheckCphsVmiMsg(&pTpdu[tmpOffset], &setType, &indType);
        TELEPHONY_LOGI("bVmi= [%{public}d], setType=[%{public}d], indType=[%{public}d]", bVmi, setType, indType);
        if (bVmi) {
            pDeliver->dcs.bMWI = true;
            if (setType == 0) {
                pDeliver->dcs.bIndActive = false;
            } else {
                pDeliver->dcs.bIndActive = true;
            }
            if (indType == 0) {
                pDeliver->dcs.indType = SMS_VOICE_INDICATOR;
            } else if (indType == 1) {
                pDeliver->dcs.indType = SMS_VOICE2_INDICATOR;
            }
        }
    }
    /* TP-SCTS */
    offset += GsmSmsParamCodec::DecodeTime(&pTpdu[offset], &(pDeliver->timeStamp));
    /* TP-UD */
    udLen = GsmSmsUDataCodec::DecodeUserData(&pTpdu[offset], TpduLen, pDeliver->bHeaderInd,
        pDeliver->dcs.codingScheme, &(pDeliver->userData), &(pDeliver->udData));
    return udLen + offset;
}

int GsmSmsTpduCodec::DecodeStatusReport(const unsigned char *pTpdu, int TpduLen, struct SmsStatusReport *pStatusRep)
{
    int ret = 0;
    int offset = 0;
    int udLen = 0;
    char *address = nullptr;
    char *scts = nullptr;
    char *dt = nullptr;
    unique_ptr<char *, void (*)(char **(&))> addressBuf(&address, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> sctsBuf(&scts, UniquePtrDeleterOneDimension);
    unique_ptr<char *, void (*)(char **(&))> dtBuf(&dt, UniquePtrDeleterOneDimension);
    if (pTpdu == nullptr || pStatusRep == nullptr) {
        return offset;
    }
    /* TP-MMS */
    pStatusRep->bMoreMsg = (pTpdu[offset] & 0x04) ? false : true;
    /* TP-SRQ */
    pStatusRep->bStatusReport = (pTpdu[offset] & 0x20) ? true : false;
    /* TP-UDHI */
    pStatusRep->bHeaderInd = (pTpdu[offset] & 0x40) ? true : false;
    offset++;
    /* TP-MR */
    pStatusRep->msgRef = pTpdu[offset++];
    /* TP-RA */
    offset += GsmSmsParamCodec::DecodeAddress(&pTpdu[offset], &(pStatusRep->recipAddress));
    /* TP-SCTS */
    /* Decode timestamp */
    offset += GsmSmsParamCodec::DecodeTime(&pTpdu[offset], &(pStatusRep->timeStamp));
    /* TP-DT */
    /* Decode timestamp */
    offset += GsmSmsParamCodec::DecodeTime(&pTpdu[offset], &(pStatusRep->dischargeTime));
    /* TP-Status */
    pStatusRep->status = pTpdu[offset++];
    /* TP-PI */
    pStatusRep->paramInd = pTpdu[offset++];
    /* No Parameters */
    if (pStatusRep->paramInd == 0) {
        pStatusRep->pid = SMS_NORMAL_PID;
        pStatusRep->dcs.bCompressed = false;
        pStatusRep->dcs.bMWI = false;
        pStatusRep->dcs.bIndActive = false;
        pStatusRep->dcs.msgClass = SMS_CLASS_UNKNOWN;
        pStatusRep->dcs.codingScheme = SMS_CODING_7BIT;
        pStatusRep->dcs.codingGroup = SMS_GENERAL_GROUP;
        pStatusRep->dcs.indType = SMS_OTHER_INDICATOR;
        pStatusRep->userData.headerCnt = 0;
        pStatusRep->userData.length = 0;
        ret = memset_s(pStatusRep->userData.data, sizeof(pStatusRep->userData.data), 0x00, MAX_USER_DATA_LEN + 1);
        if (ret != EOK) {
            return offset;
        }
    }
    /* TP-PID */
    if (pStatusRep->paramInd & 0x01) {
        pStatusRep->pid = ParsePid(pTpdu[offset++]);
    }
    /* TP-DCS */
    if (pStatusRep->paramInd & 0x02) {
        offset += GsmSmsParamCodec::DecodeDCS(&pTpdu[offset], &(pStatusRep->dcs));
    }
    /* TP-UDL & TP-UD */
    if (pStatusRep->paramInd & 0x04) {
        /* Decode User Data */
        udLen = GsmSmsUDataCodec::DecodeUserData(&pTpdu[offset], TpduLen, pStatusRep->bHeaderInd,
            pStatusRep->dcs.codingScheme, &(pStatusRep->userData));
    }
    return udLen + offset;
}

enum SmsPid GsmSmsTpduCodec::ParsePid(const unsigned char pid)
{
    return (enum SmsPid)pid;
}

void GsmSmsTpduCodec::DebugTpdu(const unsigned char *pTpdu, int TpduLen, const enum DecodeType type)
{
    if (pTpdu == nullptr) {
        return;
    }
    char tpduTmp[(TpduLen * HEX_BYTE_STEP) + 1];
    if (memset_s(tpduTmp, sizeof(tpduTmp), 0x00, sizeof(tpduTmp)) != EOK) {
        return;
    }
    for (int i = 0; i < TpduLen; i++) {
        uint8_t step = HEX_BYTE_STEP;
        const int len = sizeof(tpduTmp) - (i * step);
        if (snprintf_s(tpduTmp + (i * step), len - 1, len - 1, "%02X", static_cast<uint32_t>(pTpdu[i])) < 0) {
            TELEPHONY_LOGE("DebugTpdu snprintf_s error");
            return;
        }
    }
    switch (type) {
        case DECODE_SUBMIT_TYPE:
            break;
        case DECODE_DELIVER_TYPE:
            break;
        case DECODE_STATUS_REP_TYPE:
            break;
        default:
            break;
    }
}
} // namespace Telephony
} // namespace OHOS