# AirOS Framework

智路OS框架是智路OS的核心，它提供了完整的系统架构，包括：
- 硬件抽象层（HAL）
- 应用程序框架（AF）
- 服务框架（SF）
- 基础库（BD）

## 1 Framework
```bash
├── air_service     # 感知服务
│   ├── framework
│   └── modules
├── app             # 应用  
│   ├── framework
│   └── modules
├── device_service  # 设备服务
│   ├── framework
│   └── modules
└── base
```
**框架(Framework)** 属于智路OS框架层，负责加载及运行组件，规定了各类组件的接口及输入输出，包含感知服务框架、设备服务框架、应用程序框架。

### 1.1 框架运行 
框架默认安装运行路径为`/home/airos/os/`，下面以信号灯设备服务为例，介绍框架运行流程：

- 安装包加载：框架启动时首先进行所有安装包的加载，默认路径`device/lib/traffic_light`，通过读取安装包配置获取需要加载的库文件，进行动态加载。
- 框架配置读取：在运行目录寻找框架的配置文件，默认在`conf/traffic_light`目录，通过配置的组件名称`gat_traffic_light`在所有已安装的包中寻找该组件是否已注册，如果已注册则实例化。
- 框架初始化：实例化后调用组件的初始化函数，完成组件的初始化。
- 框架运行：调用组件的Start函数，完成组件的运行。

其他框架组件的加载流程类似，框架配置均产出在`conf/`目录下，应用包安装在app目录下，设备服务包安装在`device/`目录下，感知服务包安装在`modules/`目录下。

### 1.2 组件接口 
框架以C++基类的方式定义了组件的接口及输出，并且提供了注册接口，通过注册接口将组件注册到框架中。以单相机感知服务为例，介绍组件接口：
- 组件基类：单相机感知组件的基类为`BaseCameraPerception`，定义了`Init`和`Perception`两个接口，分别用于初始化及感知处理。
- 注册接口：`REGISTER_CAMERA_PERCEPTION`，通过该接口将组件注册到框架中。

开发者必须实现`Init`和`Perception`接口，并且注册组件，可以通过智路OS包管理工具便捷开发。

## 2 Pipeline

框架加载组件后，各个服务和应用之间通过中间件通信，可以串联完成完整的业务，即Pipeline。

目前智路OS框架提供了两个示例的完整Pipeline，分别是[Traffic Light Pipeline](./Traffic_Light_Pipeline.md)和[Perception Pipeline](./Perception_Pipeline.md)。

