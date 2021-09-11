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

#include "gsm_sms_udata_codec.h"

#include <cstdio>
#include <cstring>
#include <memory>

#include "securec.h"
#include "sms_common_utils.h"
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

int GsmSmsUDataCodec::EncodeUserData(
    const struct SmsUserData *(&pUserData), SmsCodingScheme CodingScheme, char *(&pEncodeData))
{
    int encodeSize = 0;
    switch (CodingScheme) {
        case SMS_CODING_7BIT:
            encodeSize = EncodeGSMData(pUserData, pEncodeData);
            break;
        case SMS_CODING_8BIT:
            encodeSize = Encode8bitData(pUserData, pEncodeData);
            break;
        case SMS_CODING_UCS2:
            encodeSize = EncodeUCS2Data(pUserData, pEncodeData);
            break;
        default:
            break;
    }
    return encodeSize;
}

int GsmSmsUDataCodec::DecodeUserData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd,
    SmsCodingScheme CodingScheme, struct SmsUserData *pUserData)
{
    int decodeSize = 0;
    if (memset_s(pUserData, sizeof(struct SmsUserData), 0x00, sizeof(struct SmsUserData)) != EOK) {
        return decodeSize;
    }
    switch (CodingScheme) {
        case SMS_CODING_7BIT:
            decodeSize = DecodeGSMData(pTpdu, tpduLen, bHeaderInd, pUserData, NULL);
            break;
        case SMS_CODING_8BIT:
            decodeSize = Decode8bitData(pTpdu, bHeaderInd, pUserData, NULL);
            break;
        case SMS_CODING_UCS2:
            decodeSize = DecodeUCS2Data(pTpdu, tpduLen, bHeaderInd, pUserData, NULL);
            break;
        default:
            break;
    }

    return decodeSize;
}

int GsmSmsUDataCodec::DecodeUserData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd,
    SmsCodingScheme CodingScheme, struct SmsUserData *pUserData, struct SmsTpud *pTPUD)
{
    int decodeSize = 0;
    if (memset_s(pUserData, sizeof(struct SmsUserData), 0x00, sizeof(struct SmsUserData)) != EOK) {
        return decodeSize;
    }
    switch (CodingScheme) {
        case SMS_CODING_7BIT:
            decodeSize = DecodeGSMData(pTpdu, tpduLen, bHeaderInd, pUserData, pTPUD);
            break;
        case SMS_CODING_8BIT:
            decodeSize = Decode8bitData(pTpdu, bHeaderInd, pUserData, pTPUD);
            break;
        case SMS_CODING_UCS2:
            decodeSize = DecodeUCS2Data(pTpdu, tpduLen, bHeaderInd, pUserData, pTPUD);
            break;
        case SMS_CODING_EUCKR:
            decodeSize = DecodeUCS2Data(pTpdu, tpduLen, bHeaderInd, pUserData, pTPUD);
            break;
        default:
            break;
    }
    return decodeSize;
}

int GsmSmsUDataCodec::EncodeGSMData(const struct SmsUserData *pUserData, char *pEncodeData)
{
    int headerLen = 0;
    int offset = 0;
    int fillBits = 0;
    int packSize = 0;
    int encodeLen = 0;
    unsigned char udhl = 0x00;

    if (pUserData->headerCnt > 0) {
        offset = HEX_BYTE_STEP;
    } else {
        offset = 0x01;
    }
    TELEPHONY_LOGI("pUserData->headerCnt [%{public}d]", pUserData->headerCnt);
    for (int i = 0; i < pUserData->headerCnt; i++) {
        headerLen = EncodeHeader(pUserData->header[i], &(pEncodeData[offset]));
        TELEPHONY_LOGI("headerLen [%{public}d]", headerLen);
        udhl += headerLen;
        offset += headerLen;
    }
    TELEPHONY_LOGI("udhl [%{public}u]", udhl);
    if (udhl > 0) {
        fillBits = ((udhl + 1) * BYTE_BIT) % ENCODE_BYTE_BIT; /* + UDHL */
    }
    if (fillBits > 0) {
        fillBits = ENCODE_BYTE_BIT - fillBits;
    }
    TELEPHONY_LOGI("fillBits [%{public}d] dataLen [%{public}d]", fillBits, pUserData->length);
    /* Set UDL, UDHL */
    if (udhl > 0) {
        pEncodeData[0] = (((udhl + 1) * 0x08) + fillBits + (pUserData->length * 0x07)) / 0x07;
        pEncodeData[1] = udhl;
    } else {
        pEncodeData[0] = (char)pUserData->length;
    }

    packSize = SmsCommonUtils::Pack7bitChar(
        (unsigned char *)pUserData->data, pUserData->length, fillBits, (unsigned char *)&pEncodeData[offset]);
    encodeLen = offset + packSize;
    TELEPHONY_LOGI("packSize [%{public}d] encodeLen [%{public}d]", packSize, encodeLen);
    return encodeLen;
}

