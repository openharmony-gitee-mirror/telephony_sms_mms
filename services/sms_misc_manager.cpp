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

#include "sms_misc_manager.h"

#include <memory>

#include "hril_sms_parcel.h"
#include "short_message.h"
#include "string_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
SmsMiscManager::SmsMiscManager(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : AppExecFwk::EventHandler(runner), slotId_(slotId)
{}

bool SmsMiscManager::SetCBConfig(bool enable, uint32_t fromMsgId, uint32_t toMsgId, uint8_t netType)
{
    bool ret = false;
    if ((fromMsgId < 0) || (toMsgId > RANG_MAX) || (fromMsgId > toMsgId) || (netType != GSM_TYPE)) {
        return ret;
    }
    oldRangeList_ = rangeList_;
    if (enable) {
        ret = OpenCBRange(fromMsgId, toMsgId);
        if (!ret) {
            // if it is open,it will return true.
            return true;
        }
    } else {
        ret = CloseCBRange(fromMsgId, toMsgId);
        if (!ret) {
            // if it is close,it will return true.
            return true;
        }
    }
    if (ret) {
        ret = SendDataToRil(false, oldRangeList_);
        if (!ret) {
            rangeList_ = oldRangeList_;
            return ret;
        } else {
            oldRangeList_.clear();
        }
        ret = SendDataToRil(true, rangeList_);
        if (!ret) {
            rangeList_ = oldRangeList_;
            return ret;
        }
    }
    return ret;
}

std::list<SmsMiscManager::gsmCBRangeInfo> SmsMiscManager::GetRangeInfo() const
{
    return rangeList_;
}

// from 3GPP TS 27.005 3.3.4 Select Cell Broadcast Message Types
bool SmsMiscManager::OpenCBRange(uint32_t fromMsgId, uint32_t toMsgId)
{
    bool isMerge = false;
    uint32_t startPos = fromMsgId;
    uint32_t endPos = toMsgId;
    if (rangeList_.size() == 0) {
        rangeList_.emplace_back(fromMsgId, toMsgId);
        return true;
    }
    auto iter = rangeList_.begin();
    while (iter != rangeList_.end()) {
        auto oldIter = iter++;
        auto &info = *oldIter;
        if (fromMsgId >= info.fromMsgId && toMsgId <= info.toMsgId) {
            return false;
        } else if (!isMerge) {
            if (static_cast<int32_t>(toMsgId) < static_cast<int32_t>(info.fromMsgId) - 1) {
                break;
            } else if (static_cast<int32_t>(toMsgId) == static_cast<int32_t>(info.fromMsgId) - 1) {
                endPos = info.toMsgId;
                rangeList_.erase(oldIter);
                break;
            } else if (toMsgId <= info.toMsgId) {
                endPos = info.toMsgId;
                rangeList_.erase(oldIter);
                break;
            } else if ((static_cast<int32_t>(toMsgId) == static_cast<int32_t>(info.fromMsgId) - 1) ||
                (toMsgId == info.fromMsgId)) {
                endPos = info.toMsgId;
                rangeList_.erase(oldIter);
            } else if (((fromMsgId >= info.fromMsgId && fromMsgId <= info.toMsgId && toMsgId > info.toMsgId) ||
                (static_cast<int32_t>(fromMsgId) - 1 == static_cast<int32_t>(info.toMsgId)) ||
                ((fromMsgId < info.fromMsgId) && (toMsgId > info.toMsgId)))) {
                isMerge = true;
                startPos = (startPos < info.fromMsgId) ? startPos : info.fromMsgId;
                rangeList_.erase(oldIter);
            }
        } else {
            if (static_cast<int32_t>(toMsgId) < static_cast<int32_t>(info.fromMsgId) - 1) {
                endPos = toMsgId;
                break;
            } else if (static_cast<int32_t>(toMsgId) == static_cast<int32_t>(info.fromMsgId) - 1) {
                endPos = info.toMsgId;
                rangeList_.erase(oldIter);
                break;
            } else if (toMsgId >= info.fromMsgId && toMsgId <= info.toMsgId) {
                endPos = info.toMsgId;
                rangeList_.erase(oldIter);
                break;
            } else if (toMsgId > info.toMsgId) {
                rangeList_.erase(oldIter);
            }
        }
    }
    rangeList_.emplace_back(startPos, endPos);
    rangeList_.sort();
    return true;
}

// from 3GPP TS 27.005 3.3.4 Select Cell Broadcast Message Types
bool SmsMiscManager::CloseCBRange(uint32_t fromMsgId, uint32_t toMsgId)
{
    bool isChange = false;
    auto iter = rangeList_.begin();
    while (iter != rangeList_.end()) {
        auto oldIter = iter++;
        auto &info = *oldIter;
        if (fromMsgId >= info.fromMsgId && toMsgId <= info.toMsgId) {
            isChange = true;
            if (info.fromMsgId == fromMsgId && info.toMsgId == toMsgId) {
                rangeList_.erase(oldIter);
                break;
            } else if (info.fromMsgId == fromMsgId && info.toMsgId != toMsgId) {
                rangeList_.emplace_back(toMsgId + 1, info.toMsgId);
                rangeList_.erase(oldIter);
                break;
            } else if (info.fromMsgId != fromMsgId && info.toMsgId == toMsgId) {
                rangeList_.emplace_back(info.fromMsgId, fromMsgId - 1);
                rangeList_.erase(oldIter);
                break;
            } else if (fromMsgId > info.fromMsgId && toMsgId < info.toMsgId) {
                rangeList_.emplace_back(info.fromMsgId, fromMsgId - 1);
                rangeList_.emplace_back(toMsgId + 1, info.toMsgId);
                rangeList_.erase(oldIter);
                break;
            }
        } else if (fromMsgId <= info.fromMsgId && toMsgId >= info.toMsgId && !isChange) {
            isChange = true;
            rangeList_.erase(oldIter);
        } else if (isChange && toMsgId >= info.toMsgId) {
            rangeList_.erase(oldIter);
        } else if (isChange && toMsgId < info.toMsgId && toMsgId >= info.fromMsgId) {
            rangeList_.emplace_back(toMsgId + 1, info.toMsgId);
            rangeList_.erase(oldIter);
            rangeList_.sort();
            break;
        } else if ((fromMsgId > info.fromMsgId && fromMsgId < info.toMsgId) && toMsgId >= info.toMsgId &&
            !isChange) {
            isChange = true;
            rangeList_.emplace_back(info.fromMsgId, fromMsgId - 1);
            rangeList_.erase(oldIter);
            rangeList_.sort();
        } else if (fromMsgId == info.toMsgId && toMsgId >= info.toMsgId && !isChange) {
            isChange = true;
            rangeList_.emplace_back(info.fromMsgId, fromMsgId - 1);
            rangeList_.erase(oldIter);
            rangeList_.sort();
        }
    }
    if (isChange) {
        rangeList_.sort();
        return true;
    } else {
        return false;
    }
}

void SmsMiscManager::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager ProcessEvent event == nullptr");
        return;
    }

    int eventId = 0;
    TELEPHONY_LOGD("SmsMiscManager::ProcessEvent");
    eventId = event->GetInnerEventId();
    switch (eventId) {
        case SET_CB_CONFIG_FINISH: {
            std::unique_lock<std::mutex> lock(mutex_);
            isSuccess_ = true;
            std::shared_ptr<HRilRadioResponseInfo> res = event->GetSharedObject<HRilRadioResponseInfo>();
            if (res != nullptr) {
                isSuccess_ = (res->error == HRilErrType::NONE);
            }
            NotifyHasResponse();
            break;
        }
        case SET_SMSC_ADDR_FINISH: {
            std::unique_lock<std::mutex> lock(mutex_);
            isSuccess_ = true;
            std::shared_ptr<HRilRadioResponseInfo> res = event->GetSharedObject<HRilRadioResponseInfo>();
            if (res != nullptr) {
                isSuccess_ = (res->error == HRilErrType::NONE);
            }
            NotifyHasResponse();
            break;
        }
        case GET_SMSC_ADDR_FINISH: {
            std::unique_lock<std::mutex> lock(mutex_);
            std::shared_ptr<ServiceCenterAddress> addr = event->GetSharedObject<ServiceCenterAddress>();
            if (addr != nullptr) {
                smscAddr_ = addr->address;
            }
            NotifyHasResponse();
            break;
        }
        default:
            TELEPHONY_LOGI("SmsMiscManager::ProcessEvent default!");
            break;
    }
}

