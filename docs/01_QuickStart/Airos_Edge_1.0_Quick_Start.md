# Airos Edge 1.0 Quick Start
## 文件结构
```
.
|-- AUTHORS.md
|-- BUILD
|-- LICENSE
|-- README.md
|-- WORKSPACE
|-- base\
|-- build_airos.sh
|-- docker\
|-- middleware\
|-- modules\
|-- scripts\
|-- setup.bash
|-- third_party\
`-- workspace.bzl
```
文件说明
- base
  
Base层：定义为与外部接入的硬件设备交互以及其它基本的功能层。包含：插件注册、硬件接入、文件操作、图像处理、矩阵等子模块。（暂时只支持英伟达平台，硬件抽象模块暂无）
- middleware

middleware层：定义为系统级应用，对整个AirOS框架内外提供系统级服务、接口。包含：设备服务、通信中间件、协议栈、SDK、高精地图、Paddle等子模块。
- modules

Module层：定义为功能级应用模块。包含：红绿灯pipeline、感知pipeline等具体独立的应用。
- docker

docker镜像、容器的操作脚本
- scripts

基础运行的脚本
- third_party

此项目的三方依赖库

## 快速开始
我们提供了编译、产出、运行的docker环境，省去了开发者在开发机进行依赖库安装、环境变量配置等诸多环境配置工作。

缺省docker镜像版本为ubuntu18_x86-64，如需要其他版本镜像欢迎邮件联系我们。

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

### 编译
```bash
# 在docker内工程目录下执行
bash build_airos.sh build
```
### 产出
```bash
# 在docker内工程目录下执行
bash build_airos.sh release
```
产出位于`/airos/output`，产出目录的结构和代码结构相对应


### 运行
#### 运行红绿灯Pipeline
```bash
# 1. 产出（如果已经执行，则可跳过）
bash build_airos.sh release
# 2. 配置环境（如果已经执行，则可跳过）
source setup.bash
# 3. 运行
cyber_launch start air_service/launch/traffic_light_pipeline.launch
```

### 清理编译
```bash
# 在docker内工程目录下执行
bash build_airos.sh clean
```

### 调试
#### 通信层调试工具
往往一个pipeline会包含多个子模块，当一个pipeline启动后，各个子模块之间通过通信层完成相互之间的通信，发送者向指定通道发送消息，接受者通过订阅相应的通道完成接收。

我们提供了通道操作的工具集，可便捷的访问或者查看通道属性和消息内容，也可以将通道内的消息以文件的形式持久化保存。

工具集为多个可执行的命令行工具，包含：
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
