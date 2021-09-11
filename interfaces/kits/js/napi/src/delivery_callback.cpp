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
#include "napi_util.h"

namespace OHOS {
namespace Telephony {
DeliveryCallback::DeliveryCallback(bool hasCallback, napi_env env, napi_ref thisVarRef, napi_ref callbackRef)
    : hasCallback_(hasCallback), env_(env), thisVarRef_(thisVarRef), callbackRef_(callbackRef)
{}

DeliveryCallback::~DeliveryCallback()
{
    env_ = nullptr;
    thisVarRef_ = nullptr;
    callbackRef_ = nullptr;
}

void DeliveryCallback::OnSmsDeliveryResult(const std::u16string pdu)
{
    if (hasCallback_) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env_, callbackRef_, &callbackFunc);
        napi_value callbackValues[2] = {0};
        if (!pdu.empty()) {
            std::string pdu8 = NapiUtil::ToUtf8(pdu);
            callbackValues[0] = NapiUtil::CreateUndefined(env_);
            napi_create_object(env_, &callbackValues[1]);
            size_t dataSize = pdu8.size();
            uint32_t forDataSize = (uint32_t)dataSize;
            napi_value arrayValue = nullptr;
            napi_create_array(env_, &arrayValue);
            for (uint32_t i = 0; i < forDataSize; ++i) {
                napi_value element = nullptr;
                int32_t intValue = pdu8[i];
                napi_create_int32(env_, intValue, &element);
                napi_set_element(env_, arrayValue, i, element);
            }
            std::string pduStr = "pdu";
            napi_set_named_property(env_, callbackValues[1], pduStr.c_str(), arrayValue);
        } else {
            callbackValues[0] = NapiUtil::CreateErrorMessage(env_, "pdu empty");
            callbackValues[1] = NapiUtil::CreateUndefined(env_);
        }
        napi_value callbackResult = nullptr;
        napi_value thisVar = nullptr;
        napi_get_reference_value(env_, thisVarRef_, &thisVar);
        napi_call_function(env_, thisVar, callbackFunc, 2, callbackValues, &callbackResult);
        napi_delete_reference(env_, thisVarRef_);
        napi_delete_reference(env_, callbackRef_);
        napi_close_handle_scope(env_, scope);
    }
}
} // namespace Telephony
} // namespace OHOS