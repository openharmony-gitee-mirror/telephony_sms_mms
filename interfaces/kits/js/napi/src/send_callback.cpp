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

#include "send_callback.h"
#include "telephony_log_wrapper.h"
#include "napi_util.h"

namespace OHOS {
namespace Telephony {
SendCallback::SendCallback(bool hasCallback, napi_env env, napi_ref thisVarRef, napi_ref callbackRef)
    : hasCallback_(hasCallback), env_(env), thisVarRef_(thisVarRef), callbackRef_(callbackRef)
{}

SendCallback::~SendCallback()
{
    env_ = nullptr;
    thisVarRef_ = nullptr;
    callbackRef_ = nullptr;
}

void SendCallback::OnSmsSendResult(const ISendShortMessageCallback::SmsSendResult result)
{
    TELEPHONY_LOGD("OnSmsSendResult hasCallback_ = %d", hasCallback_);
    if (hasCallback_) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env_, callbackRef_, &callbackFunc);
        napi_value callbackValues[2] = {0};
        callbackValues[0] = NapiUtil::CreateUndefined(env_);
        napi_create_object(env_, &callbackValues[1]);
        SendSmsResult wrapResult = WrapSmsSendResult(result);
        napi_value sendResultValue = nullptr;
        napi_create_int32(env_, wrapResult, &sendResultValue);
        napi_set_named_property(env_, callbackValues[1], "result", sendResultValue);
        napi_value callbackResult = nullptr;
        napi_value thisVar = nullptr;
        napi_get_reference_value(env_, thisVarRef_, &thisVar);
        napi_call_function(env_, thisVar, callbackFunc, 2, callbackValues, &callbackResult);
        TELEPHONY_LOGD("OnSmsSendResult napi_call_function satatus");
        napi_delete_reference(env_, callbackRef_);
        napi_close_handle_scope(env_, scope);
    }
}

SendSmsResult SendCallback::WrapSmsSendResult(const ISendShortMessageCallback::SmsSendResult result)
{
    switch (result) {
        case ISendShortMessageCallback::SmsSendResult::SEND_SMS_SUCCESS: {
            return SendSmsResult::SEND_SMS_SUCCESS;
        }
        case ISendShortMessageCallback::SmsSendResult::SEND_SMS_FAILURE_RADIO_OFF: {
            return SendSmsResult::SEND_SMS_FAILURE_RADIO_OFF;
        }
        case ISendShortMessageCallback::SmsSendResult::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE: {
            return SendSmsResult::SEND_SMS_FAILURE_SERVICE_UNAVAILABLE;
        }
        default: {
            return SendSmsResult::SEND_SMS_FAILURE_UNKNOWN;
        }
    }
}
} // namespace Telephony
} // namespace OHOS