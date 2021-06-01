/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Copyright (C) 2014 Samsung Electronics Co., Ltd. All rights reserved
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
#include "msg_text_convert.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include "glib.h"
#include "securec.h"
#include "sms_hilog_wrapper.h"
namespace OHOS {
namespace SMS {
template<typename T>
inline void UniquePtrDeleterOneDimension(T **(&ptr))
{
    if (ptr && *ptr) {
        delete[] *ptr;
        *ptr = nullptr;
    }
}
using msg_encode_type_t = unsigned char;
using namespace std;
std::shared_ptr<MsgTextConvert> MsgTextConvert::instance_ = nullptr;
MsgTextConvert::MsgTextConvert()
{
    InitExtCharList();
    InitUCS2ToGSM7DefList();
    InitUCS2ToExtList();
    InitUCS2ToTurkishList();
    InitUCS2ToSpanishList();
    InitUCS2ToPortuList();
    InitUCS2ToReplaceCharList();
}

MsgTextConvert::~MsgTextConvert()
{
    extCharList_.clear();
    ucs2toGSM7DefList.clear();
    ucs2toGSM7ExtList.clear();
    ucs2toTurkishList.clear();
    ucs2toSpanishList.clear();
    ucs2toPortuList.clear();
    replaceCharList.clear();
}

MsgTextConvert *MsgTextConvert::Instance()
{
    if (!instance_) {
        HILOG_INFO("pInstance is nullptr. Now creating instance.\r\n");
        struct make_shared_enabler : public MsgTextConvert {};
        instance_ = std::make_shared<make_shared_enabler>();
    }
    return instance_.get();
}

void MsgTextConvert::InitExtCharList()
{
    extCharList_.clear();
    extCharList_[0x000C] = MSG_GSM7EXT_CHAR;
    extCharList_[0x005B] = MSG_GSM7EXT_CHAR;
    extCharList_[0x005C] = MSG_GSM7EXT_CHAR;
    extCharList_[0x005D] = MSG_GSM7EXT_CHAR;
    extCharList_[0x005E] = MSG_GSM7EXT_CHAR;
    extCharList_[0x007B] = MSG_GSM7EXT_CHAR;
    extCharList_[0x007C] = MSG_GSM7EXT_CHAR;
    extCharList_[0x007D] = MSG_GSM7EXT_CHAR;
    extCharList_[0x007E] = MSG_GSM7EXT_CHAR;
    extCharList_[0x20AC] = MSG_GSM7EXT_CHAR;

    extCharList_[0x00E7] = MSG_TURKISH_CHAR;
    extCharList_[0x011E] = MSG_TURKISH_CHAR;
    extCharList_[0x011F] = MSG_TURKISH_CHAR;
    extCharList_[0x01E6] = MSG_TURKISH_CHAR;
    extCharList_[0x01E7] = MSG_TURKISH_CHAR;
    extCharList_[0x0130] = MSG_TURKISH_CHAR;
    extCharList_[0x0131] = MSG_TURKISH_CHAR;
    extCharList_[0x015E] = MSG_TURKISH_CHAR;
    extCharList_[0x015F] = MSG_TURKISH_CHAR;

    extCharList_[0x00C1] = MSG_SPANISH_CHAR;
    extCharList_[0x00E1] = MSG_SPANISH_CHAR;
    extCharList_[0x00CD] = MSG_SPANISH_CHAR;
    extCharList_[0x00ED] = MSG_SPANISH_CHAR;
    extCharList_[0x00D3] = MSG_SPANISH_CHAR;
    extCharList_[0x00F3] = MSG_SPANISH_CHAR;
    extCharList_[0x00DA] = MSG_SPANISH_CHAR;
    extCharList_[0x00FA] = MSG_SPANISH_CHAR;

    extCharList_[0x00D4] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00F4] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00CA] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00EA] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00C0] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00E7] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00C3] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00E3] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00D5] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00F5] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00C2] = MSG_PORTUGUESE_CHAR;
    extCharList_[0x00E2] = MSG_PORTUGUESE_CHAR;
}

void MsgTextConvert::InitUCS2ToGSM7DefList()
{
    ucs2toGSM7DefList.clear();
    for (unsigned char i = 0; i < GSM7_DEFLIST_LEN; i++) {
        ucs2toGSM7DefList[g_GSM7BitToUCS2[i]] = i;
    }
}

void MsgTextConvert::InitUCS2ToExtList()
{
    ucs2toGSM7ExtList.clear();
    ucs2toGSM7ExtList[0x005B] = 0x3C; /* [ */
    ucs2toGSM7ExtList[0x005D] = 0x3E; /* ] */
    ucs2toGSM7ExtList[0x007B] = 0x28; /* { */
    ucs2toGSM7ExtList[0x007D] = 0x29; /* } */
    ucs2toGSM7ExtList[0x000C] = 0x0A; /* Page Break */
    ucs2toGSM7ExtList[0x005C] = 0x2F; /* \ */
    ucs2toGSM7ExtList[0x005E] = 0x14; /* ^ */
    ucs2toGSM7ExtList[0x007C] = 0x40; /* | */
    ucs2toGSM7ExtList[0x007E] = 0x3D; /* ~ */
    ucs2toGSM7ExtList[0x20AC] = 0x65; /* € */
}

void MsgTextConvert::InitUCS2ToTurkishList()
{
    /* Turkish */
    ucs2toTurkishList.clear();
    ucs2toTurkishList[0x005B] = 0x3C; /* [ */
    ucs2toTurkishList[0x005D] = 0x3E; /* ] */
    ucs2toTurkishList[0x007B] = 0x28; /* { */
    ucs2toTurkishList[0x007D] = 0x29; /* } */
    ucs2toTurkishList[0x000C] = 0x0A; /* Page Break */
    ucs2toTurkishList[0x005C] = 0x2F; /* \ */
    ucs2toTurkishList[0x005E] = 0x14; /* ^ */
    ucs2toTurkishList[0x007C] = 0x40; /* | */
    ucs2toTurkishList[0x007E] = 0x3D; /* ~ */
    ucs2toTurkishList[0x20AC] = 0x65; /* € */
    ucs2toTurkishList[0x00E7] = 0x63; /* c LATIN SMALL LETTER S WITH CEDILLA */
    ucs2toTurkishList[0x011E] = 0x47; /* G LATIN CAPITAL LETTER G WITH BREVE */
    ucs2toTurkishList[0x011F] = 0x67; /* g LATIN SMALL LETTER G WITH BREVE */
    ucs2toTurkishList[0x01E6] = 0x47; /* G LATIN CAPITAL LETTER G WITH CARON */
    ucs2toTurkishList[0x01E7] = 0x67; /* g LATIN SMALL LETTER G WITH CARON */
    ucs2toTurkishList[0x0130] = 0x49; /* I LATIN CAPITAL LETTER I WITH DOT ABOVE */
    ucs2toTurkishList[0x0131] = 0x69; /* i LATIN SMALL LETTER DOTLESS */
    ucs2toTurkishList[0x015E] = 0x53; /* S LATIN CAPITAL LETTER S WITH CEDILLA */
    ucs2toTurkishList[0x015F] = 0x73; /* s LATIN SMALL LETTER S WITH CEDILLA */
}

