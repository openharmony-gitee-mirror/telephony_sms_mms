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
#ifndef GSM_SMS_MESSAGE_H
#define GSM_SMS_MESSAGE_H
#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include "gsm_pdu_code_type.h"
#include "sms_base_message.h"
namespace OHOS {
namespace SMS {
#define TAPI_SIM_SMSP_ADDRESS_LEN 20
#define TAPI_NETTEXT_SMDATA_SIZE_MAX 255
#define MAX_TPDU_DATA_LEN 255
#define TAPI_TEXT_SIZE_MAX 520
#define GSM_BEAR_DATA_LEN 140
#define BYTE_BITS 8
#define CHARSET_7BIT_BITS 7
struct SpiltInfo {
    std::vector<uint8_t> encodeData;
    SMS_CODING_SCHEME_E encodeType;
    MSG_LANGUAGE_ID_T langId;
};

struct EncodeInfo {
    char tpduData_[TAPI_NETTEXT_SMDATA_SIZE_MAX + 1];
    char smcaData_[TAPI_SIM_SMSP_ADDRESS_LEN + 1];
    int tpduLen;
    int smcaLen;
    bool isMore_;
};

class GsmSmsMessage : public SmsBaseMessage {
public:
    GsmSmsMessage() = default;
    virtual ~GsmSmsMessage() = default;
    void SetFullText(const std::string &text);
    void SetDestAddress(const std::string &destaddress);
    void SetDestPort(uint32_t port);

    std::string GetFullText() const;
    std::string GetDestAddress() const;
    uint16_t GetDestPort();
    bool GetIsSmsText() const;
    bool GetGsm() const;
    std::string GetReplyAddress() const;

    int SetHeaderLang(int index, const SMS_CODING_SCHEME_E codingType, const MSG_LANGUAGE_ID_T langId);
    int SetHeaderConcat(int index, const SmsConcat &concat);
    int SetHeaderReplay(int index);

    std::shared_ptr<struct SmsTpdu> CreateDefaultSubmitSmsTpdu(const std::string &dest, const std::string &sc,
        const std::string &text, bool bStatusReport, const SMS_CODING_SCHEME_E codingScheme);
    std::shared_ptr<struct SmsTpdu> CreateDataSubmitSmsTpdu(const std::string desAddr, const std::string scAddr,
        int32_t port, const uint8_t *data, uint32_t dataLen, uint16_t msgRef8bit, bool bStatusReport);

    std::shared_ptr<struct EncodeInfo> GetSubmitEncodeInfo(const std::string &sc, bool bMore);
    std::shared_ptr<struct SmsTpdu> GreateDeliverSmsTpdu();
    std::shared_ptr<struct SmsTpdu> GreateDeliverReportSmsTpdu();
    std::shared_ptr<struct SmsTpdu> GreateStatusReportSmsTpdu();

    void SplitMessage(std::vector<struct SpiltInfo> &splitResult, const std::string &context, bool user7bitEncode,
        SMS_CODING_SCHEME_E &codingType);
    static std::shared_ptr<GsmSmsMessage> CreateMessage(const std::string &pdu);

    bool PduAnalysis(const std::string &pdu);

    void ConvertMessageDcs();
    void ConvertUserData();
    bool GetIsTypeZeroInd() const;
    bool GetIsSIMDataTypeDownload() const;

    bool IsSpecialMessage() const;

private:
    static constexpr uint16_t defaultPort_ = -1;
    std::string fullText_;
    std::string destAddress_;
    std::string replyAddress_;
    uint16_t destPort_ = -1;
    bool bSmsText_ = false;
    std::shared_ptr<struct SmsTpdu> smsTpdu_;
    void CreateDefaultSubmit(bool bStatusReport, const SMS_CODING_SCHEME_E codingScheme);
    int SetSmsTpduDesAddress(std::shared_ptr<struct SmsTpdu> &tPdu, const std::string &desAddr);
    int CalcReplayEncodeAddress(const std::string &replyAddress);
    static int GetCodeLen(unsigned char (&decodeData)[(MAX_GSM_7BIT_DATA_LEN * MAX_SEGMENT_NUM) + 1],
        SMS_CODING_SCHEME_E &codingType, std::string &msgText, bool &bAbnormal, MSG_LANGUAGE_ID_T &langId);
};
} // namespace SMS
} // namespace OHOS
#endif