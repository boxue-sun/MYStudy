<figure>
<img src="media/image2.png" style="width:6.72292in;height:9.69653in" />
</figure>

**雷达信控数据上报协议**

版本：V1.0.0

更新日期：2025-07-22

<img src="media/image3.png" style="width:2.10972in;height:0.80139in" />

**联系方式**

中信科智联科技有限公司

地址：北京市朝阳区安定路5号院19号楼城奥大厦13层

邮编：100029

公司网站：https://www.cictci.com/

技术支持邮箱：v2x-service@cictci.com

**前言**

中信科智联科技有限公司提供该文档内容以支持客户的产品设计。客户须按照文档中提供的规范、参数来设计产品。同时，您理解并同意，我司提供的参考设计仅作为示例。您同意在设计您目标产品时使用您独立的分析、评估和判断。在使用本文档所指导的任何软硬件或服务之前，请仔细阅读本声明。

**版权声明**

版权所有中信科智联科技有限公司。保留一切权利。

未经中信科智联科技有限公司书面许可，任何单位和个人不得擅自摘抄、复制本文档的部分或全部内容，并不得以任何形式传播。

本手册描述的产品中，可能包含中信科智联科技有限公司及其可能存在的许可人享有版权的软件。除非获得相关权利人的许可，否则，人和人不能以任何形式对前述软件进行复制、分发、修改、摘录、反编译、反汇编、解密、反向工程、出租、转让、分许可等侵犯软件版权的行为，但是适用法禁止此类限制的除外。

**注意**

由于产品版本升级或其他原因，中信科智联科技有限公司保留随时修改本文档中任何信息的权利，无需提前通知且不承担任何责任。除非另有约定，本文档仅作为使用指导，文中的所有陈述、信息和建议不构成任何明示或暗示的担保。

**责任限制**

在适用法律允许的范围内，中信科智联科技有限公司在任何情况下，都不对因使用本手册相关内容及本手册描述的产品而产生的任何特殊的、附带的、间接的、继发性的损害进行赔偿，也不对任何利润、数据、商誉或预期节约的损失进行赔偿。

在相关法律允许的范围内，在任何情况下，中信科智联科技有限公司对您因为使用描述的产品而遭受的损失的最大责任（除在涉及人身伤害的情况中根据使用的法律规定的损害赔偿外）以您购买本产品所支付的价款为限。

**进出口管制**

若需将本手册描述的产品（包括但不限于产品中的软件及技术数据等）出口、再出口或者进口，您应遵守相关国家或有关政府适用的进出口管制法律、法规、程序、禁令或获得相应许可为前提。除本公司事先承诺，不对该出口、再出口或者进口不侵犯目的国或地区第三方权益进行任何明示或暗示的承诺。您应自行承担因未遵守前述约定造成的不利后果并赔偿由此给中信科智联科技有限公司造成的所有损失及费用。中信科智联公司认为有违背进出口管制风险，有权拒绝履行部分或全部义务，直至履行业务不再受到进出口管制影响为止，如该合理预期履行义务会持续受到影响，有权单方面中止、终止直至解除合同。

**第三方权利**

您理解本文档可能涉及一个或多个属于第三方的软硬件和文档（统称第三方材料）。您对此类材料的使用应受本文档的所有限制和义务约束。

中信科智联科技有限公司对第三方材料不做任何明示或暗示的保证或陈述，包括但不限于任何暗示或法定的适销性或特性用途的适用性、平静受益权、系统集成、信息准确性以及许可技术或被许可人使用许可技术相关的不侵犯任何第三方知识产权的保证。本文档中的任何内容都不构成中信科智联科技有限公司对任何我司产品或任何其他软硬件、设备、工具、信息或产品的开发、增强、修改、分销、营销、销售、提供销售或以其他方式维持生产的陈述或保证。此外，中信科智联科技有限公司免除因交易过程、使用或贸易而产生的任何和所有保证。

**免责声明**

1.  中信科智联科技有限公司不承担任何因未能遵守有关操作或涉及规范而造成损害的责任。

2.  中信科智联科技有限公司不承担因本文档中的任何因不准确、遗漏或使用本文档中的信息而产生的任何责任。

3.  中信科智联科技有限公司尽力确保开发中功能的完整性、准确性、及时性，但不排除上述功能错误或遗漏的可能。除非另有协议规定，否则中信科智联科技有限公司对开发中功能的使用不做任何暗示或法定的保证。在适用法律允许的最大范围内，中信科智联科技有限公司不对任何因使用开发中功能而遭受的损害承担责任，无论此类损害是否可以预见。

4.  中信科智联科技有限公司对第三方网站及第三方资源的信息、内容、广告、商业报价、产品、服务和材料的可访问性、安全性、准确性、可用性、合法性和完整性不承担任何法律责任。

**\**

修订记录

<table>
<colgroup>
<col style="width: 14%" />
<col style="width: 10%" />
<col style="width: 75%" />
</colgroup>
<tbody>
<tr>
<td>日期</td>
<td>版本</td>
<td>修订内容</td>
</tr>
<tr>
<td>2025-07-22</td>
<td>V1.0.0</td>
<td><ol type="1">
<li><p>创建文件</p></li>
</ol></td>
</tr>
</tbody>
</table>

# 

# **目 录**

