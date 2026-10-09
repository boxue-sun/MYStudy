#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <mutex>

#include "cyber/cyber.h"
#include "cyber/service_discovery/topology_manager.h"
#include "cyber/service_discovery/specific_manager/channel_manager.h"

#include "middleware/protocol/rtsp_tool/channel_data_processor.h"

using apollo::cyber::service_discovery::TopologyManager;
using apollo::cyber::service_discovery::ChannelManager;

// 配置结构
struct AppConfig {
    bool debug = false;
    bool show_help = false;
    double fps = 25.0; // 默认帧率
    std::vector<std::string> channels;
    std::string keyword = "h264"; // 默认关键字为 h264
    std::string ip_address = "127.0.0.1"; // IP地址
    uint16_t port = 8554; // 端口号
};

struct ChannelProcessor {
    std::shared_ptr<ChannelDataProcessor> processor;
    std::shared_ptr<apollo::cyber::Reader<os::v2x::device::ipcamera::CompressedImage>> reader;
};

void PrintHelp(const char* program_name) {
    std::cout << "\nRTSP Tool - CyberRT Camera Channel Transfer to RTSP Server Utility\n"
              << "Version: 1.0\n\n"
              << "Usage: " << program_name << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  -h, --help            Show this help information\n"
              << "  -d, --debug           Enable debug mode (detailed output)\n"
              << "  -c, --channels LIST   Monitor specific channels (comma-separated<,>)\n"
              << "  -k, --keyword KEY     Filter channels containing keyword\n"
              << "  -f, --fps             Camera video fps, default is 25, 0 means dynamic detect\n"
              << "  -i, --ip IP           Specify the IP address, default is 127.0.0.1\n"
              << "  -p, --port PORT       Specify the port number, default is 8554\n\n"
              << "Monitoring Modes:\n"
              << "  Default mode:         Monitor all channels in the system, keyword default is h264\n"
              << "  Specific channels:    Monitor only specified channels (-c option)\n"
              << "  Keyword filtering:    Monitor channels containing keyword (-k option)\n\n"
              << "Examples:\n"
              << "  " << program_name << "                                                                            # Monitor all channels\n"
              << "  " << program_name << " -d                                                                         # Enable debug mode\n"
              << "  " << program_name << " -c /sensor/ipcamera/h264/172_20_65_196,/sensor/ipcamera/h264/172_20_65_195 # Monitor specific channels\n"
              << "  " << program_name << " -k ipcamera                                                                # Monitor channels with 'ipcamera'\n"
              << "  " << program_name << " -c /rtsp/stream -d                                                         # Debug mode for specific channel\n"
              << "  " << program_name << " -i 192.168.1.100 -p 8554                                                   # Specify IP and port\n\n"
              << std::endl;
}

// 解析命令行参数
AppConfig ParseArguments(int argc, char* argv[]) {
    AppConfig config;
    
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        
        // 帮助选项
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            config.show_help = true;
        }
        // 调试模式
        else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--debug") == 0) {
            config.debug = true;
        }
        // 指定通道
        else if (strcmp(arg, "-c") == 0 || strcmp(arg, "--channels") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -c/--channels option\n";
                config.show_help = true;
                break;
            }
            std::string list = argv[++i];
            size_t pos = 0;
            while ((pos = list.find(',')) != std::string::npos) {
                config.channels.push_back(list.substr(0, pos));
                list = list.substr(pos + 1);
            }
            if (!list.empty()) config.channels.push_back(list);
        }
        // 关键字过滤
        else if (strcmp(arg, "-k") == 0 || strcmp(arg, "--keyword") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -k/--keyword option\n";
                config.show_help = true;
                break;
            }
            config.keyword = argv[++i];
        }
        // 帧率设置
        else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--fps") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -f/--fps option\n";
                config.show_help = true;
                break;
            }
            try {
                config.fps = std::stod(argv[++i]);
                if (config.fps < 0) {
                    std::cerr << "Warning: FPS set to negative value, using dynamic detection\n";
                    config.fps = 0;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid FPS value: " << e.what() << "\n";
                config.show_help = true;
            }
        }
        // IP地址设置
        else if (strcmp(arg, "-i") == 0 || strcmp(arg, "--ip") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -i/--ip option\n";
                config.show_help = true;
                break;
            }
            config.ip_address = argv[++i];
        }
        // 端口号设置
        else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--port") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -p/--port option\n";
                config.show_help = true;
                break;
            }
            try {
                config.port = static_cast<uint16_t>(std::stoi(argv[++i]));
                if (config.port < 1 || config.port > 65535) {
                    std::cerr << "Error: Port number must be between 1 and 65535\n";
                    config.show_help = true;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid port value: " << e.what() << "\n";
                config.show_help = true;
            }
        }
        // 未知选项
        else {
            std::cerr << "Error: Unknown option '" << arg << "'\n";
            config.show_help = true;
        }
    }
    
    return config;
}

