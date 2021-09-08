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

#include "short_message_test.h"

#include <iostream>
#include <memory>

#include "string_utils.h"

namespace OHOS {
namespace Telephony {
using namespace OHOS::Telephony;
ShortMessage *ShortMessageTest::TestCreateMessage() const
{
    std::vector<unsigned char> pdu = StringUtils::HexToByteVector(
        "0891683108200075F4240D91688129562983F6000012408"
        "00102"
        "142302C130");
    ShortMessage *message = ShortMessage::CreateMessage(pdu, u"3gpp");
    if (message == nullptr) {
        std::cout << "TestCreateMessage fail!!!" << std::endl;
    } else {
        std::cout << "TestCreateMessage success!!!" << std::endl;
    }
    return message;
}

void ShortMessageTest::TestGetVisibleMessageBody(const ShortMessage &message) const
{
    std::cout << "GetVisibleMessageBody = " << StringUtils::ToUtf8(message.GetVisibleMessageBody()) << std::endl;
}

void ShortMessageTest::TestShowShortMessage(const ShortMessage &message) const
{
    std::cout << "GetSmscAddr = " << StringUtils::ToUtf8(message.GetScAddress()) << std::endl;
    std::cout << "GetVisibleMessageBody = " << StringUtils::ToUtf8(message.GetVisibleMessageBody()) << std::endl;
    std::cout << "GetVisibleRawAddress = " << StringUtils::ToUtf8(message.GetVisibleRawAddress()) << std::endl;
    std::cout << "GetScTimestamp = " << message.GetScTimestamp() << std::endl;
    std::cout << "GetProtocolId = " << message.GetProtocolId() << std::endl;
    std::cout << "GetStatus = " << message.GetStatus() << std::endl;
    std::cout << "GetMessageClass = " << message.GetMessageClass() << std::endl;
    std::cout << "HasReplyPath = " << message.HasReplyPath() << std::endl;
    std::cout << "IsSmsStatusReportMessage = " << message.IsSmsStatusReportMessage() << std::endl;
    std::cout << "IsReplaceMessage = " << message.IsReplaceMessage() << std::endl;
    std::cout << "HasReplyPath = " << message.HasReplyPath() << std::endl;
    std::cout << "raw pdu = " << StringUtils::StringToHex(message.GetPdu()) << std::endl;
}
} // namespace Telephony
} // namespace OHOS