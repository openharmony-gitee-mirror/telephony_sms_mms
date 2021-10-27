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

#include "sms_common_utils.h"
#include "string_utils.h"
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

int GetSegmentSize(
    SmsCodingScheme codingScheme, int dataLen, bool bPortNum, MSG_LANGUAGE_ID_T langId, int replyAddrLen)
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

    if (bPortNum) {
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

int GsmSmsMessage::CalcReplyEncodeAddress(const std::string &replyAddress)
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
            TELEPHONY_LOGE("CalcReplyEncodeAddress memset_s error!");
            return addrLen;
        }
        ret = memcpy_s(replyAddr.address, sizeof(replyAddr.address), replyAddress.c_str(), MAX_ADDRESS_LEN);
        if (ret != EOK) {
            TELEPHONY_LOGE("CalcReplyEncodeAddress memory_s error!");
            return addrLen;
        }
        addrLen = GsmSmsParamCodec::EncodeAddress(&replyAddr, &encodedAddr);
        if (encodedAddr != nullptr) {
            delete encodedAddr;
        }
    }
    return addrLen;
}

int GsmSmsMessage::SetSmsTpduDestAddress(std::shared_ptr<struct SmsTpdu> &tPdu, const std::string &desAddr)
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
            TELEPHONY_LOGE("SetSmsTpduDestAddress memcpy_s error!");
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
            TELEPHONY_LOGE("SetSmsTpduDestAddress memcpy_s error!");
            return addLen;
        }
        tPdu->data.submit.destAddress.address[MAX_ADDRESS_LEN] = '\0';
    }
    return addLen;
}

int GsmSmsMessage::SetHeaderLang(int index, const SmsCodingScheme codingType, const MSG_LANGUAGE_ID_T langId)
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

