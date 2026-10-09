#include "spill_reporter.h"
#include "base/common/math_util.h"
NAMESPACE_START_SPILL_REPORTER_COMPONENT_RADAR
SpillReporterComponentAdapter::SpillReporterComponentAdapter()
    : Configurable<SpillReporterConfiger>(MODULE_NAME, MODULE_CONFIG_DIR, MODULE_CFG_NAME)
{

}

SpillReporterComponentAdapter::~SpillReporterComponentAdapter()
{

}

bool SpillReporterComponentAdapter::Init()
{
    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // 创建一个 CURL 句柄，循环复用以提高性能
    m_ftpHandle = curl_easy_init();
    if(!m_ftpHandle) 
    {
        SPILL_REPORTER_ERROR_PRINT << "Failed to init curl";
        return false;
    }

    airos::base::workparam::WorkParam::getWorkParamFromFile(getConfiger().workParamFilePath, m_omWorkParamConfiger);
    SPILL_REPORTER_DEBUG_PRINT << "Camera DevNo transform";
    for(int i = 0; i < m_omWorkParamConfiger.sensorDeviceWorkParamList.size(); i++)
    {
        if(m_omWorkParamConfiger.sensorDeviceWorkParamList[i].deviceType == airos::base::workparam::WorkParamDeviceType::WorkParamDeviceTypeCamera && !m_omWorkParamConfiger.sensorDeviceWorkParamList[i].deviceEsn.empty())
        {
            // size_t start_pos = m_omWorkParamConfiger.sensorDeviceWorkParamList[i].deviceIp.find_last_of(".");
            // if(start_pos == std::string::npos)
            // {
            //     continue;
            // }
            // std::string tempid = m_omWorkParamConfiger.sensorDeviceWorkParamList[i].deviceIp.substr(start_pos + 1);
            std::string tempid = m_omWorkParamConfiger.sensorDeviceWorkParamList[i].deviceSn;
            m_DevNoMap[tempid] = m_omWorkParamConfiger.sensorDeviceWorkParamList[i].deviceEsn;
            SPILL_REPORTER_DEBUG_PRINT << "old: " << tempid << ", new: " << m_omWorkParamConfiger.sensorDeviceWorkParamList[i].deviceEsn;
        }
    }
    m_MecNo = m_omWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
    SPILL_REPORTER_DEBUG_PRINT << "get MecNo: " << m_MecNo;

    m_crossNo = std::to_string(m_omWorkParamConfiger.mecDeviceWorkParam.crossID);
    m_ftpUsername = getConfiger().FtpUsername;
    m_ftpPassword = getConfiger().FtpPassword;
    m_basicFtpUrl = getConfiger().basicFtpUrl;
    m_basicJpgDir = getConfiger().Dir;
    m_ftpLocalPort = getConfiger().FtpLocalPort;
    return true;
}

