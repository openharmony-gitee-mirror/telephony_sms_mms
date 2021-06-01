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
#ifndef SMS_SERVICE_H
#define SMS_SERVICE_H
#include <memory>
#include "sms_interface_stub.h"
#include "system_ability.h"
#include "system_ability_definition.h"
namespace OHOS {
namespace SMS {
enum ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class SmsService : public SystemAbility, public SmsInterfaceStub, public std::enable_shared_from_this<SmsService> {
    DECLARE_DELAYED_SINGLETON(SmsService)
    DECLARE_SYSTEM_ABILITY(SmsService) // necessary
public:
    void OnStart() override;
    void OnStop() override;
    void OnDump() override;

private:
    bool Init();
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    bool registerToService_ = false;
};
} // namespace SMS
} // namespace OHOS
#endif