int GsmSmsUDataCodec::Encode8bitData(const struct SmsUserData *pUserData, char *pEncodeData)
{
    int headerLen = 0;
    int offset = HEX_BYTE_STEP;
    int fillBits = 0;
    int encodeLen = 0;
    unsigned char udhl = 0x00;

    if (pUserData->headerCnt > 0) {
        offset = 0x02;
    } else {
        offset = 0x01;
    }
    /* Encode User Data Header */
    for (int i = 0; i < pUserData->headerCnt; i++) {
        headerLen = EncodeHeader(pUserData->header[i], &(pEncodeData[offset]));
        udhl += headerLen;
        offset += headerLen;
    }
    TELEPHONY_LOGI("fillBits [%{public}d]", fillBits);
    TELEPHONY_LOGI("dataLen [%{public}d]", pUserData->length);
    /* Set UDL, UDHL */
    if (udhl > 0) {
        pEncodeData[0] = (udhl + 1) + fillBits + pUserData->length;
        pEncodeData[1] = udhl;
    } else {
        pEncodeData[0] = (char)pUserData->length;
    }
    if (memcpy_s(&(pEncodeData[offset]), MAX_TPDU_DATA_LEN - offset, pUserData->data, pUserData->length) != EOK) {
        TELEPHONY_LOGE("Encode8bitData memcpy_s error");
        return encodeLen;
    }
    encodeLen = offset + pUserData->length;
    return encodeLen;
}

int GsmSmsUDataCodec::EncodeUCS2Data(const struct SmsUserData *pUserData, char *pEncodeData)
{
    int headerLen = 0;
    int offset = HEX_BYTE_STEP;
    int fillBits = 0;
    int encodeLen = 0;
    unsigned char udhl = 0x00;

    if (pUserData->headerCnt > 0) {
        offset = HEX_BYTE_STEP;
    } else {
        offset = 1;
    }
    /* Encode User Data Header */
    for (int i = 0; i < pUserData->headerCnt; i++) {
        headerLen = EncodeHeader(pUserData->header[i], &(pEncodeData[offset]));
        udhl += headerLen;
        offset += headerLen;
    }
    TELEPHONY_LOGI("fillBits [%{public}d] dataLen [%{public}d]", fillBits, pUserData->length);
    /* Set UDL, UDHL */
    if (udhl > 0) {
        pEncodeData[0] = (udhl + 1) + fillBits + pUserData->length;
        pEncodeData[1] = udhl;
    } else {
        pEncodeData[0] = (char)pUserData->length;
    }
    if (memcpy_s(&(pEncodeData[offset]), MAX_TPDU_DATA_LEN - offset, pUserData->data, pUserData->length) != EOK) {
        TELEPHONY_LOGE("EncodeUCS2Data memcpy_s error");
        return encodeLen;
    }
    encodeLen = offset + pUserData->length;
    return encodeLen;
}

