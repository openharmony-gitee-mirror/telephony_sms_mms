# 短彩信<a name="ZH-CN_TOPIC_0000001105544742"></a>


- [简介<a name="section117mcpsimp"></a>](#简介)
- [目录<a name="section125mcpsimp"></a>](#目录)
- [约束<a name="section129mcpsimp"></a>](#约束)
- [接口说明<a name="section134mcpsimp"></a>](#接口说明)
- [使用说明<a name="section170mcpsimp"></a>](#使用说明)
  - [发送短信<a name="section172mcpsimp"></a>](#发送短信)
  - [创建ShortMessage对象<a name="section181mcpsimp"></a>](#创建shortmessage对象)
- [相关仓<a name="section189mcpsimp"></a>](#相关仓)

## 简介<a name="section117mcpsimp"></a>

短彩信模块为移动数据用户提供短信收发和彩信编解码功能，主要功能有GSM/CDMA短信收发、短信PDU（Protocol data unit，协议数据单元）编解码、Wap Push接收处理 、小区广播接收、彩信通知、 彩信编解码和SIM卡短信记录增删改查等。

**图 1**  短彩信模块架构图<a name="fig420553511549"></a>
![](figures/zh-cn_architecture-of-the-sms-and-mms-module.png)

短彩信模块由接口管理类、短信发送管理类、短信接收管理类，和彩信编解码工具类组成。其中：

-   接口管理类：SmsInterfaceManager 负责对外提供短信发送、SIM卡短信记录操作和配置相关接口，负责创建SmsSendManager 和SmsReceiveManager 对象。
-   短信发送管理类： SmsSendManager 负责监听 IMS网络状态；创建GSM\(GsmSmsSender\) 和CDMA\(CdmaSmsSender\) 的短信发送器对象，并根据网络制式调度对应发送对象发送短信功能。
-   短信接收管理类： SmsReceiveManager 负责短信接收，监听来自RIL层的新短信信息；创建GSM\(GsmSmsReceiveHander\) 和CDMA\(CdmaSmsReceiveHandler\) 对象；创建SmsWapPushHandler 和SmsCellBroadcastHandler 对象。
-   彩信编解码类：负责彩信PDU的编解码处理。

## 目录<a name="section125mcpsimp"></a>

```
/base/telephony/sms_mms
├─ interfaces               # 对外暴露的接口
│  └─ kits
├─ sa_profile               # 启动配置文件
├─ services                 # 服务内部代码
│  ├─ include               # 头文件目录
│  ├─ cdma                  # CDMA制式源文件
│  └─ gsm                   # GSM制式源文件
├─ test                     # 单元测试目录
└─ utils                    # 通用工具相关
```

## 约束<a name="section129mcpsimp"></a>

-   开发语言：JavaScript。
-   软件约束，需要与以下服务配合使用：Telephony核心服务（core\_service）；依赖[glib](https://gitlab.gnome.org/GNOME/glib)库。
-   硬件约束，需要搭载的设备支持以下硬件：可以进行独立蜂窝通信的Modem以及SIM卡。

## 接口说明<a name="section134mcpsimp"></a>

**表 1**  短彩信对外提供的接口

<a name="table136mcpsimp"></a>
<table><thead align="left"><tr id="row142mcpsimp"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="entry143mcpsimpp0"><a name="entry143mcpsimpp0"></a><a name="entry143mcpsimpp0"></a>接口名称</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="entry144mcpsimpp0"><a name="entry144mcpsimpp0"></a><a name="entry144mcpsimpp0"></a>接口描述</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="entry145mcpsimpp0"><a name="entry145mcpsimpp0"></a><a name="entry145mcpsimpp0"></a>所需权限</p>
</th>
</tr>
</thead>
<tbody><tr id="row146mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1011717318415"><a name="p1011717318415"></a><a name="p1011717318415"></a>function sendMessage(options: SendMessageOptions): void;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="entry148mcpsimpp0"><a name="entry148mcpsimpp0"></a><a name="entry148mcpsimpp0"></a>发送短信，通过callback异步获取短信发送结果。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry149mcpsimpp0"><a name="entry149mcpsimpp0"></a><a name="entry149mcpsimpp0"></a>SystemPermission.SEND_MESSAGES</p>
</td>
</tr>
<tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function createMessage(pdu: Array&lt;number&gt;, specification: string, callback: AsyncCallback&lt;ShortMessage&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>根据协议数据单元（PDU）和指定的短信协议创建短信实例，使用callback方式异步返回创建的短信实例。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>无</p>
</td>
</tr>
<tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function createMessage(pdu: Array&lt;number&gt;, specification: string): Promise&lt;ShortMessage&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>根据协议数据单元（PDU）和指定的短信协议创建短信实例，使用Promise方式异步返回创建的短信实例。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>无</p>
</td>
</tr>
<tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function getDefaultSmsSlotId(callback: AsyncCallback&lt;number&gt;): void</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>获取发送短信的默认SIM卡槽，使用callback方式异步返回默认SIM卡槽。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>无</p>
</td>
</tr>
<tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function getDefaultSmsSlotId():Promise&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>获取发送短信的默认SIM卡，使用Promise方式异步返回默认SIM卡槽。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>无</p>
</td>
</tr>
<tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function setSmscAddr(slotId: number, smscAddr: string, callback: AsyncCallback&lt;void&gt;): void</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>设置短信服务中心地址，使用callback方式异步返回undefined。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>ohos.permission.SET_TELEPHONY_STATE</p>
</td>
</tr>
<tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function setSmscAddr(slotId: number, smscAddr: string): Promise&lt;void&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>设置短信服务中心地址，使用Promise方式异步返回undefined。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>ohos.permission.SET_TELEPHONY_STATE</p>
</td>
</tr>
    <tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function getSmscAddr(slotId: number, callback: AsyncCallback&lt;string&gt;): void</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>获取短信服务中心地址，使用callback方式异步返回短信服务中心地址。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>ohos.permission.GET_TELEPHONY_STATE</p>
</td>
</tr>
    <tr id="row166mcpsimp"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="entry167mcpsimpp0"><a name="entry167mcpsimpp0"></a><a name="entry167mcpsimpp0"></a>function getSmscAddr(slotId: number): Promise&lt;string&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p43948592416"><a name="p43948592416"></a><a name="p43948592416"></a>获取短信服务中心地址，使用Promise方式异步返回短信服务中心地址。</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="entry169mcpsimpp0"><a name="entry169mcpsimpp0"></a><a name="entry169mcpsimpp0"></a>ohos.permission.GET_TELEPHONY_STATE</p>
</td>
</tr>
</tbody>
</table>

**表 2**  发送短信接口参数SendMessageOptions说明

<a name="table137771821149"></a>

<table><thead align="left"><tr id="row16777142181416"><th class="cellrowborder" valign="top" width="25%" id="mcps1.2.5.1.1"><p id="p1677762111420"><a name="p1677762111420"></a><a name="p1677762111420"></a>参数</p>
</th>
<th class="cellrowborder" valign="top" width="25%" id="mcps1.2.5.1.2"><p id="p32826484210"><a name="p32826484210"></a><a name="p32826484210"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="25%" id="mcps1.2.5.1.3"><p id="p117778231415"><a name="p117778231415"></a><a name="p117778231415"></a>描述</p>
</th>
<th class="cellrowborder" valign="top" width="25%" id="mcps1.2.5.1.4"><p id="p1725516817152"><a name="p1725516817152"></a><a name="p1725516817152"></a>是否必填</p>
</th>
</tr>
</thead>
<tbody><tr id="row1577719210142"><td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.1 "><p id="p197771201411"><a name="p197771201411"></a><a name="p197771201411"></a>slotId</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.2 "><p id="p16282184822111"><a name="p16282184822111"></a><a name="p16282184822111"></a>number</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.3 "><p id="p07777221417"><a name="p07777221417"></a><a name="p07777221417"></a>卡槽id</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.4 "><p id="p22552080151"><a name="p22552080151"></a><a name="p22552080151"></a>是</p>
</td>
</tr>
<tr id="row177779231416"><td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.1 "><p id="p12777928146"><a name="p12777928146"></a><a name="p12777928146"></a>destinationHost</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.2 "><p id="p19282948182112"><a name="p19282948182112"></a><a name="p19282948182112"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.3 "><p id="p157771125144"><a name="p157771125144"></a><a name="p157771125144"></a>接收端的电话号码</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.4 "><p id="p192550811156"><a name="p192550811156"></a><a name="p192550811156"></a>是</p>
</td>
</tr>
<tr id="row20777162111419"><td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.1 "><p id="p1777814218147"><a name="p1777814218147"></a><a name="p1777814218147"></a>serviceCenter</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.2 "><p id="p0282348112110"><a name="p0282348112110"></a><a name="p0282348112110"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.3 "><p id="p137781826141"><a name="p137781826141"></a><a name="p137781826141"></a>短信中心服务地址</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.4 "><p id="p1025511815152"><a name="p1025511815152"></a><a name="p1025511815152"></a>否</p>
</td>
</tr>
<tr id="row187781529145"><td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.1 "><p id="p1377810211419"><a name="p1377810211419"></a><a name="p1377810211419"></a>content</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.2 "><p id="p6282648152118"><a name="p6282648152118"></a><a name="p6282648152118"></a>content | Array&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.3 "><p id="p47789219147"><a name="p47789219147"></a><a name="p47789219147"></a>短信内容</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.4 "><p id="p325517891510"><a name="p325517891510"></a><a name="p325517891510"></a>是</p>
</td>
</tr>
<tr id="row37780215141"><td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.1 "><p id="p37784221410"><a name="p37784221410"></a><a name="p37784221410"></a>destinationPort</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.2 "><p id="p1428274815211"><a name="p1428274815211"></a><a name="p1428274815211"></a>number</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.3 "><p id="p87781524149"><a name="p87781524149"></a><a name="p87781524149"></a>接收端端口号</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.4 "><p id="p7255386157"><a name="p7255386157"></a><a name="p7255386157"></a>发送数据短信必填</p>
</td>
</tr>
<tr id="row14778152151417"><td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.1 "><p id="p1877814231418"><a name="p1877814231418"></a><a name="p1877814231418"></a>sendCallback</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.2 "><p id="p3283448172111"><a name="p3283448172111"></a><a name="p3283448172111"></a>AsyncCallback&lt;ISendShortMessageCallback&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.3 "><p id="p16778182121416"><a name="p16778182121416"></a><a name="p16778182121416"></a>发送结果回调</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.4 "><p id="p7255108141513"><a name="p7255108141513"></a><a name="p7255108141513"></a>是</p>
</td>
</tr>
<tr id="row107782241418"><td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.1 "><p id="p147781321142"><a name="p147781321142"></a><a name="p147781321142"></a>deliveryCallback</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.2 "><p id="p172835481214"><a name="p172835481214"></a><a name="p172835481214"></a>AsyncCallback&lt;IDeliveryShortMessageCallback&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.3 "><p id="p5778624149"><a name="p5778624149"></a><a name="p5778624149"></a>送达报告回调</p>
</td>
<td class="cellrowborder" valign="top" width="25%" headers="mcps1.2.5.1.4 "><p id="p172553821519"><a name="p172553821519"></a><a name="p172553821519"></a>是</p>
</td>
</tr>
</tbody>
</table>

**表 3**  ISendShortMessageCallback类型说明

<a name="table6490122972417"></a>
<table><thead align="left"><tr id="row194901529162416"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="p849172910245"><a name="p849172910245"></a><a name="p849172910245"></a>参数</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="p849182916242"><a name="p849182916242"></a><a name="p849182916242"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p144911029162413"><a name="p144911029162413"></a><a name="p144911029162413"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row16491142916241"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p549162942410"><a name="p549162942410"></a><a name="p549162942410"></a>result</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p144911129112418"><a name="p144911129112418"></a><a name="p144911129112418"></a>SendSmsResult</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p14491129172419"><a name="p14491129172419"></a><a name="p14491129172419"></a>发送结果</p>
</td>
</tr>
<tr id="row204911029112418"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p4491172917241"><a name="p4491172917241"></a><a name="p4491172917241"></a>url</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p1549132914244"><a name="p1549132914244"></a><a name="p1549132914244"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p349115291248"><a name="p349115291248"></a><a name="p349115291248"></a>URL地址</p>
</td>
</tr>
<tr id="row19491172912247"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p54911329122419"><a name="p54911329122419"></a><a name="p54911329122419"></a>isLastPart</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p154918290248"><a name="p154918290248"></a><a name="p154918290248"></a>boolean</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p104913290240"><a name="p104913290240"></a><a name="p104913290240"></a>是否是最后一条短信</p>
</td>
</tr>
</tbody>
</table>

**表 4**  SendSmsResult枚举值

<a name="table2375639122617"></a>
<table><thead align="left"><tr id="row1376123932614"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="p2376339102616"><a name="p2376339102616"></a><a name="p2376339102616"></a>名称</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="p153769391263"><a name="p153769391263"></a><a name="p153769391263"></a>值</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p6376193918267"><a name="p6376193918267"></a><a name="p6376193918267"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row1376103912610"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p0376113932616"><a name="p0376113932616"></a><a name="p0376113932616"></a>SEND_SMS_SUCCESS</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p3376193932611"><a name="p3376193932611"></a><a name="p3376193932611"></a>0</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1737613982617"><a name="p1737613982617"></a><a name="p1737613982617"></a>发送成功</p>
</td>
</tr>
<tr id="row20376143917261"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p2037653913260"><a name="p2037653913260"></a><a name="p2037653913260"></a>SEND_SMS_FAILURE_UNKNOWN</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p4376139182611"><a name="p4376139182611"></a><a name="p4376139182611"></a>1</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p153761539102611"><a name="p153761539102611"></a><a name="p153761539102611"></a>发送失败，原因未知</p>
</td>
</tr>
<tr id="row163766391268"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1376239112618"><a name="p1376239112618"></a><a name="p1376239112618"></a>SEND_SMS_FAILURE_RADIO_OFF</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p1737683972619"><a name="p1737683972619"></a><a name="p1737683972619"></a>2</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p17376103942619"><a name="p17376103942619"></a><a name="p17376103942619"></a>发送失败，因为Modem关闭</p>
</td>
</tr>
<tr id="row837613915269"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p143761039162613"><a name="p143761039162613"></a><a name="p143761039162613"></a>SEND_SMS_FAILURE_SERVICE_UNAVAILABLE</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p1237793992616"><a name="p1237793992616"></a><a name="p1237793992616"></a>3</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p537743910267"><a name="p537743910267"></a><a name="p537743910267"></a>发送失败，因为网络不可用</p>
</td>
</tr>
</tbody>
</table>

**表 5**  IDeliveryShortMessageCallback类型说明

<a name="table13234515285"></a>
<table><thead align="left"><tr id="row1432445122815"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="p1232134532811"><a name="p1232134532811"></a><a name="p1232134532811"></a>参数</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="p1432114502811"><a name="p1432114502811"></a><a name="p1432114502811"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p123224515283"><a name="p123224515283"></a><a name="p123224515283"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row183254532810"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p9331453286"><a name="p9331453286"></a><a name="p9331453286"></a>pdu</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p333114542810"><a name="p333114542810"></a><a name="p333114542810"></a>Array&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1559582618298"><a name="p1559582618298"></a><a name="p1559582618298"></a>PDU（Protocol data unit，协议数据单元）数组</p>
</td>
</tr>
</tbody>
</table>
**表 6**  createMessage接口参数说明

<a name="table12931820163016"></a>

<table><thead align="left"><tr id="row8293152043010"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="p72931220133012"><a name="p72931220133012"></a><a name="p72931220133012"></a>参数</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="p13293172053015"><a name="p13293172053015"></a><a name="p13293172053015"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p16293162014301"><a name="p16293162014301"></a><a name="p16293162014301"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row112937206307"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p13293320193019"><a name="p13293320193019"></a><a name="p13293320193019"></a>pdu</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p429315201307"><a name="p429315201307"></a><a name="p429315201307"></a>Array&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p12293102014308"><a name="p12293102014308"></a><a name="p12293102014308"></a>PDU（Protocol data unit，协议数据单元）数组</p>
</td>
</tr>
<tr id="row6293182012308"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p2293820143020"><a name="p2293820143020"></a><a name="p2293820143020"></a>specification</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p14293152012301"><a name="p14293152012301"></a><a name="p14293152012301"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1293420163019"><a name="p1293420163019"></a><a name="p1293420163019"></a>协议类型（3gpp或3gpp2）</p>
</td>
</tr>
</tbody>
</table>
**表 7**  getDefaultSmsSlotId接口参数说明

<a name="table12931820163016"></a>

<table><thead align="left"><tr id="row8293152043010"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="p72931220133012"><a name="p72931220133012"></a><a name="p72931220133012"></a>参数</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="p13293172053015"><a name="p13293172053015"></a><a name="p13293172053015"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p16293162014301"><a name="p16293162014301"></a><a name="p16293162014301"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row112937206307"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p13293320193019"><a name="p13293320193019"></a><a name="p13293320193019"></a>callback</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p429315201307"><a name="p429315201307"></a><a name="p429315201307"></a>AsyncCallback&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="46.1%" headers="mcps1.1.5.1.4 "><p id="p440103618212"><a name="p440103618212"></a><a name="p440103618212"></a>回调函数。</p>
<a name="ul9211219141510"></a><a name="ul9211219141510"></a><ul id="ul9211219141510"><li>0：卡槽1</li><li>1：卡槽2</li></ul>
</td>
</td>
</tr>
</tbody>
</table>

**表 8**  setSmscAddr接口参数说明

<a name="table12931820163016"></a>

<table><thead align="left"><tr id="row94271538039"><th class="cellrowborder" valign="top" width="17.66%" id="mcps1.1.5.1.1"><p id="p12427738837"><a name="p12427738837"></a><a name="p12427738837"></a>参数名</p>
</th>
<th class="cellrowborder" valign="top" width="28.18%" id="mcps1.1.5.1.2"><p id="p104274381133"><a name="p104274381133"></a><a name="p104274381133"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="8.06%" id="mcps1.1.5.1.3"><p id="p24274381834"><a name="p24274381834"></a><a name="p24274381834"></a>必填</p>
</th>
<th class="cellrowborder" valign="top" width="46.1%" id="mcps1.1.5.1.4"><p id="p3427738938"><a name="p3427738938"></a><a name="p3427738938"></a>说明</p>
</th>
</tr>
</thead>
<tbody><tr id="row184651552734"><td class="cellrowborder" valign="top" width="17.66%" headers="mcps1.1.5.1.1 "><p id="p546614524317"><a name="p546614524317"></a><a name="p546614524317"></a>slotId</p>
</td>
<td class="cellrowborder" valign="top" width="28.18%" headers="mcps1.1.5.1.2 "><p id="p104661852439"><a name="p104661852439"></a><a name="p104661852439"></a>number</p>
</td>
<td class="cellrowborder" valign="top" width="8.06%" headers="mcps1.1.5.1.3 "><p id="p1635101510417"><a name="p1635101510417"></a><a name="p1635101510417"></a>是</p>
</td>
<td class="cellrowborder" valign="top" width="46.1%" headers="mcps1.1.5.1.4 "><p id="p34931544121518"><a name="p34931544121518"></a><a name="p34931544121518"></a>SIM卡槽ID：</p>
<a name="ul3493164411516"></a><a name="ul3493164411516"></a><ul id="ul3493164411516"><li>0：卡槽1</li><li>1：卡槽2</li></ul>
</td>
</tr>
<tr id="row8924552151"><td class="cellrowborder" valign="top" width="17.66%" headers="mcps1.1.5.1.1 "><p id="p119251252255"><a name="p119251252255"></a><a name="p119251252255"></a>smscAddr</p>
</td>
<td class="cellrowborder" valign="top" width="28.18%" headers="mcps1.1.5.1.2 "><p id="p129253521658"><a name="p129253521658"></a><a name="p129253521658"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="8.06%" headers="mcps1.1.5.1.3 "><p id="p8958171314615"><a name="p8958171314615"></a><a name="p8958171314615"></a>是</p>
</td>
<td class="cellrowborder" valign="top" width="46.1%" headers="mcps1.1.5.1.4 "><p id="p14925352154"><a name="p14925352154"></a><a name="p14925352154"></a>短信服务中心（SMSC）地址。</p>
</td>
</tr>
<tr id="row4427938632"><td class="cellrowborder" valign="top" width="17.66%" headers="mcps1.1.5.1.1 "><p id="p64275383316"><a name="p64275383316"></a><a name="p64275383316"></a>callback</p>
</td>
<td class="cellrowborder" valign="top" width="28.18%" headers="mcps1.1.5.1.2 "><p id="p11427183811320"><a name="p11427183811320"></a><a name="p11427183811320"></a>AsyncCallback&lt;void&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="8.06%" headers="mcps1.1.5.1.3 "><p id="p20427238435"><a name="p20427238435"></a><a name="p20427238435"></a>是</p>
</td>
<td class="cellrowborder" valign="top" width="46.1%" headers="mcps1.1.5.1.4 "><p id="p174278382318"><a name="p174278382318"></a><a name="p174278382318"></a>回调函数。</p>
</td>
</tr>
</tbody>
</table>

**表 9**  getSmscAddr接口参数说明

<a name="table12931820163016"></a>

<table><thead align="left"><tr id="row108899481812"><th class="cellrowborder" valign="top" width="17.66%" id="mcps1.1.5.1.1"><p id="p10889242180"><a name="p10889242180"></a><a name="p10889242180"></a>参数名</p>
</th>
<th class="cellrowborder" valign="top" width="28.18%" id="mcps1.1.5.1.2"><p id="p13889142181"><a name="p13889142181"></a><a name="p13889142181"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="8.06%" id="mcps1.1.5.1.3"><p id="p089019419181"><a name="p089019419181"></a><a name="p089019419181"></a>必填</p>
</th>
<th class="cellrowborder" valign="top" width="46.1%" id="mcps1.1.5.1.4"><p id="p48901943187"><a name="p48901943187"></a><a name="p48901943187"></a>说明</p>
</th>
</tr>
</thead>
<tbody><tr id="row1789084151812"><td class="cellrowborder" valign="top" width="17.66%" headers="mcps1.1.5.1.1 "><p id="p16890124201811"><a name="p16890124201811"></a><a name="p16890124201811"></a>slotId</p>
</td>
<td class="cellrowborder" valign="top" width="28.18%" headers="mcps1.1.5.1.2 "><p id="p138907411185"><a name="p138907411185"></a><a name="p138907411185"></a>number</p>
</td>
<td class="cellrowborder" valign="top" width="8.06%" headers="mcps1.1.5.1.3 "><p id="p789064191816"><a name="p789064191816"></a><a name="p789064191816"></a>是</p>
</td>
<td class="cellrowborder" valign="top" width="46.1%" headers="mcps1.1.5.1.4 "><p id="p1085441121615"><a name="p1085441121615"></a><a name="p1085441121615"></a>SIM卡槽ID：</p>
<a name="ul285420116165"></a><a name="ul285420116165"></a><ul id="ul285420116165"><li>0：卡槽1</li><li>1：卡槽2</li></ul>
</td>
</tr>
<tr id="row18906441819"><td class="cellrowborder" valign="top" width="17.66%" headers="mcps1.1.5.1.1 "><p id="p78915410183"><a name="p78915410183"></a><a name="p78915410183"></a>callback</p>
</td>
<td class="cellrowborder" valign="top" width="28.18%" headers="mcps1.1.5.1.2 "><p id="p1789116417184"><a name="p1789116417184"></a><a name="p1789116417184"></a>AsyncCallback&lt;string&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="8.06%" headers="mcps1.1.5.1.3 "><p id="p108912047185"><a name="p108912047185"></a><a name="p108912047185"></a>是</p>
</td>
<td class="cellrowborder" valign="top" width="46.1%" headers="mcps1.1.5.1.4 "><p id="p188916414188"><a name="p188916414188"></a><a name="p188916414188"></a>回调函数。</p>
</td>
</tr>
</tbody>
</table>

**表 10**  创建短信异步回调ShortMessage说明

<a name="table12931820163016"></a>

<table><thead align="left"><tr id="row1369315832016"><th class="cellrowborder" valign="top" width="23.34%" id="mcps1.1.4.1.1"><p id="p1871313815402"><a name="p1871313815402"></a><a name="p1871313815402"></a>变量</p>
</th>
<th class="cellrowborder" valign="top" width="19.06%" id="mcps1.1.4.1.2"><p id="p93231392263"><a name="p93231392263"></a><a name="p93231392263"></a>类型</p>
</th>
<th class="cellrowborder" valign="top" width="57.599999999999994%" id="mcps1.1.4.1.3"><p id="p145500193814"><a name="p145500193814"></a><a name="p145500193814"></a>说明</p>
</th>
</tr>
</thead>
<tbody><tr id="row13344184392519"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p85256525218"><a name="p85256525218"></a><a name="p85256525218"></a>emailAddress</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p107115501923"><a name="p107115501923"></a><a name="p107115501923"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p202695241341"><a name="p202695241341"></a><a name="p202695241341"></a>电子邮件地址。</p>
</td>
</tr>
<tr id="row206942882013"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p773310541229"><a name="p773310541229"></a><a name="p773310541229"></a>emailMessageBody</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p181441457223"><a name="p181441457223"></a><a name="p181441457223"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p15573202884013"><a name="p15573202884013"></a><a name="p15573202884013"></a>电子邮件正文。</p>
</td>
</tr>
<tr id="row116945832019"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p12155162811211"><a name="p12155162811211"></a><a name="p12155162811211"></a>hasReplyPath</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p14794525926"><a name="p14794525926"></a><a name="p14794525926"></a>boolean</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p13754174585919"><a name="p13754174585919"></a><a name="p13754174585919"></a>收到的短信是否包含“TP-回复路径”，默认为false。</p>
<a name="ul1470498307"></a><a name="ul1470498307"></a><ul id="ul1470498307"><li>“TP-回复路径”：移动电话可循发送SMS消息的短消息中心进行回复。</li></ul>
</td>
</tr>
<tr id="row253218315237"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p140111161132"><a name="p140111161132"></a><a name="p140111161132"></a>isEmailMessage</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p146951018131"><a name="p146951018131"></a><a name="p146951018131"></a>boolean</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p887603743519"><a name="p887603743519"></a><a name="p887603743519"></a>收到的短信是否为电子邮件。</p>
</td>
</tr>
<tr id="row859720172311"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p166361920323"><a name="p166361920323"></a><a name="p166361920323"></a>isReplaceMessage</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p01812236216"><a name="p01812236216"></a><a name="p01812236216"></a>boolean</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p022761413014"><a name="p022761413014"></a><a name="p022761413014"></a>收到的短信是否为“替换短信”，默认为false。</p>
<a name="ul42891226707"></a><a name="ul42891226707"></a><ul id="ul42891226707"><li>“替换短信”有关详细信息，参见 “3GPP TS 23.040 9.2.3.9”。</li></ul>
</td>
</tr>
<tr id="row764111193410"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p1410313450214"><a name="p1410313450214"></a><a name="p1410313450214"></a>isSmsStatusReportMessage</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p1644311475213"><a name="p1644311475213"></a><a name="p1644311475213"></a>boolean</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p5701488344"><a name="p5701488344"></a><a name="p5701488344"></a>当前消息是否为“短信状态报告”，默认为false。</p>
<a name="ul14539561111"></a><a name="ul14539561111"></a><ul id="ul14539561111"><li>“短信状态报告”是一种特定格式的短信，被用来从Service Center到Mobile Station传送状态报告。</li></ul>
</td>
</tr>
<tr id="row109475100345"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p19837318220"><a name="p19837318220"></a><a name="p19837318220"></a>messageClass</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p161336593118"><a name="p161336593118"></a><a name="p161336593118"></a><a href="#section141712166453">ShortMessageClass</a></p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p55610014386"><a name="p55610014386"></a><a name="p55610014386"></a>短信类型。</p>
</td>
</tr>
<tr id="row13623612346"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p5114831626"><a name="p5114831626"></a><a name="p5114831626"></a>pdu</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p17402133611219"><a name="p17402133611219"></a><a name="p17402133611219"></a>Array&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p177241026133411"><a name="p177241026133411"></a><a name="p177241026133411"></a>SMS消息中的协议数据单元 （PDU）。</p>
</td>
</tr>
<tr id="row498012918341"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p4552841928"><a name="p4552841928"></a><a name="p4552841928"></a>protocolId</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p11390871024"><a name="p11390871024"></a><a name="p11390871024"></a>number</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p16572083814"><a name="p16572083814"></a><a name="p16572083814"></a>发送短信时使用的协议标识。</p>
</td>
</tr>
<tr id="row87231262349"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p32951810223"><a name="p32951810223"></a><a name="p32951810223"></a>scAddress</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p4631111219219"><a name="p4631111219219"></a><a name="p4631111219219"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p136431815349"><a name="p136431815349"></a><a name="p136431815349"></a>短信服务中心（SMSC）地址。</p>
</td>
</tr>
<tr id="row1294964403418"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p1253815422"><a name="p1253815422"></a><a name="p1253815422"></a>scTimestamp</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p1498814171127"><a name="p1498814171127"></a><a name="p1498814171127"></a>number</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p294716100345"><a name="p294716100345"></a><a name="p294716100345"></a>SMSC时间戳。</p>
</td>
</tr>
<tr id="row169548183414"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p16266133910216"><a name="p16266133910216"></a><a name="p16266133910216"></a>status</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p1955441329"><a name="p1955441329"></a><a name="p1955441329"></a>number</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p59491344113419"><a name="p59491344113419"></a><a name="p59491344113419"></a>SMS-STATUS-REPORT消息中的短信状态指示短信服务中心（SMSC）发送的短信状态。</p>
</td>
</tr>
<tr id="row1526817245344"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p156631801631"><a name="p156631801631"></a><a name="p156631801631"></a>userRawData</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p861981315311"><a name="p861981315311"></a><a name="p861981315311"></a>Array&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p8487922358"><a name="p8487922358"></a><a name="p8487922358"></a>除协议头部的用户数据，即未解码的短信内容。</p>
</td>
</tr>
<tr id="row183801940133512"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p396619450120"><a name="p396619450120"></a><a name="p396619450120"></a>visibleMessageBody</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p819218501110"><a name="p819218501110"></a><a name="p819218501110"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p85619013384"><a name="p85619013384"></a><a name="p85619013384"></a>短信正文。</p>
</td>
</tr>
<tr id="row98751137153511"><td class="cellrowborder" valign="top" width="23.34%" headers="mcps1.1.4.1.1 "><p id="p12309536110"><a name="p12309536110"></a><a name="p12309536110"></a>visibleRawAddress</p>
</td>
<td class="cellrowborder" valign="top" width="19.06%" headers="mcps1.1.4.1.2 "><p id="p559135510111"><a name="p559135510111"></a><a name="p559135510111"></a>string</p>
</td>
<td class="cellrowborder" valign="top" width="57.599999999999994%" headers="mcps1.1.4.1.3 "><p id="p55617011389"><a name="p55617011389"></a><a name="p55617011389"></a>发送者地址。</p>
</td>
</tr>
</tbody>
</table>

完整的JS API说明以及实例代码请参考：[收发短信](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/js-reference/apis/js-apis-sms.md)

## 使用说明<a name="section170mcpsimp"></a>

### 发送短信<a name="section172mcpsimp"></a>

以发送普通文本短信为例，主要步骤和代码如下：

1.  构造SendMessageOptions对象，传入必要的参数；若关注发送结果或送达报告，需要传入sendCallback或deliveryCallback对象。
2.  可以通过callback或者Promise的方式调用sendMessage接口。
3.  该接口为异步接口，相关执行结果会从callback中返回。

    ```
    import sms from "@ohos.telephony.sms";

    let msg: SendMessageOptions = {
      slotId: 0,
      destinationHost: '123xxxxxxxx',
      content: '这是一封短信',
      sendCallback: (err, data) => {
        if (err) {
          // 接口调用失败，err非空
          console.error(`failed to send message because ${err.message}`);
          return;
        }
        // 接口调用成功，err为空
        console.log(`success to send message: ${data.result}`);
      }
    }

    // 调用接口
    sms.sendMessage(msg);
    ```


### 创建ShortMessage对象<a name="section181mcpsimp"></a>

以解析3gpp类型的PDU，调用createMessage为例，主要步骤和代码如下：

1.  构造短信的PDU，指定协议类型为3gpp或者3gpp2。
2.  可以通过callback或者Promise的方式调用createMessage接口。
3.  该接口为异步接口，成功后获取ShortMessage 对象属性来得到解析后的短信息数据。

    ```
    import sms from "@ohos.telephony.sms";

    let pdu = [80, 80, 80]; // 这里只是表示是短信PDU，并非真实短信
    let specification = "3gpp";

    // 调用接口【callback方式】
    sms.createMessage(pdu, specification, (err, value) => {
      if (err) {
        // 接口调用失败，err非空
        console.error(`failed to createMessage because ${err.message}`);
        return;
      }
      // 接口调用成功，err为空
      console.log(`success to createMessage: ${value}`);
    });

    // 调用接口【Promise方式】
    let promise = sms.createMessage(pdu, specification);
    promise.then((value) => {
      // 接口调用成功，此处可以实现成功场景分支代码。
      console.log(`success to createMessage: ${value}`);
    }).catch((err) => {
      // 接口调用失败，此处可以实现失败场景分支代码。
      console.error(`failed to createMessage because ${err.message}`);
    });
    ```


## 相关仓<a name="section189mcpsimp"></a>

[电话服务子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E7%94%B5%E8%AF%9D%E6%9C%8D%E5%8A%A1%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

**telephony_sms_mms** 

[telephony_core_service](https://gitee.com/openharmony/telephony_core_service/blob/master/README_zh.md)