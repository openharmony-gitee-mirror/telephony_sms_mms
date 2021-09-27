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

#include "core_manager.h"
#include "hril_sms_parcel.h"
#include "event_handler.h"
#include "event_runner.h"

#include "i_sms_service_interface.h"
#include "sms_send_indexer.h"

namespace OHOS {
namespace Telephony {
class SmsSender : public AppExecFwk::EventHandler {
public:
    virtual ~SmsSender();
    SmsSender(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId,
        std::function<void(std::shared_ptr<SmsSendIndexer>)> &sendRetryFun);

    virtual void TextBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr,
        const std::string &text, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveryCallback) = 0;

    virtual void DataBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, int32_t port,
        const uint8_t *data, uint32_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliveryCallback) = 0;

    virtual void SendSmsToRil(const std::shared_ptr<SmsSendIndexer> &smsIndexer) = 0;
    void HandleMessageResponse(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    virtual void Init() = 0;
    virtual void ResendTextDelivery(const std::shared_ptr<SmsSendIndexer> &smsIndexer) = 0;
    virtual void ResendDataDelivery(const std::shared_ptr<SmsSendIndexer> &smsIndexer) = 0;

    static void SendResultCallBack(
        const std::shared_ptr<SmsSendIndexer> &indexer, ISendShortMessageCallback::SmsSendResult result);
    static void SendResultCallBack(
        const sptr<ISendShortMessageCallback> &sendCallback, ISendShortMessageCallback::SmsSendResult result);

protected:
    int32_t slotId_ = -1;
    bool isRadioOn_ = false;
    NetDomainType netDomainType_ = NetDomainType::NET_DOMAIN_CS;
    NetWorkType netWorkType_ = NetWorkType::NET_TYPE_UNKNOWN;
    std::list<std::shared_ptr<SmsSendIndexer>> reportList_;

    void SendCacheMapTimeoutCheck();
    bool SendCacheMapLimitCheck(const sptr<ISendShortMessageCallback> &sendCallback);
    bool SendCacheMapAddItem(int64_t id, const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    bool SendCacheMapEraseItem(int64_t id);
    std::shared_ptr<SmsSendIndexer> FindCacheMapAndTransform(const AppExecFwk::InnerEvent::Pointer &event);
    uint8_t GetMsgRef8Bit();
    int64_t GetMsgRef64Bit();
    virtual void StatusReportAnalysis(const AppExecFwk::InnerEvent::Pointer &event) = 0;
    void SendMessageSucceed(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    void SendMessageFailed(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
    bool GetImsDomain() const;
    enum NetWorkType GetNetWorkType() const;
    enum NetDomainType GetNetDomainType() const;
    std::shared_ptr<Core> GetCore() const;

private:
    uint8_t msgRef8bit_ = 0;
    int64_t msgRef64bit_ = 0;
    std::function<void(std::shared_ptr<SmsSendIndexer>)> sendRetryFun_;
    std::unordered_map<int64_t, std::shared_ptr<SmsSendIndexer>> sendCacheMap_;
    std::mutex sendCacheMapMutex_;
    static constexpr uint16_t EXPIRED_TIME = 60 * 3;
    static constexpr uint16_t DELAY_MAX_TIME_MSCE = 2000;
    static constexpr uint8_t MSG_QUEUE_LIMIT = 25;
    static constexpr uint8_t MAX_REPORT_LIST_LIMIT = 25;
    static constexpr uint8_t MAX_SEND_RETRIES = 3;

    SmsSender(const SmsSender &) = delete;
    SmsSender(const SmsSender &&) = delete;
    SmsSender &operator=(const SmsSender &) = delete;
    SmsSender &operator=(const SmsSender &&) = delete;
    void HandleResend(const std::shared_ptr<SmsSendIndexer> &smsIndexer);
};
} // namespace Telephony
} // namespace OHOS
#endif