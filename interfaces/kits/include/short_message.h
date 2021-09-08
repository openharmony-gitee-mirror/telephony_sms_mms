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

#ifndef SHORT_MESSAGE_H
#define SHORT_MESSAGE_H

#include <codecvt>
#include <locale>
#include <memory>

#include "parcel.h"

namespace OHOS {
namespace Telephony {
class ShortMessage : public Parcelable {
public:
    /**
     * @brief SmsMessageClass
     * Indicates the SMS type.
     * from 3GPP TS 23.038 V4.3.0 5 CBS Data Coding Scheme
     */
    enum SmsMessageClass {
        /** class0 Indicates an instant message, which is displayed immediately after being received. */
        SMS_INSTANT_MESSAGE = 0,
        /** class1 Indicates an SMS message that can be stored on the device or SIM card based on the storage
           status. */
        SMS_OPTIONAL_MESSAGE,
        /** class2 Indicates an SMS message containing SIM card information, which is to be stored in a SIM card. */
        SMS_SIM_MESSAGE,
        /** class3 Indicates an SMS message to be forwarded to another device. */
        SMS_FORWARD_MESSAGE,
        /** Indicates an unknown type. */
        SMS_CLASS_UNKNOWN,
    };

    /**
     * @brief SimMessageStatus
     * from 3GPP TS 51.011 V4.0.0 (2001-03) section 10.5.3 Parameter Definitions
     */
    enum SmsSimMessageStatus {
        SMS_SIM_MESSAGE_STATUS_FREE = 0, /** status free space ON SIM */
        SMS_SIM_MESSAGE_STATUS_READ = 1, /** REC READ received read message */
        SMS_SIM_MESSAGE_STATUS_UNREAD = 3, /** REC UNREAD received unread message */
        SMS_SIM_MESSAGE_STATUS_SENT = 5, /** "STO SENT" stored sent message (only applicable to SMs) */
        SMS_SIM_MESSAGE_STATUS_UNSENT = 7, /** "STO UNSENT" stored unsent message (only applicable to SMs) */
        SMS_SIM_MESSAGE_STATUS_UNKNOWN, /** Indicates an unknown status. */
    };

    /**
     * @brief GetVisibleMessageBody
     * Obtains the SMS message body
     * @return std::u16string
     */
    std::u16string GetVisibleMessageBody() const;

    /**
     * @brief GetVisibleRawAddress
     * Obtains the address of the sender, which is to be displayed on the UI
     * @return std::u16string
     */
    std::u16string GetVisibleRawAddress() const;

    /**
     * @brief GetMessageClass
     * Obtains the SMS type
     * @return SmsMessageClass
     */
    SmsMessageClass GetMessageClass() const;

    /**
     * @brief GetScAddress
     * Obtains the short message service center (SMSC) address
     * @return std::u16string
     */
    std::u16string GetScAddress() const;

    /**
     * @brief GetScTimestamp
     * Obtains the SMSC timestamp
     * @return int64_t
     */
    int64_t GetScTimestamp() const;

    /**
     * @brief IsReplaceMessage
     * Checks whether the received SMS is a "replace short message"
     * @return true
     * @return false
     */
    bool IsReplaceMessage() const;

    /**
     * @brief GetStatus
     *
     * @return int32_t
     */
    int32_t GetStatus() const;

    /**
     * @brief IsSmsStatusReportMessage
     *
     * @return true
     * @return false
     */
    bool IsSmsStatusReportMessage() const;

    /**
     * @brief HasReplyPath
     * Checks whether the received SMS contains "TP-Reply-Path"
     * @return true
     * @return false
     */
    bool HasReplyPath() const;

    /**
     * @brief Get the Icc Message Status object
     *
     * @return SmsSimMessageStatus
     */
    SmsSimMessageStatus GetIccMessageStatus() const;

    /**
     * @brief GetProtocolId
     *
     * @return int32_t
     */
    int32_t GetProtocolId() const;

    /**
     * @brief Get the Pdu
     *
     * @return std::vector<unsigned char>
     */
    std::vector<unsigned char> GetPdu() const;

    /**
     * @brief Create a Message object
     * Creates an SMS message instance based on the
     * protocol data unit (PDU) and the specified SMS protocol
     * @param pdu
     * @param specification
     * @return ShortMessage*
     */
    static ShortMessage *CreateMessage(std::vector<unsigned char> &pdu, std::u16string specification);

    /**
     * @brief Create a Icc Message object
     * Creates an SMS message instance based on the
     * ICC protocol data unit (PDU) and the specified SMS protocol
     * @param pdu
     * @param specification
     * @return ShortMessage
     */
    static ShortMessage CreateIccMessage(std::vector<unsigned char> &pdu, std::string specification);
    ~ShortMessage() = default;
    ShortMessage() = default;
    virtual bool Marshalling(Parcel &parcel) const override;
    static ShortMessage UnMarshalling(Parcel &parcel);

private:
    bool ReadFromParcel(Parcel &parcel);

    static constexpr int MIN_ICC_PDU_LEN = 1;
    std::u16string visibleMessageBody_;
    std::u16string visibleRawAddress_;
    SmsMessageClass messageClass_ = SMS_CLASS_UNKNOWN;
    SmsSimMessageStatus simMessageStatus_ = SMS_SIM_MESSAGE_STATUS_UNKNOWN;
    std::u16string scAddress_;
    int64_t scTimestamp_ = 0;
    bool isReplaceMessage_ = false;
    int32_t status_ = -1;
    bool isSmsStatusReportMessage_ = false;
    bool hasReplyPath_ = false;
    int32_t protocolId_ = -1;
    std::vector<unsigned char> pdu_;
};
} // namespace Telephony
} // namespace OHOS
#endif