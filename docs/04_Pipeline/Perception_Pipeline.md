# Perception Pipeline

## 1 模块介绍
| 功能             | 文档链接                                                     |
| ---------------- | ------------------------------------------------------------ |
| 单相机感知       | [Perception Camera Service](../02_Service/Perception_Camera_Service.md)|
| 多传感器融合       | [Perception Fusion Service](../02_Service/Perception_Fusion_Service.md)|
| Lidar感知检测      | [Perception Lidar Service](../02_Service/Perception_Lidar_Service.md)|

## 2 快速开始
获取项目产出
```bash
bash docker/scripts/docker_start.sh # 拉取docker镜像并启动容器
bash docker/scripts/docker_into.sh # 进入docker容器
bash build_airos.sh release # 编译且发布
```

运行感知Pipeline，执行以下命令：
```bash
source /home/airos/os/setup.bash # 初始化运行环境
airos_launch perception # 拉起感知pipeline
```

运行感知可视化，执行以下命令：
```bash
source /home/airos/os/setup.bash # 初始化运行环境
airos_launch camera_viz # 拉起相机感知可视化
airos_launch fusion_viz # 拉起多传感器融合可视化
```