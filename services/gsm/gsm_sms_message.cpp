/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "gsm_sms_message.h"
#include <cmath>
#include <string>
#include <vector>
#include "securec.h"
#include "sms_hilog_wrapper.h"
#include "string_utils.h"
namespace OHOS {
namespace SMS {
using namespace std;
template<typename T>
inline void UniquePtrDeleterOneDimension(T **(&ptr))
{
    if (ptr && *ptr) {
        delete[] *ptr;
        *ptr = nullptr;
    }
}

int GetSegmentSize(
    SMS_CODING_SCHEME_E codingScheme, int dataLen, bool bPortNum, MSG_LANGUAGE_ID_T langId, int replyAddrLen)
{
    int headerLen = 1;
    int concat = 5;
    int port = 6;
    int lang = 3;
    int reply = 2;
    int headerSize = 0;
    int segSize = 0;
    int maxSize = 0;
    if (codingScheme == SMS_CODING_7BIT) {
        maxSize = MAX_GSM_7BIT_DATA_LEN;
    } else if (codingScheme == SMS_CODING_8BIT || codingScheme == SMS_CODING_UCS2) {
        maxSize = MAX_UCS2_DATA_LEN;
    }

    if (bPortNum == true) {
        headerSize += port;
    }

    if (langId != MSG_ID_RESERVED_LANG) {
        headerSize += lang;
    }

    if (replyAddrLen > 0) {
        headerSize += reply;
        headerSize += replyAddrLen;
    }

    if (codingScheme == SMS_CODING_7BIT) {
        if ((dataLen + headerSize) > maxSize) {
            segSize = ((GSM_BEAR_DATA_LEN * BYTE_BITS) - ((headerLen + concat + headerSize) * BYTE_BITS)) /
                CHARSET_7BIT_BITS;
        } else {
            segSize = dataLen;
        }
    } else if (codingScheme == SMS_CODING_8BIT || codingScheme == SMS_CODING_UCS2) {
        if ((dataLen + headerSize) > maxSize) {
            segSize = GSM_BEAR_DATA_LEN - (headerLen + concat + headerSize);
        } else {
            segSize = dataLen;
        }
    }

    return segSize;
}

int GsmSmsMessage::CalcReplayEncodeAddress(const std::string &replyAddress)
{
    int ret = 0;
    int addrLen = 0;
    char *encodedAddr = nullptr;
    unique_ptr<char *, void (*)(char **(&))> addressBuf(&encodedAddr, UniquePtrDeleterOneDimension);
    if (replyAddress.length() > 0) {
        struct SmsAddress replyAddr = {};
        replyAddr.ton = SMS_TON_NATIONAL;
        replyAddr.npi = SMS_NPI_ISDN;
        ret = memset_s(replyAddr.address, sizeof(replyAddr.address), 0x00, MAX_ADDRESS_LEN + 1);
        if (ret != EOK) {
            HILOG_ERROR("CalcReplayEncodeAddress memset_s error!");
            return addrLen;
        }
        ret = memcpy_s(replyAddr.address, sizeof(replyAddr.address), replyAddress.c_str(), MAX_ADDRESS_LEN);
        if (ret != EOK) {
            HILOG_ERROR("CalcReplayEncodeAddress memory_s error!");
            return addrLen;
        }
        addrLen = GsmSmsParamCodec::EncodeAddress(&replyAddr, &encodedAddr);
        if (encodedAddr != nullptr) {
            delete encodedAddr;
        }
    }
    return addrLen;
}

int GsmSmsMessage::SetSmsTpduDesAddress(std::shared_ptr<struct SmsTpdu> &tPdu, const std::string &desAddr)
{
    int ret = 0;
    int addLen = 0;
    if (tPdu == nullptr) {
        return addLen;
    }
    addLen = desAddr.length();
    tPdu->data.submit.destAddress.ton = SMS_TON_UNKNOWN;
    tPdu->data.submit.destAddress.npi = SMS_NPI_ISDN;
    if (addLen < MAX_ADDRESS_LEN) {
        ret = memcpy_s(tPdu->data.submit.destAddress.address, sizeof(tPdu->data.submit.destAddress.address),
            desAddr.c_str(), addLen);
        if (ret != EOK) {
            HILOG_ERROR("SetSmsTpduDesAddress memcpy_s error!");
            return addLen;
        }
        tPdu->data.submit.destAddress.address[addLen] = '\0';
    } else {
        if (desAddr[0] == '+') {
            ret = memcpy_s(tPdu->data.submit.destAddress.address, sizeof(tPdu->data.submit.destAddress.address),
                desAddr.c_str(), MAX_ADDRESS_LEN);
        } else {
            ret = memcpy_s(tPdu->data.submit.destAddress.address, sizeof(tPdu->data.submit.destAddress.address),
                desAddr.c_str(), MAX_ADDRESS_LEN - 1);
        }
        if (ret != EOK) {
            HILOG_ERROR("SetSmsTpduDesAddress memcpy_s error!");
            return addLen;
        }
        tPdu->data.submit.destAddress.address[MAX_ADDRESS_LEN] = '\0';
    }
    return addLen;
}

int GsmSmsMessage::SetHeaderLang(int index, const SMS_CODING_SCHEME_E codingType, const MSG_LANGUAGE_ID_T langId)
{
    int ret = 0;
    if (smsTpdu_ == nullptr) {
        return ret;
    }
    switch (smsTpdu_->tpduType) {
        case SMS_TPDU_SUBMIT:
            if (codingType == SMS_CODING_7BIT && langId != MSG_ID_RESERVED_LANG) {
                smsTpdu_->data.submit.userData.header[index].udhType = SMS_UDH_SINGLE_SHIFT;
                smsTpdu_->data.submit.userData.header[index].udh.singleShift.langId = langId;
                ret++;
            }
            break;
        default:
            break;
    }
    return ret;
}

int GsmSmsMessage::SetHeaderConcat(int index, const SmsConcat &concat)
{
    int ret = 0;
    if (smsTpdu_ == nullptr) {
        return ret;
    }
    switch (smsTpdu_->tpduType) {
        case SMS_TPDU_SUBMIT:
            if (concat.is8Bits) {
                smsTpdu_->data.submit.userData.header[index].udhType = SMS_UDH_CONCAT_8BIT;
                smsTpdu_->data.submit.userData.header[index].udh.concat8bit.msgRef = concat.msgRef;
                smsTpdu_->data.submit.userData.header[index].udh.concat8bit.totalSeg = concat.totalSeg;
                smsTpdu_->data.submit.userData.header[index].udh.concat8bit.seqNum = concat.seqNum;
            } else {
                smsTpdu_->data.submit.userData.header[index].udhType = SMS_UDH_CONCAT_16BIT;
                smsTpdu_->data.submit.userData.header[index].udh.concat16bit.msgRef = concat.msgRef;
                smsTpdu_->data.submit.userData.header[index].udh.concat16bit.totalSeg = concat.totalSeg;
                smsTpdu_->data.submit.userData.header[index].udh.concat16bit.seqNum = concat.seqNum;
            }
            ret++;
            break;
        default:
            break;
    }
    return ret;
}

int GsmSmsMessage::SetHeaderReplay(int index)
{
    int ret = 0;
    std::string replay = GetReplyAddress();
    if (smsTpdu_ == nullptr && replay.length() <= 0) {
        return ret;
    }
    switch (smsTpdu_->tpduType) {
        case SMS_TPDU_SUBMIT: {
            smsTpdu_->data.submit.bReplyPath = true;
            smsTpdu_->data.submit.userData.header[index].udhType = SMS_UDH_ALTERNATE_REPLY_ADDRESS;
            smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.ton = SMS_TON_NATIONAL;
            smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.npi = SMS_NPI_ISDN;
            ret = memset_s(smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.address,
                sizeof(smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.address), 0x00,
                MAX_ADDRESS_LEN + 1);
            if (ret != EOK) {
                HILOG_ERROR("SetHeaderReplay memset_s error!");
                return ret;
            }
            ret = memcpy_s(smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.address,
                sizeof(smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.address), replay.c_str(),
                replay.length());
            if (ret != EOK) {
                HILOG_ERROR("SetHeaderReplay memcpy_s error!");
                return ret;
            }
        } break;
        default:
            break;
    }
    return ret;
}

int GsmSmsMessage::GetCodeLen(unsigned char (&decodeData)[(MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1],
    SMS_CODING_SCHEME_E &codingType, std::string &msgText, bool &bAbnormal, MSG_LANGUAGE_ID_T &langId)
{
    int dataLen = msgText.length();
    int encodeLen = 0;
    MsgTextConvert *textCvt = MsgTextConvert::Instance();
    if (textCvt == nullptr) {
        HILOG_ERROR("MsgTextConvert Instance is nullptr");
        return encodeLen;
    }
    switch (codingType) {
        case SMS_CODING_7BIT: {
            const unsigned char *pMsgText = (const unsigned char *)msgText.c_str();
            unsigned char *pDestText = decodeData;
            MSG_LANGUAGE_ID_T *pLangId = &langId;
            bool *pIncludeAbnormalChar = &bAbnormal;
            encodeLen = textCvt->ConvertUTF8ToGSM7bit(pDestText, (MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1,
                pMsgText, (int)dataLen, pLangId, pIncludeAbnormalChar);
            break;
        }
        case SMS_CODING_8BIT:
            if (memcpy_s(decodeData, sizeof(decodeData), msgText.c_str(), dataLen) != EOK) {
                HILOG_ERROR("SplitMessage SMS_CHARSET_8BIT memcpy_s error!");
                return encodeLen;
            }
            encodeLen = dataLen;
            break;
        case SMS_CODING_UCS2:
            encodeLen = textCvt->ConvertUTF8ToUCS2(decodeData, (MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1,
                (unsigned char *)msgText.c_str(), (int)dataLen);
            break;
        case SMS_CODING_AUTO:
        default:
            SMS_CODING_SCHEME_E encodeType = SMS_CODING_AUTO;
            encodeLen = textCvt->ConvertUTF8ToAuto(decodeData, (MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1,
                (unsigned char *)msgText.c_str(), (int)dataLen, &encodeType);
            codingType = encodeType;
            break;
    }
    return encodeLen;
}

void GsmSmsMessage::SplitMessage(std::vector<struct SpiltInfo> &splitResult, const std::string &context,
    bool user7bitEncode, SMS_CODING_SCHEME_E &codingType)
{
    int segCount = 0;
    int encodeLen = 0;
    std::string replyAddress;
    std::string msgText = context;
    if (msgText.empty()) {
        return;
    }

    MSG_LANGUAGE_ID_T langId = MSG_ID_RESERVED_LANG;
    codingType = user7bitEncode ? SMS_CODING_7BIT : SMS_CODING_AUTO;

    bool bAbnormal = false;
    const int bufSize = (MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1;
    unsigned char decodeData[bufSize];
    if (memset_s(decodeData, sizeof(decodeData), 0x00, sizeof(decodeData)) != EOK) {
        HILOG_ERROR("SplitMessage memset_s error!");
        return;
    }
    encodeLen = GetCodeLen(decodeData, codingType, msgText, bAbnormal, langId);
    HILOG_INFO("encodeLen = %{public}d\r\n", encodeLen);
    if (encodeLen <= 0) {
        return;
    }
    int segSize = 0;
    int index = 0;
    segSize = GetSegmentSize(codingType, encodeLen, false, langId, MAX_ADD_PARAM_LEN);
    if (segSize > 0) {
        segCount = ceil((double)encodeLen / (double)segSize);
    }
    for (int i = 0; i < segCount; i++) {
        int userDataLen = 0;
        struct SpiltInfo spiltInfo;
        spiltInfo.langId = langId;
        spiltInfo.encodeType = codingType;
        uint8_t textData[TAPI_TEXT_SIZE_MAX + 1];
        (void)memset_s(textData, sizeof(textData), 0x00, sizeof(textData));
        if ((i + 1) == segCount) {
            userDataLen = encodeLen - (i * segSize);
        } else {
            userDataLen = segSize;
        }
        spiltInfo.encodeData = std::vector<uint8_t>(&decodeData[index], &decodeData[index] + userDataLen);
        splitResult.push_back(spiltInfo);
        index += segSize;
    }
}

void GsmSmsMessage::CreateDefaultSubmit(bool bStatusReport, const SMS_CODING_SCHEME_E codingScheme)
{
    smsTpdu_ = std::make_shared<struct SmsTpdu>();
    smsTpdu_->tpduType = SMS_TPDU_SUBMIT;
    smsTpdu_->data.submit.bHeaderInd = false;
    smsTpdu_->data.submit.bRejectDup = false;
    smsTpdu_->data.submit.bStatusReport = bStatusReport;
    smsTpdu_->data.submit.bReplyPath = false;
    smsTpdu_->data.submit.msgRef = 0;
    smsTpdu_->data.submit.dcs.bCompressed = false;
    smsTpdu_->data.submit.dcs.msgClass = SMS_MSG_CLASS_E::SMS_CLASS_UNKNOWN;
    smsTpdu_->data.submit.dcs.codingGroup = SMS_GENERAL_GROUP;
    smsTpdu_->data.submit.dcs.codingScheme = codingScheme;
    smsTpdu_->data.submit.pid = SMS_NORMAL_PID;
    smsTpdu_->data.submit.vpf = SMS_VPF_NOT_PRESENT;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::CreateDefaultSubmitSmsTpdu(const std::string &dest,
    const std::string &sc, const std::string &text, bool bStatusReport,
    const SMS_CODING_SCHEME_E codingScheme = SMS_CODING_7BIT)
{
    SetFullText(text);
    SetScAddress(sc);
    SetDestAddress(dest);
    CreateDefaultSubmit(bStatusReport, codingScheme);
    SetSmsTpduDesAddress(smsTpdu_, dest);
    return smsTpdu_;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::CreateDataSubmitSmsTpdu(const std::string desAddr,
    const std::string scAddr, int32_t port, const uint8_t *data, uint32_t dataLen, uint16_t msgRef8bit,
    bool bStatusReport)
{
    SetScAddress(scAddr);
    SetDestAddress(desAddr);
    CreateDefaultSubmit(bStatusReport, SMS_CODING_7BIT);
    SetSmsTpduDesAddress(smsTpdu_, desAddr);
    int endcodeLen = 0;
    bool bAbnormal = false;
    MsgTextConvert *textCvt = MsgTextConvert::Instance();
    if (textCvt == nullptr) {
        return nullptr;
    }
    MSG_LANGUAGE_ID_T langId = MSG_ID_RESERVED_LANG;
    SMS_CODING_SCHEME_E pCodingType = SMS_CODING_7BIT;
    const int bufSize = (MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1;
    unsigned char encodeData[bufSize];
    if (memset_s(encodeData, sizeof(encodeData), 0x00, sizeof(encodeData)) != EOK) {
        return nullptr;
    }
    const unsigned char *pMsgText = (const unsigned char *)data;
    unsigned char *pDestText = encodeData;
    MSG_LANGUAGE_ID_T *pLangId = &langId;
    bool *pIncludeAbnormalChar = &bAbnormal;
    endcodeLen =
        textCvt->ConvertUTF8ToGSM7bit(pDestText, bufSize, pMsgText, (int)dataLen, pLangId, pIncludeAbnormalChar);
    int segSize = 0;
    segSize = GetSegmentSize(pCodingType, endcodeLen, false, langId, MAX_ADD_PARAM_LEN);
    if (segSize != 1 || smsTpdu_ == nullptr) {
        return nullptr;
    }
    int headerCnt = 0;
    if (memset_s(smsTpdu_->data.submit.userData.data, sizeof(smsTpdu_->data.submit.userData.data), 0x00,
        sizeof(smsTpdu_->data.submit.userData.data)) != EOK) {
        return nullptr;
    }
    if (memcpy_s(smsTpdu_->data.submit.userData.data, sizeof(smsTpdu_->data.submit.userData.data), encodeData,
        endcodeLen) != EOK) {
        return nullptr;
    }
    smsTpdu_->data.submit.userData.data[endcodeLen] = 0;
    smsTpdu_->data.submit.msgRef = msgRef8bit;
    /* Set User Data Header Port Information */
    smsTpdu_->data.submit.userData.header[headerCnt].udhType = SMS_UDH_APP_PORT_16BIT;
    smsTpdu_->data.submit.userData.header[headerCnt].udh.appPort16bit.destPort = ((unsigned short)port & 0xFFFF);
    smsTpdu_->data.submit.userData.header[headerCnt].udh.appPort16bit.originPort = 0;
    headerCnt++;
    /* Set User Data Header for Alternate Reply Address */
    headerCnt += SetHeaderReplay(headerCnt);
    /* Set User Data Header for National Language Single Shift */
    headerCnt += SetHeaderLang(headerCnt, pCodingType, langId);
    smsTpdu_->data.submit.userData.headerCnt = headerCnt;
    return smsTpdu_;
}

std::shared_ptr<struct EncodeInfo> GsmSmsMessage::GetSubmitEncodeInfo(const std::string &sc, bool bMore)
{
    int ret = 0;
    int encodeSmscLen = 0;
    char tpduBuf[MAX_TPDU_DATA_LEN];
    unsigned char encodeSmscAddr[MAX_SMSC_LEN];
    (void)memset_s(encodeSmscAddr, sizeof(encodeSmscAddr), 0x00, sizeof(encodeSmscAddr));
    (void)memset_s(tpduBuf, sizeof(tpduBuf), 0x00, sizeof(tpduBuf));
    if ((!sc.empty()) && (sc.length() < MAX_SMSC_LEN)) {
        struct SmsAddress pAddress;
        ret = memset_s(&pAddress.address, sizeof(pAddress.address), 0x00, sizeof(pAddress.address));
        if (ret != EOK) {
            HILOG_ERROR("GetSubmitEncodeInfo memset_s error!");
            return nullptr;
        }
        ret = memcpy_s(&pAddress.address, sizeof(pAddress.address), sc.data(), sc.length());
        if (ret != EOK) {
            HILOG_ERROR("GetSubmitEncodeInfo memcpy_s error!");
            return nullptr;
        }
        pAddress.address[sc.length()] = '\0';
        if (sc[0] == '+') {
            pAddress.ton = SMS_TON_INTERNATIONAL;
        } else {
            pAddress.ton = SMS_TON_NATIONAL;
        }
        pAddress.npi = SMS_NPI_ISDN; /* app cannot set this value */
        encodeSmscLen = GsmSmsParamCodec::EncodeSMSC(&pAddress, encodeSmscAddr, sizeof(encodeSmscAddr));
    }
    std::shared_ptr<struct EncodeInfo> info = std::make_shared<struct EncodeInfo>();
    int bufLen = GsmSmsTpduCodec::EncodeTpdu(smsTpdu_.get(), tpduBuf, sizeof(tpduBuf));
    if (bufLen > 0 && info != nullptr) {
        ret = memcpy_s(info->smcaData_, sizeof(info->smcaData_), encodeSmscAddr, encodeSmscLen);
        if (ret != EOK) {
            HILOG_ERROR("GetSubmitEncodeInfo encodeSmscAddr memcpy_s error!");
            return nullptr;
        }
        ret = memcpy_s(info->tpduData_, sizeof(info->tpduData_), tpduBuf, bufLen);
        if (ret != EOK) {
            HILOG_ERROR("GetSubmitEncodeInfo memcpy_s error!");
            return nullptr;
        }
        info->smcaLen = encodeSmscLen;
        info->tpduLen = bufLen;
        info->isMore_ = bMore;
    }
    return info;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::GreateDeliverSmsTpdu()
{
    smsTpdu_ = std::make_shared<struct SmsTpdu>();
    smsTpdu_->tpduType = SMS_TPDU_DELIVER;
    smsTpdu_->data.deliver.bHeaderInd = false;
    return smsTpdu_;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::GreateDeliverReportSmsTpdu()
{
    smsTpdu_ = std::make_shared<struct SmsTpdu>();
    smsTpdu_->tpduType = SMS_TPDU_DELIVER_REP;
    smsTpdu_->data.deliverRep.bHeaderInd = false;
    smsTpdu_->data.deliverRep.paramInd = 0x00;
    return smsTpdu_;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::GreateStatusReportSmsTpdu()
{
    smsTpdu_ = std::make_shared<struct SmsTpdu>();
    smsTpdu_->tpduType = SMS_TPDU_STATUS_REP;
    return smsTpdu_;
}

std::shared_ptr<GsmSmsMessage> GsmSmsMessage::CreateMessage(const std::string &pdu)
{
    std::shared_ptr<GsmSmsMessage> message = std::make_shared<GsmSmsMessage>();
    message->smsTpdu_ = std::make_shared<struct SmsTpdu>();
    (void)memset_s(message->smsTpdu_.get(), sizeof(struct SmsTpdu), 0x00, sizeof(struct SmsTpdu));
    std::string pduData = StringUtils::HexToString(pdu);
    message->rawPdu_ = StringUtils::HexToByteVector(pdu);
    if (message->PduAnalysis(pduData)) {
        return message;
    }
    return nullptr;
}

bool GsmSmsMessage::PduAnalysis(const string &pdu)
{
    bool result = true;
    if (smsTpdu_ == nullptr || pdu.empty()) {
        HILOG_ERROR("GsmSmsMessage::PduAnalysis smsTpdu is null");
        return false;
    }
    struct SmsAddress smsc;
    if (memset_s(&smsc, sizeof(struct SmsAddress), 0x00, sizeof(struct SmsAddress)) != EOK) {
        HILOG_ERROR("PduAnalysis memset_s error!");
        return false;
    }
    int smscLen = GsmSmsParamCodec::DecodeSMSC((const unsigned char *)pdu.c_str(), smsc);
    if (smscLen > 0) {
        scAddress_ = smsc.address;
    }
    int decodeLen = GsmSmsTpduCodec::DecodeTpdu(
        (const unsigned char *)pdu.c_str() + smscLen, pdu.length() - smscLen, smsTpdu_.get());
    if (decodeLen <= 0) {
        return false;
    }
    switch (smsTpdu_->tpduType) {
        case SMS_TPDU_DELIVER:
            protocolId_ = (int)(smsTpdu_->data.deliver.pid);
            hasReplayPath_ = smsTpdu_->data.deliver.bReplyPath;
            bMoreMsg_ = smsTpdu_->data.deliver.bMoreMsg;

            bStatusRoretMessage_ = smsTpdu_->data.deliver.bStatusReport;
            bMoreMsg_ = smsTpdu_->data.deliver.bMoreMsg;
            bHeaderInd_ = smsTpdu_->data.deliver.bHeaderInd;
            originatingAddress_ = smsTpdu_->data.deliver.originAddress.address;
            headerCnt_ = smsTpdu_->data.deliver.userData.headerCnt;
            scTimestamp_ = GsmSmsParamCodec::ConvertTime(&(smsTpdu_->data.deliver.timeStamp));
            ConvertMessageDcs();
            ConvertUserData();
            break;
        case SMS_TPDU_STATUS_REP:
            protocolId_ = (int)(smsTpdu_->data.statusRep.pid);
            msgRef_ = smsTpdu_->data.statusRep.msgRef;
            bMoreMsg_ = smsTpdu_->data.statusRep.bMoreMsg;
            bStatusRoretMessage_ = smsTpdu_->data.statusRep.bStatusReport;
            bHeaderInd_ = smsTpdu_->data.statusRep.bHeaderInd;
            status_ = smsTpdu_->data.statusRep.status;
            scTimestamp_ = GsmSmsParamCodec::ConvertTime(&(smsTpdu_->data.statusRep.timeStamp));
            ConvertMessageDcs();
            ConvertUserData();
            break;
        default:
            result = false;
            break;
    }
    return result;
}

void GsmSmsMessage::ConvertMessageDcs()
{
    if (smsTpdu_ == nullptr) {
        HILOG_ERROR("GsmSmsMessage::ConvertMessageDcs smsTpdu is null");
        return;
    }
    switch (smsTpdu_->tpduType) {
        case SMS_TPDU_DELIVER:
            bCompressed_ = smsTpdu_->data.deliver.dcs.bCompressed;
            codingScheme_ = smsTpdu_->data.deliver.dcs.codingScheme;
            codingGroup_ = smsTpdu_->data.deliver.dcs.codingGroup;
            bIndActive_ = smsTpdu_->data.deliver.dcs.bIndActive;
            bMwi_ = smsTpdu_->data.deliver.dcs.bMWI;
            bMwiSense_ = smsTpdu_->data.deliver.dcs.bIndActive; /* Indicates vmail notification set/clear */
            ConvertMessageClass(smsTpdu_->data.deliver.dcs.msgClass);
            break;
        case SMS_TPDU_STATUS_REP:
            bCompressed_ = smsTpdu_->data.statusRep.dcs.bCompressed;
            codingScheme_ = smsTpdu_->data.statusRep.dcs.codingScheme;
            codingGroup_ = smsTpdu_->data.statusRep.dcs.codingGroup;
            bIndActive_ = smsTpdu_->data.statusRep.dcs.bIndActive;
            ConvertMessageClass(smsTpdu_->data.statusRep.dcs.msgClass);
            break;
        default:
            break;
    }
}

void GsmSmsMessage::ConvertUserData()
{
    int ret = 0;
    int dataSize = 0;
    if (smsTpdu_ == nullptr) {
        return;
    }
    ret = memset_s(&smsUserData_, sizeof(struct SmsUserData), 0x00, sizeof(struct SmsUserData));
    if (ret != EOK) {
        return;
    }
    switch (smsTpdu_->tpduType) {
        case SMS_TPDU_DELIVER:
            headerDataLen_ = smsTpdu_->data.deliver.userData.length;
            ret = memcpy_s(&smsUserData_, sizeof(struct SmsUserData), &(smsTpdu_->data.deliver.userData),
                sizeof(struct SmsUserData));
            break;
        case SMS_TPDU_STATUS_REP:
            headerDataLen_ = smsTpdu_->data.statusRep.userData.length;
            ret = memcpy_s(&smsUserData_, sizeof(struct SmsUserData), &(smsTpdu_->data.statusRep.userData),
                sizeof(struct SmsUserData));
            break;
        default:
            break;
    }
    if (ret != EOK) {
        return;
    }
    if (smsUserData_.length > 0) {
        MsgTextConvert *textCvt = MsgTextConvert::Instance();
        if (textCvt == nullptr) {
            return;
        }
        unsigned char buff[MAX_MSG_TEXT_LEN + 1] = {0};
        if (codingScheme_ == SMS_CODING_7BIT) {
            MSG_LANG_INFO_S langInfo = {
                0,
            };
            langInfo.bSingleShift = false;
            langInfo.bLockingShift = false;
            dataSize = textCvt->ConvertGSM7bitToUTF8(
                buff, MAX_MSG_TEXT_LEN, (unsigned char *)smsUserData_.data, smsUserData_.length, &langInfo);
        } else if (codingScheme_ == SMS_CODING_UCS2) {
            dataSize = textCvt->ConvertUCS2ToUTF8(
                buff, MAX_MSG_TEXT_LEN, (unsigned char *)smsUserData_.data, smsUserData_.length);
        }
        visibleMessageBody_.insert(0, (char *)buff, dataSize);
    }
}

void GsmSmsMessage::SetFullText(const std::string &text)
{
    fullText_ = text;
}

void GsmSmsMessage::SetDestAddress(const std::string &address)
{
    destAddress_ = address;
}

void GsmSmsMessage::SetDestPort(uint32_t port)
{
    destPort_ = port;
}

std::string GsmSmsMessage::GetFullText() const
{
    return fullText_;
}

std::string GsmSmsMessage::GetReplyAddress() const
{
    return replyAddress_;
}

std::string GsmSmsMessage::GetDestAddress() const
{
    return destAddress_;
}

uint16_t GsmSmsMessage::GetDestPort()
{
    std::shared_ptr<SmsAppPortAddr> portAddress = GetPortAddress();
    if (portAddress == nullptr) {
        return defaultPort_;
    }
    destPort_ = static_cast<uint16_t>(portAddress->destPort);
    return destPort_;
}

bool GsmSmsMessage::GetIsSmsText() const
{
    return bSmsText_;
}

bool GsmSmsMessage::GetGsm() const
{
    return true;
}

bool GsmSmsMessage::GetIsTypeZeroInd() const
{
    return (GetProtocolId() == 0x40);
}

bool GsmSmsMessage::GetIsSIMDataTypeDownload() const
{
    int procotolId = GetProtocolId();
    return GetMessageClass() == SMS_SIM_MESSAGE && (procotolId == 0x7f || procotolId == 0x7c);
}

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.2 Special SMS Message Indication
bool GsmSmsMessage::IsSpecialMessage() const
{
    bool result = false;
    if (GetIsTypeZeroInd()) {
        HILOG_DEBUG("GsmSmsMessage:: IsTypeZeroInd");
        result = true;
    }
    // 9.2.3.9	TP Protocol Identifier (TP PID)
    if (GetIsSIMDataTypeDownload()) {
        HILOG_DEBUG("GsmSmsMessage:: GetIsSIMDataTypeDownload");
        result = true;
    }
    if (IsMwiSet() || IsMwiClear()) {
        HILOG_DEBUG("GsmSmsMessage::Mwi Message");
        result = true;
    }
    return result;
}
} // namespace SMS
} // namespace OHOS