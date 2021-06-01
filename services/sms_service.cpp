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
#include "sms_service.h"
#include <string>
#include "sms_hilog_wrapper.h"
namespace OHOS {
namespace SMS {
using namespace std;
using namespace AppExecFwk;
using namespace HiviewDFX;
bool g_registerResult = SystemAbility::MakeAndRegisterAbility(DelayedSingleton<SmsService>::GetInstance().get());

SmsService::SmsService() : SystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, true) {}

SmsService::~SmsService() {}

void SmsService::OnStart()
{
    HILOG_INFO("enter SmsService::OnStart");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOG_ERROR("msService has already started.");
        return;
    }
    if (!Init()) {
        HILOG_ERROR("failed to init SmsService");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    HILOG_INFO("SmsService::OnStart start service.");
}

bool SmsService::Init()
{
    if (!registerToService_) {
        bool ret = Publish(DelayedSingleton<SmsService>::GetInstance().get());
        if (!ret) {
            HILOG_ERROR("SmsService::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
        InitModule();
    }
    HILOG_INFO("SmsService::Init init successfully.");
    return true;
}

void SmsService::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    HILOG_INFO("SmsService::OnStop stop service.");
}

void SmsService::OnDump()
{
    HILOG_DEBUG("SmsService OnDump");
}
} // namespace SMS
} // namespace OHOS