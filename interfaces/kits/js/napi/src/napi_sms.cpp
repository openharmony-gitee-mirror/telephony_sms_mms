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

#include "napi_sms.h"

#include <cstring>
#include <memory>

#include "refbase.h"

#include "delivery_callback.h"
#include "send_callback.h"
#include "telephony_log_wrapper.h"
#include "short_message.h"
#include "napi_util.h"
#include "short_message_manager.h"

namespace OHOS {
namespace Telephony {
namespace {
const std::string slotIdStr = "slotId";
const std::string destinationHostStr = "destinationHost";
const std::string serviceCenterStr = "serviceCenter";
const std::string contentStr = "content";
const std::string destinationPortStr = "destinationPort";
const std::string sendCallbackStr = "sendCallback";
const std::string deliveryCallbackStr = "deliveryCallback";
std::unique_ptr<ShortMessageManager> g_shortMessageManager;
} // namespace
static bool IsShortMessageManagerInit()
{
    return g_shortMessageManager != nullptr && g_shortMessageManager->IsConnect();
}

static void SetPropertyArray(napi_env env, napi_value object, std::string name, std::vector<unsigned char> pdu)
{
    napi_value array = nullptr;
    napi_create_array(env, &array);
    int size = pdu.size();
    for (int i = 0; i < size; i++) {
        napi_value element = nullptr;
        napi_create_int32(env, pdu[i], &element);
        napi_set_element(env, array, i, element);
    }
    napi_set_named_property(env, object, name.c_str(), array);
}

static MessageStatus WrapSimMessageStatus(int32_t status)
{
    switch (status) {
        case MESSAGE_HAVE_READ: {
            return MESSAGE_HAVE_READ;
        }
        case MESSAGE_UNREAD: {
            return MESSAGE_UNREAD;
        }
        case MESSAGE_HAS_BEEN_SENT: {
            return MESSAGE_HAS_BEEN_SENT;
        }
        case MESSAGE_NOT_SENT: {
            return MESSAGE_NOT_SENT;
        }
        default: {
            return MESSAGE_UNKNOWN_STATUS;
        }
    }
}

static int32_t GetDefaultSmsSlotId()
{
    return 1;
}

static bool MatchObjectProperty(
    napi_env env, napi_value object, std::initializer_list<std::pair<std::string, napi_valuetype>> pairList)
{
    if (object == nullptr) {
        return false;
    }
    for (auto beg = pairList.begin(); beg != pairList.end(); ++beg) {
        if (!NapiUtil::HasNamedTypeProperty(env, object, beg->second, beg->first)) {
            return false;
        }
    }
    return true;
}

static std::string Get64StringFromValue(napi_env env, napi_value value)
{
    char msgChars[NORMAL_STRING_SIZE] = {0};
    size_t strLength = 0;
    napi_get_value_string_utf8(env, value, msgChars, NORMAL_STRING_SIZE, &strLength);
    TELEPHONY_LOGD("Get64StringFromValue strLength = %{public}zu", strLength);
    if (strLength > 0) {
        return std::string(msgChars, 0, strLength);
    } else {
        return "";
    }
}

static std::u16string GetU16StrFromNapiValue(napi_env env, napi_value value)
{
    char strChars[PROPERTY_NAME_SIZE] = {0};
    size_t strLength = 0;
    napi_get_value_string_utf8(env, value, strChars, BUFF_LENGTH, &strLength);
    std::string str8(strChars, strLength);
    return NapiUtil::ToUtf16(str8);
}

static bool ActuallySendMessage(napi_env env, SendMessageContext &parameter)
{
    if (parameter.slotId == 0) {
        TELEPHONY_LOGD("ActuallySendMessage parameter.slotId == 0 illegal slotId");
        return false;
    }
    if (parameter.destinationHost.empty()) {
        TELEPHONY_LOGD("ActuallySendMessage destinationHost empty illegal parameter");
        return false;
    }
    bool hasSendCallback = parameter.sendCallbackRef != nullptr;
    std::unique_ptr<SendCallback> sendCallback =
        std::make_unique<SendCallback>(hasSendCallback, env, parameter.thisVarRef, parameter.sendCallbackRef);
    if (sendCallback == nullptr) {
        return false;
    }
    bool hasDeliveryCallback = parameter.deliveryCallbackRef != nullptr;
    std::unique_ptr<DeliveryCallback> deliveryCallback = std::make_unique<DeliveryCallback>(
        hasDeliveryCallback, env, parameter.thisVarRef, parameter.deliveryCallbackRef);
    if (deliveryCallback == nullptr) {
        return false;
    }
    if (IsShortMessageManagerInit()) {
        if (parameter.messageType == TEXT_MESSAGE_PARAMETER_MATCH) {
            if (!parameter.textContent.empty()) {
                g_shortMessageManager->SendMessage(parameter.slotId, parameter.destinationHost,
                    parameter.serviceCenter, parameter.textContent, sendCallback.release(),
                    deliveryCallback.release());
                TELEPHONY_LOGD("NativeSendMessage SendTextMessage");
                return true;
            }
        } else if (parameter.messageType == RAW_DATA_MESSAGE_PARAMETER_MATCH) {
            if (parameter.rawDataContent.size() > 0) {
                uint16_t arrayLength = static_cast<uint16_t>(parameter.rawDataContent.size());
                g_shortMessageManager->SendMessage(parameter.slotId, parameter.destinationHost,
                    parameter.serviceCenter, parameter.destinationPort, &parameter.rawDataContent[0], arrayLength,
                    sendCallback.release(), deliveryCallback.release());
                TELEPHONY_LOGD("NativeSendMessage SendRawDataMessage");
                return true;
            }
        }
    }
    return false;
}

static void NativeSendMessage(napi_env env, void *data)
{
    auto asyncContext = static_cast<SendMessageContext *>(data);
    if (asyncContext != nullptr) {
        bool isNativeSendMessage = ActuallySendMessage(env, *asyncContext);
        asyncContext->resolved = isNativeSendMessage;
    }
}

static void SendMessageCallback(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<SendMessageContext *>(data);
    if (!asyncContext->resolved) {
        TELEPHONY_LOGD("SendMessageCallback status == REJECT");
        if (asyncContext->thisVarRef != nullptr) {
            TELEPHONY_LOGD("SendMessageCallback thisVarRef != nullptr");
            napi_value thisVar = nullptr;
            napi_get_reference_value(env, asyncContext->thisVarRef, &thisVar);
            if (asyncContext->sendCallbackRef != nullptr) {
                TELEPHONY_LOGD("SendMessageCallback sendCallbackRef != nullptr");
                napi_value sendCallback = nullptr;
                napi_get_reference_value(env, asyncContext->sendCallbackRef, &sendCallback);
                napi_value callbackValues[2] = {0};
                callbackValues[0] = NapiUtil::CreateErrorMessage(env, "parameter illegal");
                napi_get_undefined(env, &callbackValues[1]);
                napi_value undefined = nullptr;
                napi_get_undefined(env, &undefined);
                napi_value callbackResult = nullptr;
                napi_call_function(env, undefined, sendCallback, 2, callbackValues, &callbackResult);
                TELEPHONY_LOGD("SendMessageCallback after napi_call_function");
                napi_delete_reference(env, asyncContext->sendCallbackRef);
            }
            if (asyncContext->deliveryCallbackRef != nullptr) {
                napi_delete_reference(env, asyncContext->deliveryCallbackRef);
            }
            napi_delete_reference(env, asyncContext->thisVarRef);
        }
    }
    napi_delete_async_work(env, asyncContext->work);
}

static int32_t MatchSendMessageParameters(napi_env env, napi_value parameters[], size_t parameterCount)
{
    bool match = (parameterCount == 1) && NapiUtil::MatchParameters(env, parameters, {napi_object});
    if (match) {
        napi_value object = parameters[0];
        bool hasSlotId = NapiUtil::HasNamedTypeProperty(env, object, napi_number, slotIdStr);
        bool hasDestinationHost = NapiUtil::HasNamedTypeProperty(env, object, napi_string, destinationHostStr);
        bool hasContent = NapiUtil::HasNamedProperty(env, object, contentStr);
        bool hasNecessaryParameter = hasSlotId && hasDestinationHost && hasContent;
        if (hasNecessaryParameter) {
            napi_value contentValue = NapiUtil::GetNamedProperty(env, object, contentStr);
            bool contentIsStr = NapiUtil::MatchValueType(env, contentValue, napi_string);
            bool contentIsObj = NapiUtil::MatchValueType(env, contentValue, napi_object);
            bool contentIsArray = false;
            if (contentIsObj) {
                napi_is_array(env, contentValue, &contentIsArray);
            }
            bool serviceCenterTypeMatch =
                NapiUtil::MatchOptionPropertyType(env, object, napi_string, serviceCenterStr);
            bool sendCallbackTypeMatch =
                NapiUtil::MatchOptionPropertyType(env, object, napi_function, sendCallbackStr);
            bool deliveryCallbackTypeMatch =
                NapiUtil::MatchOptionPropertyType(env, object, napi_function, deliveryCallbackStr);
            if (contentIsStr && serviceCenterTypeMatch && sendCallbackTypeMatch && deliveryCallbackTypeMatch) {
                return TEXT_MESSAGE_PARAMETER_MATCH;
            } else if (contentIsArray) {
                bool destindationPortMatch =
                    NapiUtil::MatchOptionPropertyType(env, object, napi_number, destinationPortStr);
                if (serviceCenterTypeMatch && sendCallbackTypeMatch && deliveryCallbackTypeMatch &&
                    destindationPortMatch) {
                    return RAW_DATA_MESSAGE_PARAMETER_MATCH;
                }
            }
        }
    }
    return MESSAGE_PARAMETER_NOT_MATCH;
}

static void ParseMessageParameter(
    int32_t messageMatchResult, napi_env env, napi_value object, SendMessageContext &context)
{
    context.messageType = messageMatchResult;
    context.slotId = GetDefaultSmsSlotId();
    napi_value slotIdValue = nullptr;
    napi_get_named_property(env, object, slotIdStr.data(), &slotIdValue);
    napi_get_value_int32(env, slotIdValue, &context.slotId);
    napi_value destinationHostValue = NapiUtil::GetNamedProperty(env, object, destinationHostStr);
    context.destinationHost = GetU16StrFromNapiValue(env, destinationHostValue);
    if (NapiUtil::HasNamedProperty(env, object, serviceCenterStr)) {
        napi_value serviceCenterValue = NapiUtil::GetNamedProperty(env, object, serviceCenterStr);
        context.serviceCenter = GetU16StrFromNapiValue(env, serviceCenterValue);
    }
    if (NapiUtil::HasNamedProperty(env, object, sendCallbackStr)) {
        napi_value sendCallbackValue = NapiUtil::GetNamedProperty(env, object, sendCallbackStr);
        napi_create_reference(env, sendCallbackValue, 1, &context.sendCallbackRef);
    }
    if (NapiUtil::HasNamedProperty(env, object, deliveryCallbackStr)) {
        napi_value deliveryCallbackValue = NapiUtil::GetNamedProperty(env, object, deliveryCallbackStr);
        napi_create_reference(env, deliveryCallbackValue, 1, &context.deliveryCallbackRef);
    }
    napi_value contentValue = NapiUtil::GetNamedProperty(env, object, contentStr);
    if (messageMatchResult == TEXT_MESSAGE_PARAMETER_MATCH) {
        char contentChars[MAX_TEXT_SHORT_MESSAGE_LENGTH] = {0};
        size_t contentLength = 0;
        napi_get_value_string_utf8(env, contentValue, contentChars, MAX_TEXT_SHORT_MESSAGE_LENGTH, &contentLength);
        std::string text(contentChars, contentLength);
        context.textContent = NapiUtil::ToUtf16(text);
    }
    if (messageMatchResult == RAW_DATA_MESSAGE_PARAMETER_MATCH) {
        int32_t destinationPort = DEFAULT_PORT;
        napi_value destinationPortValue = nullptr;
        napi_get_named_property(env, object, destinationPortStr.data(), &destinationPortValue);
        napi_get_value_int32(env, destinationPortValue, &destinationPort);
        context.destinationPort = static_cast<uint16_t>(destinationPort);
        napi_value elementValue = nullptr;
        int32_t element = 0;
        uint32_t valueArraySize = 0;
        napi_get_array_length(env, contentValue, &valueArraySize);
        auto arraySize = static_cast<int32_t>(valueArraySize);
        for (int32_t i = 0; i < arraySize; i++) {
            napi_get_element(env, contentValue, i, &elementValue);
            napi_get_value_int32(env, elementValue, &element);
            context.rawDataContent.push_back((uint8_t)element);
        }
    }
}

static napi_value SendMessage(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[1] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    int32_t messageMatchResult = MatchSendMessageParameters(env, parameters, parameterCount);
    NAPI_ASSERT(env, messageMatchResult != MESSAGE_PARAMETER_NOT_MATCH, "type mismatch");
    auto asyncContext = new SendMessageContext();
    ParseMessageParameter(messageMatchResult, env, parameters[0], *asyncContext);
    napi_create_reference(env, thisVar, 1, &asyncContext->thisVarRef);
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "SendMessage", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env, nullptr, resourceName, NativeSendMessage, SendMessageCallback,
        (void *)asyncContext, &(asyncContext->work));
    napi_queue_async_work(env, asyncContext->work);
    return NapiUtil::CreateUndefined(env);
}

static void NativeCreateMessage(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeCreateMessage start");
    auto asyncContext = static_cast<CreateMessageContext *>(data);
    if (!asyncContext->specification.empty() && !asyncContext->pdu.empty()) {
        TELEPHONY_LOGD("NativeCreateMessage before CreateMessage");
        std::u16string specification16 = NapiUtil::ToUtf16(asyncContext->specification);
        asyncContext->shortMessage = ShortMessage::CreateMessage(asyncContext->pdu, specification16);
        if (asyncContext->shortMessage != nullptr) {
            TELEPHONY_LOGD("NativeCreateMessage CreateMessage success");
            asyncContext->resolved = true;
        } else {
            TELEPHONY_LOGD("NativeCreateMessage CreateMessage faied");
            asyncContext->resolved = false;
        }
    } else {
        asyncContext->resolved = false;
    }
    TELEPHONY_LOGD("NativeCreateMessage end");
}

static napi_value CreateShortMessageValue(napi_env env, const ShortMessage *shortMessage)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    NapiUtil::SetPropertyStringUtf8(
        env, object, "visibleMessageBody", NapiUtil::ToUtf8(shortMessage->GetVisibleMessageBody()));
    NapiUtil::SetPropertyStringUtf8(
        env, object, "visibleRawAddress", NapiUtil::ToUtf8(shortMessage->GetVisibleRawAddress()));
    NapiUtil::SetPropertyInt32(env, object, "messageClass", shortMessage->GetMessageClass());
    NapiUtil::SetPropertyInt32(env, object, "protocolId", shortMessage->GetProtocolId());
    NapiUtil::SetPropertyStringUtf8(env, object, "scAddress", NapiUtil::ToUtf8(shortMessage->GetScAddress()));
    NapiUtil::SetPropertyInt32(env, object, "scTimestamp", shortMessage->GetScTimestamp());
    NapiUtil::SetPropertyBoolean(env, object, "isReplaceMessage", shortMessage->IsReplaceMessage());
    NapiUtil::SetPropertyBoolean(env, object, "hasReplyPath", shortMessage->HasReplyPath());
    SetPropertyArray(env, object, "pdu", shortMessage->GetPdu());
    NapiUtil::SetPropertyInt32(env, object, "status", shortMessage->GetStatus());
    NapiUtil::SetPropertyBoolean(env, object, "isSmsStatusReportMessage", shortMessage->IsSmsStatusReportMessage());
    NapiUtil::SetPropertyInt32(env, object, "simMessageStatus", shortMessage->GetIccMessageStatus());
    return object;
}