int GsmSmsMessage::SetHeaderReply(int index)
{
    int ret = 0;
    std::string reply = GetReplyAddress();
    if (smsTpdu_ == nullptr && reply.length() <= 0) {
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
                TELEPHONY_LOGE("SetHeaderReply memset_s error!");
                return ret;
            }
            ret = memcpy_s(smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.address,
                sizeof(smsTpdu_->data.submit.userData.header[index].udh.alternateAddress.address), reply.c_str(),
                reply.length());
            if (ret != EOK) {
                TELEPHONY_LOGE("SetHeaderReply memcpy_s error!");
                return ret;
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

int GsmSmsMessage::GetCodeLen(unsigned char (&decodeData)[(MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1],
    SmsCodingScheme &codingType, std::string &msgText, bool &bAbnormal, MSG_LANGUAGE_ID_T &langId)
{
    int dataLen = msgText.length();
    int encodeLen = 0;
    MsgTextConvert *textCvt = MsgTextConvert::Instance();
    if (textCvt == nullptr) {
        TELEPHONY_LOGE("MsgTextConvert Instance is nullptr");
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
                TELEPHONY_LOGE("SplitMessage SMS_CHARSET_8BIT memcpy_s error!");
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
            SmsCodingScheme encodeType = SMS_CODING_AUTO;
            encodeLen = textCvt->ConvertUTF8ToAuto(decodeData, (MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1,
                (unsigned char *)msgText.c_str(), (int)dataLen, &encodeType);
            codingType = encodeType;
            break;
    }
    return encodeLen;
}

void GsmSmsMessage::SplitMessage(std::vector<struct SplitInfo> &splitResult, const std::string &context,
    bool user7bitEncode, SmsCodingScheme &codingType)
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
        TELEPHONY_LOGE("SplitMessage memset_s error!");
        return;
    }
    encodeLen = GetCodeLen(decodeData, codingType, msgText, bAbnormal, langId);
    TELEPHONY_LOGI("encodeLen = %{public}d\r\n", encodeLen);
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
        struct SplitInfo splitInfo;
        splitInfo.langId = langId;
        splitInfo.encodeType = codingType;
        uint8_t textData[TAPI_TEXT_SIZE_MAX + 1];
        (void)memset_s(textData, sizeof(textData), 0x00, sizeof(textData));
        if ((i + 1) == segCount) {
            userDataLen = encodeLen - (i * segSize);
        } else {
            userDataLen = segSize;
        }
        splitInfo.encodeData = std::vector<uint8_t>(&decodeData[index], &decodeData[index] + userDataLen);
        splitResult.push_back(splitInfo);
        index += segSize;
    }
}

void GsmSmsMessage::CreateDefaultSubmit(bool bStatusReport, const SmsCodingScheme codingScheme)
{
    smsTpdu_ = std::make_shared<struct SmsTpdu>();
    smsTpdu_->tpduType = SMS_TPDU_SUBMIT;
    smsTpdu_->data.submit.bHeaderInd = false;
    smsTpdu_->data.submit.bRejectDup = false;
    smsTpdu_->data.submit.bStatusReport = bStatusReport;
    smsTpdu_->data.submit.bReplyPath = false;
    smsTpdu_->data.submit.msgRef = 0;
    smsTpdu_->data.submit.dcs.bCompressed = false;
    smsTpdu_->data.submit.dcs.msgClass = SmsMessageClass::SMS_CLASS_UNKNOWN;
    smsTpdu_->data.submit.dcs.codingGroup = SMS_GENERAL_GROUP;
    smsTpdu_->data.submit.dcs.codingScheme = codingScheme;
    smsTpdu_->data.submit.pid = SMS_NORMAL_PID;
    smsTpdu_->data.submit.vpf = SMS_VPF_NOT_PRESENT;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::CreateDefaultSubmitSmsTpdu(const std::string &dest,
    const std::string &sc, const std::string &text, bool bStatusReport,
    const SmsCodingScheme codingScheme = SMS_CODING_7BIT)
{
    SetFullText(text);
    SetSmscAddr(sc);
    SetDestAddress(dest);
    CreateDefaultSubmit(bStatusReport, codingScheme);
    SetSmsTpduDestAddress(smsTpdu_, dest);
    return smsTpdu_;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::CreateDataSubmitSmsTpdu(const std::string desAddr,
    const std::string scAddr, int32_t port, const uint8_t *data, uint32_t dataLen, uint16_t msgRef8bit,
    bool bStatusReport)
{
    int headerCnt = 0;
    SetSmscAddr(scAddr);
    SetDestAddress(desAddr);
    CreateDefaultSubmit(bStatusReport, SMS_CODING_7BIT);
    SetSmsTpduDestAddress(smsTpdu_, desAddr);
    if (smsTpdu_ == nullptr) {
        TELEPHONY_LOGE("smsTpdu_ is nullptr!");
        return nullptr;
    }
    if (memset_s(smsTpdu_->data.submit.userData.data, sizeof(smsTpdu_->data.submit.userData.data), 0x00,
        sizeof(smsTpdu_->data.submit.userData.data)) != EOK) {
        TELEPHONY_LOGE("memset_s return EFAIL!");
        return nullptr;
    }
    if (dataLen > sizeof(smsTpdu_->data.submit.userData.data)) {
        if (memcpy_s(smsTpdu_->data.submit.userData.data, sizeof(smsTpdu_->data.submit.userData.data), data,
            sizeof(smsTpdu_->data.submit.userData.data)) != EOK) {
            TELEPHONY_LOGE("memcpy_s return EFAIL!");
            return nullptr;
        }
    } else {
        if (memcpy_s(smsTpdu_->data.submit.userData.data, sizeof(smsTpdu_->data.submit.userData.data), data,
            dataLen) != EOK) {
            TELEPHONY_LOGE("memcpy_s return EFAIL!");
            return nullptr;
        }
    }
    MSG_LANGUAGE_ID_T langId = MSG_ID_RESERVED_LANG;
    SmsCodingScheme pCodingType = SMS_CODING_8BIT;
    if (dataLen < sizeof(smsTpdu_->data.submit.userData.data)) {
        smsTpdu_->data.submit.userData.data[dataLen] = 0;
    }
    smsTpdu_->data.submit.msgRef = msgRef8bit;
    /* Set User Data Header Port Information */
    smsTpdu_->data.submit.userData.header[headerCnt].udhType = SMS_UDH_APP_PORT_16BIT;
    smsTpdu_->data.submit.userData.header[headerCnt].udh.appPort16bit.destPort = ((unsigned short)port & 0xFFFF);
    smsTpdu_->data.submit.userData.header[headerCnt].udh.appPort16bit.originPort = 0;
    headerCnt++;
    /* Set User Data Header for Alternate Reply Address */
    headerCnt += SetHeaderReply(headerCnt);
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
            TELEPHONY_LOGE("GetSubmitEncodeInfo memset_s error!");
            return nullptr;
        }
        ret = memcpy_s(&pAddress.address, sizeof(pAddress.address), sc.data(), sc.length());
        if (ret != EOK) {
            TELEPHONY_LOGE("GetSubmitEncodeInfo memcpy_s error!");
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
            TELEPHONY_LOGE("GetSubmitEncodeInfo encodeSmscAddr memcpy_s error!");
            return nullptr;
        }
        ret = memcpy_s(info->tpduData_, sizeof(info->tpduData_), tpduBuf, bufLen);
        if (ret != EOK) {
            TELEPHONY_LOGE("GetSubmitEncodeInfo memcpy_s error!");
            return nullptr;
        }
        info->smcaLen = encodeSmscLen;
        info->tpduLen = bufLen;
        info->isMore_ = bMore;
    }
    return info;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::CreateDeliverSmsTpdu()
{
    smsTpdu_ = std::make_shared<struct SmsTpdu>();
    smsTpdu_->tpduType = SMS_TPDU_DELIVER;
    smsTpdu_->data.deliver.bHeaderInd = false;
    return smsTpdu_;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::CreateDeliverReportSmsTpdu()
{
    smsTpdu_ = std::make_shared<struct SmsTpdu>();
    smsTpdu_->tpduType = SMS_TPDU_DELIVER_REP;
    smsTpdu_->data.deliverRep.bHeaderInd = false;
    smsTpdu_->data.deliverRep.paramInd = 0x00;
    return smsTpdu_;
}

std::shared_ptr<struct SmsTpdu> GsmSmsMessage::CreateStatusReportSmsTpdu()
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
    if (smsTpdu_ == nullptr || pdu.empty() || pdu.length() > MAX_TPDU_DATA_LEN) {
        TELEPHONY_LOGE("GsmSmsMessage::PduAnalysis smsTpdu is null");
        return false;
    }
    struct SmsAddress smsc;
    if (memset_s(&smsc, sizeof(struct SmsAddress), 0x00, sizeof(struct SmsAddress)) != EOK) {
        TELEPHONY_LOGE("PduAnalysis memset_s error!");
        return false;
    }
    int smscLen = GsmSmsParamCodec::DecodeSMSC((const unsigned char *)pdu.c_str(), pdu.length(), smsc);
    if (smscLen > 0) {
        scAddress_ = smsc.address;
    }
    if (smscLen >= static_cast<int>(pdu.length())) {
        TELEPHONY_LOGE("PduAnalysis pdu is invalid!");
        return false;
    }

    unsigned char tempPdu[TAPI_TEXT_SIZE_MAX + 1] = {0};
    if (memcpy_s(tempPdu, sizeof(tempPdu), (pdu.c_str() + smscLen), (pdu.length() - smscLen)) != EOK) {
        return false;
    }
    int decodeLen = GsmSmsTpduCodec::DecodeTpdu(tempPdu, sizeof(tempPdu), smsTpdu_.get());
    if (decodeLen <= 0) {
        TELEPHONY_LOGE("decodeLen <= 0.");
        return false;
    }
    switch (smsTpdu_->tpduType) {
        case SMS_TPDU_DELIVER:
            AnalysisMsgDeliver(smsTpdu_->data.deliver);
            break;
        case SMS_TPDU_STATUS_REP:
            AnalysisMsgStatusReport(smsTpdu_->data.statusRep);
            break;
        case SMS_TPDU_SUBMIT:
            AnalysisMsgSubmit(smsTpdu_->data.submit);
            break;
        default:
            TELEPHONY_LOGE("tpduType is unknown.");
            result = false;
            break;
    }
    return result;
}

void GsmSmsMessage::AnalysisMsgDeliver(const SmsDeliver &deliver)
{
    protocolId_ = (int)(deliver.pid);
    hasReplyPath_ = deliver.bReplyPath;
    bStatusReportMessage_ = deliver.bStatusReport;
    bMoreMsg_ = deliver.bMoreMsg;
    bHeaderInd_ = deliver.bHeaderInd;
    originatingAddress_ = deliver.originAddress.address;
    headerCnt_ = deliver.userData.headerCnt;
    ConvertMsgTimeStamp(deliver.timeStamp);
    ConvertMessageDcs();
    ConvertUserData();
}

void GsmSmsMessage::AnalysisMsgStatusReport(const SmsStatusReport &statusRep)
{
    protocolId_ = (int)(statusRep.pid);
    msgRef_ = statusRep.msgRef;
    bMoreMsg_ = statusRep.bMoreMsg;
    bStatusReportMessage_ = statusRep.bStatusReport;
    bHeaderInd_ = statusRep.bHeaderInd;
    status_ = statusRep.status;
    ConvertMsgTimeStamp(statusRep.timeStamp);
    ConvertMessageDcs();
    ConvertUserData();
}

void GsmSmsMessage::AnalysisMsgSubmit(const SmsSubmit &submit)
{
    protocolId_ = static_cast<int>(submit.pid);
    msgRef_ = submit.msgRef;
    bStatusReportMessage_ = submit.bStatusReport;
    bHeaderInd_ = submit.bHeaderInd;
    originatingAddress_ = submit.destAddress.address;
    ConvertMsgTimeStamp(submit.validityPeriod);
    ConvertMessageDcs();
    ConvertUserData();
}

void GsmSmsMessage::ConvertMessageDcs()
{
    if (smsTpdu_ == nullptr) {
        TELEPHONY_LOGE("GsmSmsMessage::ConvertMessageDcs smsTpdu is null");
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
        case SMS_TPDU_SUBMIT:
            bCompressed_ = smsTpdu_->data.submit.dcs.bCompressed;
            codingScheme_ = smsTpdu_->data.submit.dcs.codingScheme;
            codingGroup_ = smsTpdu_->data.submit.dcs.codingGroup;
            bIndActive_ = smsTpdu_->data.submit.dcs.bIndActive;
            bMwi_ = smsTpdu_->data.submit.dcs.bMWI;
            bMwiSense_ = smsTpdu_->data.submit.dcs.bIndActive;
            ConvertMessageClass(smsTpdu_->data.submit.dcs.msgClass);
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
        case SMS_TPDU_SUBMIT:
            headerDataLen_ = smsTpdu_->data.submit.userData.length;
            ret = memcpy_s(
                &smsUserData_, sizeof(SmsUserData), &(smsTpdu_->data.submit.userData), sizeof(SmsUserData));
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
            MsgLangInfo langInfo = {
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
        return DEFAULT_PORT;
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
    int protocolId = GetProtocolId();
    return (GetMessageClass() == SmsMessageClass::SMS_SIM_MESSAGE) && (protocolId == 0x7f || protocolId == 0x7c);
}

void GsmSmsMessage::ConvertMsgTimeStamp(const struct SmsTimeStamp &times)
{
    if (times.format == SMS_TIME_ABSOLUTE) {
        scTimestamp_ = SmsCommonUtils::ConvertTime(times.time.absolute);
    } else {
        scTimestamp_ = time(nullptr);
    }
}

// from 3GPP TS 23.040 V5.1.0 9.2.3.24.2 Special SMS Message Indication
bool GsmSmsMessage::IsSpecialMessage() const
{
    bool result = false;
    if (GetIsTypeZeroInd()) {
        TELEPHONY_LOGD("GsmSmsMessage:: IsTypeZeroInd");
        result = true;
    }
    // 9.2.3.9	TP Protocol Identifier (TP PID)
    if (GetIsSIMDataTypeDownload()) {
        TELEPHONY_LOGD("GsmSmsMessage:: GetIsSIMDataTypeDownload");
        result = true;
    }
    if (IsMwiSet() || IsMwiClear()) {
        TELEPHONY_LOGD("GsmSmsMessage::Mwi Message");
        result = true;
    }
    return result;
}
} // namespace Telephony
} // namespace OHOS