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
#include "sms_receive_manager.h"
#include "cdma_sms_receive_handler.h"
#include "gsm_sms_receive_handler.h"
#include "sms_hilog_wrapper.h"
namespace OHOS {
namespace SMS {
using namespace std;
SmsReceiveManager::SmsReceiveManager(int32_t slotId) : slotId_(slotId) {}

SmsReceiveManager::~SmsReceiveManager()
{
    gsmSmsReceiveHandler_->UnRegisterHandler();
    gsmSmsReceiveRunner_->Stop();
    cdmaSmsReceiveRunner_->Stop();
}

void SmsReceiveManager::Init()
{
    gsmSmsReceiveRunner_ = AppExecFwk::EventRunner::Create("gsmSmsReceiveHandler" + to_string(slotId_));
    if (gsmSmsReceiveRunner_ == nullptr) {
        HILOG_ERROR("failed to create GsmEventReceiverRunner");
        return;
    }
    cdmaSmsReceiveRunner_ = AppExecFwk::EventRunner::Create("cdmaSmsReceiveHandler" + to_string(slotId_));
    if (cdmaSmsReceiveRunner_ == nullptr) {
        HILOG_ERROR("failed to create CdmaEventReceiverRunner");
        return;
    }

    gsmSmsReceiveHandler_ = std::make_shared<GsmSmsReceiveHandler>(gsmSmsReceiveRunner_, slotId_);
    if (gsmSmsReceiveHandler_ == nullptr) {
        HILOG_ERROR("failed to create GsmSmsReceiveHandler");
        return;
    }
    cdmaSmsReceiveHandler_ = std::make_shared<CdmaSmsReceiveHandler>(cdmaSmsReceiveRunner_, slotId_);
    if (cdmaSmsReceiveHandler_ == nullptr) {
        HILOG_ERROR("failed to create CdmaSmsReceiveHandler");
        return;
    }

    gsmSmsReceiveHandler_->RegisterHandler();
    cdmaSmsReceiveRunner_->Run();
    gsmSmsReceiveRunner_->Run();

    HILOG_INFO("gsmSmsReceiveRunner_->Run().");
    HILOG_INFO("cdmaSmsReceiveRunner_->Run().");
}

void SmsReceiveManager::SetCdmaSender(const weak_ptr<SmsSender> &smsSender)
{
    if (cdmaSmsReceiveHandler_ != nullptr) {
        CdmaSmsReceiveHandler *cdmaSmsReceiveHandler = (CdmaSmsReceiveHandler *)(cdmaSmsReceiveHandler_.get());
        cdmaSmsReceiveHandler->SetCdmaSender(smsSender);
    }
}
} // namespace SMS
} // namespace OHOS