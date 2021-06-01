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
#include "send_short_message_callback_stub.h"
namespace OHOS {
int32_t SendShortMessageCallbackStub::OnSmsSendResult(const SMS::ISendShortMessageCallback::SmsSendResult result)
{
    switch (result) {
        case SMS::ISendShortMessageCallback::SmsSendResult::SEND_SMS_SUCCESS:
            printf("SendShortMessageCallback OnSmsSendResult::SEND_SMS_SUCCESS\r\n");
            break;
        case SMS::ISendShortMessageCallback::SmsSendResult::SEND_SMS_FAILURE_RADIO_OFF:
            printf("SendShortMessageCallback OnSmsSendResult::SEND_SMS_FAILURE_RADIO_OFF\r\n");
            break;
        case SMS::ISendShortMessageCallback::SmsSendResult::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE:
            printf("SendShortMessageCallback OnSmsSendResult::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE\r\n");
            break;
        case SMS::ISendShortMessageCallback::SmsSendResult::SEND_SMS_FAILURE_UNKNOWN:
            printf("SendShortMessageCallback OnSmsSendResult::SEND_SMS_FAILURE_UNKNOWN\r\n");
            break;
        default:
            break;
    }
    return 0;
}

int SendShortMessageCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case ON_SMS_SEND_RESULT: {
            int32_t result = data.ReadInt32();
            OnSmsSendResult(static_cast<SMS::ISendShortMessageCallback::SmsSendResult>(result));
            return 0;
        }
        default: {
            break;
        }
    }
    return -1;
}
} // namespace OHOS