void MsgTextConvert::InitUCS2ToSpanishList()
{
    /* Spanish */
    ucs2toSpanishList.clear();
    ucs2toSpanishList[0x005B] = 0x3C; /* [ */
    ucs2toSpanishList[0x005D] = 0x3E; /* ] */
    ucs2toSpanishList[0x007B] = 0x28; /* { */
    ucs2toSpanishList[0x007D] = 0x29; /* } */
    ucs2toSpanishList[0x000C] = 0x0A; /* Page Break */
    ucs2toSpanishList[0x005C] = 0x2F; /* \ */
    ucs2toSpanishList[0x005E] = 0x14; /* ^ */
    ucs2toSpanishList[0x007C] = 0x40; /* | */
    ucs2toSpanishList[0x007E] = 0x3D; /* ~ */
    ucs2toSpanishList[0x20AC] = 0x65; /* € */
    ucs2toSpanishList[0x00C1] = 0x41; /* A LATIN CAPITAL LETTER A WITH ACUTE */
    ucs2toSpanishList[0x00E1] = 0x61; /* a LATIN SMALL LETTER A WITH ACUTE */
    ucs2toSpanishList[0x00CD] = 0x49; /* I LATIN CAPITAL LETTER I WITH ACUTE */
    ucs2toSpanishList[0x00ED] = 0x69; /* i LATIN SMALL LETTER I WITH ACUTE */
    ucs2toSpanishList[0x00D3] = 0x4F; /* O LATIN CAPITAL LETTER O WITH ACUTE */
    ucs2toSpanishList[0x00F3] = 0x6F; /* o LATIN SMALL LETTER O WITH ACUTE */
    ucs2toSpanishList[0x00DA] = 0x55; /* U LATIN CAPITAL LETTER U WITH ACUTE */
    ucs2toSpanishList[0x00FA] = 0x75; /* u LATIN SMALL LETTER U WITH ACUTE */
}

void MsgTextConvert::InitUCS2ToPortuList()
{
    /* Portuguese */
    ucs2toPortuList.clear();
    ucs2toPortuList[0x005B] = 0x3C; /* [ */
    ucs2toPortuList[0x005D] = 0x3E; /* ] */
    ucs2toPortuList[0x007B] = 0x28; /* { */
    ucs2toPortuList[0x007D] = 0x29; /* } */
    ucs2toPortuList[0x000C] = 0x0A; /* Page Break */
    ucs2toPortuList[0x005C] = 0x2F; /* \ */
    ucs2toPortuList[0x005E] = 0x14; /* ^ */
    ucs2toPortuList[0x007C] = 0x40; /* | */
    ucs2toPortuList[0x007E] = 0x3D; /* ~ */
    ucs2toPortuList[0x20AC] = 0x65; /* € */
    ucs2toPortuList[0x00D4] = 0x0B; /* O LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
    ucs2toPortuList[0x00F4] = 0x0C; /* o LATIN SMALL LETTER O WITH CIRCUMFLEX */
    ucs2toPortuList[0x00C1] = 0x0E; /* A LATIN CAPITAL LETTER A WITH ACUTE */
    ucs2toPortuList[0x00E1] = 0x0F; /* a LATIN SMALL LETTER A WITH ACUTE */
    ucs2toPortuList[0x00CA] = 0x1F; /* E LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
    ucs2toPortuList[0x00EA] = 0x05; /* e LATIN SMALL LETTER E WITH CIRCUMFLEX */
    ucs2toPortuList[0x00C0] = 0x41; /* A LATIN CAPITAL LETTER A WITH GRAVE */
    ucs2toPortuList[0x00E7] = 0x09; /* c LATIN SMALL LETTER C WITH CEDILLA */
    ucs2toPortuList[0x00CD] = 0x49; /* I LATIN CAPITAL LETTER I WITH ACUTE */
    ucs2toPortuList[0x00ED] = 0x69; /* i LATIN SMALL LETTER I WITH ACUTE */
    ucs2toPortuList[0x00D3] = 0x4F; /* O LATIN CAPITAL LETTER O WITH ACUTE */
    ucs2toPortuList[0x00F3] = 0x6F; /* o LATIN SMALL LETTER O WITH ACUTE */
    ucs2toPortuList[0x00DA] = 0x55; /* U LATIN CAPITAL LETTER U WITH ACUTE */
    ucs2toPortuList[0x00FA] = 0x75; /* u LATIN SMALL LETTER U WITH ACUTE */
    ucs2toPortuList[0x00C3] = 0x61; /* A LATIN CAPITAL LETTER A WITH TILDE */
    ucs2toPortuList[0x00E3] = 0x7B; /* a LATIN SMALL LETTER A WITH TILDE */
    ucs2toPortuList[0x00D5] = 0x5C; /* O LATIN CAPITAL LETTER O WITH TILDE */
    ucs2toPortuList[0x00F5] = 0x7C; /* o LATIN SMALL LETTER O WITH TILDE */
    ucs2toPortuList[0x00C2] = 0x61; /* A LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
    ucs2toPortuList[0x00E2] = 0x7F; /* a LATIN SMALL LETTER A WITH CIRCUMFLEX */
    ucs2toPortuList[0x03A6] = 0x12; /* Φ GREEK CAPITAL LETTER PHI */
    ucs2toPortuList[0x0393] = 0x13; /* Γ GREEK CAPITAL LETTER GAMMA */
    ucs2toPortuList[0x03A9] = 0x15; /* Ω GREEK CAPITAL LETTER OMEGA */
    ucs2toPortuList[0x03A0] = 0x16; /* Π GREEK CAPITAL LETTER PI */
    ucs2toPortuList[0x03A8] = 0x17; /* Ψ GREEK CAPITAL LETTER PSI */
    ucs2toPortuList[0x03A3] = 0x18; /* Σ GREEK CAPITAL LETTER SIGMA */
    ucs2toPortuList[0x0398] = 0x19; /* Θ GREEK CAPITAL LETTER THETA */
}