int GsmSmsUDataCodec::DecodeGSMData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd,
    struct SmsUserData *pUserData, struct SmsTpud *pTPUD)
{
    int offset = 0;
    int udl = 0;
    int udhl = 0;
    int headerLen = 0;
    int fillBits = 0;
    int octetUdl = 0;
    /* UDL */
    udl = pTpdu[offset++];
    octetUdl = (udl * ENCODE_BYTE_BIT) / BYTE_BIT;
    TELEPHONY_LOGI("udl= %{public}d, tpdulen= %{public}d, octetUdl= %{public}d, bHeaderInd= %{public}d", udl,
        tpduLen, octetUdl, bHeaderInd);
    if (udl > MAX_GSM_7BIT_DATA_LEN || octetUdl > tpduLen) {
        pUserData->length = 0;
        pUserData->headerCnt = 0;
        return 0;
    }
    /* Setting for Wap Push */
    if (pTPUD != NULL) {
        pTPUD->udl = udl;
        if (memcpy_s(pTPUD->ud, sizeof(pTPUD->ud), &(pTpdu[offset]), udl) != EOK) {
            TELEPHONY_LOGE("DecodeGSMData memcpy_s error");
            return 0;
        }
        pTPUD->ud[udl] = '\0';
    }
    /* Decode User Data Header */
    if (bHeaderInd == true) {
        /* UDHL */
        udhl = pTpdu[offset++];
        TELEPHONY_LOGI("udhl = %{public}d", udhl);
        pUserData->headerCnt = 0;
        for (int i = 0; offset < udhl; i++) {
            headerLen = DecodeHeader(&(pTpdu[offset]), &(pUserData->header[i]));
            if (headerLen <= 0) {
                TELEPHONY_LOGI("Error to Header. headerLen [%{public}d]", headerLen);
                ResetUserData(*pUserData);
                return 0;
            }
            offset += headerLen;
            if (offset > (udhl + HEX_BYTE_STEP)) {
                TELEPHONY_LOGI("Error to Header. offset [%{public}d] > (udhl [%{public}d] + 2)", offset, udhl);
                ResetUserData(*pUserData);
                return 0;
            }
            pUserData->headerCnt++;
        }
    } else {
        pUserData->headerCnt = 0;
    }

    TELEPHONY_LOGI("headerCnt = %{public}d", pUserData->headerCnt);
    if (udhl > 0) {
        fillBits = ((udl * ENCODE_BYTE_BIT) - ((udhl + 1) * BYTE_BIT)) % ENCODE_BYTE_BIT;
        udl = ((udl * ENCODE_BYTE_BIT) - ((udhl + 1) * BYTE_BIT)) / ENCODE_BYTE_BIT;
    }
    TELEPHONY_LOGI("fillBits = %{public}d udhl = %{public}d udl = %{public}d offset = %{public}d", fillBits, udhl,
        udl, offset);
    pUserData->length =
        SmsCommonUtils::Unpack7bitChar(&(pTpdu[offset]), udl, fillBits, (unsigned char *)pUserData->data);
    return pUserData->length;
}

void GsmSmsUDataCodec::ResetUserData(struct SmsUserData &userData)
{
    userData.length = 0;
    if (memset_s(userData.data, sizeof(userData.data), 0x00, sizeof(userData.data)) != EOK) {
        TELEPHONY_LOGE("ResetUserData memset_s error!");
        return;
    }
    TELEPHONY_LOGI("ResetUserData memset_s ok!");
}

int GsmSmsUDataCodec::Decode8bitData(
    const unsigned char *pTpdu, bool bHeaderInd, struct SmsUserData *pUserData, struct SmsTpud *pTPUD)
{
    int offset = 0;
    int udl = 0;
    int udhl = 0;
    int headerLen = 0;
    /* UDL */
    udl = pTpdu[offset++];
    if (udl > MAX_UCS2_DATA_LEN) {
        pUserData->length = 0;
        return 0;
    }
    /* Setting for Wap Push */
    if (pTPUD != nullptr) {
        pTPUD->udl = udl;
        if (memcpy_s(pTPUD->ud, sizeof(pTPUD->ud), &(pTpdu[offset]), udl) != EOK) {
            return 0;
        }
        pTPUD->ud[udl] = '\0';
    }
    TELEPHONY_LOGI("udl = %{public}d  bHeaderInd = %{public}d", udl, bHeaderInd);
    /* Decode User Data Header */
    if (bHeaderInd) {
        /* UDHL */
        udhl = pTpdu[offset++];
        TELEPHONY_LOGI("udhl = %{public}d", udhl);
        pUserData->headerCnt = 0;
        for (int i = 0; offset < udhl; i++) {
            headerLen = DecodeHeader(&(pTpdu[offset]), &(pUserData->header[i]));
            if (headerLen <= 0) {
                pUserData->length = 0;
                if (memset_s(pUserData->data, sizeof(pUserData->data), 0x00, sizeof(pUserData->data)) != EOK) {
                    TELEPHONY_LOGE("memset_s fail.");
                }
                return 0;
            }
            offset += headerLen;
            if (offset > (udhl + HEX_BYTE_STEP)) {
                pUserData->length = 0;
                if (memset_s(pUserData->data, sizeof(pUserData->data), 0x00, sizeof(pUserData->data)) != EOK) {
                    TELEPHONY_LOGE("memset_s fail.");
                }
                return 0;
            }
            pUserData->headerCnt++;
        }
    } else {
        pUserData->headerCnt = 0;
    }
    if (udhl > 0) {
        pUserData->length = (udl) - (udhl + 1);
    } else {
        pUserData->length = udl;
    }
    TELEPHONY_LOGI("pUserData->length= %{public}d  offset= %{public}d", pUserData->length, offset);
    if (memcpy_s(pUserData->data, sizeof(pUserData->data), &(pTpdu[offset]), pUserData->length) != EOK) {
        return 0;
    }
    return pUserData->length;
}

