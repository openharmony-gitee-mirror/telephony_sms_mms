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

#ifndef SMS_WAP_PUSH_HANDLER_H
#define SMS_WAP_PUSH_HANDLER_H

#include <memory>

#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace Telephony {
class SmsWapPushHandler : public AppExecFwk::EventHandler {
public:
    SmsWapPushHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    virtual ~SmsWapPushHandler();
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

protected:
private:
    int32_t slotId_;
};
} // namespace Telephony
} // namespace OHOS
#endif