static void CreateMessageCallback(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<CreateMessageContext *>(data);
    if (status == napi_ok) {
        if (asyncContext->resolved) {
            napi_value callbackValue = CreateShortMessageValue(env, asyncContext->shortMessage);
            NapiUtil::Handle2ValueCallback(env, asyncContext, callbackValue);
        } else {
            NapiUtil::Handle2ValueCallback(
                env, asyncContext, NapiUtil::CreateErrorMessage(env, "create message error"));
        }
    } else {
        NapiUtil::Handle2ValueCallback(env, asyncContext,
            NapiUtil::CreateErrorMessage(env, "create message error,cause napi_status = " + std::to_string(status)));
    }
}

static bool MatchCreateMessageParameter(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    bool typeMatch = false;
    switch (parameterCount) {
        case 2: {
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object, napi_string});
            break;
        }
        case 3:
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object, napi_string, napi_function});
            break;
        default: {
            return false;
        }
    }
    if (typeMatch) {
        bool isArray = false;
        napi_is_array(env, parameters[0], &isArray);
        return isArray;
    }
    return false;
}

static napi_value CreateMessage(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchCreateMessageParameter(env, parameters, parameterCount), "type mismatch");
    auto asyncContext = new CreateMessageContext();
    asyncContext->specification = Get64StringFromValue(env, parameters[1]);
    TELEPHONY_LOGD("CreateMessage specification = %{public}s", asyncContext->specification.c_str());
    uint32_t arrayLength = 0;
    napi_get_array_length(env, parameters[0], &arrayLength);
    napi_value elementValue = nullptr;
    int32_t element = 0;
    auto arraySize = static_cast<int32_t>(arrayLength);
    for (int32_t i = 0; i < arraySize; i++) {
        napi_get_element(env, parameters[0], i, &elementValue);
        napi_get_value_int32(env, elementValue, &element);
        asyncContext->pdu.push_back((unsigned char)element);
    }
    TELEPHONY_LOGD("CreateMessage pdu size = %{public}zu", asyncContext->pdu.size());
    if (parameterCount == 3) {
        napi_create_reference(env, parameters[2], 1, &(asyncContext->callbackRef));
    }
    return NapiUtil::HandleAsyncWork(
        env, asyncContext, "CreateMessage", NativeCreateMessage, CreateMessageCallback);
}

static bool MatchSetDefaultSmsSlotIdParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case 1: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number});
        }
        case 2:
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_function});
        default: {
            return false;
        }
    }
}

static void NativeSetDefaultSmsSlotId(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeSetDefaultSmsSlotId start");
    auto context = static_cast<SetDefaultSmsSlotIdContext *>(data);
    TELEPHONY_LOGD("NativeSetDefaultSmsSlotId start 1");
    if (IsShortMessageManagerInit()) {
        TELEPHONY_LOGD("NativeSetDefaultSmsSlotId start 2");
        context->setResult = g_shortMessageManager->SetDefaultSmsSlotId(context->slotId);
        TELEPHONY_LOGD("NativeSetDefaultSmsSlotId context->setResult = %{public}d", context->setResult);
        context->resolved = true;
    } else {
        context->resolved = false;
    }
    TELEPHONY_LOGD("NativeSetDefaultSmsSlotId end");
}

static void SetDefaultSmsSlotIdCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<SetDefaultSmsSlotIdContext *>(data);
    TELEPHONY_LOGD("SetDefaultSmsSlotIdCallback status = %{public}d", status);
    if (status == napi_ok) {
        if (context->resolved) {
            napi_value callbackValue = nullptr;
            napi_get_boolean(env, context->setResult, &callbackValue);
            if (context->callbackRef != nullptr) {
                napi_value recvValue = NapiUtil::CreateUndefined(env);
                napi_value callbackFunc = nullptr;
                napi_get_reference_value(env, context->callbackRef, &callbackFunc);
                napi_value callbackValues[2] = {NapiUtil::CreateUndefined(env), callbackValue};
                napi_value resultValue = nullptr;
                napi_call_function(env, recvValue, callbackFunc, 2, callbackValues, &resultValue);
            } else {
                napi_resolve_deferred(env, context->deferred, callbackValue);
            }
        } else {
            NapiUtil::Handle2ValueCallback(
                env, context, NapiUtil::CreateErrorMessage(env, "set default sms slot id error"));
        }
    } else {
        NapiUtil::Handle2ValueCallback(env, context,
            NapiUtil::CreateErrorMessage(
                env, "set default sms slot id error cause napi_status = " + std::to_string(status)));
    }
}

