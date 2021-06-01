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
#include "cdma_sms_receive_handler.h"
#include "sms_hilog_wrapper.h"
namespace OHOS {
namespace SMS {
CdmaSmsReceiveHandler::CdmaSmsReceiveHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t subId)
    : SmsReceiveHandler(runner, subId)
{
    HILOG_INFO("%{public}d", slotId_);
}

int32_t CdmaSmsReceiveHandler::HandleSmsByType(const std::shared_ptr<SmsBaseMessage> &smsBaseMessage)
{
    return 0;
}

void CdmaSmsReceiveHandler::ReplySmsToSmsc(int result, const std::shared_ptr<SmsBaseMessage> &response) {}

void CdmaSmsReceiveHandler::SetCdmaSender(const std::weak_ptr<SmsSender> &smsSender)
{
    cdmaSmsSender_ = smsSender;
}

std::shared_ptr<SmsBaseMessage> CdmaSmsReceiveHandler::TransformMessageInfo(
    const std::shared_ptr<SmsMessageInfo> &info)
{
    (void)info;
    return nullptr;
}
} // namespace SMS
} // namespace OHOS