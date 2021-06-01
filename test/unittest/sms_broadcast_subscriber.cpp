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
namespace OHOS {
namespace SMS {
using namespace OHOS::EventFwk;
SmsBroadcastSubscriber::SmsBroadcastSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void SmsBroadcastSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::cout << "CommonEventPublishOrderedEventTest::Subscriber OnReceiveEvent" << std::endl;
    OHOS::EventFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    if (action == SMS_ACTION_RECEIVE) {
        int msgcode = GetCode();
        std::string msgdata = GetData();
        std::cout << "Sms Receive::OnReceiveEvent msgcode" << msgcode << std::endl;
        std::cout << "Sms Receive::OnReceiveEvent data = " << msgdata.data() << std::endl;
        const std::vector<std::string> pdus = want.GetStringArrayParam("pdu");
        for (unsigned int index = 0; index < pdus.size(); ++index) {
            std::cout << "Sms Receive::OnReceiveEvent pdus = " << pdus[index] << std::endl;
        }
    } else {
        std::cout << "CommonEventPublishOrderedEventTest::Subscriber OnReceiveEvent do nothing" << std::endl;
    }
}
} // namespace SMS
} // namespace OHOS