### traffic_light设备
模块提供了接入AirOS-edge traffic_light设备所需要的标准接口，定义了AirOS-edge结构化的traffic_light设备输出数据类型，提供traffic_light设备的注册工厂。

用户需要将traffic_light设备采集的数据封装成如下的AirOS-edge结构化的traffic_light输出数据类型
```protobuf
message TrafficLightBaseData
{
    repeated OneLightInfo light_info_list = 1;   // 信号灯上所有灯组的步色信息列表
    required uint64 time_stamp = 2;              // UNIX时间戳，精确到ms
    optional int32 period = 3;                   // 红绿灯方案总周期（秒数）
    optional double confidence = 4;              // 置信度，取值为0和1之间的小数，0代表红绿灯信息完全不准确，1代表红绿灯信息完全准确
    optional string vendor = 5;                  // 信号机厂商
    optional string model = 6;                   // 信号机型号
    optional string software_version = 7;        // 信号机软件/固件版本
    required DataSource data_source = 8;         // 输入源类型
    required DeviceWorkState work_status = 9;    // 信号机运行状态
    required ControlMode control_mode = 10;      // 信号机控制模式
    repeated bytes original_data_list = 11;      // 信号机原始报文帧数据
    optional int32 period_count_down = 12;       // 周期内倒计时（取值大于等于1）
    optional uint32 sequence_num = 13;           // 消息序列码
};
```
详细的字段定义请参阅：[traffic_light_data.proto](../proto/traffic_light_data.proto)
### 接口含义 
用户需要实现4个接口：`Init`、`Start`、`WriteToDevice`、`GetState`

#### `Init`接口
用于读入traffic_light设备的初始化配置文件，实现traffic_light设备的初始化。

#### `Start` 接口
用于启动traffic_light设备，获取AirOS-edge结构化的标准traffic_light设备输出数据。

#### `WriteToDevice` 接口
用于将控制信息写入traffic_light设备

#### `GetState` 接口
用于实现traffic_light设备的状态查询，返回traffic_light设备的运行状态。

### 使用方式
1. 在traffic_light目录下建立具体traffic_light设备的目录（建议），添加具体设备的`.h`头文件，引用traffic_light接口头文件，并继承traffic_light接口抽象类（以`dummy_traffic_light`为例）
    
    引入traffic_light接口头文件
    ```c++
    #include "traffic_light/device_base.h"
    ```

    继承traffic_light接口抽象类
    ```c++
    class DummyTrafficLight : public TrafficLightDevice {
    public:
        // 构造函数，由AirOS-edge框架并调用，并提供traffic_light数据回调函数
        DummyTrafficLight(const TrafficLightCallBack& cb): TrafficLightDevice(cb) {}
        ~DummyTrafficLight() = default;
        
        // traffic_light设备初始化接口，config_file文件内容由用户定义
        bool Init(const std::string& config_file) override;
        
        // traffic_light设备启动接口
        void Start() override;

        // 控制信息写入traffic_light设备接口
        void WriteToDevice(const std::shared_ptr<const TrafficLightReciveData>& re_proto) override;

        // 设备状态查询接口
        TrafficLightDeviceState GetState() override;
    };
    ```
2. 添加具体设备的`.cpp`文件，引入traffic_light设备注册工厂，实现相应的接口，用注册宏将具体设备注册给traffic_light工厂
    
    引入traffic_light设备注册工厂：
    ```c++
    #include "dummy_traffic_light.h"
    // 引入traffic_light设备注册工厂
    #include "traffic_light/device_factory.h"
    ```

    实现traffic_light初始化接口：
    ```c++
    bool DummyTrafficLight::Init(const std::string& config_file) {
        /*
            解析config_file参数，初始化traffic_light各项参数...
        */

        /*
            初始化traffic_light设备...
        */

        return true;
    }
    ```
    
    实现traffic_light启动接口
    ```c++
    void DummyTrafficLight::Start() {
        while(true) 
        {
            /*
            制备AirOS-edge结构化的traffic_light输出数据...
            auto data = std::make_shared<TrafficLightBaseData>();
            ...
            */

            /*
            将结构化的traffic_light输出数据传递给AirOS-edge框架提供的回调函数
            sender_(data);
            ...
            */
        }
    }
    ```

    实现traffic_light控制信息写入接口
    ```c++
    void DummyTrafficLight::WriteToDevice(const std::shared_ptr<const TrafficLightReciveData>& re_proto) {
       /*
        写入re_proto内容
        ...
        */
    }
    ```

    实现traffic_light状态查询接口
    ```c++
    TrafficLightDeviceState GetState() override {
        /*
            返回traffic_light当前运行状态
        */
    }
    ```

    将traffic_light设备注册给traffic_light设备工厂
    ```c++
    // "dummy_traffic_light"为注册的具体traffic_light设备名称
    V2XOS_TRAFFIC_LIGHT_REG_FACTORY(DummyTrafficLight, "dummy_traffic_light");
    ```
3. AirOS-edge框架将会以如下方式构造和启动`dummy_traffic_light`设备

    基于注册设备时的key，利用设备工厂构建指定的设备
    ```c++
    ...
    device_ = TrafficLightDeviceFactory::Instance().GetUnique("dummy_traffic_light", proc_traffic_light_data);
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

    *`proc_traffic_light_data` 为AirOS-edge框架内部定义的traffic_light结构化输出数据处理函数*