static napi_value SetDefaultSmsSlotId(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGD("SetDefaultSmsSlotId start");
    size_t parameterCount = 2;
    napi_value parameters[2] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    TELEPHONY_LOGD("SetDefaultSmsSlotId start 1");
    NAPI_ASSERT(env, MatchSetDefaultSmsSlotIdParameters(env, parameters, parameterCount), "type mismatch");
    TELEPHONY_LOGD("SetDefaultSmsSlotId start 2");
    auto context = new SetDefaultSmsSlotIdContext();
    napi_get_value_int32(env, parameters[0], &context->slotId);
    TELEPHONY_LOGD("SetDefaultSmsSlotId start 3");
    if (parameterCount == 2) {
        TELEPHONY_LOGD("SetDefaultSmsSlotId start 4");
        napi_create_reference(env, parameters[1], 1, &context->callbackRef);
        TELEPHONY_LOGD("SetDefaultSmsSlotId start 5");
    }
    napi_value result = nullptr;
    if (context->callbackRef == nullptr) {
        napi_create_promise(env, &context->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "SetDefaultSmsSlotId", NAPI_AUTO_LENGTH, &resourceName);
    TELEPHONY_LOGD("SetDefaultSmsSlotId start 6");
    napi_create_async_work(env, nullptr, resourceName, NativeSetDefaultSmsSlotId, SetDefaultSmsSlotIdCallback,
        (void *)context, &(context->work));
    TELEPHONY_LOGD("SetDefaultSmsSlotId start 7");
    napi_queue_async_work(env, context->work);
    TELEPHONY_LOGD("SetDefaultSmsSlotId start 8");
    return result;
}

static bool MatchGetDefaultSmsSlotIdParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case 0: {
            return true;
        }
        case 1: {
            return NapiUtil::MatchParameters(env, parameters, {napi_function});
        }
        default: {
            return false;
        }
    }
}

static void NativeGetDefaultSmsSlotId(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeGetDefaultSmsSlotId start ");
    auto context = static_cast<GetDefaultSmsSlotIdContext *>(data);
    TELEPHONY_LOGD("NativeGetDefaultSmsSlotId start 1");
    if (IsShortMessageManagerInit()) {
        TELEPHONY_LOGD("NativeGetDefaultSmsSlotId start 2");
        context->defaultSmsSlotId = g_shortMessageManager->GetDefaultSmsSlotId();
        TELEPHONY_LOGD(
            "NativeGetDefaultSmsSlotId start context->defaultSmsSlotId  = %{public}d", context->defaultSmsSlotId);
        if (context->defaultSmsSlotId > ShortMessageManager::ERROR_SERVICE_NOT_AVAILABLE) {
            context->resolved = true;
        } else {
            context->resolved = false;
        }
    } else {
        context->resolved = false;
    }
    TELEPHONY_LOGD("NativeGetDefaultSmsSlotId start 3");
}

static void GetDefaultSmsSlotIdCallback(napi_env env, napi_status status, void *data)
{
    TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback start");
    auto context = static_cast<GetDefaultSmsSlotIdContext *>(data);
    TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback 1");
    if (status == napi_ok) {
        TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback 2");
        if (context->resolved) {
            TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback 3");
            napi_value callbackValue = nullptr;
            napi_create_int32(env, context->defaultSmsSlotId, &callbackValue);
            TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback 4");
            NapiUtil::Handle2ValueCallback(env, context, callbackValue);
        } else {
            TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback 5");
            NapiUtil::Handle2ValueCallback(
                env, context, NapiUtil::CreateErrorMessage(env, "get default sms slot id error"));
        }
    } else {
        TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback 6");
        NapiUtil::Handle2ValueCallback(env, context,
            NapiUtil::CreateErrorMessage(
                env, "get default sms slot id error cause napi_status = " + std::to_string(status)));
        TELEPHONY_LOGD("GetDefaultSmsSlotIdCallback 7");
    }
}

