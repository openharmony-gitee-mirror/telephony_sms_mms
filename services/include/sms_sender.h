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
#ifndef SMS_SENDER_H
#define SMS_SENDER_H
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include "phone_manager.h"
#include "hril_sms_parcel.h"
#include "event_handler.h"
#include "event_runner.h"
#include "i_sms_service_interface.h"
#include "sms_send_indexer.h"
#include "sms_sender.h"
namespace OHOS {
namespace SMS {
class SmsSender : public AppExecFwk::EventHandler {
public:
    virtual ~SmsSender();
    SmsSender(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId,
        std::function<void(std::shared_ptr<SmsSendIndexer>)> &sendRetryFun);

    virtual void TextBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr,
        const std::string &text, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveCallback) = 0;

    virtual void DataBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, int32_t port,
        const uint8_t *data, uint32_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveCallback) = 0;

    virtual void StatusReportAnalysis(const std::shared_ptr<SmsSendIndexer> &smsIndexer) = 0;
    virtual void SendSmsToRil(const std::shared_ptr<SmsSendIndexer> &smsIndexer) = 0;
    void HandleMessageResponse(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    void SendMessageSucceed(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    void SendMessageFailed(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    bool GetImsDomain() const;
    virtual void Init();
    enum NetWorkType GetNetWorkType() const;
    enum NetDomainType GetNetDomainType() const;

protected:
    int32_t slotId_ = -1;
    bool isRadioOn_ = false;
    IRilManager *rilManager_ = nullptr;
    NetDomainType netDomainType_ = NetDomainType::NET_DOMAIN_CS;
    NetWorkType netWorkType_ = NetWorkType::NET_TYPE_UNKOWN;
    std::list<std::shared_ptr<SmsSendIndexer>> reportList_;
    void SendCacheMapTimeoutCheck();
    bool SendCacheMapLimitCheck(const sptr<ISendShortMessageCallback> &sendCallback);
    bool SendCacheMapAddItem(int64_t id, const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    bool SendCacheMapEraseItem(int64_t id);
    std::shared_ptr<SmsSendIndexer> FindCacheMapAndTransform(const std::shared_ptr<SendSmsResultInfo> &info);

private:
    void HandleResend(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    std::function<void(std::shared_ptr<SmsSendIndexer>)> sendRetryFun_;
    std::unordered_map<int64_t, std::shared_ptr<SmsSendIndexer>> sendCacheMap_;
    std::mutex sendCacheMapMutex_;
    static constexpr uint16_t expiredTime_ = 60 * 3;
    static constexpr uint16_t delayMaxTimeMsce_ = 2000;
    static constexpr uint8_t msgQueueLimit_ = 25;
    static constexpr uint8_t maxReportListLimit_ = 25;
    static constexpr uint8_t maxSendRetries_ = 3;
    SmsSender(const SmsSender &) = delete;
    SmsSender(const SmsSender &&) = delete;
    SmsSender &operator=(const SmsSender &) = delete;
    SmsSender &operator=(const SmsSender &&) = delete;
};
} // namespace SMS
} // namespace OHOS
#endif