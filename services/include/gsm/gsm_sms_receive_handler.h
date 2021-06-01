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
#ifndef GSM_SMS_RECEIVE_HANDLER_H
#define GSM_SMS_RECEIVE_HANDLER_H
#include "event_handler.h"
#include "event_runner.h"
#include "sms_receive_indexer.h"
#include "sms_receive_handler.h"
namespace OHOS {
namespace SMS {
class GsmSmsReceiveHandler : public SmsReceiveHandler {
public:
    GsmSmsReceiveHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    ~GsmSmsReceiveHandler() override;

    void RegisterHandler();
    void UnRegisterHandler();

protected:
    virtual int32_t HandleSmsByType(const std::shared_ptr<SmsBaseMessage> &smsBaseMessage) override;
    virtual void ReplySmsToSmsc(int result, const std::shared_ptr<SmsBaseMessage> &response) override;
    std::shared_ptr<SmsBaseMessage> TransformMessageInfo(const std::shared_ptr<SmsMessageInfo> &info) override;
};
} // namespace SMS
} // namespace OHOS
#endif