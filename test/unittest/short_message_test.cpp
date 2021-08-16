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

#include <inttypes.h>
#include "short_message_test.h"
#include <memory>
#include "string_utils.h"
namespace OHOS {
namespace SMS {
ShortMessage *ShortMessageTest::TestCreateMessage() const
{
    std::vector<unsigned char> pdu = StringUtils::HexToByteVector(
        "0891683108200075F4240D91688129562983F6000012408"
        "00102"
        "142302C130");
    ShortMessage *message = ShortMessage::CreateMessage(pdu, u"3gpp");
    if (message == nullptr) {
        printf("TestCreateMessage fail!!! \n");
    } else {
        printf("TestCreateMessage succeess!!! \n");
    }
    return message;
}

void ShortMessageTest::TestGetVisbleMessageBody(const ShortMessage &message) const
{
    printf("GetVisibleMessageBody = %s\r\n", StringUtils::ToUtf8(message.GetVisibleMessageBody()).c_str());
}

void ShortMessageTest::TestShowShortMessage(const ShortMessage &message) const
{
    printf("GetScAddress = %s\r\n", StringUtils::ToUtf8(message.GetScAddress()).c_str());
    printf("GetVisibleMessageBody = %s\r\n", StringUtils::ToUtf8(message.GetVisibleMessageBody()).c_str());
    printf("GetVisibleRawAddress = %s\r\n", StringUtils::ToUtf8(message.GetVisibleRawAddress()).c_str());
    printf("GetScTimestamp = %" PRIu64 "\r\n", message.GetScTimestamp());
    printf("GetProtocolId = %d\r\n", message.GetProtocolId());
    printf("GetStatus = %d\r\n", message.GetStatus());
    printf("GetMessageClass = %d\r\n", message.GetMessageClass());
    printf("HasReplyPath = %d\r\n", message.HasReplyPath());
    printf("IsSmsStatusReportMessage = %d\r\n", message.IsSmsStatusReportMessage());
    printf("IsReplaceMessage = %d\r\n", message.IsReplaceMessage());
    printf("HasReplyPath = %d\r\n", message.HasReplyPath());
    printf("raw pdu = %s\r\n", StringUtils::StringToHex(message.GetPdu()).c_str());
}
} // namespace SMS
} // namespace OHOS
