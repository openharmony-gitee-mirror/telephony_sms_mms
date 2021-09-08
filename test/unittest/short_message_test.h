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

#ifndef SHORT_MESSAGE_TEST_H
#define SHORT_MESSAGE_TEST_H

#include "short_message.h"

namespace OHOS {
namespace Telephony {
class ShortMessageTest {
public:
    void TestGetVisibleMessageBody(const ShortMessage &message) const;
    void TestShowShortMessage(const ShortMessage &message) const;
    ShortMessage *TestCreateMessage() const;
};
} // namespace Telephony
} // namespace OHOS
#endif