void MsgTextConvert::InitUCS2ToReplaceCharList()
{
    /* character replacement table */
    replaceCharList.clear();
    replaceCharList[0x00E0] = 0x61; /* a LATIN SMALL LETTER A WITH GRAVE */
    replaceCharList[0x00E1] = 0x61; /* a LATIN SMALL LETTER A WITH ACUTE */
    replaceCharList[0x00E2] = 0x61; /* a LATIN SMALL LETTER A WITH CIRCUMFLEX */
    replaceCharList[0x00E3] = 0x61; /* a LATIN SMALL LETTER A WITH TILDE */
    replaceCharList[0x00E4] = 0x61; /* a LATIN SMALL LETTER A WITH DIAERESIS */
    replaceCharList[0x00E5] = 0x61; /* a LATIN SMALL LETTER A WITH RING ABOVE */
    replaceCharList[0x00E6] = 0x61; /* a LATIN SMALL LETTER AE */
    replaceCharList[0x0101] = 0x61; /* a LATIN SMALL LETTER A WITH MACRON */
    replaceCharList[0x0103] = 0x61; /* a LATIN SMALL LETTER A WITH BREVE */
    replaceCharList[0x0105] = 0x61; /* a LATIN SMALL LETTER A WITH OGONEK */
    replaceCharList[0x01CE] = 0x61; /* a LATIN SMALL LETTER A WITH CARON */

    replaceCharList[0x00C0] = 0x41; /* A LATIN CAPITAL LETTER A WITH GRAVE */
    replaceCharList[0x00C1] = 0x41; /* A LATIN CAPITAL LETTER A WITH ACUTE */
    replaceCharList[0x00C2] = 0x41; /* A LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
    replaceCharList[0x00C3] = 0x41; /* A LATIN CAPITAL LETTER A WITH TILDE */
    replaceCharList[0x00C4] = 0x41; /* A LATIN CAPITAL LETTER A WITH DIAERESIS */
    replaceCharList[0x00C5] = 0x41; /* A LATIN CAPITAL LETTER A WITH RING ABOVE */
    replaceCharList[0x00C6] = 0x41; /* A LATIN CAPITAL LETTER AE */
    replaceCharList[0x0100] = 0x41; /* A LATIN CAPITAL LETTER A WITH MACRON */
    replaceCharList[0x0102] = 0x41; /* A LATIN CAPITAL LETTER A WITH BREVE */
    replaceCharList[0x0104] = 0x41; /* A LATIN CAPITAL LETTER A WITH OGONEK */
    replaceCharList[0x01CD] = 0x41; /* A LATIN CAPITAL LETTER A WITH CARON */

    replaceCharList[0x00E7] = 0x63; /* c LATIN SMALL LETTER C WITH CEDILLA */
    replaceCharList[0x0107] = 0x63; /* c LATIN SMALL LETTER C WITH ACUTE */
    replaceCharList[0x0109] = 0x63; /* c LATIN SMALL LETTER C WITH CIRCUMFLEX */
    replaceCharList[0x010B] = 0x63; /* c LATIN SMALL LETTER C WITH DOT ABOVE */
    replaceCharList[0x010D] = 0x63; /* c LATIN SMALL LETTER C WITH CARON */

    replaceCharList[0x00C7] = 0x43; /* C LATIN CAPITAL LETTER C WITH CEDILLA */
    replaceCharList[0x0106] = 0x43; /* C LATIN CAPITAL LETTER C WITH ACUTE */
    replaceCharList[0x0108] = 0x43; /* C LATIN CAPITAL LETTER C WITH CIRCUMFLEX */
    replaceCharList[0x010A] = 0x43; /* C LATIN CAPITAL LETTER C WITH DOT ABOVE */
    replaceCharList[0x010C] = 0x43; /* C LATIN CAPITAL LETTER C WITH CARON */

    replaceCharList[0x010F] = 0x64; /* d LATIN SMALL LETTER D WITH CARON */
    replaceCharList[0x0111] = 0x64; /* d LATIN SMALL LETTER D WITH STROKE */

    replaceCharList[0x010E] = 0x44; /* D LATIN CAPITAL LETTER D WITH CARON */
    replaceCharList[0x0110] = 0x44; /* D LATIN CAPITAL LETTER D WITH STROKE */

    replaceCharList[0x00E8] = 0x65; /* e LATIN SMALL LETTER E WITH GRAVE */
    replaceCharList[0x00E9] = 0x65; /* e LATIN SMALL LETTER E WITH ACUTE */
    replaceCharList[0x00EA] = 0x65; /* e LATIN SMALL LETTER E WITH CIRCUMFLEX */
    replaceCharList[0x00EB] = 0x65; /* e LATIN SMALL LETTER E WITH DIAERESIS */
    replaceCharList[0x0113] = 0x65; /* e LATIN SMALL LETTER E WITH MACRON */
    replaceCharList[0x0115] = 0x65; /* e LATIN SMALL LETTER E WITH BREVE */
    replaceCharList[0x0117] = 0x65; /* e LATIN SMALL LETTER E WITH DOT ABOVE */
    replaceCharList[0x0119] = 0x65; /* e LATIN SMALL LETTER E WITH OGONEK */
    replaceCharList[0x011B] = 0x65; /* e LATIN SMALL LETTER E WITH CARON */
    replaceCharList[0x0259] = 0x65; /* e LATIN SMALL LETTER SCHWA */

    replaceCharList[0x00C8] = 0x45; /* E LATIN CAPITAL LETTER E WITH GRAVE */
    replaceCharList[0x00C9] = 0x45; /* E LATIN CAPITAL LETTER E WITH ACUTE */
    replaceCharList[0x00CA] = 0x45; /* E LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
    replaceCharList[0x00CB] = 0x45; /* E LATIN CAPITAL LETTER E WITH DIAERESIS */
    replaceCharList[0x0112] = 0x45; /* E LATIN CAPITAL LETTER E WITH MACRON */
    replaceCharList[0x0114] = 0x45; /* E LATIN CAPITAL LETTER E WITH BREVE */
    replaceCharList[0x0116] = 0x45; /* E LATIN CAPITAL LETTER E WITH DOT ABOVE */
    replaceCharList[0x0118] = 0x45; /* E LATIN CAPITAL LETTER E WITH OGONEK */
    replaceCharList[0x011A] = 0x45; /* E LATIN CAPITAL LETTER E WITH CARON */
    replaceCharList[0x018F] = 0x45; /* E LATIN CAPITAL LETTER SCHWA */

    replaceCharList[0x011D] = 0x67; /* g LATIN SMALL LETTER G WITH CIRCUMFLEX */
    replaceCharList[0x011F] = 0x67; /* g LATIN SMALL LETTER G WITH BREVE */
    replaceCharList[0x0121] = 0x67; /* g LATIN SMALL LETTER G WITH DOT ABOVE */
    replaceCharList[0x0123] = 0x67; /* g LATIN SMALL LETTER G WITH CEDILLA */
    replaceCharList[0x01E7] = 0x67; /* g LATIN SMALL LETTER G WITH CARON */
    replaceCharList[0x01F5] = 0x67; /* g LATIN SMALL LETTER G WITH ACUTE */
    replaceCharList[0x1E21] = 0x67; /* g LATIN SMALL LETTER G WITH MACRON */

    replaceCharList[0x011C] = 0x47; /* G LATIN CAPITAL LETTER G WITH CIRCUMFLEX */
    replaceCharList[0x011E] = 0x47; /* G LATIN CAPITAL LETTER G WITH BREVE */
    replaceCharList[0x0120] = 0x47; /* G LATIN CAPITAL LETTER G WITH DOT ABOVE */
    replaceCharList[0x0122] = 0x47; /* G LATIN CAPITAL LETTER G WITH CEDILLA */
    replaceCharList[0x01E6] = 0x47; /* G LATIN CAPITAL LETTER G WITH CARON */
    replaceCharList[0x01F4] = 0x47; /* G LATIN CAPITAL LETTER G WITH ACUTE */
    replaceCharList[0x1E20] = 0x47; /* G LATIN CAPITAL LETTER G WITH MACRON */

    replaceCharList[0x00EC] = 0x69; /* i LATIN SMALL LETTER I WITH GRAVE */
    replaceCharList[0x00ED] = 0x69; /* i LATIN SMALL LETTER I WITH ACUTE */
    replaceCharList[0x00EE] = 0x69; /* i LATIN SMALL LETTER I WITH CIRCUMFLEX */
    replaceCharList[0x00EF] = 0x69; /* i LATIN SMALL LETTER I WITH DIAERESIS */
    replaceCharList[0x0129] = 0x69; /* i LATIN SMALL LETTER I WITH TILDE */
    replaceCharList[0x012B] = 0x69; /* i LATIN SMALL LETTER I WITH MACRON */
    replaceCharList[0x012D] = 0x69; /* i LATIN SMALL LETTER I WITH BREVE */
    replaceCharList[0x012F] = 0x69; /* i LATIN SMALL LETTER I WITH OGONEK */
    replaceCharList[0x01D0] = 0x69; /* i LATIN SMALL LETTER I WITH CARON */
    replaceCharList[0x0131] = 0x69; /* i LATIN SMALL LETTER DOTLESS I */

    replaceCharList[0x00CC] = 0x49; /* I LATIN CAPITAL LETTER I WITH GRAVE */
    replaceCharList[0x00CD] = 0x49; /* I LATIN CAPITAL LETTER I WITH ACUTE */
    replaceCharList[0x00CE] = 0x49; /* I LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
    replaceCharList[0x00CF] = 0x49; /* I LATIN CAPITAL LETTER I WITH DIAERESIS */
    replaceCharList[0x0128] = 0x49; /* I LATIN CAPITAL LETTER I WITH TILDE */
    replaceCharList[0x012A] = 0x49; /* I LATIN CAPITAL LETTER I WITH MACRON */
    replaceCharList[0x012C] = 0x49; /* I LATIN CAPITAL LETTER I WITH BREVE */
    replaceCharList[0x012E] = 0x49; /* I LATIN CAPITAL LETTER I WITH OGONEK */
    replaceCharList[0x0130] = 0x49; /* I LATIN CAPITAL LETTER I WITH DOT ABOVE */

    replaceCharList[0x0137] = 0x6B; /* k LATIN SMALL LETTER K WITH CEDILLA */

    replaceCharList[0x0136] = 0x4B; /* K LATIN CAPITAL LETTER K WITH CEDILLA */

    replaceCharList[0x013A] = 0x6C; /* l LATIN SMALL LETTER L WITH ACUTE */
    replaceCharList[0x013C] = 0x6C; /* l LATIN SMALL LETTER L WITH CEDILLA */
    replaceCharList[0x013E] = 0x6C; /* l LATIN SMALL LETTER L WITH CARON */
    replaceCharList[0x0140] = 0x6C; /* l LATIN SMALL LETTER L WITH MIDDLE DOT */
    replaceCharList[0x0142] = 0x6C; /* l LATIN SMALL LETTER L WITH STROKE */

    replaceCharList[0x0139] = 0x4C; /* L LATIN CAPITAL LETTER L WITH ACUTE */
    replaceCharList[0x013B] = 0x4C; /* L LATIN CAPITAL LETTER L WITH CEDILLA */
    replaceCharList[0x013D] = 0x4C; /* L LATIN CAPITAL LETTER L WITH CARON */
    replaceCharList[0x013F] = 0x4C; /* L LATIN CAPITAL LETTER L WITH MIDDLE DOT */
    replaceCharList[0x0141] = 0x4C; /* L LATIN CAPITAL LETTER L WITH STROKE */

    replaceCharList[0x00F1] = 0x6E; /* n LATIN SMALL LETTER N WITH TILDE */
    replaceCharList[0x0144] = 0x6E; /* n LATIN SMALL LETTER N WITH ACUTE */
    replaceCharList[0x0146] = 0x6E; /* n LATIN SMALL LETTER N WITH CEDILLA */
    replaceCharList[0x0148] = 0x6E; /* n LATIN SMALL LETTER N WITH CARON */

    replaceCharList[0x00D1] = 0x4E; /* N LATIN CAPITAL LETTER N WITH TILDE */
    replaceCharList[0x0143] = 0x4E; /* N LATIN CAPITAL LETTER N WITH ACUTE */
    replaceCharList[0x0145] = 0x4E; /* N LATIN CAPITAL LETTER N WITH CEDILLA */
    replaceCharList[0x0147] = 0x4E; /* N LATIN CAPITAL LETTER N WITH CARON */

    replaceCharList[0x00F2] = 0x6F; /* o LATIN SMALL LETTER O WITH GRAVE */
    replaceCharList[0x00F3] = 0x6F; /* o LATIN SMALL LETTER O WITH ACUTE */
    replaceCharList[0x00F4] = 0x6F; /* o LATIN SMALL LETTER O WITH CIRCUMFLEX */
    replaceCharList[0x00F5] = 0x6F; /* o LATIN SMALL LETTER O WITH TILDE */
    replaceCharList[0x00F6] = 0x6F; /* o LATIN SMALL LETTER O WITH DIAERESIS */
    replaceCharList[0x00F8] = 0x6F; /* o LATIN SMALL LETTER O WITH STROKE */
    replaceCharList[0x014D] = 0x6F; /* o LATIN SMALL LETTER O WITH MACRON */
    replaceCharList[0x014F] = 0x6F; /* o LATIN SMALL LETTER O WITH BREVE */
    replaceCharList[0x01D2] = 0x6F; /* o LATIN SMALL LETTER O WITH CARON */
    replaceCharList[0x01EB] = 0x6F; /* o LATIN SMALL LETTER O WITH OGONEK */
    replaceCharList[0x0151] = 0x6F; /* o LATIN SMALL LETTER O WITH DOUBLE ACUTE */
    replaceCharList[0x0153] = 0x6F; /* LATIN SMALL LIGATURE OE */

    replaceCharList[0x00D2] = 0x4F; /* O LATIN CAPITAL LETTER O WITH GRAVE */
    replaceCharList[0x00D3] = 0x4F; /* O LATIN CAPITAL LETTER O WITH ACUTE */
    replaceCharList[0x00D4] = 0x4F; /* O LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
    replaceCharList[0x00D5] = 0x4F; /* O LATIN CAPITAL LETTER O WITH TILDE */
    replaceCharList[0x00D6] = 0x4F; /* O LATIN CAPITAL LETTER O WITH DIAERESIS */
    replaceCharList[0x00D8] = 0x4F; /* O LATIN CAPITAL LETTER O WITH STROKE */
    replaceCharList[0x014C] = 0x4F; /* O LATIN CAPITAL LETTER O WITH MACRON */
    replaceCharList[0x014E] = 0x4F; /* O LATIN CAPITAL LETTER O WITH BREVE */
    replaceCharList[0x01D1] = 0x4F; /* O LATIN CAPITAL LETTER O WITH CARON */
    replaceCharList[0x01EA] = 0x4F; /* O LATIN CAPITAL LETTER O WITH OGONEK */
    replaceCharList[0x0150] = 0x4F; /* O LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */
    replaceCharList[0x0152] = 0x4F; /* LATIN CAPITAL LIGATURE OE */

    replaceCharList[0x0155] = 0x72; /* r LATIN SMALL LETTER R WITH ACUTE */
    replaceCharList[0x0157] = 0x72; /* r LATIN SMALL LETTER R WITH CEDILLA */
    replaceCharList[0x0159] = 0x72; /* r LATIN SMALL LETTER R WITH CARON */

    replaceCharList[0x0154] = 0x52; /* R LATIN CAPITAL LETTER R WITH ACUTE */
    replaceCharList[0x0156] = 0x52; /* R LATIN CAPITAL LETTER R WITH CEDILLA */
    replaceCharList[0x0158] = 0x52; /* R LATIN CAPITAL LETTER R WITH CARON */

    replaceCharList[0x015B] = 0x73; /* s LATIN SMALL LETTER S WITH ACUTE */
    replaceCharList[0x015D] = 0x73; /* s LATIN SMALL LETTER S WITH CIRCUMFLEX */
    replaceCharList[0x015F] = 0x73; /* s LATIN SMALL LETTER S WITH CEDILLA */
    replaceCharList[0x0161] = 0x73; /* s LATIN SMALL LETTER S WITH CARON */

    replaceCharList[0x015A] = 0x53; /* S LATIN CAPITAL LETTER S WITH ACUTE */
    replaceCharList[0x015C] = 0x53; /* S LATIN CAPITAL LETTER S WITH CIRCUMFLEX */
    replaceCharList[0x015E] = 0x53; /* S LATIN CAPITAL LETTER S WITH CEDILLA */
    replaceCharList[0x0160] = 0x53; /* S LATIN CAPITAL LETTER S WITH CARON */

    replaceCharList[0x00FE] = 0x74; /* t LATIN CAPITAL LETTER THORN */
    replaceCharList[0x0163] = 0x74; /* t LATIN SMALL LETTER T WITH CEDILLA */
    replaceCharList[0x0165] = 0x74; /* t LATIN SMALL LETTER T WITH CARON */
    replaceCharList[0x0167] = 0x74; /* t LATIN SMALL LETTER T WITH STROKE */
    replaceCharList[0x021B] = 0x74; /* t LATIN SMALL LETTER T WITH COMMA BELOW */

    replaceCharList[0x00DE] = 0x54; /* T LATIN CAPITAL LETTER THORN */
    replaceCharList[0x0162] = 0x54; /* T LATIN CAPITAL LETTER T WITH CEDILLA */
    replaceCharList[0x0164] = 0x54; /* T LATIN CAPITAL LETTER T WITH CARON */
    replaceCharList[0x0166] = 0x54; /* T LATIN CAPITAL LETTER T WITH STROKE */

    replaceCharList[0x00F9] = 0x75; /* u LATIN SMALL LETTER U WITH GRAVE */
    replaceCharList[0x00FA] = 0x75; /* u LATIN SMALL LETTER U WITH ACUTE */
    replaceCharList[0x00FB] = 0x75; /* u LATIN SMALL LETTER U WITH CIRCUMFLEX */
    replaceCharList[0x00FC] = 0x75; /* u LATIN SMALL LETTER U WITH DIAERESIS */
    replaceCharList[0x0169] = 0x75; /* u LATIN SMALL LETTER U WITH TILDE */
    replaceCharList[0x016B] = 0x75; /* u LATIN SMALL LETTER U WITH MACRON */
    replaceCharList[0x016D] = 0x75; /* u LATIN SMALL LETTER U WITH BREVE */
    replaceCharList[0x016F] = 0x75; /* u LATIN SMALL LETTER U WITH RING ABOVE */
    replaceCharList[0x0171] = 0x75; /* u LATIN SMALL LETTER U WITH DOUBLE ACUTE */
    replaceCharList[0x0173] = 0x75; /* u LATIN SMALL LETTER U WITH OGONEK */
    replaceCharList[0x01D4] = 0x75; /* u LATIN SMALL LETTER U WITH CARON */

    replaceCharList[0x00D9] = 0x55; /* U LATIN CAPITAL LETTER U WITH GRAVE */
    replaceCharList[0x00DA] = 0x55; /* U LATIN CAPITAL LETTER U WITH ACUTE */
    replaceCharList[0x00DB] = 0x55; /* U LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
    replaceCharList[0x00DC] = 0x55; /* U LATIN CAPITAL LETTER U WITH DIAERESIS */
    replaceCharList[0x0168] = 0x55; /* U LATIN CAPITAL LETTER U WITH TILDE */
    replaceCharList[0x016A] = 0x55; /* U LATIN CAPITAL LETTER U WITH MACRON */
    replaceCharList[0x016C] = 0x55; /* U LATIN CAPITAL LETTER U WITH BREVE */
    replaceCharList[0x016E] = 0x55; /* U LATIN CAPITAL LETTER U WITH RING ABOVE */
    replaceCharList[0x0170] = 0x55; /* U LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */
    replaceCharList[0x0172] = 0x55; /* U LATIN CAPITAL LETTER U WITH OGONEK */
    replaceCharList[0x01D3] = 0x55; /* U LATIN CAPITAL LETTER U WITH CARON */

    replaceCharList[0x00FD] = 0x79; /* y LATIN SMALL LETTER Y WITH ACUTE */
    replaceCharList[0x00FF] = 0x79; /* y LATIN SMALL LETTER Y WITH DIAERESIS */
    replaceCharList[0x0177] = 0x79; /* y LATIN SMALL LETTER Y WITH CIRCUMFLEX */
    replaceCharList[0x0233] = 0x79; /* y LATIN SMALL LETTER Y WITH MACRON */
    replaceCharList[0x1EF3] = 0x79; /* y LATIN SMALL LETTER Y WITH GRAVE */
    replaceCharList[0x1EF9] = 0x79; /* y LATIN SMALL LETTER Y WITH TILDE */

    replaceCharList[0x00DD] = 0x59; /* Y LATIN CAPITAL LETTER Y WITH ACUTE */
    replaceCharList[0x0176] = 0x59; /* Y LATIN CAPITAL LETTER Y WITH CIRCUMFLEX */
    replaceCharList[0x0178] = 0x59; /* Y LATIN CAPITAL LETTER Y WITH DIAERESIS */
    replaceCharList[0x0232] = 0x59; /* Y LATIN CAPITAL LETTER Y WITH MACRON */
    replaceCharList[0x1EF2] = 0x59; /* Y LATIN CAPITAL LETTER Y WITH GRAVE */
    replaceCharList[0x1EF8] = 0x59; /* Y LATIN CAPITAL LETTER Y WITH TILDE */

    replaceCharList[0x017A] = 0x7A; /* z LATIN SMALL LETTER Z WITH ACUTE */
    replaceCharList[0x017C] = 0x7A; /* z LATIN SMALL LETTER Z WITH DOT ABOVE */
    replaceCharList[0x017E] = 0x7A; /* z LATIN SMALL LETTER Z WITH CARON */

    replaceCharList[0x0179] = 0x5A; /* Z LATIN CAPITAL LETTER Z WITH ACUTE */
    replaceCharList[0x017B] = 0x5A; /* Z LATIN CAPITAL LETTER Z WITH DOT ABOVE */
    replaceCharList[0x017D] = 0x5A; /* Z LATIN CAPITAL LETTER Z WITH CARON */
}

