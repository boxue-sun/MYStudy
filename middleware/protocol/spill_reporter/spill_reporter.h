/*感知遗洒事件处理模块*/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_SPILL_REPORTER_COMPONENT_H
#define AIROS_MIDDLEWARE_PROTOCOL_SPILL_REPORTER_COMPONENT_H

#include "middleware/runtime/src/air_middleware_component.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "middleware/protocol/om_common/namespace.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_common.h"
NAMESPACE_START_SPILL_REPORTER_COMPONENT_RADAR
#define  LOG_KEY_SPILL_REPORTER "[spill_reporter]"
#define SPILL_REPORTER_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_SPILL_REPORTER
#define SPILL_REPORTER_WARN_PRINT LOG_WARN_IF << LOG_KEY_SPILL_REPORTER
#define SPILL_REPORTER_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_SPILL_REPORTER
#define SPILL_REPORTER_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_SPILL_REPORTER
#define SPILL_REPORTER_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_SPILL_REPORTER


#define MODULE_CONFIG_DIR "/home/airos/protocol/om"
#define MODULE_NAME "SpillReporterComponentAdapter"
#define MODULE_CFG_NAME "SpillReporterComponentAdapter.flag"
// 结构体用于保存上传文件的详细信息
struct UploadFileContext 
{
    FILE *fp;
    std::string remote_path;
    std::string filename;
};

// 定义一个结构体用于传递数据
struct UploadData 
{
    const char* readptr;
    size_t sizeleft;
};

//遗洒事件本地存储json结构
struct SingleCloudSpillEvent: public afl::base::SerializableData
{
    std::string CrossNo = "";
    std::string DevNo = "";
    std::string Timestamp = "";
    std::string ID = "";//事件编号
    int EvtType = 3;//事件类型，遗洒事件：3
    double Lon = 0.000000;//02坐标系，经度
    double Lat = 0.000000;//02坐标系，纬度
    double Ele = 0.000000;//02坐标系，海拔
    // std::string EvtV = "";//事件视频文件url，mp4格式，可能包含多个，以逗号隔开
    // std::string EvtP = "";//事件图片文件url，jpg格式，可能包含多个，以逗号隔开
    uint64_t utctime = 0;
private:
    virtual void serialize(afl::base::json &j) override
    {
        j = {
            {"CrossNo", CrossNo}
            ,{"DevNo", DevNo}
            ,{"Timestamp", Timestamp}
            ,{"ID", ID}
            ,{"EvtType", EvtType}
            ,{"Lon", Lon}
            ,{"Lat", Lat}
            ,{"Ele", Ele}
            // ,{"EvtV", EvtV}
            // ,{"EvtP", EvtP}
        };   
    }
    virtual void deserialize(const afl::base::json &j) override
    {

    }
};

//配置参数
struct SpillReporterConfiger: public afl::base::ConfigerData<SpillReporterConfiger>
{
    double GapTime = 10.0;//s
    double GapDistance = 10.0;//m
    std::string Dir = "/home/airos/spillreporter";
    std::string workParamFilePath = "/home/airos/common_config/work_param_config.flag";
    std::string basicFtpUrl = "ftp://172.20.65.193:2121/";
    std::string FtpUsername = "ftpuser";
    std::string FtpPassword = "password";
    int FtpLocalPort = 40888;

private:
    virtual void writeToFile(ConfigBlock &j) override
    {
        j = {
            {"GapTime", GapTime}
            ,{"GapDistance", GapDistance}
            ,{"Dir", Dir}
            ,{"workParamFilePath", workParamFilePath}
            ,{"basicFtpUrl", basicFtpUrl}
            ,{"FtpUsername", FtpUsername}
            ,{"FtpPassword", FtpPassword}
            ,{"FtpLocalPort", FtpLocalPort}
        };
    }

    virtual void readFromFile(const ConfigBlock &j) override
    {
        GapTime = j.at("GapTime").get<double>();
        GapDistance = j.at("GapDistance").get<double>();
        Dir = j.at("Dir").get<std::string>();
        workParamFilePath = j.at("workParamFilePath").get<std::string>();
        basicFtpUrl = j.at("basicFtpUrl").get<std::string>();
        FtpUsername = j.at("FtpUsername").get<std::string>();
        FtpPassword = j.at("FtpPassword").get<std::string>();
        FtpLocalPort = j.at("FtpLocalPort").get<int>();
    }
};

class SpillReporterComponentAdapter: public airos::middleware::ComponentAdapter<airos::usecase::EventOutputResult>
                    ,public afl::base::Configurable<SpillReporterConfiger>
{
public:
    SpillReporterComponentAdapter();
    virtual ~SpillReporterComponentAdapter();
    bool Init() override;
    bool Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame) override;

private:
    std::string utc2string(uint64_t utctime);
    std::string transformDevNo(std::string devno);
    std::vector<std::string> split_picture_paths(const std::string& picture_str);//分割多张图片路径

    std::string get_current_date();
    static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
    static size_t read_file_callback(void *ptr, size_t size, size_t nmemb, void *stream);
    static size_t read_data_callback(char* ptr, size_t size, size_t nmemb, void* userp);
    bool check_remote_file_exists(const std::string& remote_dir, const std::string& filename);//检查远程文件是否存在
    bool create_remote_directory(const std::string& path);//创建远程目录
    bool ensure_remote_path_exists(const std::string& full_path);//递归创建远程目录
    bool upload_file_ftp(const std::string& local_file, const std::string& remote_dir, const std::string& filename);//上传图片文件
    bool upload_data_ftp(const std::string& remote_dir, const std::string& data, const std::string& filename);

private:
    std::map<std::string, SingleCloudSpillEvent> m_SpillMap;
    std::map<std::string, std::string> m_DevNoMap;
    airos::base::workparam::OmWorkParamConfiger  m_omWorkParamConfiger;
    std::string m_MecNo;
    CURL *	m_ftpHandle = nullptr;
    CURLcode m_ftpRepCode = CURLE_OK;
    std::string m_basicFtpUrl = "";
    std::string m_ftpUsername = "";
    std::string m_ftpPassword = "";
    std::string m_crossNo = "";
    std::string m_basicJpgDir = "/home/airos/";
    int m_ftpLocalPort = 0;
};

REGISTER_AIROS_COMPONENT_CLASS(SpillReporterComponent, airos::usecase::EventOutputResult);
NAMESPACE_ENDED_SPILL_REPORTER_COMPONENT_RADAR
#endif