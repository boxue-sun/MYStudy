### RSU设备
模块提供了接入AirOS-edge RSU设备所需要的标准接口，定义了AirOS-edge结构化的RSU设备输出数据类型，提供RSU设备的注册工厂。

用户需要将rsu返回的数据封装成如下的AirOS-edge结构化的rsu输出数据类型
```protobuf
message RSUData {
    required RSUDataType type = 1;    // 数据类型
    required bytes data = 2;          // ASN编码数据
    required uint64 time_stamp = 3;   // UNIX时间戳，精确到ms
    optional uint32 sequence_num = 4; // 消息序列码
}
```
详细的字段定义请参阅：[rsu_data.proto](../proto/rsu_data.proto)
### 接口含义 
用户需要实现4个接口：`Init`、`Start`、`WriteToDevice`、`GetState`

#### `Init`接口
用于读入RSU设备的初始化配置文件，实现RSU设备的初始化。

#### `Start` 接口
用于启动RSU设备，获取AirOS-edge结构化的标准RSU设备输出数据。

#### `WriteToDevice` 接口
用于将消息写入RSU设备

#### `GetState` 接口
用于实现RSU设备的状态查询，返回RSU设备的运行状态。

### 使用方式
1. 在RSU目录下建立具体RSU设备的目录（建议），添加具体设备的`.h`头文件，引用RSU接口头文件，并继承RSU接口抽象类（以`dummy_rsu`为例）
    
    引入RSU接口头文件
    ```c++
    #include "rsu/device_base.h"
    ```

    继承RSU接口抽象类
    ```c++
    class DummyRSU : public RSUDevice {
    public:
        // 构造函数，由AirOS-edge框架并调用，并提供RSU数据回调函数
        DummyRSU(const RSUCallBack& cb): RSUDevice(cb) {}
        ~DummyRSU() = default;
        
        // RSU设备初始化接口，config_file文件内容由用户定义
        bool Init(const std::string& config_file) override;
        
        // RSU设备启动接口
        void Start() override;

        // 消息写入接口
        void WriteToDevice(const std::shared_ptr<const RSUData>& re_proto) override;

        // 设备状态查询接口
        RSUDeviceState GetState() override;
    };
    ```
2. 添加具体设备的`.cpp`文件，引入RSU设备注册工厂，实现相应的接口，用注册宏将具体设备注册给RSU工厂
    
    引入RSU设备注册工厂：
    ```c++
    #include "dummy_rsu.h"
    // 引入RSU设备注册工厂
    #include "rsu/device_factory.h"
    ```

    实现RSU初始化接口：
    ```c++
    bool DummyRSU::Init(const std::string& config_file) {
        /*
            解析config_file参数，初始化RSU各项参数...
        */

        /*
            初始化RSU设备...
        */

        return true;
    }
    ```
    
    实现RSU启动接口
    ```c++
    void DummyRSU::Start() {
        while(true) 
        {
            /*
            制备AirOS-edge结构化的RSU输出数据...
            auto data = std::make_shared<RSUData>();
            ...
            */

            /*
            将结构化的RSU输出数据传递给AirOS-edge框架提供的回调函数
            sender_(data);
            ...
            */
        }
    }
    ```

    实现消息写入接口
    ```c++
    void WriteToDevice(const std::shared_ptr<const RSUData>& re_proto) {
        /*
        写入re_proto内容
        ...
        */
    }
    ```

    实现RSU状态查询接口
    ```c++
    RSUDeviceState GetState() override {
        /*
            返回RSU当前运行状态
        */
    }
    ```

    将RSU设备注册给RSU设备工厂
    ```c++
    // "dummy_rsu"为注册的具体RSU设备名称
    V2XOS_RSU_REG_FACTORY(DummyRSU, "dummy_rsu");
    ```
3. AirOS-edge框架将会以如下方式构造和启动`dummy_rsu`设备

    基于注册设备时的key，利用设备工厂构建指定的设备
    ```c++
    ...
    device_ = RSUDeviceFactory::Instance().GetUnique("dummy_rsu", proc_rsu_data);
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

    *`proc_RSU_data` 为AirOS-edge框架内部定义的RSU结构化输出数据处理函数*