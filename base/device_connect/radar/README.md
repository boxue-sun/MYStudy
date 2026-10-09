### radar设备
模块提供了接入AirOS-edge radar设备所需要的标准接口，定义了AirOS-edge结构化的radar设备输出数据类型，提供radar设备的注册工厂。

用户需要将radar采集的数据封装成如下的AirOS-edge结构化的radar输出数据类型
```protobuf
message RadarObstacles {
    optional RadarHeader header = 1;  // header
    repeated PerceptionObstacle obstacle = 2;  // An array of obstacles
}
```
详细的字段定义请参阅：[radar_data.proto](../proto/radar_data.proto)
### 接口含义 
用户需要实现3个接口：`Init`、`Start`、`GetState`

#### `Init`接口
用于读入radar设备的初始化配置文件，实现radar设备的初始化。

#### `Start` 接口
用于启动radar设备，获取AirOS-edge结构化的标准radar设备输出数据。

#### `GetState` 接口
用于实现radar设备的状态查询，返回radar设备的运行状态。

### 使用方式
1. 在radar目录下建立具体radar设备的目录（建议），添加具体设备的`.h`头文件，引用radar接口头文件，并继承radar接口抽象类（以`dummy_radar`为例）
    
    引入radar接口头文件
    ```c++
    #include "radar/device_base.h"
    ```

    继承radar接口抽象类
    ```c++
    class DummyRadar : public RadarDevice {
    public:
        // 构造函数，由AirOS-edge框架并调用，并提供radar数据回调函数
        DummyRadar(const RadarCallBack& cb): RadarDevice(cb) {}
        ~DummyRadar() = default;
        
        // radar设备初始化接口，config_file文件内容由用户定义
        bool Init(const std::string& config_file) override;
        
        // radar设备启动接口
        void Start() override;

        // 设备状态查询接口
        RadarDeviceState GetState() override;
    };
    ```
2. 添加具体设备的`.cpp`文件，引入radar设备注册工厂，实现相应的接口，用注册宏将具体设备注册给radar工厂
    
    引入radar设备注册工厂：
    ```c++
    #include "dummy_radar.h"
    // 引入radar设备注册工厂
    #include "radar/device_factory.h"
    ```

    实现radar初始化接口：
    ```c++
    bool DummyRadar::Init(const std::string& config_file) {
        /*
            解析config_file参数，初始化radar各项参数...
        */

        /*
            初始化radar设备...
        */

        return true;
    }
    ```
    
    实现radar启动接口
    ```c++
    void DummyRadar::Start() {
        while(true) 
        {
            /*
            制备AirOS-edge结构化的radar输出数据...
            auto data = std::make_shared<RadarObstacles>();
            ...
            */

            /*
            将结构化的radar输出数据传递给AirOS-edge框架提供的回调函数
            sender_(data);
            ...
            */
        }
    }
    ```

    实现radar状态查询接口
    ```c++
    RadarDeviceState GetState() override {
        /*
            返回radar当前运行状态
        */
    }
    ```

    将radar设备注册给radar设备工厂
    ```c++
    // "dummy_radar"为注册的具体radar设备名称
    V2XOS_Radar_REG_FACTORY(DummyRadar, "dummy_radar");
    ```
3. AirOS-edge框架将会以如下方式构造和启动`dummy_radar`设备

    基于注册设备时的key，利用设备工厂构建指定的设备
    ```c++
    ...
    device_ = RadarDeviceFactory::Instance().GetUnique("dummy_radar", proc_radar_data);
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

    *`proc_radar_data` 为AirOS-edge框架内部定义的radar结构化输出数据处理函数*