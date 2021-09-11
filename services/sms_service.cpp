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

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
using namespace AppExecFwk;
using namespace HiviewDFX;
bool g_registerResult = SystemAbility::MakeAndRegisterAbility(DelayedSingleton<SmsService>::GetInstance().get());

SmsService::SmsService() : SystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, true) {}

SmsService::~SmsService() {}

void SmsService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TELEPHONY_LOGE("msService has already started.");
        return;
    }
    if (!Init()) {
        TELEPHONY_LOGE("failed to init SmsService");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    TELEPHONY_LOGI("SmsService::OnStart start service.");
}

bool SmsService::Init()
{
    if (!registerToService_) {
        bool ret = Publish(DelayedSingleton<SmsService>::GetInstance().get());
        if (!ret) {
            TELEPHONY_LOGE("SmsService::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
        WaitCoreServiceToInit();
        InitModule();
    }
    return true;
}

void SmsService::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    TELEPHONY_LOGI("SmsService::OnStop stop service.");
}

void SmsService::OnDump()
{
    TELEPHONY_LOGD("SmsService OnDump");
}

bool SmsService::WaitCoreServiceToInit()
{
    bool ret = false;
    for (uint32_t i = 0; i < CONNECT_MAX_TRY_COUNT; i++) {
        int slotId = CoreManager::DEFAULT_SLOT_ID;
        std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
        TELEPHONY_LOGI("connect core service count: %{public}d", i);
        if (core != nullptr && core->IsInitCore()) {
            ret = true;
            TELEPHONY_LOGI("SmsService Connection successful");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CONNECT_SERVICE_WAIT_TIME));
    }
    TELEPHONY_LOGI("SmsService connect core service init ok.");
    return ret;
}
} // namespace Telephony
} // namespace OHOS