bool SpillReporterComponentAdapter::Proc(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
    if(now % (24*60*60*1000) <= 20*1000)//00:00:00-00:00:20清空map
    {
        m_SpillMap.clear();
    }
    if(mecDeviceData == nullptr 
        || mecDeviceData->events().empty() 
        || !mecDeviceData->has_header())
    {
        SPILL_REPORTER_DEBUG_PRINT << "not recv events";
        return false;
    }

    for(int i = 0; i < mecDeviceData->events_size(); i++)
    {
        auto event_info = mecDeviceData->events(i);
        if(event_info.event_type_mec() != 3
            || !event_info.has_position_gcs()
            || !event_info.has_event_picture())
        {
            SPILL_REPORTER_DEBUG_PRINT << "current event is not spill or do not have position";
            continue;
        }

        std::string temp_devno = transformDevNo(mecDeviceData->header().dev_no());
        SingleCloudSpillEvent scpe;
        scpe.CrossNo = m_crossNo;
        scpe.DevNo = temp_devno;
        scpe.Timestamp = utc2string(mecDeviceData->header().timestamp_millisecond());
        scpe.ID = event_info.id_str();
        scpe.EvtType = 3;
        scpe.Lon = event_info.position_gcs().lon();
        scpe.Lat = event_info.position_gcs().lat();
        scpe.Ele = event_info.position_gcs().ele();
        // scpe.EvtV = event_info.event_video();
        // scpe.EvtP = event_info.event_picture();
        scpe.utctime = mecDeviceData->header().timestamp_millisecond();

        std::string dateStr = get_current_date();//yyyymmdd

        if(m_SpillMap.find(scpe.ID) == m_SpillMap.end())//未找到
        {
            std::string repeatID = "";
            for(auto& iter : m_SpillMap)
            {
                uint64_t temp_time = iter.second.utctime;
                if(scpe.utctime - temp_time > getConfiger().GapTime*1000)
                {
                    SPILL_REPORTER_DEBUG_PRINT << "pass 10s since last spill";
                    continue;
                }
                double temp_distance = airos::base::MathUtil::getDistance(scpe.Lat, scpe.Lon, iter.second.Lat, iter.second.Lon);
                if(temp_distance > getConfiger().GapDistance)
                {
                    SPILL_REPORTER_DEBUG_PRINT << "distance more than 10m";
                    continue;                 
                }
                //与已有遗洒事件重复，合并EvtV和EvtP到已有遗洒事件json文件中
                SPILL_REPORTER_DEBUG_PRINT << "distance less than 10m";
                repeatID = iter.first;
                break;                            
            }
            if(repeatID == "")//新事件，ftp上传图片和json描述文件
            {
                afl::base::json jspill = scpe;
                std::string jsonDesc = jspill.dump();

                //上传图片路径：crossNo + 日期yyyymmdd + 事件ID + 相机ID + jpg文件
                std::string remote_pictrue_path = dateStr;
                std::string remote_pictrue_name = m_crossNo + "_" + temp_devno + "_" + scpe.ID + ".jpg";
                std::vector<std::string> picture_paths = split_picture_paths(event_info.event_picture());
                for(const auto& picture_path : picture_paths)
                {
                    std::string local_pictrue_path = m_basicJpgDir + picture_path;

                    ensure_remote_path_exists(remote_pictrue_path);
                    if(upload_file_ftp(local_pictrue_path, remote_pictrue_path, remote_pictrue_name))
                    {
                        SPILL_REPORTER_DEBUG_PRINT << "ftp upload jpg success!";
                    }
                    else
                    {
                        SPILL_REPORTER_ERROR_PRINT << "ftp upload jpg failed!";
                    }                    
                }

                //上传json文件路径：crossNo + 日期yyyymmdd + "EventsDesc" + json文件
                std::string remote_jsonDesc_path = dateStr;
                std::string remote_jsonDesc_name = m_crossNo + "_" + temp_devno + "_" + scpe.ID + ".json";
                ensure_remote_path_exists(remote_jsonDesc_path);
                if(upload_data_ftp(remote_jsonDesc_path, jsonDesc, remote_jsonDesc_name))
                {
                    SPILL_REPORTER_DEBUG_PRINT << "ftp upload json desc success!";
                }
                else
                {
                    SPILL_REPORTER_ERROR_PRINT << "ftp upload json desc failed!";
                }

                m_SpillMap[scpe.ID] = scpe;
            }
            else//被去重，ftp上传图片
            {
                //上传图片路径：crossNo + 日期yyyymmdd + 事件ID + 相机ID + jpg文件
                std::string remote_pictrue_path = dateStr;
                //std::string remote_pictrue_name = m_crossNo + "_" + temp_devno + "_" + scpe.ID + ".jpg";
                
                std::vector<std::string> picture_paths = split_picture_paths(event_info.event_picture());
                std::string remote_pictrue_name = m_crossNo + "_" + temp_devno + "_" + scpe.ID + ".jpg";
                for(const auto& picture_path : picture_paths)
                {
                    std::string local_pictrue_path = m_basicJpgDir + picture_path;
                    
                    ensure_remote_path_exists(remote_pictrue_path);
                    if(upload_file_ftp(local_pictrue_path, remote_pictrue_path, remote_pictrue_name))
                    {
                        SPILL_REPORTER_DEBUG_PRINT << "ftp upload jpg success!";
                    }
                    else
                    {
                        SPILL_REPORTER_ERROR_PRINT << "ftp upload jpg failed!";
                    }

                }
            }
        }
        else//找到相同ID遗洒事件，ftp上传图片
        {
            //上传图片路径：crossNo + 日期yyyymmdd + 事件ID + 相机ID + jpg文件
            std::string remote_pictrue_path = dateStr;
            std::string remote_pictrue_name = m_crossNo + "_" + temp_devno + "_" + scpe.ID + ".jpg";

            std::vector<std::string> picture_paths = split_picture_paths(event_info.event_picture());
            for(const auto& picture_path : picture_paths)
            {
                std::string local_pictrue_path = m_basicJpgDir + picture_path;

                ensure_remote_path_exists(remote_pictrue_path);
                if(upload_file_ftp(local_pictrue_path, remote_pictrue_path, remote_pictrue_name))
                {
                    SPILL_REPORTER_DEBUG_PRINT << "ftp upload jpg success!";
                }
                else
                {
                    SPILL_REPORTER_ERROR_PRINT << "ftp upload jpg failed!";
                }
            }
        }
    }
}