int MsgTextConvert::ConvertUTF8ToGSM7bit(OUT unsigned char *(&pDestText), IN int maxLength,
    IN const unsigned char *(&pSrcText), IN int srcTextLen, OUT MSG_LANGUAGE_ID_T *(&pLangId),
    OUT bool *(&bIncludeAbnormalChar))
{
    int utf8Length = 0;
    int gsm7bitLength = 0;
    int ucs2Length = 0;

    if (srcTextLen <= 0 && pSrcText) {
        utf8Length = strlen((char *)pSrcText);
        srcTextLen = utf8Length;
    } else {
        utf8Length = srcTextLen;
    }

    int maxUCS2Length = utf8Length; /* max # of UCS2 chars, NOT bytes. when all utf8 chars are only one byte,
                                       UCS2Length is maxUCS2 Length. otherwise (ex: 2 bytes of UTF8 is one char)
                                       UCS2Length must be  less than utf8Length  */

    WCHAR *pUCS2Text = nullptr;
    unique_ptr<WCHAR *, void (*)(WCHAR * *(&))> buf(&pUCS2Text, UniquePtrDeleterOneDimension);
    pUCS2Text = (WCHAR *)new (std::nothrow) char[maxUCS2Length * sizeof(WCHAR)];
    if (pUCS2Text == nullptr) {
        return gsm7bitLength;
    }
    (void)memset_s(pUCS2Text, maxUCS2Length * sizeof(WCHAR), 0x00, maxUCS2Length * sizeof(WCHAR));

    HILOG_INFO("srcTextLen = %{public}d", srcTextLen);
    HILOG_INFO("temp buffer size = %{public}d", maxUCS2Length * sizeof(WCHAR));
    HILOG_INFO("max dest Length = %{public}d", maxLength);

    ucs2Length = ConvertUTF8ToUCS2((unsigned char *)pUCS2Text, maxUCS2Length * sizeof(WCHAR), pSrcText, srcTextLen);
    gsm7bitLength = ConvertUCS2ToGSM7bit(pDestText, maxLength, reinterpret_cast<unsigned char *>(pUCS2Text),
        ucs2Length, pLangId, bIncludeAbnormalChar);
    return gsm7bitLength;
}

