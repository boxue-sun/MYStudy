### ipcamera设备
模块提供了接入AirOS-edge ipcamera设备所需要的标准接口，定义了AirOS-edge结构化的ipcamera设备输出数据类型，提供ipcamera设备的注册工厂。支持多路RTSP流接入。

用户需要将ipcamera设备采集的数据封装成如下的AirOS-edge结构化的ipcamera输出数据类型
```protobuf
message CompressedImage {
    optional Header header = 1;
    optional string frame_id = 2;

    // Specifies the format of the data
    //  Acceptable values: jpeg, png, h265, h264
    optional string format = 3; 

    optional bytes data = 4; // Compressed image buffer
    optional double measurement_time = 5; //frame  gps times from Image messag
    optional uint32 frame_type = 6; //IDR:1, others:0
    optional uint32 exposure_time = 7; // image exposure time, unit is us, 0 means invalid
}
```
详细的字段定义请参阅：[ipcamera.proto](../proto/ipcamera.proto)

### 接口含义 
用户需要实现4个接口：`Init`、`Start`、`WriteToDevice`、`GetState`

### 多路流支持
模块支持多路RTSP流接入，每路流独立管理：
- 支持配置文件中的`rtsp_urls`列表配置多路流
- 支持配置文件中的`output_topics`列表指定输出话题
- 每路流独立线程拉流，自动重连
- 数据发布到配置文件中指定的输出话题
- 支持SEI时间戳解析和关键帧检测

#### `Init`接口
用于读入ipcamera设备的初始化配置文件，实现ipcamera设备的初始化。

#### `Start` 接口
用于启动ipcamera设备，获取AirOS-edge结构化的标准ipcamera设备输出数据。

#### `WriteToDevice` 接口
用于将控制信息写入ipcamera设备，接收 `IpCameraReceiveData` 类型的控制数据

#### `GetState` 接口
用于实现ipcamera设备的状态查询，返回ipcamera设备的运行状态。

### 使用方式
1. 在ipcamera目录下建立具体ipcamera设备的目录（建议），添加具体设备的`.h`头文件，引用ipcamera接口头文件，并继承ipcamera接口抽象类（以`dummy_ipcamera`为例）
    
    引入ipcamera接口头文件
    ```c++
    #include "ipcamera/device_base.h"
    ```

    继承ipcamera接口抽象类
    ```c++
    class DummyIpCamera : public IpCameraDevice {
    public:
        // 构造函数，由AirOS-edge框架调用，并提供多路流ipcamera数据回调函数
        // 回调函数签名：void(const std::string& stream_id, const IpCameraDataType& data)
        DummyIpCamera(const IpCameraCallBack& cb): IpCameraDevice(cb) {}
        ~DummyIpCamera() = default;
        
        // ipcamera设备初始化接口，config_file文件内容由用户定义
        bool Init(const std::string& config_file) override;
        
        // ipcamera设备启动接口
        void Start() override;

        // 控制信息写入ipcamera设备接口
        void WriteToDevice(const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) override;

        // 设备状态查询接口
        IpCameraDeviceState GetState() override;
    };
    ```
2. 添加具体设备的`.cpp`文件，引入ipcamera设备注册工厂，实现相应的接口，用注册宏将具体设备注册给ipcamera工厂
    
    引入ipcamera设备注册工厂：
    ```c++
    #include "dummy_ipcamera.h"
    // 引入ipcamera设备注册工厂
    #include "ipcamera/device_factory.h"
    ```

    实现ipcamera初始化接口：
    ```c++
    bool DummyIpCamera::Init(const std::string& config_file) {
        /*
            解析config_file参数，初始化ipcamera各项参数...
        */

        /*
            初始化ipcamera设备...
        */

        return true;
    }
    ```
    
    实现ipcamera启动接口
    ```c++
    void DummyIpCamera::Start() {
        // 多路流示例：为每个流启动独立线程
        for (const auto& stream_info : streams_) {
            std::thread([this, stream_info]() {
                while (!stop_.load()) {
                    /*
                    制备AirOS-edge结构化的ipcamera输出数据...
                    auto data = std::make_shared<os::v2x::device::ipcamera::CompressedImage>();
                    ...
                    */

                    /*
                    将结构化的ipcamera输出数据传递给AirOS-edge框架提供的回调函数
                    sender_(stream_info.first, data);  // 多路流回调，包含stream_id
                    ...
                    */
                }
            }).detach();
        }
    }
    ```

    实现ipcamera控制信息写入接口
    ```c++
    void DummyIpCamera::WriteToDevice(const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) {
       /*
        写入receive_data内容
        ...
        */
    }
    ```

    实现ipcamera状态查询接口
    ```c++
    IpCameraDeviceState GetState() override {
        /*
            返回ipcamera当前运行状态
        */
    }
    ```

    将ipcamera设备注册给ipcamera设备工厂
    ```c++
    // "dummy_ipcamera"为注册的具体ipcamera设备名称
    V2XOS_IPCAMERA_REG_FACTORY(DummyIpCamera, "dummy_ipcamera");
    ```
3. AirOS-edge框架将会以如下方式构造和启动`dummy_ipcamera`设备

    基于注册设备时的key，利用设备工厂构建指定的设备
    ```c++
    ...
    // 多路流回调函数
    auto multi_stream_callback = [this](const std::string& stream_id, const IpCameraDataType& data) {
        // 处理多路流数据，按stream_id分发到不同话题
        std::string topic = "/sensor/ipcamera/h264/" + stream_id;
        Send(topic, data);
    };
    
    device_ = IpCameraDeviceFactory::Instance().GetUnique("dummy_ipcamera", multi_stream_callback);
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

    *`multi_stream_callback` 为AirOS-edge框架内部定义的多路流ipcamera结构化输出数据处理函数*

### 配置文件示例
支持多路流配置：

```yaml
frame_id: "camera_frame"
# 多路RTSP拉流地址配置
rtsp_urls:
  - "rtsp://192.168.1.100:554/stream1"
  - "rtsp://192.168.1.101:554/stream2"
  - "rtsp://192.168.1.102:554/stream3"
# 输出话题配置（与rtsp_urls一一对应）
output_topics:
  - "/sensor/ipcamera/h264/192_168_1_100"
  - "/sensor/ipcamera/h264/192_168_1_101"
  - "/sensor/ipcamera/h264/192_168_1_102"
reconnect_interval: 5  # 重连间隔（秒）
buffer_size: 1024000   # 缓冲区大小
connection_timeout: 10 # 连接超时时间（秒）
enable_sei_timestamp: true  # 是否启用SEI时间戳解析
```

**话题发布规则：**
- 话题：使用配置文件中指定的 `output_topics` 列表
- 每个RTSP URL对应一个输出话题