std::string SpillReporterComponentAdapter::utc2string(uint64_t utctime)
{
    char buf[32] = { 0 };
    struct tm tm_time;

    time_t seconds = static_cast<time_t>(utctime/1000);
    int millseconds = utctime % (1000);

    localtime_r(&seconds, &tm_time);

    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d:%03d", tm_time.tm_year + 1900,
                 tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, millseconds);
    return buf;
}

std::vector<std::string> SpillReporterComponentAdapter::split_picture_paths(const std::string& picture_str)
{
    std::vector<std::string> paths;
    std::stringstream ss(picture_str);
    std::string item;
    
    while (std::getline(ss, item, ','))
    {
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        
        if (start != std::string::npos && end != std::string::npos)
        {
            paths.push_back(item.substr(start, end - start + 1));
        }
    }
    
    return paths;
}

std::string SpillReporterComponentAdapter::transformDevNo(std::string devno)
{
    std::string DevNo = devno;
    // size_t pos = devno.find("_");
    // if(pos == std::string::npos)
    // {
    //     return DevNo;
    // }

    // std::string part_devno = devno.substr(pos + 1);

    if(m_DevNoMap.find(DevNo) != m_DevNoMap.end())
    {
        DevNo = m_DevNoMap[DevNo];
        SPILL_REPORTER_DEBUG_PRINT << "transform DevNo: " << devno << " -> " << DevNo;
    }

    return DevNo;
}

std::string SpillReporterComponentAdapter::get_current_date()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buf[20];
    sprintf(buf, "%04d%02d%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
    return std::string(buf);
}

size_t SpillReporterComponentAdapter::write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return size * nmemb;
}

size_t SpillReporterComponentAdapter::read_file_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    UploadFileContext *ctx = (UploadFileContext *)stream;
    if (ferror(ctx->fp)) 
    {
        return CURL_READFUNC_ABORT;
    }
    return fread(ptr, size, nmemb, ctx->fp);
}

size_t SpillReporterComponentAdapter::read_data_callback(char* ptr, size_t size, size_t nmemb, void* userp)
{
    struct UploadData* upload = (struct UploadData*)userp;
    size_t max_copy = size * nmemb;

    if (max_copy < 1 || upload->sizeleft <= 0) 
    {
        return 0;
    }

    size_t copy_size = (upload->sizeleft < max_copy) ? upload->sizeleft : max_copy;
    memcpy(ptr, upload->readptr, copy_size);
    
    upload->readptr += copy_size;
    upload->sizeleft -= copy_size;

    return copy_size;
}

