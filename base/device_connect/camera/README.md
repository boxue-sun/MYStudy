### 相机设备
模块提供了接入AIR-OS相机设备所需要的标准接口，定义了AIR-OS结构化的相机输出数据类型，提供相机设备的注册工厂。

用户需要将相机采集的数据封装成如下的AIR-OS结构化的相机输出数据类型
```c++
struct CameraData {
    std::shared_ptr<HWMemory> image; ///<图片指针
    ImageDeviceType device_type;     ///<图片所在设备类型
    uint32_t device_id;              ///<设备ID
    CameraDeviceType camera_type;    ///<相机类型
    std::string camera_name;         ///<相机名称
    ImageMode mode;                  ///<图像模式
    uint32_t height;                 ///<高度
    uint32_t width;                  ///<宽度
    uint64_t timestamp;              ///<时间戳
    uint32_t sequence_num;           ///<序列号
};
```
详细的字段定义请参阅：[device_base.h](./device_base.h)

### 接口含义 
用户需要实现3个接口：`Init`、`GetImage`、`GetState`

#### `Init`接口
用于读入相机的初始化配置文件，实现相机的初始化。

#### `GetImage` 接口
用于实现相机的取图功能，依据指定的相机名称，获取AIROS结构化的标准相机输出数据。

#### `GetState` 接口
用于实现相机的状态查询，返回相机的运行状态。

### 使用方式
1. 在camera目录下建立具体相机设备的目录（建议），添加具体设备的`.h`头文件，引用相机接口头文件，并继承相机接口抽象类（以`dummy_camera`为例）
    
    引入相机接口头文件
    ```c++
    #include "camera/device_base.h"
    ```

    继承相机接口抽象类
    ```c++
    class DummyCamera : public CameraDevice {
    public:
        DummyCamera() = default;
        ~DummyCamera() = default;
        
        // 相机设备初始化接口，config_file文件内容由用户定义
        bool Init(const std::string& config_file) override;
        
        // 取图接口
        std::shared_ptr<CameraData> GetImage(const std::string& camera_name) override;

        // 设备状态查询接口
        CameraDeviceState GetState() override;
    };
    ```
2. 添加具体设备的`.cpp`文件，引入相机设备注册工厂，实现相应的接口，用注册宏将具体设备注册给相机工厂
    
    引入相机设备注册工厂：
    ```c++
    #include "dummy_camera.h"
    // 引入相机设备注册工厂
    #include "camera/device_factory.h"
    ```

    实现相机初始化接口：
    ```c++
    bool DummyCamera::Init(const std::string& config_file) {
        /*
            解析config_file参数，初始化相机各项参数...
        */

        /*
            初始化相机设备...
        */

        return true;
    }
    ```
    
    实现相机取图接口
    ```c++
    std::shared_ptr<CameraData> DummyCamera::GetImage(const std::string& camera_name) {
        /*
            根据camera_name，获取原始相机输出数据...
        */

        /*
            制备AIR-OS结构化的相机输出数据...
            auto data = std::make_shared<CameraData>();
            ...
        */

        // 返回数据
        return data;
    }
    ```

    实现相机状态查询接口
    ```c++
    CameraDeviceState GetState(const std::string& camera_name) override {
        /*
            根据camera_name，返回指定相机状态
        */
    }
    ```

    将相机设备注册给相机设备工厂
    ```c++
    // "dummy_camera"为注册的具体相机设备名称
    V2XOS_CAMERA_REG_FACTORY(DummyCamera, "dummy_camera");
    ```
3. AIR-OS框架将会以如下方式构造和启动`dummy_camera`设备

    基于注册设备时的key，利用设备工厂构建指定的设备
    ```c++
    ...
    auto device = CameraDeviceFactory::Instance().GetUnique("dummy_camera");
    if (device == nullptr) {
        return 0;
    }
    ...
    ```

    初始化设备
    ```c++
    ...
    if (!device->Init("xxx/config.yaml")) {
        ...
        return 0;
    }
    ...
    ```

    取图
    ```c++
    ...
    auto camera_data = device->GetImage("camera7")
    if (!camera_data) {
        ...
        return 0;
    }
    ...
    ```