static napi_value GetDefaultSmsSlotId(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGD("GetDefaultSmsSlotId start");
    size_t parameterCount = 1;
    napi_value parameters[1] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    TELEPHONY_LOGD("GetDefaultSmsSlotId start 1");
    NAPI_ASSERT(env, MatchGetDefaultSmsSlotIdParameters(env, parameters, parameterCount), "type mismatch");
    auto context = new GetDefaultSmsSlotIdContext();
    napi_status statusValue = napi_get_value_int32(env, parameters[0], &context->defaultSmsSlotId);
    TELEPHONY_LOGD("GetDefaultSmsSlotId statusValue = %{public}d", statusValue);
    if (parameterCount == 1) {
        napi_create_reference(env, parameters[0], 1, &context->callbackRef);
    }
    napi_value result = NapiUtil::HandleAsyncWork(
        env, context, "SetDefaultSmsSlotId", NativeGetDefaultSmsSlotId, GetDefaultSmsSlotIdCallback);
    TELEPHONY_LOGD("GetDefaultSmsSlotId end");
    return result;
}

static bool MatchSetSmscAddrParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case 2: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_string});
        }
        case 3: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_string, napi_function});
        }
        default: {
            return false;
        }
    }
}

static void NativeSetSmscAddr(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeSetSmscAddr start ");
    auto context = static_cast<SetSmscAddrContext *>(data);
    TELEPHONY_LOGD("NativeSetSmscAddr start 1");
    if (IsShortMessageManagerInit()) {
        context->setResult =
            g_shortMessageManager->SetScAddress(context->slotId, NapiUtil::ToUtf16(context->smscAddr));
        TELEPHONY_LOGD("NativeSetSmscAddr setResult = %{public}d", context->setResult);
        context->resolved = true;
    } else {
        context->resolved = false;
    }
}
static void SetSmscAddrCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<SetSmscAddrContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "set smsc addr error by ipc");
        }
    } else {
        callbackValue =
            NapiUtil::CreateErrorMessage(env, "set smsc addr error napi_status = " + std::to_string(status));
    }
    NapiUtil::Handle1ValueCallback(env, context, callbackValue);
}

static napi_value SetSmscAddr(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGD("SetSmscAddr start");
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchSetSmscAddrParameters(env, parameters, parameterCount), "type mismatch");
    TELEPHONY_LOGD("SetSmscAddr start after MatchSetSmscAddrParameters");
    auto context = new SetSmscAddrContext();
    TELEPHONY_LOGD("SetSmscAddr start after SetSmscAddrContext contruct");
    napi_get_value_int32(env, parameters[0], &context->slotId);
    context->smscAddr = Get64StringFromValue(env, parameters[1]);
    TELEPHONY_LOGD("SetSmscAddr smscAddr = %{publc}s", context->smscAddr.data());
    if (parameterCount == 3) {
        napi_create_reference(env, parameters[2], 1, &context->callbackRef);
    }
    TELEPHONY_LOGD("SetSmscAddr before end");
    return NapiUtil::HandleAsyncWork(env, context, "SetSmscAddr", NativeSetSmscAddr, SetSmscAddrCallback);
}

static bool MatchGetSmscAddrParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case 1: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number});
        }
        case 2: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_function});
        }
        default: {
            return false;
        }
    }
}
static void NativeGetSmscAddr(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeGetSmscAddr start ");
    auto context = static_cast<GetSmscAddrContext *>(data);
    if (IsShortMessageManagerInit()) {
        TELEPHONY_LOGD("NativeGetSmscAddr IsShortMessageManagerInit true");
        context->smscAddr = NapiUtil::ToUtf8(g_shortMessageManager->GetScAddress(context->slotId));
        TELEPHONY_LOGD("NativeGetSmscAddr smscAddr = %{public}s", context->smscAddr.data());
        context->resolved = true;
    } else {
        context->resolved = false;
    }
}

static void GetSmscAddrCallback(napi_env env, napi_status status, void *data)
{
    TELEPHONY_LOGD("GetSmscAddrCallback start ");
    auto context = static_cast<GetSmscAddrContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        TELEPHONY_LOGD("GetSmscAddrCallback napi_status ok");
        if (context->resolved) {
            TELEPHONY_LOGD("GetSmscAddrCallback start napi_create_string_utf8");
            napi_create_string_utf8(env, context->smscAddr.data(), context->smscAddr.length(), &callbackValue);
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "get smsc addr error by ipc");
        }
    } else {
        callbackValue =
            NapiUtil::CreateErrorMessage(env, "get smsc addr error,napi_status = " + std ::to_string(status));
    }
    TELEPHONY_LOGD("GetSmscAddrCallback start 5");
    NapiUtil::Handle2ValueCallback(env, context, callbackValue);
}

static napi_value GetSmscAddr(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGD("GetSmscAddr start ");
    size_t parameterCount = 2;
    napi_value parameters[2] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    TELEPHONY_LOGD("GetSmscAddr start ");
    NAPI_ASSERT(env, MatchGetSmscAddrParameters(env, parameters, parameterCount), "type mismatch");
    TELEPHONY_LOGD("GetSmscAddr start 2");
    auto context = new GetSmscAddrContext();
    TELEPHONY_LOGD("GetSmscAddr start 3");
    napi_get_value_int32(env, parameters[0], &context->slotId);
    TELEPHONY_LOGD("GetSmscAddr start 4");
    if (parameterCount == 2) {
        TELEPHONY_LOGD("GetSmscAddr start 5");
        napi_create_reference(env, parameters[1], 1, &context->callbackRef);
        TELEPHONY_LOGD("GetSmscAddr start 6");
    }
    return NapiUtil::HandleAsyncWork(env, context, "GetSmscAddr", NativeGetSmscAddr, GetSmscAddrCallback);
}

static bool MatchAddSimMessageParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    bool typeMatch = false;
    switch (parameterCount) {
        case 1: {
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object});
            break;
        }
        case 2: {
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object, napi_function});
            break;
        }
        default: {
            break;
        }
    }
    if (typeMatch) {
        return MatchObjectProperty(env, parameters[0],
            {{"slotId", napi_number}, {"smsc", napi_string}, {"status", napi_number}, {"pdu", napi_string}});
    }
    return false;
}

