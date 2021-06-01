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
#include "delivery_short_message_callback_stub.h"

namespace OHOS {
int32_t DeliveryShortMessageCallbackStub::OnSmsDeliveryResult(const std::u16string pdu)
{
    return 0;
}

int DeliveryShortMessageCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case ON_SMS_DELIVERY_RESULT: {
            auto result = data.ReadString16();
            OnSmsDeliveryResult(result);
            return 0;
        }
        default: {
            OnSmsDeliveryResult(u"");
            break;
        }
    }
    return -1;
}
} // namespace OHOS