int MsgTextConvert::ConvertUTF8ToUCS2(
    OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen)
{
    gsize textLen;
    unsigned char *unicodeTemp = (unsigned char *)pDestText;
    gsize ucs2Length = 0;
    gsize remainedBuffer = maxLength;

#ifdef CONVERT_DUMP
    int srcLen = srcTextLen;
    const unsigned char *pTempSrcText = pSrcText;
    const unsigned char *pTempDestText = pDestText;
#endif

    if (maxLength == 0 || pSrcText == nullptr || pDestText == nullptr) {
        HILOG_ERROR("UTF8 to UCS2 Failed as text length is 0");
        return -1;
    }
    /* nullptr terminated string */
    if (srcTextLen == -1) {
        textLen = strlen((char *)pSrcText);
        srcTextLen = textLen;
    } else {
        textLen = srcTextLen;
    }

    GIConv cd;
    int err = 0;
    cd = g_iconv_open("UTF16BE", "UTF8");
    if (cd != nullptr) {
        err = g_iconv(cd, (char **)&pSrcText, (gsize *)&textLen, (char **)&unicodeTemp, (gsize *)&remainedBuffer);
    }

    if (err < 0) {
        HILOG_ERROR("Error in g_iconv.");
        ucs2Length = -1;
    } else {
        ucs2Length = maxLength - remainedBuffer;
    }

#ifdef CONVERT_DUMP
    HILOG_INFO("########## Dump UTF8 -> UCS2");
    ConvertDumpTextToHex((unsigned char *)pTempSrcText, srcLen);
    ConvertDumpTextToHex((unsigned char *)pTempDestText, ucs2Length);
#endif

    g_iconv_close(cd);
    return ucs2Length;
}

