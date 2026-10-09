### ins设备
模块提供了接入AirOS-edge ins设备所需要的标准接口，定义了AirOS-edge结构化的ins输出数据类型，提供ins设备的注册工厂。

用户需要将ins采集的数据封装成如下的AirOS-edge结构化的ins输出数据类型
```protobuf
message AoGPSFrame{
  //INS solution type.
  enum Type {
    // Do NOT use.
    // Invalid solution due to insufficient observations, no initial GNSS, ...
    INVALID = 0;
    // Use with caution. The covariance matrix may be unavailable or incorrect.
    // High-variance result due to aligning, insufficient vehicle dynamics, ...
    CONVERGING = 1;
    // Safe to use. The INS has fully converged.
    GOOD = 2;
  }
  //Altitude(meters). Positive is above reference (e.g., sea level).
  optional float altitude = 1;
  //Vertical speed(meters/second)
  optional float climb = 2;
  //Direction deviation angle(degrees)
  optional float direction = 3;
  //Horizontal accuracy
  optional float hacc = 4;
  //Horizontal dilution of precision
  optional float hdop = 5;
  //Latitude(degrees). Positive is north of equator; negative is south.
  optional float latitude = 6;
  //Longitude (degrees). Positive is east of prime meridian; negative west.
  optional float longitude = 7;
  // East/north/up in meters per second.
  optional Point3D linearVelocity = 8;
  //(usec)
  optional uint64 timestamp_us = 9;
  optional UtcDate utcData = 10;
  optional UtcTime utcTime = 11;
  //Vertical accuracy
  optional float vacc = 12;
  //Vertical dilution of precision
  optional float vdop = 13;
  //sequence_num
  optional uint32 sequence_num = 14;
  //INS solution type.
  optional Type type = 15;
}

message AoIMUFrame{
  //Triaxial acceleration
  optional Point3D acceleration = 1;
  //IMU heading angle
  optional float heading = 2;
  //IMU heading angle type
  optional HeadingType headingType = 3;
  //magnetometer(utesla)
  optional double magnetometer = 4;
  //orientation(degrees)
  optional double orientation = 5;
  //orientationQuaternion(x,y,z,2)
  optional Quaternion orientationQuaternion = 6;
  //(usec)
  optional double timestamp_us = 7;
  //around forward/left/up axes in radians per second.
  optional Point3D angularVelocity = 8;
  //sequence_num
  optional uint32 sequence_num = 9;
}
```
详细的字段定义请参阅：[ins_data.proto](../proto/ins_data.proto)
### 接口含义 
用户需要实现3个接口：`Init`、`Start`、`GetState`

#### `Init`接口
用于读入ins的初始化配置文件，实现ins的初始化。

#### `Start` 接口
用于启动ins设备，获取AirOS-edge结构化的标准ins输出数据。

#### `GetState` 接口
用于实现ins的状态查询，返回ins的运行状态。

### 使用方式
1. 在ins目录下建立具体ins设备的目录（建议），添加具体设备的`.h`头文件，引用ins接口头文件，并继承ins接口抽象类（以`dummy_ins`为例）
    
    引入ins接口头文件
    ```c++
    #include "ins/device_base.h"
    ```

    继承ins接口抽象类
    ```c++
    class DummyIns : public InsDevice {
    public:
        DummyIns(const GnssCallBack& cb_1, const IMUCallBack& cb_2)
            : InsDevice(cb_1, cb_2) {}
        virtual ~DummyIns() = default;

        bool Init(const std::string& config_file) override;

        void Start() override;

        InsDeviceState GetState() override {
            return InsDeviceState::NORMAL;
        }
    };
    ```
2. 添加具体设备的`.cpp`文件，引入ins设备注册工厂，实现相应的接口，用注册宏将具体设备注册给ins工厂
    
    引入ins设备注册工厂：
    ```c++
    #include "dummy_ins.h"
    // 引入ins设备注册工厂
    #include "ins/device_factory.h"
    ```

    实现ins初始化接口：
    ```c++
    bool DummyIns::Init(const std::string& config_file) {
        std::ofstream fs;
        fs.open(config_file, std::ios::in);
        if (!fs.is_open()) {
            return false;
        }
        /*
        解析config_file参数，初始化相机各项参数
        code here...

        */
        return true;
    }
    ```
    
    实现ins启动接口
    ```c++
    void DummyIns::Start() {
        int i = 0;
        while(i < 5) {
            // 设备数据产生与格式化制备
            auto data1 = std::make_shared<AoGPSFrame>();
            auto data2 = std::make_shared<AoIMUFrame>();
            /*
            填充格式化的输出数据
            code here...

            */
            data1->mutable_header()->set_sequence_num(i);
            data2->mutable_header()->set_sequence_num(i);
            ++i;
            // 将结构化数据输出给回调函数
            sender_1_(data1);
            sender_2_(data2);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```

    实现lidar状态查询接口
    ```c++
    InsDeviceState GetState() override {
        /*
            返回ins当前运行状态
        */
    }
    ```

    将ins设备注册给ins设备工厂
    ```c++
    // 将DummyIns设备以”dummy_ins“名称注册给ins设备工厂
    V2XOS_INS_REG_FACTORY(DummyIns, "dummy_ins");
    ```
3. AirOS-edge框架将会以如下方式构造和启动`dummy_ins`设备

    基于注册设备时的key，利用设备工厂构建指定的设备
    ```c++
    ...
    auto device = InsDeviceFactory::Instance().GetShared(device_type, proc_gnss_data, proc_imu_data);
    if (!device) {
        std::cerr << "Build device instance failed! skip: " << device_type << std::endl;
        continue;
    }
    ...
    ```

    初始化设备
    ```c++
    ...
    if (!device->Init(ins_conf)) {
        std::cerr << "Init " << ins_name << " failed! skip: " << ins_name << std::endl;
        continue;
    }
    ...
    ```

    启动设备
    ```c++
    ...
    task_map.emplace(ins_name, new std::thread([device](){
        device->Start();
    }));
    ...
    ```
    *`device` 与 `task_map` 为AirOS-edge框架内部持有的成员变量*

    *`proc_gnss_data, proc_imu_data` 为AirOS-edge框架内部定义的ins结构化输出数据处理函数*