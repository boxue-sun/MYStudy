#ifndef RTSP_PUSHER_H
#define RTSP_PUSHER_H

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cassert>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

#define DEBUGPRINT(format,...){\
    char buffer[64] = {0};\
    auto now = std::chrono::system_clock::now();\
    auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);\
    auto now_us_time_t = now_us.time_since_epoch().count();\
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);\
    std::tm tm_now = *std::localtime(&now_c);\
    auto milliseconds = now_us_time_t % 1000000 / 1000;\
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_now);\
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ".%03ld", milliseconds);\
    const char* file = __FILE__;\
    file = basename(file);\
    assert(file);\
    std::ostringstream oss;\
    oss << "[" << buffer << " | " << file << "(" << __LINE__ << ") | " \
        << "Thread:0x" << std::hex << std::this_thread::get_id() << "]:"; \
    std::string header = oss.str();\
    printf("%s" format "\n", header.c_str(), ##__VA_ARGS__);\
}

#define MAX_QUEUE_SIZE 256 ///< 最大队列大小，防止内存溢出

class RTSPPusher {
public:
    RTSPPusher(const std::string& rtsp_url, double fps = 0.0);
    ~RTSPPusher();
    /**
     * @brief 启动 RTSP 推流线程
     * @return true 如果启动成功，false 如果启动失败
     */
    bool Start();
    /**
     * @brief 停止 RTSP 推流线程
     */
    void Stop();
    /**
     * @brief 推送一帧数据到 RTSP 服务器
     * @param frame_data 帧数据指针
     * @param frame_size 帧数据大小
     * @param dts 帧的解码时间戳，单位为毫秒，默认为0
     * @param camera_timestamp 相机时间戳，用于写入SEI帧，单位为毫秒，默认为0
     * @return true 如果推送成功，false 如果推送失败或推送线程未运行
     */
    bool PushFrame(const uint8_t* frame_data, int frame_size, int64_t dts = 0, int64_t camera_timestamp = 0);

private:
    struct Frame {
        std::vector<uint8_t> data;
        int size;
        int64_t timestamp;  ///< 相机时间戳（毫秒）

        Frame(const uint8_t* data, int size, int64_t timestamp = 0) 
            : size(size), timestamp(timestamp) {
            this->data.resize(size);
            memcpy(this->data.data(), data, size);
        }

        std::string to_string() const {
            std::ostringstream oss;
            for (uint8_t byte : data) {
                oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte) << " ";
            }
            return oss.str();
        }
    };


    /**
     * @brief 初始化解码器和解析器
     * 该函数会根据指定的帧率和输出格式初始化解码器和解析器
     */
    void InitializeDecoderAndParser();


    /**
     * @brief 获取当前时间的毫秒时间戳
     * @return 当前时间的毫秒时间戳
     */
    int64_t GetCurrentTimeMs();


    /**
     * @brief 计算帧率
     * @param frame_timestamps 存储每帧收到的时间戳
     * @return 计算得到的帧率
     */
    double GetFrameRate(const std::vector<int64_t>& frame_timestamps);


    /**
     * @brief 打印解码器解析器信息
     * @param pCodecParserCtx 解码器解析器上下文
     */
    void PrintCodecParserInfo(AVCodecParserContext* pCodecParserCtx);


    /**
     * @brief 处理接收到的帧数据
     * @param frame 接收到的帧数据
     */
    void HandleFrame(std::unique_ptr<Frame> frame);

    void ResetOutputContext();


    /**
     * @brief 推流线程循环函数
     * 该函数会不断从帧队列中取出帧并推送到 RTSP 服务器
     */
    void PushLoop();


    /**
     * @brief 检测数据中是否包含 SEI 帧
     * @param data 数据指针
     * @param size 数据大小
     * @return true 如果包含 SEI 帧，false 否则
     */
    bool DetectSEIFrame(const uint8_t* data, int size);

    /**
     * @brief 生成大华格式的 SEI 帧
     * @param out_size 输出大小指针
     * @param timestamp 时间戳（毫秒）
     * @return SEI 帧数据指针，失败返回 nullptr
     */
    uint8_t* CreateDahuaSEIFrame(size_t* out_size, int64_t timestamp);

    /**
     * @brief 在数据前插入 SEI 帧
     * @param original_data 原始数据
     * @param original_size 原始数据大小
     * @param sei_data SEI 帧数据
     * @param sei_size SEI 帧大小
     * @param new_data 新数据指针（需要调用者释放）
     * @param new_size 新数据大小
     * @return true 如果成功，false 否则
     */
    bool InsertSEIFrame(const uint8_t* original_data, int original_size,
                       const uint8_t* sei_data, size_t sei_size,
                       uint8_t** new_data, int* new_size);

private:
    std::string rtsp_url_;              ///< RTSP 推流地址
    double fps_;                        ///< 帧率，0 表示不设置，需要动态计算
    AVFormatContext *output_ctx;        ///< 输出上下文，用于推流
    AVCodecContext *pCodecCtx_;
    AVCodecParserContext *pCodecParserCtx_;
    std::atomic<bool> running_;         ///< 推流线程是否在运行
    std::thread push_thread_;           ///< 推流线程
    std::queue<std::unique_ptr<Frame>> frame_queue_;    ///< 帧队列，用于存储待推送的帧
    std::mutex mutex_;                  ///< 互斥锁，用于保护帧队列
    std::condition_variable cond_var_;  ///< 条件变量，用于线程间同步
    bool find_idr_ = false;             ///< 是否找到 IDR 帧
    int format_ = AV_PIX_FMT_YUV420P;   ///< 输出格式，默认为 YUV420P
    int width_ = 1920;                  ///< 输出视频宽度
    int height_ = 1080;                 ///< 输出视频高度
    const std::vector<double> STANDARD_FPS = {          ///< 常见标准帧率列表
        24.0, 25.0, 30.0, 48.0, 50.0, 60.0, 120.0
    };
    std::vector<int64_t> frame_rs_;     ///< 存储每帧收到的时间戳，根据间隔计算帧率
    int64_t average_interval_ = 0;      ///< 平均间隔时间
    int64_t last_dts_ = 0;              ///< 上一帧的解码时间戳
    
    // SEI 帧相关成员变量
    bool has_sei_frame_ = false;        ///< 是否检测到原始 SEI 帧
    bool enable_dahua_sei_ = true;      ///< 是否启用大华 SEI 帧生成
};

#endif // RTSP_PUSHER_H