int MsgTextConvert::ConvertUTF8ToAuto(OUT unsigned char *pDestText, IN int maxLength,
    IN const unsigned char *pSrcText, IN int srcTextLen, OUT SMS_CODING_SCHEME_E *pCharType)
{
    int ret = 0;
    int tempTextLen = 0;
    int utf8Length = 0;
    int gsm7bitLength = 0;
    int ucs2Length = 0;
    bool bUnknown = false;
    utf8Length = srcTextLen;

    int maxUCS2Length = utf8Length;
    WCHAR pUCS2Text[maxUCS2Length];
    ret = memset_s(pUCS2Text, sizeof(pUCS2Text), 0x00, maxUCS2Length * sizeof(WCHAR));
    if (ret != EOK) {
        HILOG_ERROR("memset_s err = %{public}d", ret);
    }
    ucs2Length = ConvertUTF8ToUCS2((unsigned char *)pUCS2Text, maxUCS2Length * sizeof(WCHAR), pSrcText, srcTextLen);
    if (ucs2Length < 0) {
        *pCharType = SMS_CODING_8BIT;
        tempTextLen = (srcTextLen > maxLength) ? maxLength : srcTextLen;
        ret = memcpy_s(pDestText, tempTextLen, pSrcText, tempTextLen);
        if (ret != EOK) {
            HILOG_ERROR("memcpy_s err = %{public}d", ret);
        }
        return tempTextLen;
    } else {
#ifndef FEATURE_SMS_CDMA
        gsm7bitLength =
            ConvertUCS2ToGSM7bitAuto(pDestText, maxLength, (unsigned char *)pUCS2Text, ucs2Length, &bUnknown);
#else
        gsm7bitLength = ConvertUCS2ToASCII(pDestText, maxLength, (unsigned char *)pUCS2Text, ucs2Length, &bUnknown);
#endif
        if (bUnknown == true) {
            *pCharType = SMS_CODING_UCS2;
            if (ucs2Length > 0) {
                tempTextLen = (ucs2Length > maxLength) ? maxLength : ucs2Length;
                ret = memcpy_s(pDestText, tempTextLen, pUCS2Text, tempTextLen);
                if (ret != EOK) {
                    HILOG_ERROR("memcpy_s err = %{public}d", ret);
                }
                return tempTextLen;
            }
        } else {
#ifndef FEATURE_SMS_CDMA
            *pCharType = SMS_CODING_7BIT;
#else
            *pCharType = MSG_ENCODE_ASCII7BIT;
#endif
        }
        return gsm7bitLength;
    }
}

int MsgTextConvert::ConvertGSM7bitToUTF8(OUT unsigned char *pDestText, IN int maxLength,
    IN const unsigned char *pSrcText, IN int srcTextLen, const IN MSG_LANG_INFO_S *pLangInfo)
{
    int utf8Length = 0;
    int ucs2Length = 0;
    int maxUCS2Length = srcTextLen; /* max # of UCS2 chars, NOT bytes. when all gsm7 chars are only one byte(-there
                                       is no extension), UCS2Length is maxUCS2 Length. otherwise(ex: gsm7 char
                                       starts with 0x1b) UCS2Length must be less than gsm7 length */

    WCHAR pUCS2Text[maxUCS2Length];
    if (memset_s(pUCS2Text, sizeof(pUCS2Text), 0x00, sizeof(pUCS2Text)) != EOK) {
        HILOG_ERROR("ConvertGSM7bitToUTF8 memset_sfail");
        return utf8Length;
    }
    HILOG_INFO("srcTextLen = %{public}d", srcTextLen);
    HILOG_INFO("max dest Length = %{public}d", maxLength);
    ucs2Length = ConvertGSM7bitToUCS2(reinterpret_cast<unsigned char *>(pUCS2Text), maxUCS2Length * sizeof(WCHAR),
        pSrcText, srcTextLen, pLangInfo);
    utf8Length = ConvertUCS2ToUTF8(pDestText, maxLength, (unsigned char *)pUCS2Text, ucs2Length);
    return utf8Length;
}

int MsgTextConvert::ConvertUCS2ToUTF8(
    OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen)
{
    gsize textLen;
    gsize remainedBuffer = maxLength;
    int utf8Length;

#ifdef CONVERT_DUMP
    int srcLen = srcTextLen;
    const unsigned char *pTempSrcText = pSrcText;
#endif
    unsigned char *pTempDestText = pDestText;
    if (srcTextLen == 0 || pSrcText == nullptr || pDestText == nullptr || maxLength == 0) {
        HILOG_ERROR("UCS2 to UTF8 Failed as text length is 0");
        return false;
    }

    if (srcTextLen == -1) {
        textLen = strlen((char *)pSrcText);
        srcTextLen = textLen;
    } else {
        textLen = srcTextLen;
    }

    GIConv cd;
    int err = 0;
    cd = g_iconv_open("UTF8", "UTF16BE");
    if (cd != nullptr) {
        err = g_iconv(cd, (char **)&pSrcText, (gsize *)&textLen, (char **)&pDestText, (gsize *)&remainedBuffer);
    }

    if (err != 0) {
        HILOG_INFO("g_iconv() return value = %d", err);
    }
    utf8Length = maxLength - remainedBuffer;
    pTempDestText[utf8Length] = 0x00;
#ifdef CONVERT_DUMP
    HILOG_INFO("\n########## Dump UCS2 -> UTF8");
    ConvertDumpTextToHex((unsigned char *)pTempSrcText, srcLen);
    ConvertDumpTextToHex((unsigned char *)pTempDestText, utf8Length);
#endif
    g_iconv_close(cd);
    return utf8Length;
}

int MsgTextConvert::ConvertEUCKRToUTF8(
    OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen)
{
    gsize textLen;
    gsize remainedBuffer = maxLength;
    int utf8Length;

#ifdef CONVERT_DUMP
    int srcLen = srcTextLen;
    const unsigned char *pTempSrcText = pSrcText;
#endif
    unsigned char *pTempDestText = pDestText;
    if (srcTextLen == 0 || pSrcText == nullptr || pDestText == nullptr || maxLength == 0) {
        HILOG_ERROR("EUCKR to UTF8 Failed as text length is 0");
        return false;
    }
    if (srcTextLen == -1) {
        textLen = strlen((char *)pSrcText);
        srcTextLen = textLen;
    } else {
        textLen = srcTextLen;
    }

    GIConv cd;
    int err = 0;
    cd = g_iconv_open("UTF8", "EUCKR");
    if (cd != nullptr) {
        err = g_iconv(cd, (char **)&pSrcText, (gsize *)&textLen, (char **)&pDestText, (gsize *)&remainedBuffer);
    }
    HILOG_INFO("g_iconv() return value = %d", err);
    utf8Length = maxLength - remainedBuffer;
    pTempDestText[utf8Length] = 0x00;
#ifdef CONVERT_DUMP
    HILOG_INFO("\n########## Dump EUCKR -> UTF8\n");
    ConvertDumpTextToHex((unsigned char *)pTempSrcText, srcLen);
    ConvertDumpTextToHex((unsigned char *)pTempDestText, utf8Length);
#endif
    g_iconv_close(cd);
    return utf8Length;
}

int MsgTextConvert::ConvertSHIFTJISToUTF8(
    OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen) const
{
    gsize textLen;
    gsize remainedBuffer = maxLength;
    int utf8Length;

#ifdef CONVERT_DUMP
    int srcLen = srcTextLen;
    const unsigned char *pTempSrcText = pSrcText;
#endif
    unsigned char *pTempDestText = pDestText;
    if (srcTextLen == 0 || pSrcText == nullptr || pDestText == nullptr || maxLength == 0) {
        HILOG_ERROR("EUCKR to UTF8 Failed as text length is 0");
        return false;
    }
    if (srcTextLen == -1) {
        textLen = strlen((char *)pSrcText);
        srcTextLen = textLen;
    } else {
        textLen = srcTextLen;
    }

    GIConv cd;
    int err = 0;
    cd = g_iconv_open("UTF8", "SHIFT-JIS");
    if (cd != nullptr) {
        err = g_iconv(cd, (char **)&pSrcText, (gsize *)&textLen, (char **)&pDestText, (gsize *)&remainedBuffer);
    }

    HILOG_INFO("g_iconv() return value = %{public}d", err);
    utf8Length = maxLength - remainedBuffer;
    pTempDestText[utf8Length] = 0x00;
#ifdef CONVERT_DUMP
    HILOG_INFO("\n########## Dump SHIFT-JIS -> UTF8");
    ConvertDumpTextToHex((unsigned char *)pTempSrcText, srcLen);
    ConvertDumpTextToHex((unsigned char *)pTempDestText, utf8Length);
#endif
    g_iconv_close(cd);
    return utf8Length;
}

