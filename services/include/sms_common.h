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
#ifndef SMS_COMMON_H
#define SMS_COMMON_H
namespace OHOS {
namespace SMS {
#define SMS_FAIL_RETRY 0
#define SMS_EVENT_NEW_SMS_REPLY 48
#define MSG_SMS_RETRY_DELIVERY 49

enum SmsHandleResult {
    SMS_RESULT_OK,
    SMS_RESULT_PROCESSED,
    SMS_RESULT_OUT_OF_MEMORY,
    SMS_RESULT_REPEATED_ERROR,
    SMS_RESULT_UNKOWN_ERROR,
};

enum AckIncomeCause {
    SMS_ACK_RESULT_OK = 0x00,
    SMS_ACK_PROCESSED = 0x01,
    SMS_ACK_REPEATED_ERROR = 0x02,
    SMS_ACK_OUT_OF_MEMORY = 0x03,
    SMS_ACK_UNKOWN_ERROR = 0xFF,
};

enum NetWorkType {
    NET_TYPE_GSM = 0,
    NET_TYPE_CDMA,
    NET_TYPE_UNKOWN,
};

enum NetDomainType {
    NET_DOMAIN_CS = 0,
    NET_DOMAIN_IMS,
    NET_DOMAIN_UNKOWN,
};
} // namespace SMS
} // namespace OHOS
#endif
