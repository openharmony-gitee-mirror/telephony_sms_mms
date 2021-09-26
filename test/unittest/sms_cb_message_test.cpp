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

#include "sms_cb_message_test.h"

#include <iostream>

#include "sms_cb_message.h"

namespace OHOS {
namespace Telephony {
void SmsCbMessageTest::ProcessInput(bool &loopFlag) const
{
    int inputCMD = 0;
    std::cin >> inputCMD;
    while (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore();
        std::cin >> inputCMD;
    }
    std::cout << "inputCMD is:" << inputCMD << std::endl;
    switch (inputCMD) {
        case 0x00: {
            TestUMTSMessage();
            break;
        }
        case 0x01: {
            TestEtwsNormal();
            break;
        }
        case 0x02: {
            TestEtwsCancel();
            break;
        }
        case 0x03: {
            TestEtwsTest();
            break;
        }
        default:
            loopFlag = false;
            break;
    }
}

void SmsCbMessageTest::ProcessTest() const
{
    bool loopFlag = true;
    while (loopFlag) {
        std::cout << "\n-----------start test cbmessage api--------------\n"
                     "usage:please input a cmd num:\n"
                     "0:TestUMTSMessage\n"
                     "1:TestEtwsNormal\n"
                     "2:TestEtwsCancel\n"
                     "3:TestEtwsTest\r\n"
                     "Other key:exit \n"
                  << std::endl;
        ProcessInput(loopFlag);
    }
}

void SmsCbMessageTest::TestUMTSMessage() const
{
    const std::string data(
        "01a41f51101102ea3030a830ea30a230e130fc30eb914d4fe130c630b930c8000d000a305330"
        "8c306f8a669a137528306e30e130c330bb30"
        "fc30b8306730593002000d000aff08003200"
        "3000310033002f00310031002f003252ea30"
        "0037002000310035003a00340034ff09000d"
        "000aff0830a830ea30a25e02ff0900000000"
        "000000000000000000000000000000000000"
        "000000000000000000000000000000000000"
        "000000000000000022");
    std::shared_ptr<SmsCbMessage> msg = SmsCbMessage::CreateCbMessage(data);
    if (msg == nullptr) {
        std::cout << "CreateCbMessage fail." << std::endl;
    }
    std::cout << "msginfo: " << msg->ToString() << std::endl;
}

void SmsCbMessageTest::TestEtwsNormal() const
{
    const std::string data(
        "000011001101"
        "0D0A5BAE57CE770C531790E85C716CBF3044573065B930675730"
        "9707767A751F30025F37304463FA308C306B5099304830664E0B30553044FF086C178C615E81FF09"
        "0000000000000000000000000000");
    std::shared_ptr<SmsCbMessage> msg = SmsCbMessage::CreateCbMessage(data);
    if (msg == nullptr) {
        std::cout << "CreateCbMessage fail." << std::endl;
    }
    std::cout << "msginfo: " << msg->ToString() << std::endl;
}

void SmsCbMessageTest::TestEtwsCancel() const
{
    const std::string data(
        "000011001101"
        "0D0A5148307B3069002800310030003A0035"
        "00320029306E7DCA602557309707901F5831309253D66D883057307E3059FF086C178C615E81FF09"
        "00000000000000000000000000000000000000000000");
    std::shared_ptr<SmsCbMessage> msg = SmsCbMessage::CreateCbMessage(data);
    if (msg == nullptr) {
        std::cout << "CreateCbMessage fail." << std::endl;
    }
    std::cout << "msginfo: " << msg->ToString() << std::endl;
}

void SmsCbMessageTest::TestEtwsTest() const
{
    const std::string data(
        "000011031101"
        "0D0A5BAE57CE770C531790E85C716CBF3044"
        "573065B9306757309707300263FA308C306B5099304830664E0B30553044FF086C178C615E81FF09"
        "00000000000000000000000000000000000000000000");
    std::shared_ptr<SmsCbMessage> msg = SmsCbMessage::CreateCbMessage(data);
    if (msg == nullptr) {
        std::cout << "CreateCbMessage fail." << std::endl;
    }
    std::cout << "msginfo: " << msg->ToString() << std::endl;
}
} // namespace Telephony
} // namespace OHOS