int MsgTextConvert::ConvertUCS2ToGSM7bit(OUT unsigned char *pDestText, IN int maxLength,
    IN const unsigned char *pSrcText, IN int srcTextLen, OUT MSG_LANGUAGE_ID_T *pLangId,
    OUT bool *bIncludeAbnormalChar)
{
    /* for UNICODE */
    int outTextLen = 0;
    unsigned char lowerByte;
    unsigned char upperByte;

    if (srcTextLen == 0 || pSrcText == nullptr || pDestText == nullptr || maxLength == 0) {
        HILOG_ERROR("Invalid parameter.");
        return -1;
    }

    std::map<unsigned short, unsigned char>::iterator itChar;
    std::map<unsigned short, unsigned char>::iterator itExt;
    std::map<unsigned short, unsigned char>::iterator itReplace;

    unsigned char currType = MSG_DEFAULT_CHAR;
    unsigned char newType = MSG_DEFAULT_CHAR;
    unsigned short inText;

    /* Get Language Type by checking each character */
    for (int index = 0; index < srcTextLen; index++) {
        upperByte = pSrcText[index++];
        lowerByte = pSrcText[index];
        inText = (upperByte << 0x08) & 0xFF00;
        inText = inText | lowerByte;
        itExt = extCharList_.find(inText);
        if (itExt != extCharList_.end()) {
            newType = (unsigned char)itExt->second;
            if (newType >= currType) {
                if (inText == 0x00e7 && currType <= MSG_TURKISH_CHAR) {
                    currType = MSG_TURKISH_CHAR;
                } else {
                    currType = newType;
                }
            }
        }
    }
#ifdef CONVERT_DUMP
    HILOG_INFO("charType : [%{public}d]", currType);
#endif
    for (int index = 0; index < srcTextLen; index++) {
        upperByte = pSrcText[index++];
        lowerByte = pSrcText[index];
        inText = (upperByte << 0x08) & 0xFF00;
        inText = inText | lowerByte;
        /* Check Default Char */
        itChar = ucs2toGSM7DefList.find(inText);
        if (itChar != ucs2toGSM7DefList.end()) {
            pDestText[outTextLen++] = (unsigned char)itChar->second;
        } else {
            if (currType == MSG_GSM7EXT_CHAR) {
                itExt = ucs2toGSM7ExtList.find(inText);
                if (itExt != ucs2toGSM7ExtList.end()) {
                    /* prevent buffer overflow */
                    if (maxLength <= outTextLen + 1) {
                        HILOG_ERROR("Buffer Full.");
                        break;
                    }
                    pDestText[outTextLen++] = 0x1B;
                    pDestText[outTextLen++] = (unsigned char)itExt->second;
                } else {
                    itReplace = replaceCharList.find(inText);
                    if (itReplace != replaceCharList.end()) {
                        pDestText[outTextLen++] = (unsigned char)itReplace->second;
                    } else {
                        pDestText[outTextLen++] = 0x3F;
                    }
                    *bIncludeAbnormalChar = true;
#ifdef CONVERT_DUMP
                    HILOG_INFO("Abnormal character is included. inText : [%{public}04x]", inText);
#endif
                }
            } else if (currType == MSG_TURKISH_CHAR) {
                *pLangId = MSG_ID_TURKISH_LANG;
                itExt = ucs2toTurkishList.find(inText);
                if (itExt != ucs2toTurkishList.end()) {
                    /* prevent buffer overflow */
                    if (maxLength <= outTextLen + 1) {
                        HILOG_ERROR("Buffer Full.");
                        break;
                    }
                    pDestText[outTextLen++] = 0x1B;
                    pDestText[outTextLen++] = (unsigned char)itExt->second;
                } else {
                    itReplace = replaceCharList.find(inText);
                    if (itReplace != replaceCharList.end()) {
                        pDestText[outTextLen++] = (unsigned char)itReplace->second;
                    } else {
                        pDestText[outTextLen++] = 0x3F;
                    }
                    *bIncludeAbnormalChar = true;
#ifdef CONVERT_DUMP
                    HILOG_INFO("Abnormal character is included. inText : [%{public}04x]", inText);
#endif
                }
            } else if (currType == MSG_SPANISH_CHAR) {
                *pLangId = MSG_ID_SPANISH_LANG;
                itExt = ucs2toSpanishList.find(inText);
                if (itExt != ucs2toSpanishList.end()) {
                    /* prevent buffer overflow */
                    if (maxLength <= outTextLen + 1) {
                        HILOG_ERROR("Buffer Full.");
                        break;
                    }
                    pDestText[outTextLen++] = 0x1B;
                    pDestText[outTextLen++] = (unsigned char)itExt->second;
                } else {
                    itReplace = replaceCharList.find(inText);
                    if (itReplace != replaceCharList.end()) {
                        pDestText[outTextLen++] = (unsigned char)itReplace->second;
                    } else {
                        pDestText[outTextLen++] = 0x3F;
                    }
                    *bIncludeAbnormalChar = true;
#ifdef CONVERT_DUMP
                    HILOG_INFO("Abnormal character is included. inText : [%{public}04x]", inText);
#endif
                }
            } else if (currType == MSG_PORTUGUESE_CHAR) {
                *pLangId = MSG_ID_PORTUGUESE_LANG;
                itExt = ucs2toPortuList.find(inText);
                if (itExt != ucs2toPortuList.end()) {
                    /* prevent buffer overflow */
                    if (maxLength <= outTextLen + 1) {
                        HILOG_ERROR("Buffer Full.");
                        break;
                    }
                    pDestText[outTextLen++] = 0x1B;
                    pDestText[outTextLen++] = (unsigned char)itExt->second;
                } else {
                    itReplace = replaceCharList.find(inText);
                    if (itReplace != replaceCharList.end()) {
                        pDestText[outTextLen++] = (unsigned char)itReplace->second;
                    } else {
                        pDestText[outTextLen++] = 0x3F;
                    }
                    *bIncludeAbnormalChar = true;
#ifdef CONVERT_DUMP
                    HILOG_INFO("Abnormal character is included. inText : [%{public}04x]", inText);
#endif
                }
            } else {
                itReplace = replaceCharList.find(inText);
                if (itReplace != replaceCharList.end()) {
                    pDestText[outTextLen++] = (unsigned char)itReplace->second;
                } else {
                    pDestText[outTextLen++] = 0x3F;
                }
                *bIncludeAbnormalChar = true;
#ifdef CONVERT_DUMP
                HILOG_INFO("Abnormal character is included. inText : [%{public}04x]", inText);
#endif
            }
        }
        /* prevent buffer overflow */
        if (maxLength <= outTextLen) {
            HILOG_ERROR("Buffer full.");
            break;
        }
    }

#ifdef CONVERT_DUMP
    HILOG_INFO("\n########## Dump UCS2 -> GSM7bit");
    ConvertDumpTextToHex((unsigned char *)pSrcText, srcTextLen);
    ConvertDumpTextToHex((unsigned char *)pDestText, outTextLen);
#endif
    return outTextLen;
}