bool SpillReporterComponentAdapter::create_remote_directory(const std::string& path)
{
    std::string url = m_basicFtpUrl + path + "/"; // 确保以 / 结尾
    
    curl_easy_reset(m_ftpHandle);
    
    curl_easy_setopt(m_ftpHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_USERNAME, m_ftpUsername.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_PASSWORD, m_ftpPassword.c_str());
    
    // 关键：使用 FTP 的 MKD 命令
    curl_easy_setopt(m_ftpHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 0L); // 我们自己手动控制，以便更好的错误处理
    curl_easy_setopt(m_ftpHandle, CURLOPT_NOBODY, 1L); // 不下载目录列表
    if (m_ftpLocalPort > 0)
    {
        curl_easy_setopt(m_ftpHandle, CURLOPT_LOCALPORT, m_ftpLocalPort);
    }
    
    // 尝试创建目录
    // 注意：libcurl 没有直接的 "mkdir" 选项，我们利用 CURLOPT_CUSTOMREQUEST
    // 或者 CURLOPT_FTP_CREATE_MISSING_DIRS (CURL 7.19.1+)
    // 这里使用更稳健的方法：尝试切换到该目录，失败则创建
    
    // 方法 A: 尝试 CWD。如果成功，说明目录已存在。
    // curl_easy_setopt(m_ftpHandle, CURLOPT_CUSTOMREQUEST, "CWD");
    // m_ftpRepCode = curl_easy_perform(m_ftpHandle);
    
    // if (m_ftpRepCode == CURLE_OK) 
    // {
    //     // 目录已存在
    //     SPILL_REPORTER_DEBUG_PRINT << "[INFO] Remote directory created: " << path;
    //     return true;
    // }
    
    // 方法 B: 目录不存在，尝试 MKD
    curl_easy_setopt(m_ftpHandle, CURLOPT_CUSTOMREQUEST, "MKD");
    m_ftpRepCode = curl_easy_perform(m_ftpHandle);
    
    if (m_ftpRepCode == CURLE_OK) 
    {
        SPILL_REPORTER_DEBUG_PRINT << "[INFO] Remote directory created: " << path;
        return true;
    } 
    else 
    {
        // 某些服务器返回 550 表示已存在，虽然上面 CWD 失败了，这里可能也有其他错误
        // 简单起见，如果 MKD 失败我们假设可能存在或者权限不足，继续尝试
        SPILL_REPORTER_ERROR_PRINT << "[WARN] Failed to create directory " << path << " (Code: " << m_ftpRepCode << ")";
        return false; 
    }
    return true;
}

bool SpillReporterComponentAdapter::ensure_remote_path_exists(const std::string& full_path)
{
    std::vector<std::string> paths;
    std::string temp;
    
    // 简单的路径分割
    for(char c : full_path) 
    {
        if (c == '/') 
        {
            if (!temp.empty()) 
            {
                paths.push_back("/" + temp);
                temp.clear();
            }
        } 
        else 
        {
            temp += c;
        }
    }
    if(!temp.empty())
    {
        paths.push_back("/" + temp);
    } 

    std::string current_path;
    for (const auto& p : paths) 
    {
        current_path += p;
        create_remote_directory(current_path);
    }
    return true;
}

