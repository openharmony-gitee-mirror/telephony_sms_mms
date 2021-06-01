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
#include "kit_short_message_hilog_wrap.h"
#include "napi_util.h"
namespace OHOS {
namespace TelephonyNapi {
SendCallback::SendCallback(bool hasCallback, napi_env env, napi_value thisVar, napi_ref callbackRef)
    : hasCallback_(hasCallback), env_(env), thisVar_(thisVar), callbackRef_(callbackRef)
{}

SendCallback::~SendCallback() {}

int32_t SendCallback::OnSmsSendResult(const SMS::ISendShortMessageCallback::SmsSendResult result)
{
    if (hasCallback_) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env_, callbackRef_, &callbackFunc);
        napi_value callbackValues[TWO] = {0};
        callbackValues[0] = NapiUtil::CreateUndefined(env_);
        napi_create_object(env_, &callbackValues[1]);
        SendSmsResult wrapResult = WrapSmsSendResult(result);
        napi_value sendResultValue = nullptr;
        napi_create_int32(env_, wrapResult, &sendResultValue);
        napi_set_named_property(env_, callbackValues[1], "result", sendResultValue);
        napi_value callbackResult = nullptr;
        int satatus = napi_call_function(env_, thisVar_, callbackFunc, TWO, callbackValues, &callbackResult);
        napi_close_handle_scope(env_, scope);
        return satatus;
    }
    return -1;
}

SendSmsResult SendCallback::WrapSmsSendResult(const SMS::ISendShortMessageCallback::SmsSendResult result)
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
} // namespace TelephonyNapi
} // namespace OHOS