#ifndef FEATURE_SMS_CDMA
int MsgTextConvert::ConvertUCS2ToGSM7bitAuto(OUT unsigned char *pDestText, IN int maxLength,
    IN const unsigned char *pSrcText, IN int srcTextLen, OUT bool *pUnknown)
{
    /* for UNICODE */
    int outTextLen = 0;
    unsigned char lowerByte;
    unsigned char upperByte;

    if (srcTextLen == 0 || pSrcText == nullptr || pDestText == nullptr || maxLength == 0) {
        HILOG_ERROR("UCS2 to GSM7bit Failed as text length is 0");
        return -1;
    }

    std::map<unsigned short, unsigned char>::iterator itChar;
    std::map<unsigned short, unsigned char>::iterator itExt;

    unsigned short inText;

    for (int index = 0; index < srcTextLen; index++) {
        upperByte = pSrcText[index++];
        lowerByte = pSrcText[index];

        inText = (upperByte << 0x08) & 0xFF00;
        inText = inText | lowerByte;

        /* Check Default Char */
        itChar = ucs2toGSM7DefList.find(inText);

        if (itChar != ucs2toGSM7DefList.end()) {
            pDestText[outTextLen++] = (unsigned char)itChar->second;
        } else {
            itExt = ucs2toGSM7ExtList.find(inText);

            if (itExt != ucs2toGSM7ExtList.end()) {
                /* prevent buffer overflow */
                if (maxLength <= outTextLen + 1) {
                    HILOG_ERROR("Buffer Full.");
                    break;
                }

                pDestText[outTextLen++] = 0x1B;
                pDestText[outTextLen++] = (unsigned char)itExt->second;
            } else {
                HILOG_INFO("Abnormal character is included. inText : [%{public}04x]", inText);
                *pUnknown = true;
                return 0;
            }
        }

        /* prevent buffer overflow */
        if (maxLength <= outTextLen) {
            HILOG_ERROR("Buffer full");
            break;
        }
    }

#ifdef CONVERT_DUMP
    HILOG_INFO("\n########## Dump UCS2 -> GSM7bit");
    ConvertDumpTextToHex((unsigned char *)pSrcText, srcTextLen);
    ConvertDumpTextToHex((unsigned char *)pDestText, outTextLen);
#endif

    return outTextLen;
}

#else
int MsgTextConvert::convertUCS2ToASCII(OUT unsigned char *pDestText, IN int maxLength,
    IN const unsigned char *pSrcText, IN int srcTextLen, OUT bool *pUnknown)
{
    /* for UNICODE */
    int outTextLen = 0;
    unsigned char lowerByte;
    unsigned char upperByte;

    if (srcTextLen == 0 || pSrcText == nullptr || pDestText == nullptr || maxLength == 0) {
        HILOG_INFO("UCS2 to GSM7bit Failed as text length is 0");
        return -1;
    }

    std::map<unsigned short, unsigned char>::iterator itChar;
    std::map<unsigned short, unsigned char>::iterator itExt;

    unsigned short inText;

    for (int index = 0; index < srcTextLen; index++) {
        upperByte = pSrcText[index++];
        lowerByte = pSrcText[index];

        inText = (upperByte << 0x08) & 0xFF00;
        inText = inText | lowerByte;

        /* Check Default Char */
        if (inText > 0x007f) {
            HILOG_INFO("Abnormal character is included. inText : [%{public}04x]", inText);
            *pUnknown = true;
            return 0;
        }

        pDestText[outTextLen++] = (unsigned char)inText;

        /* prevent buffer overflow */
        if (maxLength <= outTextLen) {
            HILOG_INFO("Buffer full\n");
            break;
        }
    }

#ifdef CONVERT_DUMP
    HILOG_INFO("\n########## Dump UCS2 -> GSM7bit");
    ConvertDumpTextToHex((unsigned char *)pSrcText, srcTextLen);
    ConvertDumpTextToHex((unsigned char *)pDestText, outTextLen);
#endif

    return outTextLen;
}
#endif

int MsgTextConvert::ConvertGSM7bitToUCS2(OUT unsigned char *pDestText, IN int maxLength,
    IN const unsigned char *pSrcText, IN int srcTextLen, const IN MSG_LANG_INFO_S *pLangInfo)
{
    int outTextLen = 0;
    unsigned char lowerByte = 0;
    unsigned char upperByte = 0;

    if (srcTextLen == 0 || pSrcText == nullptr || pDestText == nullptr || maxLength == 0) {
        HILOG_ERROR("UCS2 to GSM7bit Failed as text length is 0");
        return -1;
    }

    for (int i = 0; i < srcTextLen; i++) {
        if (maxLength == 0) {
            break;
        }
        if (pSrcText[i] >= 0x80) {
            /* error */
            HILOG_ERROR("a_pTextString[i]=%x, The alpha isn't the gsm 7bit code", pSrcText[i]);
            return -1;
        }
        if (pLangInfo->bLockingShift == true) { /* National Language Locking Shift */
            HILOG_INFO("Locking Shift [%d]", pLangInfo->lockingLang);
            if (pLangInfo->lockingLang == MSG_ID_TURKISH_LANG) {
                /* Check Escape */
                if (g_TurkishLockingToUCS2[pSrcText[i]] == 0x001B) {
                    i++;
                    if (pLangInfo->bSingleShift == true) { /* National Language Single Shift */
                        HILOG_INFO("Single Shift [%{public}d]", pLangInfo->singleLang);
                        if (pLangInfo->singleLang == MSG_ID_TURKISH_LANG) {
                            lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        } else if (pLangInfo->singleLang == MSG_ID_SPANISH_LANG) {
                            lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        } else if (pLangInfo->singleLang == MSG_ID_PORTUGUESE_LANG) {
                            lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        } else {
                            lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        }
                    } else { /* GSM 7 bit Default Alphabet Extension Table */
                        lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
                        upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                    }
                } else { /* TURKISH - National Language Locking Shift */
                    lowerByte = g_TurkishLockingToUCS2[pSrcText[i]] & 0x00FF;
                    upperByte = (g_TurkishLockingToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                }
            } else if (pLangInfo->lockingLang == MSG_ID_PORTUGUESE_LANG) {
                /* Check Escape */
                if (g_PortuLockingToUCS2[pSrcText[i]] == 0x001B) {
                    i++;
                    if (pLangInfo->bSingleShift == true) { /* National Language Single Shift */
                        HILOG_INFO("Single Shift [%{public}d]", pLangInfo->singleLang);
                        if (pLangInfo->singleLang == MSG_ID_TURKISH_LANG) {
                            lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        } else if (pLangInfo->singleLang == MSG_ID_SPANISH_LANG) {
                            lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        } else if (pLangInfo->singleLang == MSG_ID_PORTUGUESE_LANG) {
                            lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        } else {
                            lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
                            upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                        }
                    } else { /* GSM 7 bit Default Alphabet Extension Table */
                        lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
                        upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                    }
                } else { /* PORTUGUESE - National Language Locking Shift */
                    lowerByte = g_PortuLockingToUCS2[pSrcText[i]] & 0x00FF;
                    upperByte = (g_PortuLockingToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                }
            }
        } else {
            /* Check Escape */
            if (g_GSM7BitToUCS2[pSrcText[i]] == 0x001B) {
                i++;
                if (pLangInfo->bSingleShift == true) { /* National Language Single Shift */
                    HILOG_INFO("Single Shift [%{public}d]", pLangInfo->singleLang);
                    if (pLangInfo->singleLang == MSG_ID_TURKISH_LANG) {
                        lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
                        upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                    } else if (pLangInfo->singleLang == MSG_ID_SPANISH_LANG) {
                        lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
                        upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                    } else if (pLangInfo->singleLang == MSG_ID_PORTUGUESE_LANG) {
                        lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
                        upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                    } else {
                        lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
                        upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                    }
                } else { /* GSM 7 bit Default Alphabet Extension Table */
                    lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
                    upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
                }
            } else {
                lowerByte = g_GSM7BitToUCS2[pSrcText[i]] & 0x00FF;
                upperByte = (g_GSM7BitToUCS2[pSrcText[i]] & 0xFF00) >> 0x08;
            }
        }
        pDestText[outTextLen++] = upperByte;
        pDestText[outTextLen++] = lowerByte;
        maxLength -= 0x02;
    }

#ifdef CONVERT_DUMP
    HILOG_INFO("\n########## Dump GSM7bit -> UCS2");
    ConvertDumpTextToHex((unsigned char *)pSrcText, srcTextLen);
    ConvertDumpTextToHex((unsigned char *)pDestText, outTextLen);
#endif
    return outTextLen;
}

void MsgTextConvert::ConvertDumpTextToHex(const unsigned char *pText, int length)
{
    if (pText == nullptr) {
        return;
    }
    HILOG_INFO("=======================================");
    HILOG_INFO("   Dump Text To Hex - Length :%{public}d", length);
    HILOG_INFO("=======================================");
    for (int i = 0; i < length; i++) {
        HILOG_INFO("[%02x]", pText[i]);
    }
    HILOG_INFO("=======================================");
}
} // namespace SMS
} // namespace OHOS