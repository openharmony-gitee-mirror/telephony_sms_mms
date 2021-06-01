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
#include "kit_short_message_hilog_wrap.h"
#include "short_message.h"
#include "short_message_manager.h"

namespace OHOS {
namespace TelephonyNapi {
static std::unique_ptr<ShortMessageManager> g_shortMessageManager;
static bool InitShortMessageManager()
{
    if (g_shortMessageManager == nullptr) {
        g_shortMessageManager = std::make_unique<ShortMessageManager>();
    }
    return g_shortMessageManager != nullptr;
}

static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

static bool MatchParamters(napi_env env, napi_value parameters[], std::initializer_list<napi_valuetype> valueTypes)
{
    int i = ZERO;
    for (auto beg = valueTypes.begin(); beg != valueTypes.end(); ++beg) {
        if (!MatchValueType(env, parameters[i], *beg)) {
            return false;
        }
        ++i;
    }
    return true;
}

static void SetPropertyInt32(napi_env env, napi_value object, std::string name, int32_t value)
{
    napi_value peopertyValue = nullptr;
    napi_create_int32(env, value, &peopertyValue);
    napi_set_named_property(env, object, name.c_str(), peopertyValue);
}

static void SetPropertyStringUtf8(napi_env env, napi_value object, std::string name, std::string value)
{
    napi_value peopertyValue = nullptr;
    char *valueChars = (char *)value.c_str();
    napi_create_string_utf8(env, valueChars, std::strlen(valueChars), &peopertyValue);
    napi_set_named_property(env, object, name.c_str(), peopertyValue);
}

static void SetPropertyBoolean(napi_env env, napi_value object, std::string name, bool value)
{
    napi_value peopertyValue = nullptr;
    napi_get_boolean(env, value, &peopertyValue);
    napi_set_named_property(env, object, name.c_str(), peopertyValue);
}

static void SetPropertyArray(napi_env env, napi_value object, std::string name, std::vector<unsigned char> pdu)
{
    napi_value array = nullptr;
    napi_create_array(env, &array);
    int size = pdu.size();
    for (int i = ZERO; i < size; i++) {
        napi_value element = nullptr;
        napi_create_int32(env, pdu[i], &element);
        napi_set_element(env, array, i, element);
    }
    napi_set_named_property(env, object, name.c_str(), array);
}

static char *GetChars(std::string str)
{
    return (char *)str.data();
}

static napi_value CreateErrorMessage(napi_env env, std::string msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, GetChars(msg), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

static std::u16string ToUtf16(std::string str)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(str);
}

static std::string ToUtf8(std::u16string str16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(str16);
}

static int32_t GetDefaultSmsSlotId()
{
    return ONE;
}

static bool HasNamedProperty(napi_env env, napi_value object, const char *propertyName)
{
    bool hasProperty = false;
    napi_has_named_property(env, object, propertyName, &hasProperty);
    return hasProperty;
}

static napi_value GetNamedProperty(napi_env env, napi_value object, const char *propertyName, bool shouldGet = true)
{
    napi_value value = nullptr;
    if (shouldGet) {
        napi_get_named_property(env, object, propertyName, &value);
    }
    return value;
}

static napi_value CreateUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static std::u16string GetU16StrFromNapiValue(napi_env env, napi_value value)
{
    char strChars[PROPERTY_NAME_SIZE] = {0};
    size_t strLength = SIZE_T_ZERO;
    napi_get_value_string_utf8(env, value, strChars, BUFF_LENGTH, &strLength);
    std::string str8(strChars, strLength);
    return ToUtf16(str8);
}

static void SendTextMessage(napi_env env, napi_value thisVar, TextMessageParameter &parameter)
{
    HILOG_DEBUG("Exec SendTextMessage start ");
    const char *sendCallbackStr = "sendCallback";
    bool hasSendCallback = HasNamedProperty(env, parameter.object, sendCallbackStr);
    napi_value sendCallbackValue = GetNamedProperty(env, parameter.object, sendCallbackStr, hasSendCallback);
    const char *deliveryCallbackStr = "deliveryCallback";
    bool hasDeliveryCallback = HasNamedProperty(env, parameter.object, deliveryCallbackStr);
    napi_value deliveryCallbackValue =
        GetNamedProperty(env, parameter.object, deliveryCallbackStr, hasDeliveryCallback);
    char contentChars[MAX_TEXT_SHORT_MESSAGE_LENGTH] = {0};
    size_t contentLength = SIZE_T_ZERO;
    napi_get_value_string_utf8(
        env, parameter.contentValue, contentChars, MAX_TEXT_SHORT_MESSAGE_LENGTH - 1, &contentLength);
    std::string text(contentChars, contentLength);
    std::u16string text16 = ToUtf16(text);
    napi_ref sendCallbackRef = nullptr;
    if (hasSendCallback) {
        napi_create_reference(env, sendCallbackValue, ONE, &sendCallbackRef);
    }
    std::unique_ptr<SendCallback> sendCallback =
        std::make_unique<SendCallback>(hasSendCallback, env, thisVar, sendCallbackRef);
    if (sendCallback == nullptr) {
        return;
    }
    napi_ref deliveryCallbackRef = nullptr;
    if (hasDeliveryCallback) {
        napi_create_reference(env, deliveryCallbackValue, ONE, &deliveryCallbackRef);
    }
    std::unique_ptr<DeliveryCallback> deliveryCallback =
        std::make_unique<DeliveryCallback>(hasDeliveryCallback, env, thisVar, deliveryCallbackRef);
    if (deliveryCallback == nullptr) {
        return;
    }
    InitShortMessageManager();
    g_shortMessageManager->SendMessage(parameter.slotId, parameter.destinationHost, parameter.serviceCenter,
        text16, sendCallback.release(), deliveryCallback.release());
}

static void SendRawDataMessage(
    napi_env env, napi_value thisVar, TextMessageParameter &parameter, uint16_t destinationPort)
{
    HILOG_DEBUG("Exec SendRawDataMessage start");
    const char *sendCallbackStr = "sendCallback";
    bool hasSendCallback = HasNamedProperty(env, parameter.object, sendCallbackStr);
    napi_value sendCallbackValue = GetNamedProperty(env, parameter.object, sendCallbackStr, hasSendCallback);
    const char *deliveryCallbackStr = "deliveryCallback";
    bool hasDeliveryCallback = HasNamedProperty(env, parameter.object, deliveryCallbackStr);
    napi_value deliveryCallbackValue =
        GetNamedProperty(env, parameter.object, deliveryCallbackStr, hasDeliveryCallback);
    void *datas = nullptr;
    size_t dataLen = SIZE_T_ZERO;
    napi_get_arraybuffer_info(env, parameter.contentValue, &datas, &dataLen);
    auto rawData = (uint8_t *)datas;
    napi_ref sendCallbackRef = nullptr;
    if (hasSendCallback) {
        napi_create_reference(env, sendCallbackValue, ONE, &sendCallbackRef);
    }
    std::unique_ptr<SendCallback> sendCallback =
        std::make_unique<SendCallback>(hasSendCallback, env, thisVar, sendCallbackRef);
    if (sendCallback == nullptr) {
        return;
    }
    napi_ref deliveryCallbackRef = nullptr;
    if (hasDeliveryCallback) {
        napi_create_reference(env, deliveryCallbackValue, ONE, &deliveryCallbackRef);
    }
    std::unique_ptr<DeliveryCallback> deliveryCallback =
        std::make_unique<DeliveryCallback>(hasDeliveryCallback, env, thisVar, deliveryCallbackRef);
    if (deliveryCallback == nullptr) {
        return;
    }
    InitShortMessageManager();
    g_shortMessageManager->SendMessage(parameter.slotId, parameter.destinationHost, parameter.serviceCenter,
        destinationPort, rawData, dataLen, sendCallback.release(), deliveryCallback.release());
    HILOG_DEBUG("Exec SendRawDataMessage end");
}

static uint16_t GetDestinationPort(napi_env env, napi_value object)
{
    uint16_t destinationPort = DEFALUT_PORT;
    int32_t wrapPort = destinationPort;
    napi_value destinationPortValue = nullptr;
    bool hasDestinationPort = false;
    const char *destinationPortStr = "destinationPort";
    napi_has_named_property(env, object, destinationPortStr, &hasDestinationPort);
    if (hasDestinationPort) {
        napi_get_named_property(env, object, destinationPortStr, &destinationPortValue);
        napi_get_value_int32(env, destinationPortValue, &wrapPort);
        destinationPort = wrapPort;
    }
    return destinationPort;
}

static napi_value SendMessage(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("Exec SendMessage start");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    NAPI_ASSERT(
        env, (argc == TelephonyNapi::ONE_PARAMTER) && MatchParamters(env, argv, {napi_object}), "type mismatch");
    napi_value object = argv[ZERO];
    const char *slotIdStr = "slotId";
    const char *destinationHostStr = "destinationHost";
    const char *serviceCenterStr = "serviceCenter";
    const char *contentStr = "content";
    bool hasSlotId = HasNamedProperty(env, object, slotIdStr);
    bool hasDestinationHost = HasNamedProperty(env, object, destinationHostStr);
    bool hasContent = HasNamedProperty(env, object, contentStr);
    NAPI_ASSERT(env, hasSlotId && hasDestinationHost && hasContent, "type mismatch");
    napi_value slotIdValue = GetNamedProperty(env, object, slotIdStr);
    NAPI_ASSERT(env, MatchValueType(env, slotIdValue, napi_number), "type mismatch");
    int32_t slotId = GetDefaultSmsSlotId();
    napi_get_value_int32(env, slotIdValue, &slotId);
    napi_value destinationHostValue = GetNamedProperty(env, object, destinationHostStr);
    NAPI_ASSERT(env, MatchValueType(env, destinationHostValue, napi_string), "type mismatch");
    std::u16string destinationHostStr16 = GetU16StrFromNapiValue(env, destinationHostValue);
    bool hasServiceCenter = HasNamedProperty(env, object, serviceCenterStr);
    std::u16string serviceCenterStr16 = u"";
    if (hasServiceCenter) {
        napi_value serviceCenterValue = GetNamedProperty(env, object, serviceCenterStr);
        NAPI_ASSERT(env, MatchValueType(env, serviceCenterValue, napi_string), "type mismatch");
        serviceCenterStr16 = GetU16StrFromNapiValue(env, serviceCenterValue);
    }
    napi_value contentValue = GetNamedProperty(env, object, contentStr);
    bool contentIsStr = MatchValueType(env, contentValue, napi_string);
    HILOG_DEBUG("Exec SendMessage before MatchTypeddArray ");
    bool contentIsObj = MatchValueType(env, contentValue, napi_object);
    bool contentIsTypedArray = false;
    if (contentIsObj) {
        napi_is_typedarray(env, contentValue, &contentIsTypedArray);
    }
    HILOG_DEBUG("Exec SendMessage after MatchTypeddArray contentIsTypedArray = %{public}d", contentIsTypedArray);
    NAPI_ASSERT(env, contentIsStr || contentIsTypedArray, "type mismatch");
    TextMessageParameter parameter = {
        .slotId = slotId,
        .destinationHost = destinationHostStr16,
        .serviceCenter = serviceCenterStr16,
        .contentValue = contentValue,
        .object = object,
    };
    if (contentIsStr) {
        SendTextMessage(env, thisVar, parameter);
    } else if (contentIsTypedArray) {
        SendRawDataMessage(env, thisVar, parameter, GetDestinationPort(env, object));
    }
    HILOG_DEBUG("Exec SendMessage end");
    return nullptr;
}

static void NativeCreateMessage(napi_env env, AsyncContext &asyncContext)
{
    std::string specification(asyncContext.specification, asyncContext.specificationLen);
    std::u16string specification16 = ToUtf16(specification);
    std::unique_ptr<OHOS::SMS::ShortMessage> shortMessage = std::make_unique<OHOS::SMS::ShortMessage>(
        *(OHOS::SMS::ShortMessage::CreateMessage(asyncContext.pdu, specification16)));
    if (shortMessage != nullptr) {
        napi_create_object(env, &asyncContext.value);
        SetPropertyStringUtf8(
            env, asyncContext.value, "visibleMessageBody", ToUtf8(shortMessage->GetVisibleMessageBody()));
        SetPropertyStringUtf8(
            env, asyncContext.value, "visibleRawAddress", ToUtf8(shortMessage->GetVisibleRawAddress()));
        SetPropertyInt32(env, asyncContext.value, "messageClass", shortMessage->GetMessageClass());
        SetPropertyInt32(env, asyncContext.value, "protocolId", shortMessage->GetProtocolId());
        SetPropertyStringUtf8(env, asyncContext.value, "scAddress", ToUtf8(shortMessage->GetScAddress()));
        SetPropertyInt32(env, asyncContext.value, "scTimestamp", shortMessage->GetScTimestamp());
        SetPropertyBoolean(env, asyncContext.value, "isReplaceMessage", shortMessage->IsReplaceMessage());
        SetPropertyBoolean(env, asyncContext.value, "hasReplyPath", shortMessage->HasReplyPath());
        SetPropertyArray(env, asyncContext.value, "pdu", shortMessage->GetPdu());
        SetPropertyInt32(env, asyncContext.value, "status", shortMessage->GetStatus());
        SetPropertyBoolean(
            env, asyncContext.value, "isSmsStatusReportMessage", shortMessage->IsSmsStatusReportMessage());
        asyncContext.status = RESOLVED;
    } else {
        asyncContext.status = REJECT;
    }
}

static void CreateMessageCallback(napi_env env, const AsyncContext &asyncContext)
{
    HILOG_DEBUG("Exec CreateMessageCallback start");
    if (asyncContext.deferred != nullptr) {
        HILOG_DEBUG("Exec CreateMessageCallback has deferred");
        if (asyncContext.status == RESOLVED) {
            napi_resolve_deferred(env, asyncContext.deferred, asyncContext.value);
        } else {
            napi_value undefined = CreateUndefined(env);
            napi_reject_deferred(env, asyncContext.deferred, undefined);
        }
    } else {
        HILOG_DEBUG("Exec CreateMessageCallback no deferred");
        napi_value result[TWO] = {0};
        if (asyncContext.status == RESOLVED) {
            result[ZERO] = CreateUndefined(env);
            result[ONE] = asyncContext.value;
        } else {
            result[ZERO] = CreateErrorMessage(env, "get short message failed");
            result[ONE] = CreateUndefined(env);
        }
        napi_value callback = nullptr;
        napi_get_reference_value(env, asyncContext.callbackRef, &callback);
        napi_call_function(env, nullptr, callback, TWO, result, nullptr);
        HILOG_DEBUG("Exec CreateMessageCallback after call_function");
    }
    napi_delete_reference(env, asyncContext.callbackRef);
    delete &asyncContext;
    HILOG_DEBUG("Exec CreateMessageCallback end");
}

static napi_value CreateMessage(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("Exec SendMessage start");
    size_t argc = 3;
    napi_value argv[3] = {0};
    napi_value thisVar;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    NAPI_ASSERT(env, (argc == TWO) || (argc == THREE), "type mismatch");
    if (argc == TWO) {
        NAPI_ASSERT(env, MatchParamters(env, argv, {napi_object, napi_string}), "type mismatch");
    } else if (argc == THREE) {
        NAPI_ASSERT(env, MatchParamters(env, argv, {napi_object, napi_string, napi_function}), "type mismatch");
    }
    bool isArray = false;
    napi_is_array(env, argv[ZERO], &isArray);
    NAPI_ASSERT(env, isArray, "type mismatch,Expects an array as first argument");
    uint32_t arrayLength = ZERO;
    napi_get_array_length(env, argv[ZERO], &arrayLength);
    std::vector<unsigned char> puds;
    napi_value elementValue = nullptr;
    int32_t element = ZERO;
    auto arraySize = static_cast<int32_t>(arrayLength);
    for (int32_t i = 0; i < arraySize; i++) {
        napi_get_element(env, argv[0], i, &elementValue);
        napi_get_value_int32(env, elementValue, &element);
        puds.push_back((unsigned char)element);
    }
    auto asyncContext = new AsyncContext();
    asyncContext->env = env;
    asyncContext->pdu = puds;
    if (argc == THREE) {
        napi_create_reference(env, argv[TWO], ONE, &(asyncContext->callbackRef));
    }
    napi_value result = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_get_value_string_utf8(
        env, argv[ONE], asyncContext->specification, BUFF_LENGTH, &(asyncContext->specificationLen));
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "CreateMessage", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) -> void { NativeCreateMessage(env, *(AsyncContext *)data); },
        [](napi_env env, napi_status status, void *data) -> void {
            CreateMessageCallback(env, *(AsyncContext *)data);
        },
        (void *)asyncContext, &(asyncContext->work));
    napi_queue_async_work(env, asyncContext->work);
    return result;
}

