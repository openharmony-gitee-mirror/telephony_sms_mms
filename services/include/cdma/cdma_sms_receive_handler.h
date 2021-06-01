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
#ifndef CDMA_SMS_RECEIVE_HANDLER_H
#define CDMA_SMS_RECEIVE_HANDLER_H
#include "event_handler.h"
#include "event_runner.h"
#include "sms_receive_indexer.h"
#include "sms_receive_handler.h"
#include "sms_send_indexer.h"
#include "sms_sender.h"
namespace OHOS {
namespace SMS {
class CdmaSmsReceiveHandler : public SmsReceiveHandler {
public:
    CdmaSmsReceiveHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    virtual ~CdmaSmsReceiveHandler() = default;
    void SetCdmaSender(const std::weak_ptr<SmsSender> &smsSender);

protected:
    virtual int32_t HandleSmsByType(const std::shared_ptr<SmsBaseMessage> &smsBaseMessage) override;
    virtual void ReplySmsToSmsc(int result, const std::shared_ptr<SmsBaseMessage> &response) override;
    virtual std::shared_ptr<SmsBaseMessage> TransformMessageInfo(
        const std::shared_ptr<SmsMessageInfo> &info) override;

private:
    std::weak_ptr<SmsSender> cdmaSmsSender_;
};
} // namespace SMS
} // namespace OHOS
#endif