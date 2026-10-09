### lidar设备
模块提供了接入AirOS-edge lidar设备所需要的标准接口，定义了AirOS-edge结构化的lidar输出数据类型，提供lidar设备的注册工厂。

用户需要将lidar采集的数据封装成如下的AirOS-edge结构化的lidar输出数据类型
```protobuf
message PointCloud {
    optional LidarHeader header = 1; 
    optional string frame_id = 2;
    optional bool is_dense = 3;
    repeated PointXYZIT point = 4;
    optional double measurement_time = 5;
    optional uint32 width = 6;
    optional uint32 height = 7;
    repeated FusionInfo fusion_info = 8;
}
```
详细的字段定义请参阅：[lidar_data.proto](../proto/lidar_data.proto)
### 接口含义 
用户需要实现3个接口：`Init`、`Start`、`GetState`

#### `Init`接口
用于读入lidar的初始化配置文件，实现lidar的初始化。

#### `Start` 接口
用于启动lidar设备，获取AirOS-edge结构化的标准lidar输出数据。

#### `GetState` 接口
用于实现lidar的状态查询，返回lidar的运行状态。

### 使用方式
1. 在lidar目录下建立具体lidar设备的目录（建议），添加具体设备的`.h`头文件，引用lidar接口头文件，并继承lidar接口抽象类（以`dummy_lidar`为例）
    
    引入lidar接口头文件
    ```c++
    #include "lidar/device_base.h"
    ```

    继承lidar接口抽象类
    ```c++
    class DummyLidar : public LidarDevice {
    public:
        // 构造函数，由AirOS-edge框架并调用，并提供lidar数据回调函数
        DummyLidar(const LidarCallBack& cb): LidarDevice(cb) {}
        ~DummyLidar() = default;
        
        // lidar设备初始化接口，config_file文件内容由用户定义
        bool Init(const std::string& config_file) override;
        
        // lidar设备启动接口
        void Start() override;

        // 设备状态查询接口
        LidarDeviceState GetState() override;
    };
    ```
2. 添加具体设备的`.cpp`文件，引入lidar设备注册工厂，实现相应的接口，用注册宏将具体设备注册给lidar工厂
    
    引入lidar设备注册工厂：
    ```c++
    #include "dummy_lidar.h"
    // 引入lidar设备注册工厂
    #include "lidar/device_factory.h"
    ```

    实现lidar初始化接口：
    ```c++
    bool DummyLidar::Init(const std::string& config_file) {
        /*
            解析config_file参数，初始化lidar各项参数...
        */

        /*
            初始化lidar设备...
        */

        return true;
    }
    ```
    
    实现lidar启动接口
    ```c++
    void DummyLidar::Start() {
        while(true) 
        {
            /*
            制备AirOS-edge结构化的lidar输出数据...
            auto data = std::make_shared<PointCloud>();
            ...
            */

            /*
            将结构化的lidar输出数据传递给AirOS-edge框架提供的回调函数
            sender_(data);
            ...
            */
        }
    }
    ```

    实现lidar状态查询接口
    ```c++
    LidarDeviceState GetState() override {
        /*
            返回lidar当前运行状态
        */
    }
    ```

    将lidar设备注册给lidar设备工厂
    ```c++
    // "dummy_lidar"为注册的具体lidar设备名称
    V2XOS_LIDAR_REG_FACTORY(DummyLidar, "dummy_lidar");
    ```
3. AirOS-edge框架将会以如下方式构造和启动`dummy_lidar`设备

    基于注册设备时的key，利用设备工厂构建指定的设备
    ```c++
    ...
    device_ = LidarDeviceFactory::Instance().GetUnique("dummy_lidar", proc_lidar_data);
    if (device_ == nullptr) {
        return 0;
    }
    ...
    ```

    初始化设备
    ```c++
    ...
    if (!device_->Init("xxx/config.yaml")) {
        ...
        return 0;
    }
    ...
    ```

    启动设备
    ```c++
    ...
    task_.reset(new std::thread([&](){device_->Start();}));
    ...
    ```
    *`device_` 与 `task_` 为AirOS-edge框架内部持有的成员变量*

    *`proc_lidar_data` 为AirOS-edge框架内部定义的lidar结构化输出数据处理函数*