int GsmSmsUDataCodec::DecodeUCS2Data(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd,
    struct SmsUserData *pUserData, struct SmsTpud *pTPUD)
{
    int offset = 0;
    int udl = 0;
    int udhl = 0;
    int headerLen = 0;
    /* UDL */
    udl = pTpdu[offset++];
    if (udl > MAX_UCS2_DATA_LEN || udl > tpduLen) {
        pUserData->length = 0;
        pUserData->headerCnt = 0;
        return 0;
    }
    /* Setting for Wap Push */
    if (pTPUD != nullptr) {
        pTPUD->udl = udl;
        if (memcpy_s(pTPUD->ud, sizeof(pTPUD->ud), &(pTpdu[offset]), udl) != EOK) {
            return 0;
        }
        pTPUD->ud[udl] = '\0';
    }
    /* Decode User Data Header */
    if (bHeaderInd == true) {
        /* UDHL */
        udhl = pTpdu[offset++];
        pUserData->headerCnt = 0;
        for (int i = 0; offset < udhl; i++) {
            headerLen = DecodeHeader(&(pTpdu[offset]), &(pUserData->header[i]));
            if (headerLen <= 0) {
                ResetUserData(*pUserData);
                return 0;
            }
            offset += headerLen;
            if (offset > (udhl + HEX_BYTE_STEP)) {
                ResetUserData(*pUserData);
                return 0;
            }
            pUserData->headerCnt++;
        }
    } else {
        pUserData->headerCnt = 0;
    }

    if (udhl > 0) {
        pUserData->length = (udl) - (udhl + 1);
    } else {
        pUserData->length = udl;
    }
    TELEPHONY_LOGI("pUserData->length= %{public}d", pUserData->length);
    if (memcpy_s(pUserData->data, sizeof(pUserData->data), &(pTpdu[offset]), pUserData->length) != EOK) {
        return 0;
    }
    pUserData->data[pUserData->length] = 0;
    return pUserData->length;
}

int GsmSmsUDataCodec::EncodeHeader(const struct SmsUDH header, char *pHeader)
{
    int index = 0;
    int addrLen = 0;
    char *encodedAddr = nullptr;
    unique_ptr<char *, void (*)(char **(&))> addressBuf(&encodedAddr, UniquePtrDeleterOneDimension);
    switch (header.udhType) {
        case SMS_UDH_CONCAT_8BIT:
        case SMS_UDH_CONCAT_16BIT:
            index += EncodeHeaderConcat(header, pHeader);
            break;
        case SMS_UDH_APP_PORT_8BIT:
            /* IEI */
            pHeader[index++] = SMS_UDH_APP_PORT_8BIT;
            /* IEDL */
            pHeader[index++] = 0x02;
            /* Dest Port */
            pHeader[index++] = header.udh.appPort8bit.destPort;
            /* Origin Port */
            pHeader[index++] = header.udh.appPort8bit.originPort;
            break;
        case SMS_UDH_APP_PORT_16BIT:
            /* IEI */
            pHeader[index++] = SMS_UDH_APP_PORT_16BIT;
            /* IEDL */
            pHeader[index++] = 0x04;
            /* Dest Port */
            pHeader[index++] = (char)(header.udh.appPort16bit.destPort >> BYTE_BIT);
            pHeader[index++] = header.udh.appPort16bit.destPort & 0x00FF;
            /* Origin Port */
            pHeader[index++] = (char)(header.udh.appPort16bit.originPort >> BYTE_BIT);
            pHeader[index++] = header.udh.appPort16bit.originPort & 0x00FF;
            break;
        case SMS_UDH_ALTERNATE_REPLY_ADDRESS:
            /* IEI */
            pHeader[index++] = SMS_UDH_ALTERNATE_REPLY_ADDRESS;
            addrLen = GsmSmsParamCodec::EncodeAddress(&(header.udh.alternateAddress), &encodedAddr);
            /* IEDL */
            pHeader[index++] = addrLen;
            /* Alternate Reply Address */
            if (memcpy_s(&pHeader[index], addrLen, encodedAddr, addrLen) != EOK) {
                TELEPHONY_LOGE("EncodeHeader memcpy_s error");
                return index;
            }
            index += addrLen;
            break;
        case SMS_UDH_SINGLE_SHIFT:
            /* IEI */
            pHeader[index++] = SMS_UDH_SINGLE_SHIFT;
            /* IEDL */
            pHeader[index++] = 0x01;
            /* National Language Identifier */
            pHeader[index++] = header.udh.singleShift.langId;
            break;
        case SMS_UDH_LOCKING_SHIFT:
            /* IEI */
            pHeader[index++] = SMS_UDH_LOCKING_SHIFT;
            /* IEDL */
            pHeader[index++] = 0x01;
            /* National Language Identifier */
            pHeader[index++] = header.udh.lockingShift.langId;
            break;
        case SMS_UDH_NONE:
        default:
            break;
    }
    return index;
}

