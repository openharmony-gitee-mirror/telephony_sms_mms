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

#include "sms_broadcast_subscriber.h"

#include "common_event_support.h"

#include "short_message.h"
#include "string_utils.h"

namespace OHOS {
namespace Telephony {
using namespace OHOS::EventFwk;
SmsBroadcastSubscriber::SmsBroadcastSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void SmsBroadcastSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::cout << "CommonEventPublishOrderedEventTest::Subscriber OnReceiveEvent" << std::endl;
    OHOS::EventFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    if (action == "usual.event.SMS_RECEIVE_COMPLETED") {
        int msgcode = GetCode();
        std::string msgdata = GetData();
        std::cout << "Sms Receive::OnReceiveEvent msgcode" << msgcode << std::endl;
        std::cout << "Sms Receive::OnReceiveEvent data = " << msgdata.data() << std::endl;
        const std::vector<std::string> pdus = want.GetStringArrayParam("pdu");
        for (unsigned int index = 0; index < pdus.size(); ++index) {
            std::vector<unsigned char> pdu = StringUtils::HexToByteVector(pdus[index]);
            ShortMessage *message = ShortMessage::CreateMessage(pdu, u"3gpp");
            if (message != nullptr) {
                std::string messageBody = StringUtils::ToUtf8(message->GetVisibleMessageBody());
                std::cout << "receive new sms = " << messageBody.c_str() << std::endl;
                delete message;
                message = nullptr;
            } else {
                std::cout << "Sms Receive::OnReceiveEvent pdus = " << pdus[index] << std::endl;
            }
        }
    } else if (action == "ohos.action.telephonySmsETWSCBReceiveFinished") {
        std::cout << "telephonySmsETWSCBReceiveFinished" << std::endl;
        std::cout << "serialNum:" << static_cast<unsigned short>(want.GetShortParam("serialNum", 0)) << std::endl;
        std::cout << "msgBody:" << want.GetStringParam("msgBody") << std::endl;
        std::cout << "recvTime:" << want.GetLongParam("recvTime", 0) << std::endl;
        std::cout << "warningType:" << static_cast<unsigned short>(want.GetShortParam("warningType", 0))
                  << std::endl;
    } else if (action == "ohos.action.telephonySmsCBReceiveFinished") {
        std::cout << "telephonySmsCBReceiveFinished" << std::endl;
        std::cout << "serialNum:" << static_cast<unsigned short>(want.GetShortParam("serialNum", 0)) << std::endl;
        std::cout << "msgBody:" << want.GetStringParam("msgBody") << std::endl;
        std::cout << "recvTime:" << want.GetLongParam("recvTime", 0) << std::endl;
        std::cout << "cbMsgType:" << static_cast<unsigned char>(want.GetCharParam("cbMsgType", ' ')) << std::endl;
        std::cout << "msgId:" << static_cast<unsigned short>(want.GetShortParam("msgId", 0)) << std::endl;
        std::cout << "langType:" << static_cast<unsigned char>(want.GetCharParam("langType", ' ')) << std::endl;
        std::cout << "dcs:" << static_cast<unsigned char>(want.GetCharParam("dcs", ' ')) << std::endl;
    } else {
        std::cout << "CommonEventPublishOrderedEventTest::Subscriber OnReceiveEvent do nothing" << std::endl;
    }
}
} // namespace Telephony
} // namespace OHOS