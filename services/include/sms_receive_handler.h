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
#ifndef SMS_RECEIVE_HANDLER_H
#define SMS_RECEIVE_HANDLER_H
#include <memory>
#include <unordered_map>
#include <vector>
#include "hril_sms_parcel.h"
#include "event_handler.h"
#include "event_runner.h"
#include "sms_base_message.h"
#include "sms_cellbroadcast_handler.h"
#include "sms_common.h"
#include "sms_receive_indexer.h"
#include "sms_wappush_handler.h"
namespace OHOS {
namespace SMS {
class SmsReceiveHandler : public AppExecFwk::EventHandler {
public:
    SmsReceiveHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    virtual ~SmsReceiveHandler();
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

protected:
    int32_t slotId_ = -1;
    IRilManager *rilManager_ = nullptr;
    std::unordered_multimap<std::string, std::shared_ptr<SmsReceiveIndexer>> receiveMap_;
    virtual int32_t HandleSmsByType(const std::shared_ptr<SmsBaseMessage> &smsBaseMessage) = 0;
    virtual void ReplySmsToSmsc(int result, const std::shared_ptr<SmsBaseMessage> &response) = 0;
    virtual std::shared_ptr<SmsBaseMessage> TransformMessageInfo(const std::shared_ptr<SmsMessageInfo> &info) = 0;
    void CombineMessagePart(const std::shared_ptr<SmsReceiveIndexer> &smsIndexer);
    bool IsRepeatedMessagePart(const std::shared_ptr<SmsReceiveIndexer> &smsIndexer);

private:
    void SendBroadcast(const std::shared_ptr<std::vector<std::string>> &pdus);
    void HandleReceivedSms(const std::shared_ptr<SmsBaseMessage> &smsBaseMessage);
    std::shared_ptr<AppExecFwk::EventRunner> smsCellBroadcastRunner_;
    std::shared_ptr<AppExecFwk::EventRunner> smsWappushRunner_;
    SmsReceiveHandler(const SmsReceiveHandler &) = delete;
    SmsReceiveHandler(const SmsReceiveHandler &&) = delete;
    SmsReceiveHandler &operator=(const SmsReceiveHandler &) = delete;
    SmsReceiveHandler &operator=(const SmsReceiveHandler &&) = delete;
    constexpr static uint16_t pduPosOffset_ = 1;
    constexpr static int32_t msgReceiveCode_ = 0;
};
} // namespace SMS
} // namespace OHOS
#endif