#ifndef _CORE_COMMON_H_
#define _CORE_COMMON_H_

#include <chrono>
#include <ctime>
#include <cstdint>
#include <string>
#include <vector>

/*
 * @brief 公共返回值
 */
#ifndef _COMMON_CODE_
#define _COMMON_CODE_
#define CC_OK                                 0    // 运行成功
#define CC_FAIL                               1    // 失败
#define CC_INVALID_ARGUMENT                   2    // 输入参数有误
#define CC_NOT_FOUND                          3    // 未找到文件，op或其他
#define CC_NOT_MATCHED                        4    // 不匹配
#define CC_ERR_WEIGHTS_NOT_SUPPORT            5    // 模型不支持
#define CC_ERR_IMAGETYPE_NOT_SUPPORT          6    // 图像像素格式不支持
#define CC_ERR_LICENSE                        7    // 授权有误
#define CC_ERR_NOT_REGISTRY                   8    // 未注册对应模块
#define CC_ERR_OUTPUT_TYPE_NOT_SUPPORT        9    // 未知的输出类型
#define CC_ERR_INFERENCE_MODE                 10   // 计算模式有误
#define CC_INVALID_DEVICE                     11   // 计算设备有误
#define CC_DEVICE_UNAVAILABLE                 12   // 计算设备长时间无法访问
#define CC_ERR_WEIGHTS_NOT_FOUND              13   // 模型未找到
#define CC_ERR_CAMERA_ID                      14   // 相机id有误
#define CC_INVALID_RADARPATH                  15   // 输入RADAR路径错误
#define CC_INVALID_CAMERAPATH                 16   // 输入CAMERA路径错误
#define CC_INVALID_CAMERA_PARAMS              17   // 输入CAMERA参数错误
#define CC_INVALID_SAVEDATA_PATH              18   // 无效的数据存储路径
#define CC_OPEN_SAVEDATAFILE_FAILED           19   // 打开存储文本失败
#define CC_CONVERT_DATA_FAILED                20   // 数据转换失败
#define CC_GET_DATA_FAILED                    21   // 获取数据失败
#define CC_GET_RESULT_FAILED                  22   // 获取结果失败
#define CC_ERR_RTSP_URL_PARSE                 23   // RTSP地址无法解析
#define CC_ERR_RTSP_UNREACHABLE               24   // RTSP无法连接
#define CC_ERR_NOT_INIT                       25   // 未初始化
#define CC_EOS                                26   // 流结束信号
#define CC_NO_NEW_DATA                        27   // 没有新的数据
#define CC_LOST_DEVICE                        28   // 设备失联或丢失
#define CC_CONNECTION_REFUSED                 29   // 连接被拒绝
#define CC_OBJECT_TYPE_NOT_MATCHED            30   // 目标类型不匹配
#define CC_OBJECT_FILTERED                    31   // 目标被过滤
#define CC_ERR_LICENSE_MISSING_FIELD          32   // 授权信息缺少字段
#define CC_ERR_LICENSE_INVALID_FIELD          33   // 授权信息字段无效
#define CC_ERR_LICENSE_VERIFY  		          34   // 授权信息校验失败
#define CC_ERR_LICENSE_EXPIRED                35   // 授权信息已过期
#define CC_ERR_LICENSE_HARDWARE               36   // 授权信息硬件不匹配
#endif

#ifndef _COMMON_MAX_MESSAGE_SIZE_
#define _COMMON_MAX_MESSAGE_SIZE_
/*
 * @brief 最大消息大小，单位：字节
 */
#define MAX_MESSAGE_SIZE 1024000
#endif

#ifndef _COMMON_MAX_INSTANCES_
#define _COMMON_MAX_INSTANCES_
/*
 * @brief 最大实例数量
 */
#define MAX_INSTANCES 64
#endif

inline uint64_t get_timestamp() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count());
}

inline void get_time(int* hour, int* minute, int* second) {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    // 转换为time_t类型
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    // 转换为tm结构体
    std::tm* now_tm = std::localtime(&now_time_t);
    // 分别获取小时、分钟和秒
    if (hour != nullptr) {
        *hour = now_tm->tm_hour;
    }
    if (minute != nullptr) {
        *minute = now_tm->tm_min;
    }
    if (second != nullptr) {
        *second = now_tm->tm_sec;
    }
}

#endif