int GsmSmsUDataCodec::EncodeHeaderConcat(const struct SmsUDH header, char *pHeader)
{
    int index = 0;
    switch (header.udhType) {
        case SMS_UDH_CONCAT_8BIT:
            /* IEI */
            pHeader[index++] = SMS_UDH_CONCAT_8BIT;
            /* IEDL */
            pHeader[index++] = 0x03;
            /* Reference Number */
            pHeader[index++] = header.udh.concat8bit.msgRef;
            /* Number of Segments */
            pHeader[index++] = header.udh.concat8bit.totalSeg;
            /* Sequence Number */
            pHeader[index++] = header.udh.concat8bit.seqNum;
            break;
        case SMS_UDH_CONCAT_16BIT:
            /* IEI */
            pHeader[index++] = SMS_UDH_CONCAT_16BIT;
            /* IEDL */
            pHeader[index++] = 0x04;
            /* Reference Number */
            pHeader[index++] = (char)(header.udh.concat16bit.msgRef >> BYTE_BIT);
            pHeader[index++] = header.udh.concat16bit.msgRef & 0x00FF;
            /* Number of Segments */
            pHeader[index++] = header.udh.concat16bit.totalSeg;
            /* Sequence Number */
            pHeader[index++] = header.udh.concat16bit.seqNum;
            break;
        case SMS_UDH_NONE:
        default:
            break;
    }
    return index;
}