[1 简介 [1](#简介)](#简介)

[2 接口概述 [1](#接口概述)](#接口概述)

> [2.1 MEC与云平台 [1](#mec与云平台)](#mec与云平台)
>
> [2.1.1 静态数据 [1](#静态数据)](#静态数据)
>
> [2.1.2 动态数据 [1](#动态数据)](#动态数据)
>
> [2.2 MEC与雷达 [1](#mec与雷达)](#mec与雷达)
>
> [2.2.1 雷达静态数据 [1](#雷达静态数据)](#雷达静态数据)
>
> [2.2.2 动态数据 [1](#动态数据-1)](#动态数据-1)

[3 静态数据接口 [1](#静态数据接口)](#静态数据接口)

> [3.1 概述 [1](#概述)](#概述)
>
> [3.2 路口配置说明 [2](#路口配置说明)](#路口配置说明)
>
> [3.3 路口配置查询接口 [2](#路口配置查询接口)](#路口配置查询接口)
>
> [3.3.1 MEC与云控平台 [2](#mec与云控平台)](#mec与云控平台)
>
> [3.3.1.1 请求详情 [2](#请求详情)](#请求详情)
>
> [3.3.1.2 响应详情 [3](#响应详情)](#响应详情)
>
> [3.3.1.3 路口配置数据结构 [3](#路口配置数据结构)](#路口配置数据结构)
>
> [3.3.1.4 分支配置数据结构 [3](#分支配置数据结构)](#分支配置数据结构)
>
> [3.3.1.5 车道配置数据结构 [3](#车道配置数据结构)](#车道配置数据结构)
>
> [3.3.1.6 路口设备配置数据结构
> [4](#路口设备配置数据结构)](#路口设备配置数据结构)
>
> [3.3.1.7 分支设备配置数据结构
> [4](#分支设备配置数据结构)](#分支设备配置数据结构)
>
> [3.3.1.8 车道设备配置数据结构
> [4](#车道设备配置数据结构)](#车道设备配置数据结构)
>
> [3.3.2 MEC与雷达 [4](#mec与雷达-1)](#mec与雷达-1)
>
> [3.3.2.1 Topic定义 [4](#topic定义)](#topic定义)
>
> [3.3.2.2 数据内容 [5](#数据内容)](#数据内容)
>
> [3.4 路口配置更新接口 [5](#路口配置更新接口)](#路口配置更新接口)
>
> [3.4.1 MEC与云空平台 [5](#mec与云空平台)](#mec与云空平台)
>
> [3.4.1.1 请求详情 [5](#请求详情-1)](#请求详情-1)
>
> [3.4.1.2 响应详情 [5](#响应详情-1)](#响应详情-1)
>
> [3.4.2 MEC与雷达 [6](#mec与雷达-2)](#mec与雷达-2)
>
> [3.4.2.1 通信协议 [6](#通信协议)](#通信协议)
>
> [3.4.2.2 数据处理流程 [6](#数据处理流程)](#数据处理流程)
>
> [3.4.2.3 Topic定义 [6](#topic定义-1)](#topic定义-1)
>
> [3.4.2.4 数据内容 [6](#数据内容-1)](#数据内容-1)

[4 动态数据接口 [7](#动态数据接口)](#动态数据接口)

> [4.1 概述 [7](#概述-1)](#概述-1)
>
> [4.1.1 通信协议 [7](#通信协议-1)](#通信协议-1)
>
> [4.1.2 数据处理流程 [7](#数据处理流程-1)](#数据处理流程-1)
>
> [4.1.3 Topic定义 [7](#topic定义-2)](#topic定义-2)
>
> [4.2 公共数据头 [8](#公共数据头)](#公共数据头)
>
> [4.3 实时轨迹数据推送接口
> [8](#实时轨迹数据推送接口)](#实时轨迹数据推送接口)
>
> [4.3.1 路口实时数据结构 [9](#路口实时数据结构)](#路口实时数据结构)
>
> [4.3.2 车道实时数据结构 [9](#车道实时数据结构)](#车道实时数据结构)
>
> [4.3.3 轨迹数据结构 [9](#轨迹数据结构)](#轨迹数据结构)
>
> [4.4 实时过车数据推送接口
> [9](#实时过车数据推送接口)](#实时过车数据推送接口)
>
> [4.4.1 路口实时数据结构
> [10](#路口实时数据结构-1)](#路口实时数据结构-1)
>
> [4.4.2 车道实时数据结构
> [10](#车道实时数据结构-1)](#车道实时数据结构-1)
>
> [4.4.3 过车数据结构 [10](#过车数据结构)](#过车数据结构)
>
> [4.4.4 实时过车数据示例 [10](#实时过车数据示例)](#实时过车数据示例)
>
> [4.5 实时排队推送接口 [10](#实时排队推送接口)](#实时排队推送接口)
>
> [4.5.1 路口实时数据结构
> [11](#路口实时数据结构-2)](#路口实时数据结构-2)
>
> [4.5.2 车道实时数据结构
> [11](#车道实时数据结构-2)](#车道实时数据结构-2)
>
> [4.5.3 排队数据结构 [11](#排队数据结构)](#排队数据结构)
>
> [4.5.4 实时排队数据示例 [11](#实时排队数据示例)](#实时排队数据示例)
>
> [4.6 实时区域状态数据接口
> [11](#实时区域状态数据接口)](#实时区域状态数据接口)
>
> [4.6.1 路口实时数据结构
> [11](#路口实时数据结构-3)](#路口实时数据结构-3)
>
> [4.6.2 车道实时数据结构
> [12](#车道实时数据结构-3)](#车道实时数据结构-3)
>
> [4.6.3 区域状态数据结构 [12](#区域状态数据结构)](#区域状态数据结构)
>
> [4.7 实时溢出数据推送接口
> [12](#实时溢出数据推送接口)](#实时溢出数据推送接口)
>
> [4.7.1 路口实时数据结构
> [12](#路口实时数据结构-4)](#路口实时数据结构-4)
>
> [4.7.2 车道实时数据结构
> [12](#车道实时数据结构-4)](#车道实时数据结构-4)
>
> [4.7.3 溢出数据结构 [13](#溢出数据结构)](#溢出数据结构)
>
> [4.7.4 实时溢出数据示例 [13](#实时溢出数据示例)](#实时溢出数据示例)
>
> [4.8 实时出口道数据接口
> [13](#实时出口道数据接口)](#实时出口道数据接口)
>
> [4.8.1 路口定时数据结构 [13](#路口定时数据结构)](#路口定时数据结构)
>
> [4.8.2 车道实时数据结构
> [13](#车道实时数据结构-5)](#车道实时数据结构-5)
>
> [4.8.3 出口道数据结构 [13](#出口道数据结构)](#出口道数据结构)
>
> [4.8.4 溢流预警区域说明 [14](#溢流预警区域说明)](#溢流预警区域说明)
>
> [4.8.5 溢流预警区域状态数据结构
> [14](#溢流预警区域状态数据结构)](#溢流预警区域状态数据结构)
>
> [4.9 定时统计数据推送接口
> [15](#定时统计数据推送接口)](#定时统计数据推送接口)
>
> [4.9.1 路口定时数据结构
> [15](#路口定时数据结构-1)](#路口定时数据结构-1)
>
> [4.9.2 统计数据结构 [15](#统计数据结构)](#统计数据结构)
>
> [4.9.3 车道统计数据结构 [15](#车道统计数据结构)](#车道统计数据结构)
>
> [4.9.4 定时流量数据示例（仅供参考）：
> [16](#定时流量数据示例仅供参考)](#定时流量数据示例仅供参考)
>
> [4.10 定时评价数据推送接口
> [16](#定时评价数据推送接口)](#定时评价数据推送接口)
>
> [4.10.1 路口定时数据结构
> [16](#路口定时数据结构-2)](#路口定时数据结构-2)
>
> [4.10.2 评价数据结构 [16](#评价数据结构)](#评价数据结构)
>
> [4.10.3 分支评价数据结构 [16](#分支评价数据结构)](#分支评价数据结构)
>
> [4.10.4 车道评价数据结构 [17](#车道评价数据结构)](#车道评价数据结构)
>
> [4.11 定时行人及非机动车数据推送接口
> [17](#定时行人及非机动车数据推送接口)](#定时行人及非机动车数据推送接口)
>
> [4.11.1 路口定时数据结构
> [17](#路口定时数据结构-3)](#路口定时数据结构-3)
>
> [4.11.2 行人及非机动车数据结构
> [17](#行人及非机动车数据结构)](#行人及非机动车数据结构)
>
> [4.11.3 分支行人及非机动车数据结构
> [17](#分支行人及非机动车数据结构)](#分支行人及非机动车数据结构)
>
> [4.12 实时设备状态数据推送接口
> [18](#实时设备状态数据推送接口)](#实时设备状态数据推送接口)
>
> [4.12.1 路口实时数据结构
> [18](#路口实时数据结构-5)](#路口实时数据结构-5)
>
> [4.12.2 设备实时数据结构 [18](#设备实时数据结构)](#设备实时数据结构)

[5 附录 A：通用数据结构及规范定义
[19](#附录-a通用数据结构及规范定义)](#附录-a通用数据结构及规范定义)

> [5.1 错误码取值表 [19](#错误码取值表)](#错误码取值表)
>
> [5.2 在线状态取值表（同国标）
> [19](#在线状态取值表同国标)](#在线状态取值表同国标)
>
> [5.3 方向取值表（同国标） [19](#方向取值表同国标)](#方向取值表同国标)
>
> [5.4 分支/车道属性取值表
> [20](#分支车道属性取值表)](#分支车道属性取值表)
>
> [5.5 5.5 流向取值表（同国标）
> [20](#流向取值表同国标)](#流向取值表同国标)
>
> [5.6 车辆类型取值表 [20](#车辆类型取值表)](#车辆类型取值表)
>
> [5.7 脉冲类型取值表 [20](#脉冲类型取值表)](#脉冲类型取值表)
>
> [5.8 断面属性取值表 [21](#断面属性取值表)](#断面属性取值表)
>
> [5.9 检测器位置取值表 [21](#检测器位置取值表)](#检测器位置取值表)
>
> [5.10 设备类型取值表 [21](#设备类型取值表)](#设备类型取值表)

# 

# 简介

该文档适用于MEC将雷达静态数据、动态数据上报到云控平台，其中对于静态数据、动态数据的数据协议以《百度智能信控系统路侧感知数据对接协议
v1.0.13 - 亦庄版更新》为基础。

# 接口概述

##  MEC与云平台

### 静态数据

- 通信方式：HTTP方式，云控平台为HTTP服务器，MEC为HTTP客户端。

- 数据封装方式： JSON格式

### 动态数据

- 通信方式：MQTT方式。

- 数据封装方式： JSON格式。

##  MEC与雷达

### 雷达静态数据

- 通信方式：MQTT方式

- 数据封装方式：JSON格式

- 雷达定时主动推送静态数据和查询响应。

### 动态数据

- 通信方式采用MQTT方式。

- 数据封装方式： JSON格式

- 雷达主动推送

# 静态数据接口

## 概述

> 静态数据接口主要对路口、车道、进出口进行统一定义。

## 路口配置说明

<img src="media/image4.png" style="width:4.23472in;height:4.47431in" />

- 路口设备：一般为路侧的计算单元或者数据汇集设备，需绑定到路口

- 分支设备：一般为覆盖一个分支方向的交通信息采集设备，如雷达、雷视等，需绑定到其输出数据对应的分支；

- 车道设备：一般为车道检测器或者虚拟检测区，需绑定到车道。

- 车道序号：见[3.3.1.3](#bookmark69)

## 路口配置查询接口

### MEC与云控平台

通信协议采用HTTP协议，云控平台作为HTTP服务器，MEC作为HTTP客户端。

#### 请求详情

- 请求方法：GET

- 请求路径：/static/device/config

- 请求参数说明：

|          |        |                                                       |
|----------|--------|-------------------------------------------------------|
| 字段     | 类型   | 说明                                                  |
| cross_id | string | 百度信控路口编号                                      |
| vendor   | string | 供应商名称                                            |
| category | string | 见[5.9](#设备类型取值表)[设备类型取值表](#bookmark71) |

- **示例请求**：

GET /static/device/config?cross_id=100001&vendor=merit&category=RADAR

#### 响应详情

- 响应格式：JSON

- 响应内容说明：

|         |             |                                                 |
|---------|-------------|-------------------------------------------------|
| 字段    | 类型        | 说明                                            |
| code    | int32       | 见[5.1](#bookmark72)[错误码取值表](#bookmark73) |
| message | string      | 错误描述                                        |
| data    | object 数组 | 见3.3.1.3 [路口配置数据结构](#bookmark75)       |

#### 路口配置数据结构

|             |              |                                                 |
|-------------|--------------|-------------------------------------------------|
| 字段        | 类型         | 说明                                            |
| place_no    | string       | 厂商路口编号                                    |
| branches    | object 数组  | 见3.3.1.4 [分支配置数据结构](#方向取值表同国标) |
| devices     | object 数组  | 见3.3.1.6 [路口设备配置数据结构](#bookmark79)   |
| update_time | int64/string | 时段表更新时间戳，精确到毫秒                    |

#### 分支配置数据结构

<table style="width:88%;">
<colgroup>
<col style="width: 13%" />
<col style="width: 14%" />
<col style="width: 59%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td>branch_no</td>
<td>int32</td>
<td>分支编号，单个路口中唯一</td>
</tr>
<tr>
<td>direction</td>
<td>int32</td>
<td>分支方向，需和信号机保持一致 见<a href="#bookmark80">5.3</a><a
href="#bookmark81">方向取值表（同国标）</a></td>
</tr>
<tr>
<td>attribute</td>
<td>int32</td>
<td>进出口属性见<a href="#bookmark82">5.4</a><a
href="#bookmark83">分支/车道属性取值表</a></td>
</tr>
<tr>
<td>lanes</td>
<td>object 数组</td>
<td>见3.3.1.5 <a href="#断面属性取值表">车道配置数据结构</a></td>
</tr>
<tr>
<td>devices</td>
<td>object 数组</td>
<td><p>分支设备，按照检测区域归属</p>
<p>见3.3.1.7 <a
href="#分支设备配置数据结构">分支设备配置数据结构</a></p></td>
</tr>
</tbody>
</table>

#### 车道配置数据结构

<table style="width:88%;">
<colgroup>
<col style="width: 14%" />
<col style="width: 13%" />
<col style="width: 59%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td>lane_no</td>
<td>int32</td>
<td>车道编号，单个路口中唯一</td>
</tr>
<tr>
<td>sequence</td>
<td>int32</td>
<td><p>车道序号，由内侧车道到外侧车道从 1 开 始计数</p>
<p><img src="media/image5.jpeg"
style="width:3.0993in;height:1.43028in" /></p></td>
</tr>
<tr>
<td>devices</td>
<td>object 数组</td>
<td>见<a href="#车道设备配置数据结构">3.3.1.6</a><a
href="#车道设备配置数据结构">车道设备配置数据结构</a></td>
</tr>
</tbody>
</table>

#### 路口设备配置数据结构

|           |        |                    |
|-----------|--------|--------------------|
| 字段      | 类型   | 说明               |
| device_id | string | 设备编号，全局唯一 |
| type      | string | 设备型号           |

#### 分支设备配置数据结构

|           |        |                    |
|-----------|--------|--------------------|
| 字段      | 类型   | 说明               |
| device_id | string | 设备编号，全局唯一 |
| type      | string | 设备型号           |

#### 车道设备配置数据结构

|  |  |  |
|----|----|----|
| 字段 | 类型 | 说明 |
| device_id | string | 设备编号，全局唯一 |
| type | string | 设备型号 |
| section | int32 | 见[5.8](#在线状态取值表同国标)[断面属性取值表](#车道实时数据结构) |

### MEC与雷达

通信协议采用MQTT协议，数据格式为JSON格式。MEC主动发布查询消息到MEC内部MQTT服务器，雷达订阅查询消息及响应。

#### Topic定义

|  |  |
|----|----|
| 数据类型 | Topic |
| 路口静态配置查询 | static/device/config/query/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 路口静态配置查询响应 | static/device/config/query/ack/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |

注：

- rscu_id ：MEC分配的ESN；

- vendor ：雷达厂商简写；

- category ：设备属性；

- cross_id ：路口id；

- device_id ：雷达设备的ESN。

#### 数据内容

- 路口静态配置查询

|  |  |  |  |
|----|----|----|----|
| 字段 | 是否可选 | 类型 | 说明 |
| cross_id | 必选 | string | 百度信控路口编号 |
| vendor | 必选 | string | 供应商名称 |
| category | 必选 | string | 见[5.9](#设备类型取值表)[设备类型取值表](#bookmark71) |

- 路口静态配置查询响应

详情见3.3.1.3路口配置数据结构。

## 路口配置更新接口

### MEC与云空平台

#### 请求详情

- 请求方法：POST

- 请求路径：/static/device/config

- 调用频率：每隔5分钟或配置变更时，推送至云控平台

- 请求参数（URL参数）

|          |        |                                                         |
|----------|--------|---------------------------------------------------------|
| 字段     | 类型   | 说明                                                    |
| cross_id | String | 百度信控路口编号                                        |
| vendor   | string | 供应商名称                                              |
| category | string | 见[5.9](#轨迹数据结构)[设备类型取值表](#车辆类型取值表) |
| replace  | int32  | 更新模式：0 合并；1 替换                                |

- 请求体内容

|      |             |                                          |
|------|-------------|------------------------------------------|
| 字段 | 类型        | 说明                                     |
| data | object 数组 | 见[3.3.1.3路口配置数据结构](#bookmark75) |

#### 响应详情

- 响应格式：JSON

- 响应内容说明：

|         |        |                                                           |
|---------|--------|-----------------------------------------------------------|
| 字段    | 类型   | 说明                                                      |
| code    | int32  | 见[5.1](#车道实时数据结构-1)[错误码取值表](#过车数据结构) |
| message | string | 错误描述                                                  |

### MEC与雷达

#### 通信协议

通信协议采用MQTT协议，数据格式为JSON格式

#### 数据处理流程

1.  雷达定时主动推送更新消息到MEC内部MQTT服务器；

2.  MEC订阅更新消息并推送至云控平台；

3.  MEC将推送结果给到雷达。

#### Topic定义

|  |  |
|----|----|
| 数据类型 | Topic |
| 路口静态配置更新 | static/device/config/update/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 路口静态配置更新响应 | static/device/config/update/ack/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |

注：

- rscu_id ：MEC分配的ESN；

- vendor ：雷达厂商简写；

- category ：设备属性；

- cross_id ：路口id；

- device_id ：雷达设备的ESN。

#### 数据内容

- 路口静态配置更新

> 见[3.3.1.3路口配置数据结构](#bookmark75)。

- 路口静态配置更新响应

|         |        |                                                           |
|---------|--------|-----------------------------------------------------------|
| 字段    | 类型   | 说明                                                      |
| code    | int32  | 见[5.1](#车道实时数据结构-1)[错误码取值表](#过车数据结构) |
| message | string | 错误描述                                                  |

# 动态数据接口

## 概述

### 通信协议

通信协议采用MQTT协议，数据格式为JSON格式。

### 数据处理流程

- 雷达定时主动推送动态数据到MEC内部MQTT服务器；

- MEC订阅动态数据并推送至云控平台；

### Topic定义

> 动态数据中各个消息对应Topic如下表所示：

|  |  |
|----|----|
| 数据类型 | Topic |
| 实时轨迹数据 | trafficMetrics/trajectories/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时过车数据 | trafficMetrics/vehiclePass/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时排队数据 | trafficMetrics/queueUp/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时区域状态数据 | trafficMetrics/areaState/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时溢出数据 | trafficMetrics/overflow/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时出口通道数据 | trafficMetrics/outlane/{rscu_id}/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 定时统计数据 | trafficMetrics/statistics/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 定时评价数据 | trafficMetrics/evaluations/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 定时行人及非机动车数据 | trafficMetrics/nonmotor/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时设备状态 | trafficMetrics/deviceStatus/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时脉冲数据 | trafficMetrics/pulse/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |
| 实时统计数据 | trafficMetrics/SingleStatistics/{rscu_id}/{vendor}/{category}/{cross_id}/{device_id} |

注:

- rscu_id ：MEC分配的ESN；

- vendor ：雷达厂商简写；

- category ：设备属性；

- cross_id ：路口id；

- device_id ：雷达设备的ESN。

## 公共数据头

动态数据中各类消息存在公共数据头部，内容如下表所示：

|  |  |  |  |
|:--:|:--:|:--:|:--:|
| 字段 | 类型 | 必选 | 说明 |
| uuid | string | 是 | 数据唯一 ID |
| device_id | string | 否（\*） | 设备 ID |
| device_time | int64/string | 否（\*） | 设备时间戳，精确到毫秒 |
| detector_id | string | 否 | 检测器（区） ID |
| detector_time | int64/string | 否 | 检测器（区）时间 |
| platform_time | int64/string | 否（\*） | 平台时间戳，精确到毫秒 |
| vendor | string | 是（#） | 供应商名称，仅需在顶层数 据结构的 head 里必选 |
| category | string | 是（#） | 见[5.10 设备类型取值表，](#车道设备配置数据结构)仅需在顶层数据结构的head里必选 |

注：

- （\*）：

<!-- -->

- 如前端设备直接推送，device id和device time必填；

- 如平台推送，platform time必填；二者必选其一；

<!-- -->

- （#）：

  - 仅需在顶层数据结构的head里必选；

## 实时轨迹数据推送接口

- 数据粒度：设备

- 采样频率： 0.1s

- 推送频率： 1s（即 1s 内所有的采样数据打包发送）

- 位置精度： \<0.2m

- 实时性： \<1s

- 准确度： 95%

### 路口实时数据结构

|          |             |                                               |
|:--------:|:-----------:|:---------------------------------------------:|
|   字段   |    类型     |                     说明                      |
|   head   |   object    |        见[4.2 公共数据头](#bookmark16)        |
| place_no |   string    |                 厂商路口编号                  |
|  lanes   | object 数组 | 见[4.3.2 车道实时数据结构](#车道实时数据结构) |

### 车道实时数据结构

|              |             |                                                 |
|:------------:|:-----------:|:-----------------------------------------------:|
|     字段     |    类型     |                      说明                       |
|     head     |   object    |         见[4.2 公共数据头](#bookmark16)         |
|   lane_no    |    int32    | 车道编号，处于路口中心区域时车 道号统一使用 255 |
| trajectories | object 数组 |      见[4.3.3 轨迹数据结构](#轨迹数据结构)      |

### 轨迹数据结构

|  |  |  |
|:--:|:--:|:--:|
| 字段 | 类型 | 说明 |
| object_id | string | 目标编号 |
| type | int32 | 见[5.6 车辆类型取值表](#车辆类型取值表) |
| length | float | 长度（米） |
| width | float | 宽度（米） |
| height | float | 高度（米）（可选） |
| longitude | float/string | 经度 |
| latitude | float/string | 纬度 |
| heading | float | 方向角，正北为 0°,顺时针方向 计算，取值范围\[0,360) |
| speed | float | 速度（km/h） |
| distance | float | 到停止线距离（米） |
| plate_no | string | 车牌号（可选） |
| plate_color | string | 车牌颜色（可选） |
| color | string | 车辆颜色（可选） |
| brand | string | 车辆品牌（可选） |
| class1 | string | 车辆一级分类（可选） |
| class2 | string | 车辆二级分类（可选） |

## 实时过车数据推送接口

- 数据粒度：设备

- 采样频率：每过一辆车产生一条数据

- 推送频率： 1s（即 1s 内所有的采样数据打包发送）

- 实时性： \<1s

- 准确度： 95%

### 路口实时数据结构

|          |             |                                                 |
|:--------:|:-----------:|:-----------------------------------------------:|
|   字段   |    类型     |                      说明                       |
|   head   |   object    |         见[4.2 公共数据头](#bookmark16)         |
| place_no |   string    |                  厂商路口编号                   |
|  lanes   | object 数组 | 见[4.4.2 车道实时数据结构](#车道实时数据结构-1) |

### 车道实时数据结构

|         |             |                                       |
|:-------:|:-----------:|:-------------------------------------:|
|  字段   |    类型     |                 说明                  |
|  head   |   object    |    见[4.2 公共数据头](#bookmark16)    |
| lane_no |    int32    |               车道编号                |
|  pass   | object 数组 | 见[4.4.3 过车数据结构](#过车数据结构) |

### 过车数据结构

|                |              |                                         |
|:--------------:|:------------:|:---------------------------------------:|
|      字段      |     类型     |                  说明                   |
|  vehicle_type  |    int32     | 见[5.6 车辆类型取值表](#车辆类型取值表) |
| vehicle_length |    float     |               车长（米）                |
|     speed      |    float     |              车速（km/h）               |
|   enter_time   | int64/string |       进入检测区时间，精确到毫秒        |
|   leave_time   | int64/string |       离开检测区时间，精确到毫秒        |
| occupancy_time |    int32     |          占压时长，精确到毫秒           |
|   head_time    |    int32     |      车头时距，精确到毫秒（可选）       |
| head_distance  |    float     |         车头间距（米）（可选）          |
|    plate_no    |    string    |             车牌号（可选）              |

### 实时过车数据示例

见附录。

## 实时排队推送接口

- 数据粒度：设备

- 推送频率： 1s

- 实时性： \<1s

- 准确度： 95%

### 路口实时数据结构

|          |             |                                                 |
|:--------:|:-----------:|:-----------------------------------------------:|
|   字段   |    类型     |                      说明                       |
|   head   |   object    |         见[4.2 公共数据头](#bookmark16)         |
| place_no |   string    |                  厂商路口编号                   |
|  lanes   | object 数组 | 见[4.5.2 车道实时数据结构](#车道实时数据结构-2) |

### 车道实时数据结构

<table>
<colgroup>
<col style="width: 29%" />
<col style="width: 17%" />
<col style="width: 53%" />
</colgroup>
<tbody>
<tr>
<td style="text-align: center;">字段</td>
<td style="text-align: center;">类型</td>
<td style="text-align: center;">说明</td>
</tr>
<tr>
<td style="text-align: center;">head</td>
<td style="text-align: center;">object</td>
<td style="text-align: left;">见<a href="#bookmark16">4.2
公共数据头</a></td>
</tr>
<tr>
<td style="text-align: center;">lane_no</td>
<td style="text-align: center;">int32</td>
<td style="text-align: left;">车道编号</td>
</tr>
<tr>
<td style="text-align: center;">queue</td>
<td style="text-align: center;">object</td>
<td style="text-align: left;"><p>静态排队信息，见<a
href="#排队数据结构">4.5.3 排队数据结构</a> 静态车辆判断依据：</p>
<p>1）车速低于 10km/h≈2.78m/s</p>
<p>2）低速状态持续 5s 以上</p></td>
</tr>
<tr>
<td style="text-align: center;">queue_dynamic</td>
<td style="text-align: center;">object</td>
<td style="text-align: left;">动态排队信息，见<a
href="#排队数据结构">4.5.3 排队数据结构</a></td>
</tr>
</tbody>
</table>

### 排队数据结构

|              |       |                        |
|:------------:|:-----:|:----------------------:|
|     字段     | 类型  |          说明          |
| queue_length | float |     排队长度（米）     |
|  queue_num   | int32 |       排队车辆数       |
|  queue_head  | float | 队首距停止线距离（米） |
|  queue_tail  | float | 队尾距停止线距离（米） |

### 实时排队数据示例

## 实时区域状态数据接口

- 数据粒度：设备

- 推送频率： 1s

- 实时性： 1s

- 准确度： 95%

- 区域定义：路口渠化段（实线段内）检测区域， 一般为距离停止线 50-60
  米处，各路口视实际情况而定（需支持修改）

### 路口实时数据结构

|          |             |                                                 |
|:--------:|:-----------:|:-----------------------------------------------:|
|   字段   |    类型     |                      说明                       |
|   head   |   object    |         见[4.2 公共数据头](#bookmark16)         |
| place_no |   string    |                  厂商路口编号                   |
|  lanes   | object 数组 | 见[4.6.2 车道实时数据结构](#车道实时数据结构-3) |

### 车道实时数据结构

|             |        |                                               |
|:-----------:|:------:|:---------------------------------------------:|
|    字段     |  类型  |                     说明                      |
|    head     | object |        见[4.2 公共数据头](#bookmark16)        |
|   lane_no   | int32  |                   车道编号                    |
| area_status | object | 见[4.6.3 区域状态数据结构](#区域状态数据结构) |

### 区域状态数据结构

|                   |       |                            |
|:-----------------:|:-----:|:--------------------------:|
|       字段        | 类型  |            说明            |
|        num        | int32 |           车辆数           |
|     occupancy     | float |         空间占有率         |
|       speed       | float |      平均速度（km/h）      |
| distance_variance | float | 车辆分布情况（车间距方差） |
|   head_distance   | float |   头车到停止线距离（米）   |
|    head_speed     | float |      头车速度（km/h）      |
|   tail_distance   | float |   尾车到停止线距离（米）   |
|    tail_speed     | float |      尾车速度（km/h）      |

## 实时溢出数据推送接口

- 数据粒度：设备

- 推送频率： 1s

- 实时性： 1s

- 准确度： 95%

### 路口实时数据结构

|          |             |                                                 |
|:--------:|:-----------:|:-----------------------------------------------:|
|   字段   |    类型     |                      说明                       |
|   head   |   object    |         见[4.2 公共数据头](#bookmark16)         |
| place_no |   string    |                  厂商路口编号                   |
|  lanes   | object 数组 | 见[4.7.2 车道实时数据结构](#车道实时数据结构-4) |

### 车道实时数据结构

<table>
<colgroup>
<col style="width: 28%" />
<col style="width: 15%" />
<col style="width: 55%" />
</colgroup>
<tbody>
<tr>
<td style="text-align: center;">字段</td>
<td style="text-align: center;">类型</td>
<td style="text-align: center;">说明</td>
</tr>
<tr>
<td style="text-align: center;">head</td>
<td style="text-align: center;">object</td>
<td style="text-align: left;">见<a href="#bookmark16">4.2
公共数据头</a></td>
</tr>
<tr>
<td style="text-align: center;">lane_no</td>
<td style="text-align: center;">int32</td>
<td style="text-align: left;">车道编号</td>
</tr>
<tr>
<td style="text-align: center;">overflow_slight</td>
<td style="text-align: center;">object</td>
<td style="text-align: left;">溢流检测区 1 状态（距停车线 30~60 米）
见<a href="#溢出数据结构">4.8.3 溢出数据结构</a></td>
</tr>
<tr>
<td style="text-align: center;">overflow_heavy</td>
<td style="text-align: center;">object</td>
<td style="text-align: left;">溢流检测区 2 状态（距停车线 0~20 米） 见<a
href="#溢出数据结构">4.8.3 溢出数据结构</a></td>
</tr>
<tr>
<td style="text-align: center;">overflow_grid_lock</td>
<td style="text-align: center;">object</td>
<td style="text-align: left;"><p>溢流检测区 3 状态（路口内）</p>
<p>见<a href="#溢出数据结构">4.7.3 溢出数据结构</a></p></td>
</tr>
</tbody>
</table>

### 溢出数据结构

|        |       |                              |
|:------:|:-----:|:----------------------------:|
|  字段  | 类型  |             说明             |
| status | int32 | 溢出状态： 0 未溢出； 1 溢出 |
| speed  | int32 |       平均速度（km/h）       |

### 实时溢出数据示例

## 实时出口道数据接口

- 数据粒度：设备

- 推送频率： 1s

- 实时性： 1s

- 准确度： 95%

### 路口定时数据结构

|          |             |                                                 |
|:--------:|:-----------:|:-----------------------------------------------:|
|   字段   |    类型     |                      说明                       |
|   head   |   object    |         见[4.2 公共数据头](#bookmark16)         |
| place_no |   string    |                  厂商路口编号                   |
|  lanes   | object 数组 | 见[4.8.2 车道实时数据结构](#车道实时数据结构-5) |

### 车道实时数据结构

|         |        |                                           |
|:-------:|:------:|:-----------------------------------------:|
|  字段   |  类型  |                   说明                    |
|  head   | object |      见[4.2 公共数据头](#bookmark16)      |
| lane_no | int32  |                 车道编号                  |
| outlane | object | 见[4.8.3 出口道数据结构](#出口道数据结构) |

### 出口道数据结构

|                        |        |                      |
|:----------------------:|:------:|:--------------------:|
|          字段          |  类型  |         说明         |
|        stop_num        | int32  |      静止车辆数      |
|     tail_distance      | float  | 尾车到出口距离（米） |
|         speed          | float  |   平均车速（km/h）   |
| overflow_forecast_area | object |   溢流预警区域状态   |

### 溢流预警区域说明

> 当出口道排队长度持续到达溢流预警位置，说明此时存在溢流风险。
>
> 在溢流预警位置设置溢流检测区域（或线段），如果此区域压占时间越长，说
> 明排队到达此处的时间越频繁， 也说明此出口道存在溢流风险。

<img src="media/image6.jpeg" style="width:5.76389in;height:2.51805in" />

> 实际标注案例（不影响[4.8
> 实时溢出数据推送接口的溢流检测框）](#bookmark36)

<img src="media/image7.png" style="width:5.76389in;height:3.03194in" />

### 溢流预警区域状态数据结构

|           |       |                  |
|:---------:|:-----:|:----------------:|
|   字段    | 类型  |       说明       |
|    num    | int32 |    压占车辆数    |
| occupancy | float |    空间占有率    |
|   speed   | float | 平均车速（km/h） |

## 定时统计数据推送接口

- 数据粒度：路口

- 推送频率：可配置（最小支持 30s）

- 实时性： 1s

- 准确度： 95%

### 路口定时数据结构

|            |             |                                       |
|:----------:|:-----------:|:-------------------------------------:|
|    字段    |    类型     |                 说明                  |
|    head    |   object    |    见[4.2 公共数据头](#bookmark16)    |
|  place_no  |   string    |             厂商路口编号              |
| statistics | object 数组 | 见[4.9.2 统计数据结构](#统计数据结构) |

### 统计数据结构

|  |  |  |
|:--:|:--:|:--:|
| 字段 | 类型 | 说明 |
| cycle_time | int32 | 统计周期时长（秒） |
| cycle_start_time | int32 | 统计开始事件（秒） |
| ~~cycle_end_time~~ | ~~int32~~ | ~~统计结束时间（秒）~~ |
| lanes | object 数组 | 见[4.9.3 车道统计数据结构](#车道统计数据结构) |

### 车道统计数据结构

|                      |        |                                 |
|:--------------------:|:------:|:-------------------------------:|
|         字段         |  类型  |              说明               |
|         head         | object | 见[4.2 公共数据头](#bookmark16) |
|       lane_no        | int32  |            车道编号             |
|        volume        | int32  |             总流量              |
|       volume_1       | int32  |           小型车流量            |
|       volume_2       | int32  |           大型车流量            |
|        speed         | float  |        平均车速（km/h）         |
|       speed85        | float  |       85 分位速度（km/h）       |
|      max_speed       | float  |        最大车速（km/h）         |
|      min_speed       | float  |        最小车速（km/h）         |
|    vehicle_length    | float  |         平均车长（米）          |
|      head_time       | float  |          平均车头时距           |
|    head_distance     | float  |          平均车头间距           |
| occupancy_time_rate  | float  |           时间占有率            |
| occupancy_space_rate | float  |           空间占有率            |

### 定时流量数据示例（仅供参考）：

## 定时评价数据推送接口

- 数据以路口为单位

- 推送频率：可配置（最小支持 30s）

- 实时性： 1s

- 准确度： 95%

### 路口定时数据结构

|             |             |                                         |
|:-----------:|:-----------:|:---------------------------------------:|
|    字段     |    类型     |                  说明                   |
|    head     |   object    |     见[4.2 公共数据头](#bookmark16)     |
|  place_no   |   string    |              厂商路口编号               |
| evaluations | object 数组 | 见 [4.10.2 评价数据结构](#评价数据结构) |

### 评价数据结构

|  |  |  |
|:--:|:--:|:--:|
| 字段 | 类型 | 说明 |
| cycle_time | int32 | 统计周期时长（秒） |
| cycle_start_time | int32 | 统计开始事件（秒） |
| ~~cycle_end_time~~ | ~~int32~~ | ~~统计结束时间（秒）~~ |
| branches | object 数组 | 见[4.10.3 分支评价数据结构](#分支评价数据结构) |
| lanes | object 数组 | 见[4.10.4 车道评价数据结构](#车道评价数据结构) |

### 分支评价数据结构

<table>
<colgroup>
<col style="width: 31%" />
<col style="width: 28%" />
<col style="width: 40%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td><blockquote>
<p>head</p>
</blockquote></td>
<td><blockquote>
<p>object</p>
</blockquote></td>
<td><blockquote>
<p>见<a href="#bookmark16">4.2 公共数据头</a></p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>branch_no</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>分支编号</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>volume</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>总体流量</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>speed</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>总体平均速度（km/h）</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>left_volume</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>左转流量</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>left_speed</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>左转平均速度（km/h）</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>straight_volume</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>直行流量</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>straight_speed</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>直行平均速度（km/h）</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>right_volume</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>右转流量</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>right_speed</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>右转平均速度（km/h）</p>
</blockquote></td>
</tr>
</tbody>
</table>

### 车道评价数据结构

<table>
<colgroup>
<col style="width: 33%" />
<col style="width: 26%" />
<col style="width: 40%" />
</colgroup>
<tbody>
<tr>
<td><blockquote>
<p>字段</p>
</blockquote></td>
<td><blockquote>
<p>类型</p>
</blockquote></td>
<td><blockquote>
<p>说明</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>head</p>
</blockquote></td>
<td><blockquote>
<p>object</p>
</blockquote></td>
<td><blockquote>
<p>见<a href="#bookmark16">4.2 公共数据头</a></p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>lane_no</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>车道编号</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>sample_num</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>样本量</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>volume</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>过停止线流量</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>max_queue_length</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>最大排队长度</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>stop</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>平均停车次数</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>delay</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>平均延误时间</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>no_stop_rate</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>平均一次通过率</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>travel_distance</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>平均行程距离</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>travel_time</p>
</blockquote></td>
<td><blockquote>
<p>float</p>
</blockquote></td>
<td><blockquote>
<p>平均行程时间</p>
</blockquote></td>
</tr>
</tbody>
</table>

## 定时行人及非机动车数据推送接口

- 数据粒度：路口

- 推送频率：可配置（最小支持 30s）

- 实时性： 1s

- 准确度： 95%

### 路口定时数据结构

<table>
<colgroup>
<col style="width: 25%" />
<col style="width: 22%" />
<col style="width: 52%" />
</colgroup>
<tbody>
<tr>
<td><blockquote>
<p>字段</p>
</blockquote></td>
<td><blockquote>
<p>类型</p>
</blockquote></td>
<td><blockquote>
<p>说明</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>head</p>
</blockquote></td>
<td><blockquote>
<p>object</p>
</blockquote></td>
<td><blockquote>
<p>见<a href="#bookmark16">4.2 公共数据头</a></p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>place_no</p>
</blockquote></td>
<td><blockquote>
<p>string</p>
</blockquote></td>
<td><blockquote>
<p>厂商路口编号</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>statistics</p>
</blockquote></td>
<td><blockquote>
<p>object 数组</p>
</blockquote></td>
<td><blockquote>
<p>见<a href="#行人及非机动车数据结构">4.11.2
行人及非机动车数据结构</a></p>
</blockquote></td>
</tr>
</tbody>
</table>

### 行人及非机动车数据结构

<table>
<colgroup>
<col style="width: 27%" />
<col style="width: 18%" />
<col style="width: 53%" />
</colgroup>
<tbody>
<tr>
<td><blockquote>
<p>字段</p>
</blockquote></td>
<td><blockquote>
<p>类型</p>
</blockquote></td>
<td><blockquote>
<p>说明</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>cycle_time</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>统计周期时长（秒）</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>cycle_start_time</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>统计开始事件（秒）</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>branches</p>
</blockquote></td>
<td><blockquote>
<p>object 数组</p>
</blockquote></td>
<td><blockquote>
<p>见<a href="#分支行人及非机动车数据结构">4.11.3
分支行人及非机动车数据结构</a></p>
</blockquote></td>
</tr>
</tbody>
</table>

### 分支行人及非机动车数据结构

<table>
<colgroup>
<col style="width: 27%" />
<col style="width: 19%" />
<col style="width: 53%" />
</colgroup>
<tbody>
<tr>
<td><blockquote>
<p>字段</p>
</blockquote></td>
<td><blockquote>
<p>类型</p>
</blockquote></td>
<td><blockquote>
<p>说明</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>head</p>
</blockquote></td>
<td><blockquote>
<p>object</p>
</blockquote></td>
<td><blockquote>
<p>见<a href="#bookmark16">4.2 公共数据头</a></p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>branch_no</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>分支编号</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>pedestrain_volume</p>
</blockquote></td>
<td><blockquote>
<p>int32</p>
</blockquote></td>
<td><blockquote>
<p>行人流量</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>non_motor_volumn</p>
</blockquote></td>
<td><blockquote>
<p>Int32</p>
</blockquote></td>
<td><blockquote>
<p>非机动车流量</p>
</blockquote></td>
</tr>
</tbody>
</table>

## 实时设备状态数据推送接口 

- 数据粒度：设备

- 采样频率：实时

- 推送频率：5min+变化推送

- 实时性：实时

### 路口实时数据结构 

<table style="width:100%;">
<colgroup>
<col style="width: 28%" />
<col style="width: 19%" />
<col style="width: 52%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td><blockquote>
<p>head</p>
</blockquote></td>
<td><blockquote>
<p>object</p>
</blockquote></td>
<td><blockquote>
<p>见 4.2 公共数据头</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>place_no</p>
</blockquote></td>
<td><blockquote>
<p>string</p>
</blockquote></td>
<td><blockquote>
<p>厂商路口编号</p>
</blockquote></td>
</tr>
<tr>
<td><blockquote>
<p>devices</p>
</blockquote></td>
<td><blockquote>
<p>object 数组</p>
</blockquote></td>
<td><blockquote>
<p>见 4.13.2 设备实时数据结构</p>
</blockquote></td>
</tr>
</tbody>
</table>

### 设备实时数据结构 

|        |        |                                 |
|--------|--------|---------------------------------|
| 字段   | 类型   | 说明                            |
| head   | object | 见 4.2 公共数据头               |
| status | string | 见 5.2 在线状态取值表（同国标） |

## 脉冲数据 

- 数据粒度：设备

- 采样频率：不固定，在检测到有⻋辆进⼊或离开虚拟线圈时应主动向信号机发送脉冲数据

- 推送频率：变化推送

- 实时性：实时

### 脉冲数据结构

<table>
<colgroup>
<col style="width: 33%" />
<col style="width: 22%" />
<col style="width: 43%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td>head</td>
<td>object</td>
<td>见<a href="#bookmark16">4.2 公共数据头</a></td>
</tr>
<tr>
<td>version</td>
<td>Int32</td>
<td><p>协议版本，用于后续协议升级兼容。</p>
<p>当前取值：固定为1（初始版本）</p></td>
</tr>
<tr>
<td>lanes</td>
<td>object数组</td>
<td>见4.13.2单路检测通道过车信息</td>
</tr>
</tbody>
</table>

### 单路检测通道过车信息 

<table>
<colgroup>
<col style="width: 21%" />
<col style="width: 12%" />
<col style="width: 66%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td>lane_no</td>
<td>int32</td>
<td><p>检测通道序号（检测器编号、线圈编号）</p>
<p>取值：0~255</p></td>
</tr>
<tr>
<td>direction</td>
<td>int32</td>
<td><p>⻋辆⽅向，取值</p>
<p>0：⻋辆离开检测区域</p>
<p>1：⻋辆进⼊检测区域</p></td>
</tr>
</tbody>
</table>

## 统计数据

- 数据粒度：设备

- 采样频率：一个统计周期（5分钟）结束后检测器主动上传最新统计数据。

- 推送频率：可配置

- 实时性：实时

### 统计数据结构

<table>
<colgroup>
<col style="width: 33%" />
<col style="width: 22%" />
<col style="width: 43%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td>head</td>
<td>object</td>
<td>见<a href="#bookmark16">4.2 公共数据头</a></td>
</tr>
<tr>
<td>version</td>
<td>Int32</td>
<td><p>协议版本，用于后续协议升级兼容。</p>
<p>当前取值：固定为1（初始版本）</p></td>
</tr>
<tr>
<td>statistics</td>
<td>object数组</td>
<td>见4.14.2单路检测通道统计数据信息</td>
</tr>
</tbody>
</table>

### 单路检测通道统计数据信息 

<table>
<colgroup>
<col style="width: 21%" />
<col style="width: 12%" />
<col style="width: 66%" />
</colgroup>
<tbody>
<tr>
<td>字段</td>
<td>类型</td>
<td>说明</td>
</tr>
<tr>
<td>lane_no</td>
<td>int32</td>
<td><p>检测通道序号（检测器编号、线圈编号）</p>
<p>取值：0~255</p></td>
</tr>
<tr>
<td>volume</td>
<td>Int32</td>
<td><p>流量（⻋辆数量）</p>
<p>取值：0~255</p></td>
</tr>
<tr>
<td>occupancy</td>
<td>int32</td>
<td><p>占有率</p>
<p>取值：0~200</p>
<p>单位：0.5%</p></td>
</tr>
<tr>
<td>speed</td>
<td>Int32</td>
<td><p>车辆平均行驶速度</p>
<p>取值：1~255。255 表示溢出</p>
<p>单位：km/h</p></td>
</tr>
<tr>
<td>length</td>
<td>int32</td>
<td><p>⻋辆平均⻋⻓</p>
<p>取值：1~255，255 表示溢出</p>
<p>单位：0.1m</p></td>
</tr>
<tr>
<td>distance</td>
<td>int32</td>
<td><p>⻋辆平均⻋头时距</p>
<p>取值：1~255，255 表示溢出</p>
<p>单位：s</p></td>
</tr>
</tbody>
</table>

# 附录 A：通用数据结构及规范定义

## 错误码取值表

|      |     |              |
|------|-----|--------------|
| 序号 | 值  | 说明         |
| 1    | 0   | 成功         |
| 2    | 11  | 请求参数错误 |
| 3    | 12  | 请求权限错误 |
| 4    | 21  | 请求处理失败 |

## 在线状态取值表（同国标）

|      |         |            |
|------|---------|------------|
| 序号 | 值      | 说明       |
| 1    | Online  | 正常在线   |
| 2    | Offline | 脱机、断线 |
| 3    | Error   | 异常故障   |

## 方向取值表（同国标）

|      |     |      |
|------|-----|------|
| 序号 | 值  | 说明 |
| 1    | 0   | 北   |
| 2    | 1   | 东北 |
| 3    | 2   | 东   |
| 4    | 3   | 东南 |
| 5    | 4   | 南   |
| 6    | 5   | 西南 |
| 7    | 6   | 西   |
| 8    | 7   | 西北 |

## 分支/车道属性取值表

|      |     |      |
|------|-----|------|
| 序号 | 值  | 说明 |
| 1    | 1   | 进口 |
| 2    | 2   | 出口 |
| 3    | 9   | 其他 |

## 5.5 流向取值表（同国标）

|      |     |            |
|------|-----|------------|
| 序号 | 值  | 说明       |
| 1    | 11  | 直行       |
| 2    | 12  | 左转       |
| 3    | 13  | 右转       |
| 4    | 21  | 直左混行   |
| 5    | 22  | 直右混行   |
| 6    | 23  | 左右混行   |
| 7    | 24  | 直左右混行 |
| 8    | 31  | 调头       |
| 9    | 99  | 其他       |

## 车辆类型取值表

|      |     |                   |
|------|-----|-------------------|
| 序号 | 值  | 说明              |
| 1    | 0   | 未知类型          |
| 2    | 1   | 小型车（9m 以下） |
| 3    | 2   | 大型车（9m 以上） |

## 脉冲类型取值表

|      |     |            |
|------|-----|------------|
| 序号 | 值  | 说明       |
| 1    | 0   | 离开检测区 |
| 2    | 1   | 进入检测区 |

## 断面属性取值表

|      |     |                |
|------|-----|----------------|
| 序号 | 值  | 说明           |
| 1    | 0   | 停止线         |
| 2    | 1   | 测速（或排队） |
| 3    | 2   | 溢出           |

## 检测器位置取值表

|      |     |      |
|------|-----|------|
| 序号 | 值  | 说明 |
| 1    | 0   | 前置 |
| 2    | 1   | 后置 |

## 设备类型取值表

|      |                   |        |
|------|-------------------|--------|
| 序号 | 值                | 说明   |
| 1    | SIGNAL_CONTROLLER | 信号机 |
| 2    | MAGNETIC          | 地磁   |
| 3    | V2X               | V2X    |
| 4    | RADAR_VIDEO       | 雷视   |
| 5    | RADAR             | 雷达   |
