# Airos Edge 2.0 Quick Start 
## 1 智路OS2.0
## 1.1 简介
智路OS路侧操作系统airos-edge自下而上分别由内核层，硬件抽象层、框架层、服务层和应用层构成；提供了一系列抽象和框架，支持设备接入、服务、应用等组件开发，兼容X86和ARM操作系统，技术架构如下所示：
  ![智路os技术架构](./images/zhilu-2.0.jpg)
**内核层**

智路OS内核层融合了进程和线程管理、内存管理、文件系统、网络栈、设备驱动、实时补丁、高可用支持以及POSIX（Portable Operating System Interface for UNIX）兼容性等关键要素，以满足面向车路协同场景的高实时性、发并性、可靠性的需求。

**抽象层**

为实现硬件与操作系统之间的交互，智路OS制定了硬件接入和输出的标准，硬件设备接入应符合以下要求：
- 设备接入：智路OS明确了硬件设备的接入方法，涵盖了设备接口定义
- 设备输出：智路OS规定了设备输出的数据结构和内容标准，包括设备状态、处理结果等
框架层
- **通信和调度中间件**：规定了通信中间件的标准接口。通信和调度中间件默认实现包含通信、服务发现、计算调度等核心模块，其中通信使用pub/sub的匿名通信模式，基于动态的服务发现去除了中心化的节点，支持进程间和跨机器通信，计算调度模块屏蔽了操作系统的底层细节。
- **服务框架**：服务框架定义了组件的接口及输出，提供了服务、设备组件、应用的加载，包含了智路OS核心服务架构

**服务层**

服务层定义了智路 OS 系统应该具备以下几项服务能力，包括传感器服务、检测服务、融合服务、信号灯服务、通信服务等。当服务组件被框架层加载并运行时，就组成了智路OS的基本服务。

**应用层**

应用程序可以根据其特定业务需求，充分利用智路 OS 提供的多样化服务，其中包括传感器服务、感知服务、通信服务以及信号灯服务等提供的标准数据。通过接口订阅，获取实时的服务数据，经过逻辑处理后发送至输出设备或与云服务器进行交互，从而完成应用的构建。

## 1.2 框架与组件
核心目录架构
```bash
├── air_service
│   ├── framework
│   └── modules
├── app
│   ├── framework
│   └── modules
├── device_service
│   ├── framework
│   └── modules
└── base
```
**框架(Framework)** 属于智路OS框架层，负责加载及运行组件，规定了各类组件的接口及输入输出

**组件(Module)** 是智路OS规定可由开发者自定义的模块，以包的形式管理、开发、部署。开源版本提供了所有标准的组件示例，可直接运行、参考开发。
| 组件类型   | 描述      |举例   |
| ---------------- | ---------------- | ---------------- |
| 设备接入  | 为系统提供数据接入能力   | 信号机、RSU、Camera、Lidar |
| 服务   | 基于接入数据为应用提供服务的组件  |  视觉感知服务、雷达感知服务、融合服务  |
| 应用  |  基于系统数据，实现事件检测、信控、V2X应用场景、规划控制等应用   |  车路协同、规划控制  |

## 2 智路OS运行
智路OS开源代码包括智路OS框架和所有标准的组件示例，可直接运行。
### 获取docker镜像并启动docker容器
```bash
# 请在源代码根目录执行
bash docker/scripts/docker_start.sh
```
### 进入docker容器
```bash
# 请在源代码根目录执行
bash docker/scripts/docker_into.sh
```
进入docker容器内部后，所在的工作目录其实是通过docker的"卷映射"将源代码根目录映射到当前所在的目录。

### 编译产出
```bash
# 在docker内工程目录下执行
bash build_airos.sh release
```
产出位于`/home/airos/os`目录


### 运行
```bash
# 1. 产出（如果已经执行，则可跳过）
bash build_airos.sh release
# 2. 配置环境（如果已经执行，则可跳过）
source /home/airos/os/setup.bash
# 3. 查看可运行的pipeline
airos_launch -h
# 4. 运行信号灯pipeline
airos_launch traffic_light
```

### 清理编译
```bash
# 在docker内工程目录下执行
bash build_airos.sh clean
```

### 调试
#### 通信层调试工具
往往一个pipeline会包含多个子模块，各个子模块之间通过中间件完成相互之间的通信。

当前智路OS默认使用CyberRT作为通信中间件，CyberRT提供了多种调试工具，可以方便的查看通信数据：
- cyber_channel
- cyber_launch
- cyber_monitor
- cyber_recorder

上述命令行工具运行时加入"-h"参数，可以显示详细的使用说明，可以参考。

工具使用方式示例：
```bash
# 1. 产出（如果已经执行，则可跳过）
bash build_airos.sh release
# 2. 配置环境（如果已经执行，则可跳过）
source setup.bash
# 3. 运行全量通道监控工具
cyber_monitor -a
```

## 3 组件开发
智路OS支持组件的开发，并且提供了集成开发环境、智路SDK、包管理工具简化开发流程
### 集成开发环境
**环境准备** 

基础环境：
- 系统：Linux及发行版，内核5.4.0及以上版本，建议Ubuntu 18.04及以上版本
- CPU：X86_64、AArch64
- 内存：8G
- Docker：建议v20.10.0及以上版本

运行环境：

如需运行智路OS框架及示例算法，需要安装
- NVIDIA驱动：455.32.00及以上版本
- CUDA：11.1及以上版本
- NVIDIA容器工具：nvidia-docker2

**获取SDK及集成开发环境** 
```bash
# 请在源代码根目录执行
bash docker/scripts/docker_start.sh sdk # 拉取智路OS集成开发环境镜像
bash docker/scripts/docker_into.sh sdk # 进入智路OS集成开发环境镜像
```
进入集成开发环境后，可以使用包管理工具airospkg进行组件的开发。具体流程参考 [智路OS路侧操作系统开发手册](https://airos-edge.readthedocs.io/)
