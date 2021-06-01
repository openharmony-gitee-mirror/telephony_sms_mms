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
namespace OHOS {
namespace SMS {
StringUtils::StringUtils() {}

StringUtils::~StringUtils() {}

uint16_t StringUtils::HexCharToInt(char c)
{
    if (c >= '0' && c <= '9') {
        return (c - '0');
    }
    if (c >= 'A' && c <= 'F') {
        return (c - 'A' + decimal_);
    }
    if (c >= 'a' && c <= 'f') {
        return (c - 'a' + decimal_);
    }
    return 0;
}

std::string StringUtils::StringToHex(const std::string &data)
{
    std::stringstream ss;
    for (std::string::size_type i = 0; i < data.size(); ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> hexOffset_;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::StringToHex(const char *data, int bytelength)
{
    std::string str("");
    std::stringstream ss;
    for (int i = 0; i < bytelength; ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> hexOffset_;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::StringToHex(const std::vector<uint8_t> &data)
{
    std::string str("");
    std::stringstream ss;
    for (std::size_t i = 0; i < data.size(); ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> hexOffset_;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::HexToString(const std::string &str)
{
    std::string result;
    for (size_t i = 0; i < str.length(); i += step2Bit_) {
        std::string byte = str.substr(i, HEX_BYTE_STEP);
        char chr = static_cast<char>(static_cast<int>(strtol(byte.c_str(), nullptr, hexadecimal_)));
        result.push_back(chr);
    }
    return result;
}

std::vector<uint8_t> StringUtils::HexToByteVector(const std::string &str)
{
    std::vector<uint8_t> ret;
    int sz = str.length();
    for (int i = 0; i < sz; i += step2Bit_) {
        auto temp = static_cast<uint8_t>((HexCharToInt(str.at(i)) << hexOffset_) | HexCharToInt(str.at(i + 1)));
        ret.push_back(temp);
    }
    return ret;
}

std::string StringUtils::ToUtf8(const std::u16string &str16)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.to_bytes(str16);
}

std::u16string StringUtils::ToUtf16(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.from_bytes(str);
}
} // namespace SMS
} // namespace OHOS