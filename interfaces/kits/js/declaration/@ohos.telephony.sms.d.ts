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

import {AsyncCallback} from "./basic";

/**
 * Provides the capabilities and methods for obtaining Short Message Service (SMS) management objects.
 *
 * @since 6
 */
declare namespace sms {

  function createMessage(pdu: Array<number>, specification: string, callback: AsyncCallback<ShortMessage>): void;
  function createMessage(pdu: Array<number>, specification: string): Promise<ShortMessage>;

  // need ohos.permission.SEND_MESSAGES
  function sendMessage(options: SendMessageOptions): void;

  export interface ShortMessage {
    visibleMessageBody: string;
    visibleRawAddress: string;
    messageClass: ShortMessageClass;
    protocolId: number;
    scAddress: string;
    scTimestamp: number;
    isReplaceMessage: boolean;
    hasReplyPath: boolean;
    pdu: Array<number>;
    status: number;
    isSmsStatusReportMessage: boolean;
    emailAddress: string;
    emailMessageBody: string;
    userRawData: Array<number>;
    isEmailMessage: boolean;
  }

  export enum ShortMessageClass {
    /** Indicates an unknown type. */
    UNKNOWN,
    /** Indicates an instant message, which is displayed immediately after being received. */
    INSTANT_MESSAGE,
    /** Indicates an SMS message that can be stored on the device or SIM card based on the storage status. */
    OPTIONAL_MESSAGE,
    /** Indicates an SMS message containing SIM card information, which is to be stored in a SIM card. */
    SIM_MESSAGE,
    /** Indicates an SMS message to be forwarded to another device. */
    FORWARD_MESSAGE
  }

  export interface SendMessageOptions {
    slotId: number;
    destinationHost: string;
    serviceCenter?: string;
    /** If the content is a string, this is a short message. If the content is a byte array, this is a data message. */
    content: string | Array<number>;
    /** If send data message, destinationPort is mandatory. Otherwise is optional. */
    destinationPort?: number;
    sendCallback?: AsyncCallback<ISendShortMessageCallback>;
    deliveryCallback?: AsyncCallback<IDeliveryShortMessageCallback>;
  }

  export interface ISendShortMessageCallback {
    result: SendSmsResult;
    url: string;
    isLastPart: boolean;
  }

  export interface IDeliveryShortMessageCallback {
    pdu: Array<number>;
  }

  export enum SendSmsResult {
    /**
     * Indicates that the SMS message is successfully sent.
     */
    SEND_SMS_SUCCESS = 0,

    /**
     * Indicates that sending the SMS message fails due to an unknown reason.
     */
    SEND_SMS_FAILURE_UNKNOWN = 1,

    /**
     * Indicates that sending the SMS fails because the modem is powered off.
     */
    SEND_SMS_FAILURE_RADIO_OFF = 2,

    /**
     * Indicates that sending the SMS message fails because the network is unavailable
     * or does not support sending or reception of SMS messages.
     */
    SEND_SMS_FAILURE_SERVICE_UNAVAILABLE = 3
  }
}

export default sms;