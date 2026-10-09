## proto 文件说明

### header.proto
信息头，主要包括timestamp、frame_id、module_name 

### perceptipn_common.proto
感知通用数据，目前只含有2D检测框 BBox2D

### perception_obstacles.proto
其最主要的结构体是 PerceptionObstacles, 是 融合和 usecase 两个模块中 cybertron 输入通道的数据类型，是检测跟踪回3D、融合两个模块中cybertron 输出通道的数据类型。

PerceptionObstacles 是障碍物级别的总集，一个 PerceptionObstacles 中包含了多个障碍物消息，每个障碍物消息用 PerceptionObstacle 表示。

### usecase.proto
其最主要的结构体是 EventOutputResult, 是 usecase 模块中 cybertron 输出通道的数据类型。

EventOutputResult 是事件级别的总集，一个 EventOutputResult 包含了多个交通事件消息，每个交通事件用 EventInformation 表示。

PerceptionObstacles 和 EventOutputResult 在各个模块之间的流转示意图：https://ku.baidu-int.com/knowledge/HFVrC7hq1Q/9-z5aSPbC9/yB6xAGeYgQ/ioJB-p1OO3DHu6