bool SpillReporterComponentAdapter::check_remote_file_exists(const std::string& remote_dir, const std::string& filename)
{
    std::string url = m_basicFtpUrl + remote_dir + "/" + filename;
    
    curl_easy_reset(m_ftpHandle);
    
    curl_easy_setopt(m_ftpHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_USERNAME, m_ftpUsername.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_PASSWORD, m_ftpPassword.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(m_ftpHandle, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(m_ftpHandle, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(m_ftpHandle, CURLOPT_HEADERFUNCTION, write_callback);
    curl_easy_setopt(m_ftpHandle, CURLOPT_HEADER, 0L);
    if (m_ftpLocalPort > 0)
    {
        curl_easy_setopt(m_ftpHandle, CURLOPT_LOCALPORT, m_ftpLocalPort);
    }
    
    m_ftpRepCode = curl_easy_perform(m_ftpHandle);
    
    SPILL_REPORTER_DEBUG_PRINT << "[CHECK DEBUG] m_ftpRepCode: " << m_ftpRepCode << std::endl;
    
    if (m_ftpRepCode == CURLE_OK) 
    {
        long response_code = 0;
        curl_easy_getinfo(m_ftpHandle, CURLINFO_RESPONSE_CODE, &response_code);
        
        double content_length = 0;
        curl_easy_getinfo(m_ftpHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
        
        SPILL_REPORTER_DEBUG_PRINT << "[CHECK DEBUG] response_code: " << response_code << ", content_length: " << content_length << std::endl;
        
        if (response_code == 213 || response_code == 350 || (response_code == 0 && content_length > 0)) 
        {
            SPILL_REPORTER_DEBUG_PRINT << "[CHECK] Remote file exists: " << url << " (size: " << content_length << ")" << std::endl;
            return true;
        }
    }
    
    SPILL_REPORTER_DEBUG_PRINT << "[CHECK] Remote file does not exist: " << url << std::endl;
    return false;
}

bool SpillReporterComponentAdapter::upload_file_ftp(const std::string& local_file, const std::string& remote_dir, const std::string& filename)
{
    struct stat file_info;
    if(stat(local_file.c_str(), &file_info) != 0) 
    {
        SPILL_REPORTER_ERROR_PRINT << "[ERROR] File not found: " << local_file << std::endl;
        return false;
    }

    FILE *fp = fopen(local_file.c_str(), "rb");
    if (!fp) 
    {
        SPILL_REPORTER_ERROR_PRINT << "[ERROR] Cannot open file: " << local_file << std::endl;
        return false;
    }

    UploadFileContext ctx;
    ctx.fp = fp;
    ctx.remote_path = remote_dir;
    ctx.filename = filename;

    std::string url = m_basicFtpUrl + remote_dir + "/" + filename;

    if (check_remote_file_exists(remote_dir, filename))
    {
        SPILL_REPORTER_DEBUG_PRINT << "[SKIP] File already exists on server: " << filename << std::endl;
        fclose(fp);
        return true;
    }

    // 重置 Curl 选项
    curl_easy_reset(m_ftpHandle);
    
    curl_easy_setopt(m_ftpHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_USERNAME, m_ftpUsername.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_PASSWORD, m_ftpPassword.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(m_ftpHandle, CURLOPT_READFUNCTION, read_file_callback);
    curl_easy_setopt(m_ftpHandle, CURLOPT_READDATA, &ctx);
    curl_easy_setopt(m_ftpHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY); // 再次保险
    // 设置文件大小（可选，有助于进度条）
    curl_easy_setopt(m_ftpHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
    if (m_ftpLocalPort > 0)
    {
        curl_easy_setopt(m_ftpHandle, CURLOPT_LOCALPORT, m_ftpLocalPort);
    }

    SPILL_REPORTER_DEBUG_PRINT << "[UPLOAD] Starting: " << filename << " -> " << url << std::endl;

    m_ftpRepCode = curl_easy_perform(m_ftpHandle);

    fclose(fp);

    if (m_ftpRepCode == CURLE_OK) 
    {
        SPILL_REPORTER_DEBUG_PRINT << "[SUCCESS] Uploaded: " << filename << std::endl;
        return true;
    } 
    else 
    {
        SPILL_REPORTER_ERROR_PRINT << "[FAIL] Upload failed: " << filename << " Error: " << curl_easy_strerror(m_ftpRepCode) << std::endl;
        return false;
    }
}

bool SpillReporterComponentAdapter::upload_data_ftp(const std::string& remote_dir, const std::string& data, const std::string& filename)
{

    struct UploadData upload_data;
    upload_data.readptr = data.c_str();
    upload_data.sizeleft = data.size();

    std::string url = m_basicFtpUrl + remote_dir + "/" + filename;
    
    curl_easy_reset(m_ftpHandle);
    
    // 1. 设置目标 URL (包含路径和文件名)
    // 例如: ftp://192.168.1.100/v2x_data/test.txt
    curl_easy_setopt(m_ftpHandle, CURLOPT_URL, url.c_str());

    // 2. 设置用户名和密码
    curl_easy_setopt(m_ftpHandle, CURLOPT_USERNAME, m_ftpUsername.c_str());
    curl_easy_setopt(m_ftpHandle, CURLOPT_PASSWORD, m_ftpPassword.c_str());
    // 3. 设置为上传模式
    curl_easy_setopt(m_ftpHandle, CURLOPT_UPLOAD, 1L);

    // 4. 设置读取数据的回调函数
    curl_easy_setopt(m_ftpHandle, CURLOPT_READFUNCTION, read_data_callback);

    // 5. 设置传递给回调函数的数据结构
    curl_easy_setopt(m_ftpHandle, CURLOPT_READDATA, &upload_data);

    // 6. 如果目录不存在，尝试自动创建 (关键步骤)
    // 1L 表示创建一级目录，2L 表示递归创建多级目录
    curl_easy_setopt(m_ftpHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

    // 7. 设置上传数据的大小
    curl_easy_setopt(m_ftpHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)upload_data.sizeleft);
    
    if (m_ftpLocalPort > 0)
    {
        curl_easy_setopt(m_ftpHandle, CURLOPT_LOCALPORT, m_ftpLocalPort);
    }

    // 执行上传
    m_ftpRepCode = curl_easy_perform(m_ftpHandle);

    if (m_ftpRepCode != CURLE_OK) 
    {
        SPILL_REPORTER_ERROR_PRINT << "curl_easy_perform() failed: " << curl_easy_strerror(m_ftpRepCode) << std::endl;
        return false;
    } 
    else 
    {
        SPILL_REPORTER_DEBUG_PRINT << "Upload successful!" << std::endl;
        return true;
    }
    
}
NAMESPACE_ENDED_SPILL_REPORTER_COMPONENT_RADAR