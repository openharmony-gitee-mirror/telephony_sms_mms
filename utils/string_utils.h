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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <codecvt>
#include <cstdint>
#include <locale>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
class StringUtils {
public:
    static std::string StringToHex(const std::string &data);
    static std::string HexToString(const std::string &str);
    static std::string StringToHex(const char *data, int byteLength);
    static std::string ToUtf8(const std::u16string &str16);
    static std::u16string ToUtf16(const std::string &str);
    static std::string StringToHex(const std::vector<uint8_t> &data);
    static std::vector<uint8_t> HexToByteVector(const std::string &str);

private:
    StringUtils();
    virtual ~StringUtils();
    static uint16_t HexCharToInt(char c);

    static constexpr uint8_t HEX_OFFSET = 4;
    static constexpr uint8_t STEP_2BIT = 2;
};
} // namespace Telephony
} // namespace OHOS
#endif