// 从 channel name 中提取标识符（IP地址或通道名）
std::string ExtractIdentifierFromChannelName(const std::string& channel_name) {
    size_t last_slash_pos = channel_name.find_last_of('/');
    if (last_slash_pos != std::string::npos && last_slash_pos + 1 < channel_name.size()) {
        std::string id_part = channel_name.substr(last_slash_pos + 1);
        // 如果包含下划线，假设是IP格式
        if (id_part.find('_') != std::string::npos) {
            std::replace(id_part.begin(), id_part.end(), '_', '.');
        }
        return id_part;
    }
    // 无法提取时使用整个通道名（替换特殊字符）
    std::string safe_name = channel_name;
    std::replace_if(safe_name.begin(), safe_name.end(), 
        [](char c) { return c == '/' || c == ':'; }, '_');
    return safe_name;
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    AppConfig appConfig = ParseArguments(argc, argv);
    
    // 显示帮助信息并退出
    if (appConfig.show_help) {
        PrintHelp(argv[0]);
        return 0;
    }

    // 存储通道处理器和互斥锁
    std::map<std::string, ChannelProcessor> channel_processors;
    std::mutex processors_mutex;
    const std::string kDefaultRTSPPrefix = "rtsp://root:root@";
    const std::string kDefaultRTSPMidFix = "/stream/";
    std::string kDefaultRTSPUrl = kDefaultRTSPPrefix + appConfig.ip_address + ":" + std::to_string(appConfig.port) + kDefaultRTSPMidFix;

    // 初始化CyberRT框架
    apollo::cyber::Init(argv[0]);

    // 创建节点
    auto listener_node = apollo::cyber::CreateNode("rtsp_tool_node");
    if (!listener_node) {
        DEBUGPRINT("Failed to create node...");
        return false;
    }
    
    // 创建通道处理器的函数
    auto CreateChannelProcessor = [&](const std::string& channel_name) -> bool {
        std::string identifier = ExtractIdentifierFromChannelName(channel_name);
        std::string rtsp_url = kDefaultRTSPUrl + identifier;

        DEBUGPRINT("Creating processor for channel:%s, RTSP URL:%s", channel_name.c_str(), rtsp_url.c_str());

        // 配置reader
        apollo::cyber::ReaderConfig config;
        config.channel_name = channel_name;
        config.pending_queue_size = 10;

        // 创建处理器
        auto processor = std::make_shared<ChannelDataProcessor>(rtsp_url, appConfig.fps, appConfig.debug);
        auto reader = listener_node->CreateReader<os::v2x::device::ipcamera::CompressedImage>(
            config, [processor](const std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>& msg) {
                processor->ProcessData(msg);
            });
        
        if (!reader) {
            DEBUGPRINT("Failed to create reader for channel: %s", channel_name.c_str());
            return false;
        } else {
            DEBUGPRINT("Reader created for channel: %s", channel_name.c_str());
        }

        // 添加到处理器映射
        {
            std::lock_guard<std::mutex> lock(processors_mutex);
            channel_processors[channel_name] = {processor, reader};
            DEBUGPRINT("Processor created for channel: %s", channel_name.c_str());
        }
        
        return true;
    };
    
    // 处理通道加入的回调
    auto HandleChannelJoin = [&](const std::string& channel_name) {
        // 检查关键字匹配
        if (appConfig.keyword.empty() || channel_name.find(appConfig.keyword) != std::string::npos) {
            DEBUGPRINT("Channel joined: %s", channel_name.c_str());
            // 检查是否已经存在处理器
            {
                std::lock_guard<std::mutex> lock(processors_mutex);
                if(channel_processors.find(channel_name) != channel_processors.end()) {
                    DEBUGPRINT("Processor already exists for channel: %s", channel_name.c_str());
                    return; // 已经存在处理器，忽略重复创建
                }
            }
            // 创建新的通道处理器
            CreateChannelProcessor(channel_name);
        } else {
            DEBUGPRINT("Channel ignored (keyword mismatch): %s", channel_name.c_str());
        }
    };
    
    // 处理通道离开的回调
    auto HandleChannelLeave = [&](const std::string& channel_name) {
        std::lock_guard<std::mutex> lock(processors_mutex);
        auto it = channel_processors.find(channel_name);
        if (it != channel_processors.end()) {
            DEBUGPRINT("Removing processor and reader for channel: %s", channel_name.c_str());
            it->second.processor->Stop();         // 停止处理器
            it->second.reader.reset();            
            listener_node->DeleteReader(it->first); // 销毁reader (不然重新播放同channel数据，创建会失败)
            channel_processors.erase(it);
        }
    };

    // 模式1: 监控指定通道
    if (!appConfig.channels.empty()) {
        DEBUGPRINT("Operating in specific channels mode with %zu channels", appConfig.channels.size());
        for (const auto& channel_name : appConfig.channels) {
            if (!CreateChannelProcessor(channel_name)) {
                DEBUGPRINT("Failed to create processor for channel: %s", channel_name.c_str());
            }
            else {
                DEBUGPRINT("Successfully created processor for channel: %s", channel_name.c_str());
            }
        }
    } 
    // 模式2: 动态监控所有通道
    else {
        DEBUGPRINT("Operating in dynamic discovery mode with keyword: %s", appConfig.keyword.c_str());

        // 获取拓扑管理器
        auto topology_manager = TopologyManager::Instance();
        if (topology_manager == nullptr) {
            DEBUGPRINT("TopologyManager instance is null, cannot monitor channels");
            return -1;
        }

        // 获取通道管理器
        auto channel_manager = topology_manager->channel_manager();
        if (channel_manager == nullptr) {
            DEBUGPRINT("ChannelManager instance is null, cannot monitor channels");
            return -2;
        }

        // 添加通道变更监听器
        channel_manager->AddChangeListener(
            [&](const apollo::cyber::proto::ChangeMsg& change_msg) {
                const auto& role_attr = change_msg.role_attr();
                const std::string channel_name = role_attr.channel_name();
                if(::apollo::cyber::proto::RoleType::ROLE_WRITER == change_msg.role_type())
                {
                    if (::apollo::cyber::proto::OperateType::OPT_JOIN == change_msg.operate_type()) {
                        DEBUGPRINT("Channel change detected: %s, operation: JOIN", channel_name.c_str());
                        HandleChannelJoin(channel_name);
                    } else {
                        DEBUGPRINT("Channel change detected: %s, operation: LEAVE", channel_name.c_str());
                        HandleChannelLeave(channel_name);
                    }
                }
            }
        );
    }

    DEBUGPRINT("RTSP Tool started successfully");
    DEBUGPRINT("Press Ctrl+C to exit");

    // 保持程序运行
    apollo::cyber::WaitForShutdown();

    // 清理所有处理器
    {
        std::lock_guard<std::mutex> lock(processors_mutex);
        DEBUGPRINT("Stopping all channel processors...");
        for (auto& entry : channel_processors) {
            entry.second.processor->Stop();
            listener_node->DeleteReader(entry.first);
            entry.second.reader.reset(); 
        }
        channel_processors.clear();
    }

    DEBUGPRINT("RTSP Tool exited gracefully");
    return 0;
}