static void NativeAddSimMessage(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeAddSimMessage start");
    auto context = static_cast<AddSimMessageContext *>(data);
    MessageStatus wrapStatus = WrapSimMessageStatus(context->status);
    TELEPHONY_LOGD("NativeAddSimMessage start wrapStatus = %{public}d", wrapStatus);
    if (wrapStatus != MESSAGE_UNKNOWN_STATUS) {
        ISmsServiceInterface::SimMessageStatus status =
            static_cast<ISmsServiceInterface::SimMessageStatus>(wrapStatus);
        if (IsShortMessageManagerInit()) {
            context->addResult = g_shortMessageManager->AddSimMessage(
                context->slotId, NapiUtil::ToUtf16(context->smsc), NapiUtil::ToUtf16(context->pdu), status);
            TELEPHONY_LOGD("NativeAddSimMessage context->addResult = %{public}d", context->addResult);
            context->resolved = true;
        } else {
            context->resolved = false;
        }
    } else {
        context->resolved = false;
    }
}

static void AddSimMessageCallback(napi_env env, napi_status status, void *data)
{
    TELEPHONY_LOGD("AddSimMessageCallback start");
    auto context = static_cast<AddSimMessageContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "add sim message error by ipc");
        }
    } else {
        callbackValue =
            NapiUtil::CreateErrorMessage(env, "add sim message error,napi_status = " + std ::to_string(status));
    }
    NapiUtil::Handle1ValueCallback(env, context, callbackValue);
}

static napi_value AddSimMessage(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGD("AddSimMessage start");
    size_t parameterCount = 2;
    napi_value parameters[2] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchAddSimMessageParameters(env, parameters, parameterCount), "type mismatch");
    auto context = new AddSimMessageContext();
    napi_value slotIdValue = NapiUtil::GetNamedProperty(env, parameters[0], "slotId");
    if (slotIdValue != nullptr) {
        napi_get_value_int32(env, slotIdValue, &context->slotId);
    }
    napi_value smscValue = NapiUtil::GetNamedProperty(env, parameters[0], "smsc");
    if (smscValue != nullptr) {
        context->smsc = Get64StringFromValue(env, smscValue);
    }
    napi_value pduValue = NapiUtil::GetNamedProperty(env, parameters[0], "pdu");
    if (pduValue != nullptr) {
        context->pdu = NapiUtil::GetStringFromValue(env, pduValue);
    }
    napi_value statusValue = NapiUtil::GetNamedProperty(env, parameters[0], "status");
    if (statusValue != nullptr) {
        int32_t messageStatus = MESSAGE_UNKNOWN_STATUS;
        napi_get_value_int32(env, statusValue, &messageStatus);
        context->status = WrapSimMessageStatus(messageStatus);
    }
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], 1, &context->callbackRef);
    }
    return NapiUtil::HandleAsyncWork(env, context, "AddSimMessage", NativeAddSimMessage, AddSimMessageCallback);
}

static bool MatchDelSimMessageParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case 2: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_number});
        }
        case 3: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_number, napi_function});
        }
        default: {
            return false;
        }
    }
}
static void NativeDelSimMessage(napi_env env, void *data)
{
    auto context = static_cast<DelSimMessageContext *>(data);
    if (IsShortMessageManagerInit()) {
        context->deleteResult = g_shortMessageManager->DelSimMessage(context->slotId, context->msgIndex);
        context->resolved = true;
    } else {
        context->resolved = false;
    }
}
static void DelSimMessageCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<DelSimMessageContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "delete sim message error by ipc");
        }
    } else {
        callbackValue =
            NapiUtil::CreateErrorMessage(env, "delete sim message error,napi_status = " + std ::to_string(status));
    }
    NapiUtil::Handle1ValueCallback(env, context, callbackValue);
}

static napi_value DelSimMessage(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchDelSimMessageParameters(env, parameters, parameterCount), "type mismatch");
    auto context = new DelSimMessageContext();
    napi_get_value_int32(env, parameters[0], &context->slotId);
    napi_get_value_int32(env, parameters[1], &context->msgIndex);
    if (parameterCount == 3) {
        napi_create_reference(env, parameters[2], 1, &context->callbackRef);
    }
    napi_value result =
        NapiUtil::HandleAsyncWork(env, context, "DelSimMessage", NativeDelSimMessage, DelSimMessageCallback);
    return result;
}

static bool MatchUpdateSimMessageParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    TELEPHONY_LOGD("MatchUpdateSimMessageParameters start");
    bool typeMatch = false;
    switch (parameterCount) {
        case 1: {
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object});
            break;
        }
        case 2: {
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object, napi_function});
            break;
        }
        default: {
            break;
        }
    }
    if (typeMatch) {
        bool propertyMatchResult = MatchObjectProperty(env, parameters[0],
            {{"slotId", napi_number}, {"msgIndex", napi_number}, {"newStatus", napi_number}, {"pdu", napi_string},
                {"smsc", napi_string}});
        TELEPHONY_LOGD(
            "MatchUpdateSimMessageParameters start propertyMatchResult = %{public}d", propertyMatchResult);
        return propertyMatchResult;
    }
    TELEPHONY_LOGD("MatchUpdateSimMessageParameters end");
    return false;
}

static void NativeUpdateSimMessage(napi_env env, void *data)
{
    auto context = static_cast<UpdateSimMessageContext *>(data);
    if (IsShortMessageManagerInit()) {
        int32_t newStatus = static_cast<int32_t>(context->newStatus);
        TELEPHONY_LOGD("NativeUpdateSimMessage newStatus = %{public}d", newStatus);
        if (!context->pdu.empty() && (newStatus > -1)) {
            std::string msgPud(context->pdu.begin(), context->pdu.end());
            TELEPHONY_LOGD("NativeUpdateSimMessage msgPud = %{public}s", msgPud.c_str());
            context->updateResult = g_shortMessageManager->UpdateSimMessage(context->slotId, context->msgIndex,
                static_cast<ISmsServiceInterface::SimMessageStatus>(context->newStatus),
                NapiUtil::ToUtf16(context->pdu), NapiUtil::ToUtf16(context->smsc));
            context->resolved = true;
        } else {
            TELEPHONY_LOGD("NativeUpdateSimMessage resolved false cause parameter invalided");
            context->resolved = false;
        }
    } else {
        TELEPHONY_LOGD("NativeUpdateSimMessage resolved false cause ipc error");
        context->resolved = false;
    }
}

