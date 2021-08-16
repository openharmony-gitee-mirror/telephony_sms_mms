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

#include "string_utils.h"
#include <iostream>
#include <sstream>
#include <string_ex.h>

namespace OHOS {
namespace Telephony {
static constexpr char HEX_TABLE[] = "0123456789ABCDEF";
StringUtils::StringUtils() {}

StringUtils::~StringUtils() {}

uint16_t StringUtils::HexCharToInt(char c)
{
    uint8_t decimal = 10;
    if (c >= '0' && c <= '9') {
        return (c - '0');
    }
    if (c >= 'A' && c <= 'F') {
        return (c - 'A' + decimal);
    }
    if (c >= 'a' && c <= 'f') {
        return (c - 'a' + decimal);
    }
    return 0;
}

std::string StringUtils::StringToHex(const std::string &data)
{
    std::stringstream ss;
    for (std::string::size_type i = 0; i < data.size(); ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> HEX_OFFSET;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::StringToHex(const char *data, int byteLength)
{
    std::string str("");
    std::stringstream ss;
    for (int i = 0; i < byteLength; ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> HEX_OFFSET;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::StringToHex(const std::vector<uint8_t> &data)
{
    std::string str("");
    std::stringstream ss;
    for (std::size_t i = 0; i < data.size(); ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> HEX_OFFSET;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::HexToString(const std::string &str)
{
    std::string result;
    uint8_t hexDecimal = 16;
    uint8_t hexStep = 2;
    if (str.length() <= 0) {
        return result;
    }
    for (size_t i = 0; i < str.length() - 1; i += STEP_2BIT) {
        std::string byte = str.substr(i, hexStep);
        char chr = 0;
        long strTemp = strtol(byte.c_str(), nullptr, hexDecimal);
        if (strTemp > 0) {
            chr = static_cast<char>(strTemp);
        }
        result.push_back(chr);
    }
    return result;
}

std::vector<uint8_t> StringUtils::HexToByteVector(const std::string &str)
{
    std::vector<uint8_t> ret;
    int sz = str.length();
    if (sz <= 0) {
        return ret;
    }
    for (int i = 0; i < (sz - 1); i += STEP_2BIT) {
        auto temp = static_cast<uint8_t>((HexCharToInt(str.at(i)) << HEX_OFFSET) | HexCharToInt(str.at(i + 1)));
        ret.push_back(temp);
    }
    return ret;
}

std::string StringUtils::ToUtf8(const std::u16string &str16)
{
    std::string ret;
    if (str16.empty()) {
        return ret;
    }
    return Str16ToStr8(str16);
}

std::u16string StringUtils::ToUtf16(const std::string &str)
{
    std::u16string ret;
    if (str.empty()) {
        return ret;
    }
    return Str8ToStr16(str);
}
} // namespace Telephony
} // namespace OHOS