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
#ifndef SEND_CALL_BACK_H
#define SEND_CALL_BACK_H
#include "send_short_message_callback_stub.h"
namespace OHOS {
class SendCallBack : public SendShortMessageCallbackStub {
public:
    SendCallBack() = default;
    ~SendCallBack() = default;
    int32_t OnSmsSendResult(const SMS::ISendShortMessageCallback::SmsSendResult result) override;
};
} // namespace OHOS
#endif // SEND_CALL_BACK_H