static void UpdateSimMessageCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<UpdateSimMessageContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "update sim message failed");
        }
    } else {
        callbackValue =
            NapiUtil::CreateErrorMessage(env, "update sim message error,napi_status = " + std ::to_string(status));
    }
    NapiUtil::Handle1ValueCallback(env, context, callbackValue);
}

static napi_value UpdateSimMessage(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGD("UpdateSimMessage start");
    size_t parameterCount = 2;
    napi_value parameters[2] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchUpdateSimMessageParameters(env, parameters, parameterCount), "type mismatch");
    TELEPHONY_LOGD("UpdateSimMessage start parameter match passed");
    auto context = new UpdateSimMessageContext();
    napi_value slotIdValue = NapiUtil::GetNamedProperty(env, parameters[0], "slotId");
    if (slotIdValue != nullptr) {
        napi_get_value_int32(env, slotIdValue, &context->slotId);
    }
    napi_value msgIndexValue = NapiUtil::GetNamedProperty(env, parameters[0], "msgIndex");
    if (msgIndexValue != nullptr) {
        napi_get_value_int32(env, msgIndexValue, &context->msgIndex);
    }
    napi_value newStatusValue = NapiUtil::GetNamedProperty(env, parameters[0], "newStatus");
    if (newStatusValue != nullptr) {
        int32_t newStatus = MESSAGE_UNKNOWN_STATUS;
        napi_get_value_int32(env, newStatusValue, &newStatus);
        context->newStatus = WrapSimMessageStatus(newStatus);
    }
    napi_value pudValue = NapiUtil::GetNamedProperty(env, parameters[0], "pdu");
    if (pudValue != nullptr) {
        context->pdu = NapiUtil::GetStringFromValue(env, pudValue);
    }
    TELEPHONY_LOGD("UpdateSimMessage pdu = %{public}s", context->pdu.c_str());
    napi_value smscValue = NapiUtil::GetNamedProperty(env, parameters[0], "smsc");
    if (smscValue != nullptr) {
        context->smsc = Get64StringFromValue(env, smscValue);
    }
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], 1, &context->callbackRef);
    }
    TELEPHONY_LOGD("UpdateSimMessage start before HandleAsyncWork");
    return NapiUtil::HandleAsyncWork(
        env, context, "UpdateSimMessage", NativeUpdateSimMessage, UpdateSimMessageCallback);
}

static bool MatchGetAllSimMessagesParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case 1: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number});
        }
        case 2: {
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_function});
        }
        default: {
            return false;
        }
    }
}

static void NativeGetAllSimMessages(napi_env env, void *data)
{
    auto context = static_cast<GetAllSimMessagesContext *>(data);
    if (IsShortMessageManagerInit()) {
        context->messageArray = g_shortMessageManager->GetAllSimMessages(context->slotId);
        context->resolved = true;
    } else {
        context->resolved = false;
    }
}

static void GetAllSimMessagesCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<GetAllSimMessagesContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_create_array(env, &callbackValue);
            int32_t arraySize = static_cast<int32_t>(context->messageArray.size());
            for (int32_t i = 0; i < arraySize; i++) {
                ShortMessage message = context->messageArray[i];
                napi_value itemValue = CreateShortMessageValue(env, &message);
                napi_set_element(env, callbackValue, i, itemValue);
            }
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "get all sim message error by ipc");
        }
    } else {
        callbackValue = NapiUtil::CreateErrorMessage(
            env, "get all sim message error,napi_status = " + std ::to_string(status));
    }
    NapiUtil::Handle2ValueCallback(env, context, callbackValue);
}

static napi_value GetAllSimMessages(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 2;
    napi_value parameters[2] = {0};
    napi_value thisVar;
    void *data;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchGetAllSimMessagesParameters(env, parameters, parameterCount), "type mismatch");
    auto context = new GetAllSimMessagesContext();
    napi_get_value_int32(env, parameters[0], &context->slotId);
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], 1, &context->callbackRef);
    }
    napi_value result = NapiUtil::HandleAsyncWork(
        env, context, "GetAllSimMessages", NativeGetAllSimMessages, GetAllSimMessagesCallback);
    return result;
}

static bool MatchSetCBConfigParameters(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    bool typeMatch = false;
    switch (parameterCount) {
        case 1: {
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object});
            break;
        }
        case 2: {
            typeMatch = NapiUtil::MatchParameters(env, parameters, {napi_object, napi_function});
            break;
        }
        default: {
            break;
        }
    }
    if (typeMatch) {
        return MatchObjectProperty(env, parameters[0],
            {
                {"slotId", napi_number},
                {"enable", napi_boolean},
                {"startMessageId", napi_number},
                {"endMessageId", napi_number},
                {"ranType", napi_number},
            });
    }
    return false;
}

static void NativeSetCBConfig(napi_env env, void *data)
{
    auto context = static_cast<CBConfigContext *>(data);
    if (IsShortMessageManagerInit()) {
        context->setResult = g_shortMessageManager->SetCBConfig(
            context->slotId, context->enable, context->startMessageId, context->endMessageId, context->ranType);
        context->resolved = true;
    } else {
        context->resolved = false;
    }
}

static void SetCBConfigCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<CBConfigContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "set cbconfig error by ipc");
        }
    } else {
        callbackValue =
            NapiUtil::CreateErrorMessage(env, "set cbconfig error,napi_status = " + std ::to_string(status));
    }
    NapiUtil::Handle1ValueCallback(env, context, callbackValue);
}

static napi_value SetCBConfig(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 2;
    napi_value parameters[2] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchSetCBConfigParameters(env, parameters, parameterCount), "type mismatch");
    auto context = new CBConfigContext();
    napi_value slotIdValue = NapiUtil::GetNamedProperty(env, parameters[0], "slotId");
    if (slotIdValue != nullptr) {
        napi_get_value_int32(env, slotIdValue, &context->slotId);
    }
    napi_value enableValue = NapiUtil::GetNamedProperty(env, parameters[0], "enable");
    if (enableValue != nullptr) {
        napi_get_value_bool(env, enableValue, &context->enable);
    }
    napi_value startMessageIdValue = NapiUtil::GetNamedProperty(env, parameters[0], "startMessageId");
    if (startMessageIdValue != nullptr) {
        napi_get_value_int32(env, startMessageIdValue, &context->startMessageId);
    }
    napi_value endMessageIdValue = NapiUtil::GetNamedProperty(env, parameters[0], "endMessageId");
    if (endMessageIdValue != nullptr) {
        napi_get_value_int32(env, endMessageIdValue, &context->endMessageId);
    }
    napi_value ranTypeValue = NapiUtil::GetNamedProperty(env, parameters[0], "ranType");
    if (ranTypeValue != nullptr) {
        napi_get_value_int32(env, ranTypeValue, &context->ranType);
    }
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], 1, &context->callbackRef);
    }
    napi_value result =
        NapiUtil::HandleAsyncWork(env, context, "SetCBConfig", NativeSetCBConfig, SetCBConfigCallback);
    return result;
}