EXTERN_C_START
napi_value InitNapiSmsRegistry(napi_env env, napi_value exports)
{
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
    napi_create_int32(env, UNKNOWN, &shortMessageClassUnknown);
    napi_create_int32(env, INSTANT_MESSAGE, &shortMessageClassInstantMessage);
    napi_create_int32(env, OPTIONAL_MESSAGE, &shortMessageClassOptionalMessage);
    napi_create_int32(env, SIM_MESSAGE, &shortMessageClassSimMessage);
    napi_create_int32(env, FORWARD_MESSAGE, &shortMessageClassForwardMessage);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("sendMessage", SendMessage),
        DECLARE_NAPI_FUNCTION("createMessage", CreateMessage),
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
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
EXTERN_C_END

static napi_module g_smsModule = {
    .nm_version = ONE,
    .nm_flags = ZERO,
    .nm_filename = nullptr,
    .nm_register_func = InitNapiSmsRegistry,
    .nm_modname = "libtelephony_sms.z.so",
    .nm_priv = ((void *)ZERO),
    .reserved = {
        (void *)ZERO
    }
};

extern "C" __attribute__((constructor)) void RegisterTelephonySmsModule(void)
{
    napi_module_register(&g_smsModule);
}
} // namespace TelephonyNapi
} // namespace OHOS