bool SmsMiscManager::IsEmpty() const
{
    return rangeList_.empty();
}

void SmsMiscManager::NotifyHasResponse()
{
    if (fairList_.size() > 0) {
        fairVar_ = fairList_.front();
        fairList_.pop_front();
    } else {
        return;
    }
    condVar_.notify_all();
}

std::string SmsMiscManager::RangeListToString(const std::list<gsmCBRangeInfo> &rangeList)
{
    std::string ret;
    bool isFirst = true;
    for (auto &item : rangeList) {
        if (isFirst) {
            isFirst = false;
        } else {
            ret += ",";
        }
        if (item.fromMsgId == item.toMsgId) {
            ret += std::to_string(item.fromMsgId);
        } else {
            ret += std::to_string(item.fromMsgId) + "-" + std::to_string(item.toMsgId);
        }
    }
    return ret;
}

bool SmsMiscManager::SendDataToRil(bool enable, std::list<gsmCBRangeInfo> &list)
{
    for (auto &item : list) {
        TELEPHONY_LOGI("[%{public}d-%{public}d]", item.fromMsgId, item.toMsgId);
    }
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<Core> core = GetCore();
    if (core != nullptr) {
        if (!list.empty()) {
            isSuccess_ = false;
            auto reply = AppExecFwk::InnerEvent::Get(SmsMiscManager::SET_CB_CONFIG_FINISH);
            reply->SetOwner(shared_from_this());
            int32_t condition = conditonVar_++;
            fairList_.push_back(condition);
            core->SetCellBroadcast(enable ? 0 : 1, RangeListToString(list), codeScheme_, reply);
            condVar_.wait(lock, [&]() { return fairVar_ == condition; });
            return isSuccess_;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool SmsMiscManager::AddSimMessage(
    const std::string &smsc, const std::string &pdu, ISmsServiceInterface::SimMessageStatus status)
{
    TELEPHONY_LOGI(
        "smscLen = %{public}zu pudLen = %{public}zu status = %{public}d", smsc.size(), pdu.size(), status);
    std::shared_ptr<Telephony::ISimSmsManager> simSmsManager = GetSimSmsManager();
    if (simSmsManager == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::AddSimMessage simSmsManager nullptr");
        return false;
    }

    std::string smscAddr(smsc);
    if (smsc.empty()) {
        smscAddr.insert(0, "00");
    }
    return simSmsManager->AddSmsToIcc(static_cast<int>(status), const_cast<std::string &>(pdu), smscAddr);
}

bool SmsMiscManager::DelSimMessage(uint32_t msgIndex)
{
    TELEPHONY_LOGI("messageIndex = %{public}d", msgIndex);
    std::shared_ptr<Telephony::ISimSmsManager> simSmsManager = GetSimSmsManager();
    if (simSmsManager == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::DelSimMessage simSmsManager nullptr");
        return false;
    }
    return simSmsManager->DelSmsIcc(msgIndex);
}

bool SmsMiscManager::UpdateSimMessage(uint32_t msgIndex, ISmsServiceInterface::SimMessageStatus newStatus,
    const std::string &pdu, const std::string &smsc)
{
    std::shared_ptr<Telephony::ISimSmsManager> simSmsManager = GetSimSmsManager();
    if (simSmsManager == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::UpdateSimMessage simSmsManager nullptr");
        return false;
    }

    std::string smscAddr(smsc);
    if (smsc.empty()) {
        smscAddr.insert(0, "00");
    }
    return simSmsManager->RenewSmsIcc(
        msgIndex, static_cast<int>(newStatus), const_cast<std::string &>(pdu), smscAddr);
}

std::vector<ShortMessage> SmsMiscManager::GetAllSimMessages()
{
    std::vector<ShortMessage> ret;
    TELEPHONY_LOGI("GetAllSimMessages");
    std::shared_ptr<Telephony::ISimSmsManager> simSmsManager = GetSimSmsManager();
    if (simSmsManager == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::GetAllSimMessages simSmsManager nullptr");
        return ret;
    }
    std::vector<std::string> pdus = simSmsManager->ObtainAllSmsOfIcc();
    for (auto &v : pdus) {
        std::vector<unsigned char> pdu = StringUtils::HexToByteVector(v);
        ret.emplace_back(ShortMessage::CreateIccMessage(pdu, "3gpp"));
    }
    return ret;
}

bool SmsMiscManager::SetSmscAddr(const std::string &scAddr)
{
    TELEPHONY_LOGI("SmsMiscManager::SetSmscAddr [%{public}s]", scAddr.c_str());
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<Core> core = GetCore();
    if (core == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::GetSmscAddr core nullptr");
        return false;
    }
    isSuccess_ = false;
    int32_t condition = conditonVar_++;
    fairList_.push_back(condition);
    auto reply = AppExecFwk::InnerEvent::Get(SmsMiscManager::SET_SMSC_ADDR_FINISH);
    reply->SetOwner(shared_from_this());
    core->SetSmsCenterAddress(0, scAddr, reply);
    condVar_.wait(lock, [&]() { return fairVar_ == condition; });
    return isSuccess_;
}

std::string SmsMiscManager::GetSmscAddr()
{
    TELEPHONY_LOGI("SmsMiscManager::GetSmscAddr");
    std::unique_lock<std::mutex> lock(mutex_);
    smscAddr_.clear();
    std::shared_ptr<Core> core = GetCore();
    if (core == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::GetSmscAddr core nullptr");
        return "";
    }
    isSuccess_ = false;
    int32_t condition = conditonVar_++;
    fairList_.push_back(condition);
    auto reply = AppExecFwk::InnerEvent::Get(SmsMiscManager::GET_SMSC_ADDR_FINISH);
    reply->SetOwner(shared_from_this());
    core->GetSmsCenterAddress(reply);
    condVar_.wait(lock, [&]() { return fairVar_ == condition; });
    return smscAddr_;
}

bool SmsMiscManager::SetDefaultSmsSlotId(int32_t slotId)
{
    TELEPHONY_LOGI("SetDefaultSmsSlotId slotId = %{public}d", slotId);
    std::shared_ptr<Telephony::ISimManager> simManager = GetSimManager();
    if (simManager == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::SetDefaultSmsSlotId simManager nullptr");
        return false;
    }
    return simManager->SetDefaultSmsSlotId(slotId);
}

int32_t SmsMiscManager::GetDefaultSmsSlotId()
{
    int32_t result = -1;
    TELEPHONY_LOGI("SetDefaultSmsSlotId");
    std::shared_ptr<Telephony::ISimManager> simManager = GetSimManager();
    if (simManager == nullptr) {
        TELEPHONY_LOGE("SmsMiscManager::SetDefaultSmsSlotId simManager nullptr");
        return result;
    }
    return simManager->GetDefaultSmsSlotId();
}

std::shared_ptr<Core> SmsMiscManager::GetCore() const
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        return core;
    }
    return nullptr;
}

std::shared_ptr<Telephony::ISimManager> SmsMiscManager::GetSimManager() const
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        return core->GetSimManager();
    }
    return nullptr;
}

std::shared_ptr<Telephony::ISimSmsManager> SmsMiscManager::GetSimSmsManager() const
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        return core->GetSimSmsManager();
    }
    return nullptr;
}
} // namespace Telephony
} // namespace OHOS