static napi_value CreateStaticPropertyValue(napi_env env, int32_t nativeValue)
{
    napi_value value = nullptr;
    napi_create_int32(env, nativeValue, &value);
    return value;
}

EXTERN_C_START
napi_value InitNapiSmsRegistry(napi_env env, napi_value exports)
{
    g_shortMessageManager = std::make_unique<ShortMessageManager>();
    napi_value sendSmsSuccess = nullptr;
    napi_value sendSmsFailureUnknown = nullptr;
    napi_value sendSmsFailureRadioOff = nullptr;
    napi_value sendSmsFailureServiceUnavailable = nullptr;
    napi_create_int32(env, SEND_SMS_SUCCESS, &sendSmsSuccess);
    napi_create_int32(env, SEND_SMS_FAILURE_UNKNOWN, &sendSmsFailureUnknown);
    napi_create_int32(env, SEND_SMS_FAILURE_RADIO_OFF, &sendSmsFailureRadioOff);
    napi_create_int32(env, SEND_SMS_FAILURE_SERVICE_UNAVAILABLE, &sendSmsFailureServiceUnavailable);
    napi_value shortMessageClassUnknown = nullptr;
    napi_value shortMessageClassInstantMessage = nullptr;
    napi_value shortMessageClassOptionalMessage = nullptr;
    napi_value shortMessageClassSimMessage = nullptr;
    napi_value shortMessageClassForwardMessage = nullptr;
    napi_create_int32(env, OHOS::Telephony::UNKNOWN, &shortMessageClassUnknown);
    napi_create_int32(env, INSTANT_MESSAGE, &shortMessageClassInstantMessage);
    napi_create_int32(env, OPTIONAL_MESSAGE, &shortMessageClassOptionalMessage);
    napi_create_int32(env, SIM_MESSAGE, &shortMessageClassSimMessage);
    napi_create_int32(env, FORWARD_MESSAGE, &shortMessageClassForwardMessage);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("sendMessage", SendMessage),
        DECLARE_NAPI_FUNCTION("createMessage", CreateMessage),
        DECLARE_NAPI_FUNCTION("setDefaultSmsSlotId", SetDefaultSmsSlotId),
        DECLARE_NAPI_FUNCTION("getDefaultSmsSlotId", GetDefaultSmsSlotId),
        DECLARE_NAPI_FUNCTION("setSmscAddr", SetSmscAddr),
        DECLARE_NAPI_FUNCTION("getSmscAddr", GetSmscAddr),
        DECLARE_NAPI_FUNCTION("addSimMessage", AddSimMessage),
        DECLARE_NAPI_FUNCTION("delSimMessage", DelSimMessage),
        DECLARE_NAPI_FUNCTION("updateSimMessage", UpdateSimMessage),
        DECLARE_NAPI_FUNCTION("getAllSimMessages", GetAllSimMessages),
        DECLARE_NAPI_FUNCTION("setCBConfig", SetCBConfig),
        DECLARE_NAPI_STATIC_PROPERTY("UNKNOWN", sendSmsSuccess),
        DECLARE_NAPI_STATIC_PROPERTY("INSTANT_MESSAGE", sendSmsFailureUnknown),
        DECLARE_NAPI_STATIC_PROPERTY("OPTIONAL_MESSAGE", sendSmsFailureRadioOff),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_MESSAGE", shortMessageClassSimMessage),
        DECLARE_NAPI_STATIC_PROPERTY("FORWARD_MESSAGE", shortMessageClassForwardMessage),
        DECLARE_NAPI_STATIC_PROPERTY("SEND_SMS_FAILURE_SERVICE_UNAVAILABLE", sendSmsFailureServiceUnavailable),
        DECLARE_NAPI_STATIC_PROPERTY("SEND_SMS_SUCCESS", sendSmsSuccess),
        DECLARE_NAPI_STATIC_PROPERTY("SEND_SMS_FAILURE_UNKNOWN", sendSmsFailureUnknown),
        DECLARE_NAPI_STATIC_PROPERTY("SEND_SMS_FAILURE_RADIO_OFF", sendSmsFailureRadioOff),
        DECLARE_NAPI_STATIC_PROPERTY("SEND_SMS_FAILURE_SERVICE_UNAVAILABLE", sendSmsFailureServiceUnavailable),
        DECLARE_NAPI_STATIC_PROPERTY("MESSAGE_UNKNOWN_STATUS",
            CreateStaticPropertyValue(env, static_cast<int32_t>(MESSAGE_UNKNOWN_STATUS))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "MESSAGE_HAVE_READ", CreateStaticPropertyValue(env, static_cast<int32_t>(MESSAGE_HAVE_READ))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "MESSAGE_UNREAD", CreateStaticPropertyValue(env, static_cast<int32_t>(MESSAGE_UNREAD))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "MESSAGE_HAS_BEEN_SENT", CreateStaticPropertyValue(env, static_cast<int32_t>(MESSAGE_HAS_BEEN_SENT))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "MESSAGE_NOT_SENT", CreateStaticPropertyValue(env, static_cast<int32_t>(MESSAGE_NOT_SENT))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
EXTERN_C_END

static napi_module g_smsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitNapiSmsRegistry,
    .nm_modname = "libtelephony_sms.z.so",
    .nm_priv = ((void *)0),
    .reserved = {(void *)0},
};

extern "C" __attribute__((constructor)) void RegisterTelephonySmsModule(void)
{
    napi_module_register(&g_smsModule);
}
} // namespace Telephony
} // namespace OHOS
