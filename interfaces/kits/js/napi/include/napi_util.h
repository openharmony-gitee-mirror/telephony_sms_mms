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

#ifndef NAPI_UTIL_H
#define NAPI_UTIL_H
#include <string>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS {
namespace TelephonyNapi {
class NapiUtil {
public:
    static napi_value CreateErrorMessage(napi_env env, std::string message);
    static napi_value CreateUndefined(napi_env env);
    static std::string ToUtf8(std::u16string str16);
};
} // namespace TelephonyNapi
} // namespace OHOS
#endif // NAPI_UTIL_H