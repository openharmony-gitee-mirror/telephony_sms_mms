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

#include <iostream>
#include <memory>

#include "delivery_call_back.h"
#include "send_call_back.h"
#include "short_message_manager.h"

using namespace OHOS::Telephony;
int main()
{
    std::cout << "enter sms_mms_api_test main" << std::endl;
    std::unique_ptr<ShortMessageManager> shortMessageManager = std::make_unique<ShortMessageManager>();
    OHOS::sptr<SendCallBack> sendCallBackPtr(new SendCallBack());
    OHOS::sptr<DeliveryCallBack> deliveryCallBackPtr(new DeliveryCallBack());
    shortMessageManager->SendMessage(1, u"12345678", u"asdf", u"sms_text", sendCallBackPtr, deliveryCallBackPtr);
    std::cout << "sms_mms_api_test main end" << std::endl;
    return 0;
}