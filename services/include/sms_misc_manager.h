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

#ifndef SMS_MISC_MANAGER_H
#define SMS_MISC_MANAGER_H

#include <vector>
#include <list>
#include <mutex>
#include <string>
#include <condition_variable>

#include "event_handler.h"
#include "event_runner.h"

#include "core_manager.h"
#include "i_sms_service_interface.h"

namespace OHOS {
namespace Telephony {
class SmsMiscManager : public AppExecFwk::EventHandler {
public:
    enum {
        SET_CB_CONFIG_FINISH = 0,
        SET_SMSC_ADDR_FINISH,
        GET_SMSC_ADDR_FINISH,
    };
    using gsmCBRangeInfo = struct RangeInfo {
        RangeInfo(uint32_t fromId, uint32_t toId)
        {
            fromMsgId = fromId;
            toMsgId = toId;
        }
        uint32_t fromMsgId = 0;
        uint32_t toMsgId = 0;
        bool operator<(const RangeInfo &other) const
        {
            return fromMsgId < other.fromMsgId;
        }
    };
    SmsMiscManager(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    virtual ~SmsMiscManager() {};
    bool SetCBConfig(bool enable, uint32_t fromMsgId, uint32_t toMsgId, uint8_t netType);
    std::list<gsmCBRangeInfo> GetRangeInfo() const;
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    bool AddSimMessage(
        const std::string &smsc, const std::string &pdu, ISmsServiceInterface::SimMessageStatus status);
    bool DelSimMessage(uint32_t msgIndex);
    bool UpdateSimMessage(uint32_t msgIndex, ISmsServiceInterface::SimMessageStatus newStatus,
        const std::string &pdu, const std::string &smsc);
    std::vector<ShortMessage> GetAllSimMessages();
    bool SetSmscAddr(const std::string &scAddr);
    std::string GetSmscAddr();
    bool SetDefaultSmsSlotId(int32_t slotId);
    int32_t GetDefaultSmsSlotId();

protected:
    bool OpenCBRange(uint32_t fromMsgId, uint32_t toMsgId);
    bool CloseCBRange(uint32_t fromMsgId, uint32_t toMsgId);
    void NotifyHasResponse();
    bool IsEmpty() const;
    std::string RangeListToString(const std::list<gsmCBRangeInfo> &rangeList);

    std::list<int32_t> fairList_;
    int32_t fairVar_ = -1;
    int32_t conditonVar_ = 0;
    bool isSuccess_ = false;
    int32_t slotId_;

private:
    bool SendDataToRil(bool enable, std::list<gsmCBRangeInfo> &list);
    std::shared_ptr<Core> GetCore() const;
    std::shared_ptr<Telephony::ISimManager> GetSimManager() const;
    std::shared_ptr<Telephony::ISimSmsManager> GetSimSmsManager() const;

    static constexpr uint32_t RANG_MAX = 65535;
    std::list<gsmCBRangeInfo> rangeList_;
    std::condition_variable condVar_;
    std::mutex mutex_;
    std::string smscAddr_;
    std::string codeScheme_ {"0-255"};
    std::list<gsmCBRangeInfo> oldRangeList_;
    static constexpr uint8_t GSM_TYPE = 1;
    static constexpr uint8_t SMS_PDU_LEN_MIN = 6;
};
} // namespace Telephony
} // namespace OHOS
#endif