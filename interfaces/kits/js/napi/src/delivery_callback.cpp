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

#include "delivery_callback.h"
#include "kit_short_message_hilog_wrap.h"
#include "napi_util.h"
namespace OHOS {
namespace TelephonyNapi {
constexpr int32_t DEFAULT_ERR = -1;
constexpr int32_t ZERO = 0;
constexpr int32_t ONE = 1;
constexpr int32_t TWO = 2;
constexpr uint32_t UINT32_ZERO = 0;

DeliveryCallback::DeliveryCallback(bool hasCallback, napi_env env, napi_value thisVar, napi_ref callbackRef)
    : hasCallback_(hasCallback), env_(env), thisVar_(thisVar), callbackRef_(callbackRef)
{}

DeliveryCallback::~DeliveryCallback()
{
    env_ = nullptr;
    thisVar_ = nullptr;
    callbackRef_ = nullptr;
}

int32_t DeliveryCallback::OnSmsDeliveryResult(const std::u16string pdu)
{
    HILOG_DEBUG("OnSmsDeliveryResult start hasCallback_ = %{public}d", hasCallback_);
    if (hasCallback_) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env_, callbackRef_, &callbackFunc);
        napi_value callbackValues[TWO] = {0};
        if (!pdu.empty()) {
            std::string pdu8 = NapiUtil::ToUtf8(pdu);
            HILOG_DEBUG("OnSmsDeliveryResult pud not empty pdu = %{public}s", pdu8.c_str());
            callbackValues[ZERO] = NapiUtil::CreateUndefined(env_);
            napi_create_object(env_, &callbackValues[ONE]);
            size_t dataSize = pdu8.size();
            uint32_t forDataSize = (uint32_t)dataSize;
            napi_value arrayValue = nullptr;
            napi_create_array(env_, &arrayValue);
            for (uint32_t i = UINT32_ZERO; i < forDataSize; ++i) {
                napi_value element = nullptr;
                int32_t intValue = pdu8[i];
                napi_create_int32(env_, intValue, &element);
                napi_set_element(env_, arrayValue, i, element);
            }
            std::string pduStr = "pdu";
            napi_set_named_property(env_, callbackValues[1], pduStr.c_str(), arrayValue);
        } else {
            callbackValues[ZERO] = NapiUtil::CreateErrorMessage(env_, "pdu empty");
            callbackValues[ONE] = NapiUtil::CreateUndefined(env_);
        }
        napi_value callbackResult = nullptr;
        int status = napi_call_function(env_, thisVar_, callbackFunc, TWO, callbackValues, &callbackResult);
        HILOG_DEBUG("OnSmsDeliveryResult napi_call_function status = %{public}d", status);
        napi_close_handle_scope(env_, scope);
        return status;
    }
    return DEFAULT_ERR;
}
} // namespace TelephonyNapi
} // namespace OHOS