int GsmSmsUDataCodec::DecodeHeader(const unsigned char *pTpdu, struct SmsUDH *pHeader)
{
    int offset = 0;
    unsigned char IEDL = 0;
    pHeader->udhType = pTpdu[offset++];
    switch (pHeader->udhType) {
        case SMS_UDH_CONCAT_8BIT: {
            IEDL = pTpdu[offset++];
            if (IEDL == 0) {
                return 0;
            }
            pHeader->udh.concat8bit.msgRef = pTpdu[offset++];
            pHeader->udh.concat8bit.totalSeg = pTpdu[offset++];
            pHeader->udh.concat8bit.seqNum = pTpdu[offset++];
            DebugDecodeHeader(pHeader);
            break;
        }
        case SMS_UDH_CONCAT_16BIT: {
            IEDL = pTpdu[offset++];
            if (IEDL == 0) {
                return 0;
            }
            pHeader->udh.concat16bit.msgRef = pTpdu[offset++];
            pHeader->udh.concat16bit.msgRef =
                (unsigned short)((pHeader->udh.concat16bit.msgRef << BYTE_BIT) | pTpdu[offset++]);
            pHeader->udh.concat16bit.totalSeg = pTpdu[offset++];
            pHeader->udh.concat16bit.seqNum = pTpdu[offset++];
            DebugDecodeHeader(pHeader);
            break;
        }
        case SMS_UDH_APP_PORT_8BIT: {
            IEDL = pTpdu[offset++];
            if (IEDL == 0) {
                return 0;
            }
            pHeader->udh.appPort8bit.destPort = pTpdu[offset++];
            pHeader->udh.appPort8bit.originPort = pTpdu[offset++];
            DebugDecodeHeader(pHeader);
            break;
        }
        case SMS_UDH_APP_PORT_16BIT: {
            IEDL = pTpdu[offset++];
            if (IEDL == 0) {
                return 0;
            }
            pHeader->udh.appPort16bit.destPort = pTpdu[offset++];
            pHeader->udh.appPort16bit.destPort =
                (unsigned short)((pHeader->udh.appPort16bit.destPort << BYTE_BIT) | pTpdu[offset++]);
            pHeader->udh.appPort16bit.originPort = pTpdu[offset++];
            pHeader->udh.appPort16bit.originPort =
                (unsigned short)((pHeader->udh.appPort16bit.originPort << BYTE_BIT) | pTpdu[offset++]);
            DebugDecodeHeader(pHeader);
            break;
        }
        case SMS_UDH_SPECIAL_SMS: {
            IEDL = pTpdu[offset++];
            if (IEDL != 0x02) {
                return 0;
            }
            TELEPHONY_LOGI("Decoding special sms udh.");
            pHeader->udh.specialInd.bStore = (bool)(pTpdu[offset] & 0x80);
            pHeader->udh.specialInd.msgInd = (unsigned short)(pTpdu[offset++] & 0x7F);
            pHeader->udh.specialInd.waitMsgNum = (unsigned short)pTpdu[offset];
            break;
        }
        case SMS_UDH_ALTERNATE_REPLY_ADDRESS: {
            IEDL = pTpdu[offset++];
            if (IEDL == 0) {
                return 0;
            }
            offset += GsmSmsParamCodec::DecodeAddress(&pTpdu[offset], &(pHeader->udh.alternateAddress));
            TELEPHONY_LOGI("alternate reply address [%{public}s]", pHeader->udh.alternateAddress.address);
            break;
        }
        case SMS_UDH_SINGLE_SHIFT: {
            IEDL = pTpdu[offset++];
            if (IEDL == 0) {
                return 0;
            }
            pHeader->udh.singleShift.langId = pTpdu[offset++];
            TELEPHONY_LOGI("singleShift.langId [%{public}02x]", pHeader->udh.singleShift.langId);
            break;
        }
        case SMS_UDH_LOCKING_SHIFT: {
            IEDL = pTpdu[offset++];
            if (IEDL == 0) {
                return 0;
            }
            pHeader->udh.lockingShift.langId = pTpdu[offset++];
            TELEPHONY_LOGI("lockingShift.langId [%{public}02x]", pHeader->udh.lockingShift.langId);
            break;
        }
        default: {
            TELEPHONY_LOGI("Not Supported Header Type [%{public}02x]", pHeader->udhType);
            IEDL = pTpdu[offset++];
            TELEPHONY_LOGI("IEDL [%{public}u]", IEDL);
            return (offset + IEDL);
            break;
        }
    }
    return offset;
}

void GsmSmsUDataCodec::DebugDecodeHeader(const struct SmsUDH *pHeader)
{
    if (pHeader == nullptr) {
        return;
    }
    switch (pHeader->udhType) {
        case SMS_UDH_CONCAT_8BIT: {
            TELEPHONY_LOGI("concat8bit.seqNum [%{public}02x]", pHeader->udh.concat8bit.seqNum);
            break;
        }
        case SMS_UDH_CONCAT_16BIT: {
            TELEPHONY_LOGI("concat16bit.seqNum [%{public}02x]", pHeader->udh.concat16bit.seqNum);
            break;
        }
        case SMS_UDH_APP_PORT_8BIT: {
            TELEPHONY_LOGI("appPort8bit.destPort [%{public}02x]", pHeader->udh.appPort8bit.destPort);
            TELEPHONY_LOGI("appPort8bit.originPort [%{public}02x]", pHeader->udh.appPort8bit.originPort);
            break;
        }
        case SMS_UDH_APP_PORT_16BIT: {
            TELEPHONY_LOGI("appPort16bit.destPort [%{public}04x]", pHeader->udh.appPort16bit.destPort);
            TELEPHONY_LOGI("appPort16bit.originPort [%{public}04x]", pHeader->udh.appPort16bit.originPort);
            break;
        }
        case SMS_UDH_SPECIAL_SMS: {
            TELEPHONY_LOGI("Decoding special sms udh.");
            break;
        }
        case SMS_UDH_ALTERNATE_REPLY_ADDRESS:
            break;
        case SMS_UDH_SINGLE_SHIFT: {
            TELEPHONY_LOGI("singleShift.langId [%{public}02x]", pHeader->udh.singleShift.langId);
            break;
        }
        case SMS_UDH_LOCKING_SHIFT: {
            TELEPHONY_LOGI("lockingShift.langId [%{public}02x]", pHeader->udh.lockingShift.langId);
            break;
        }
        default:
            break;
    }
}
} // namespace Telephony
} // namespace OHOS