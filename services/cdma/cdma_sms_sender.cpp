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
#include "cdma_sms_sender.h"
namespace OHOS {
namespace SMS {
using namespace std;
CdmaSmsSender::CdmaSmsSender(const shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId,
    function<void(shared_ptr<SmsSendIndexer>)> sendRetryFun)
    : SmsSender(runner, slotId, sendRetryFun)
{}

CdmaSmsSender::~CdmaSmsSender() {}

void CdmaSmsSender::TextBasedSmsDelivery(const string &desAddr, const string &scAddr, const string &text,
    const sptr<ISendShortMessageCallback> &sendCallback, const sptr<IDeliveryShortMessageCallback> &deliveCallback)
{}

void CdmaSmsSender::DataBasedSmsDelivery(const string &desAddr, const string &scAddr, int32_t port,
    const uint8_t *data, uint32_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
    const sptr<IDeliveryShortMessageCallback> &deliveCallback)
{}

void CdmaSmsSender::StatusReportAnalysis(const shared_ptr<SmsSendIndexer> &smsIndxer)
{
    if (smsIndxer == nullptr) {
        return;
    }
    auto iter = reportList_.begin();
    while (iter != reportList_.end()) {
        auto oldIter = iter++;
        SmsSendIndexer *oldIndexer = (*oldIter).get();
        if (oldIndexer != nullptr) {
            if (smsIndxer->GetMsgRefId() == oldIndexer->GetMsgRefId()) {
                reportList_.erase(oldIter);
            }
        }
    }
}

void CdmaSmsSender::SendSmsToRil(const shared_ptr<SmsSendIndexer> &smsIndxer) {}
} // namespace SMS
} // namespace OHOS