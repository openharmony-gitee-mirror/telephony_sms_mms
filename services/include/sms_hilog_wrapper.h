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
#ifndef SMS_HILOG_WRAPPER_H
#define SMS_HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include <string>
#include "hilog/log.h"

#ifdef HILOG_FATAL
#undef HILOG_FATAL
#endif

#ifdef HILOG_ERROR
#undef HILOG_ERROR
#endif

#ifdef HILOG_WARN
#undef HILOG_WARN
#endif

#ifdef HILOG_INFO
#undef HILOG_INFO
#endif

#ifdef HILOG_DEBUG
#undef HILOG_DEBUG
#endif

static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, 0, "SMS"};

#define HILOG_FATAL(fmt, ...)                                                                                  \
    do {                                                                                                       \
        (void)OHOS::HiviewDFX::HiLog::Fatal(LOG_LABEL,                                                         \
            std::string("[File : %{public}s Function:%{public}s Line:%{public}d]" + std::string(fmt)).c_str(), \
            __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                  \
    } while (0)

#define HILOG_ERROR(fmt, ...)                                                                                  \
    do {                                                                                                       \
        (void)OHOS::HiviewDFX::HiLog::Error(LOG_LABEL,                                                         \
            std::string("[File : %{public}s Function:%{public}s Line:%{public}d]" + std::string(fmt)).c_str(), \
            __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                  \
    } while (0)

#define HILOG_WARN(fmt, ...)                                                                                   \
    do {                                                                                                       \
        (void)OHOS::HiviewDFX::HiLog::Warn(LOG_LABEL,                                                          \
            std::string("[File : %{public}s Function:%{public}s Line:%{public}d]" + std::string(fmt)).c_str(), \
            __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                  \
    } while (0)

#define HILOG_INFO(fmt, ...)                                                                                   \
    do {                                                                                                       \
        (void)OHOS::HiviewDFX::HiLog::Info(LOG_LABEL,                                                          \
            std::string("[File : %{public}s Function:%{public}s Line:%{public}d]" + std::string(fmt)).c_str(), \
            __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                  \
    } while (0)

#define HILOG_DEBUG(fmt, ...)                                                                                  \
    do {                                                                                                       \
        (void)OHOS::HiviewDFX::HiLog::Debug(LOG_LABEL,                                                         \
            std::string("[File : %{public}s Function:%{public}s Line:%{public}d]" + std::string(fmt)).c_str(), \
            __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                  \
    } while (0)

#else

#define HILOG_FATAL(...)
#define HILOG_ERROR(...)
#define HILOG_WARN(...)
#define HILOG_INFO(...)
#define HILOG_DEBUG(...)

#endif // CONFIG_HILOG
#endif // HILOG_WRAPPER_H