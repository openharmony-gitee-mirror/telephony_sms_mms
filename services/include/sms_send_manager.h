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
#ifndef SMS_SEND_MANAGER_H
#define SMS_SEND_MANAGER_H
#include <list>
#include <memory>
#include <string>
#include "cdma_sms_sender.h"
#include "gsm_sms_sender.h"
#include "sms_send_indexer.h"
namespace OHOS {
namespace SMS {
class SmsSendManager : public AppExecFwk::EventHandler {
public:
    SmsSendManager(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    virtual ~SmsSendManager();
    void TextBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, const std::string &text,
        const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliverCallback);
    void DataBasedSmsDelivery(const std::string &desAddr, const std::string &scAddr, const uint16_t port,
        const uint8_t *data, uint16_t dataLen, const sptr<ISendShortMessageCallback> &sendCallback,
        const sptr<IDeliveryShortMessageCallback> &deliverCallback);
    void RetriedSmsDelivery(const std::shared_ptr<SmsSendIndexer> smsIndexer);
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    void Init();

private:
    SmsSendManager &operator=(const SmsSendManager &) = delete;
    SmsSendManager(const SmsSendManager &) = delete;
    void RegisterHandler();
    void UnRegisterHandler();
    NetWorkType GetNetWorkType();
    void HandlerRadioState();

    int32_t slotId_;
    enum NetWorkType csNetWorkType_ = NetWorkType::NET_TYPE_GSM;
    enum NetWorkType imsNetWorkType_ = NetWorkType::NET_TYPE_UNKOWN;
    enum NetDomainType netDomainType_ = NetDomainType::NET_DOMAIN_CS;
    IRilManager *rilManager_ = nullptr;
    std::shared_ptr<SmsSender> gsmSmsSender_;
    std::shared_ptr<SmsSender> cdmaSmsSender_;
    std::shared_ptr<AppExecFwk::EventRunner> gsmSmsSendRunner_;
    std::shared_ptr<AppExecFwk::EventRunner> cdmaSmsSendRunner_;
};
} // namespace SMS
} // namespace OHOS
#endif