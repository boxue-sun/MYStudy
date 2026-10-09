/*公交车驶入事件，音柱播报模块*/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_SOUND_PLAYER_COMPONENT_H
#define AIROS_MIDDLEWARE_PROTOCOL_SOUND_PLAYER_COMPONENT_H

#include "middleware/runtime/src/air_middleware_component.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "middleware/protocol/om_common/namespace.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "base/common/network/tcp_client.h"
NAMESPACE_START_SOUND_PLAYER_COMPONENT
#define  LOG_KEY_SOUND_PLAYER "[SOUND_PLAYER]"
#define SOUND_PLAYER_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_SOUND_PLAYER
#define SOUND_PLAYER_WARN_PRINT LOG_WARN_IF << LOG_KEY_SOUND_PLAYER
#define SOUND_PLAYER_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_SOUND_PLAYER
#define SOUND_PLAYER_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_SOUND_PLAYER
#define SOUND_PLAYER_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_SOUND_PLAYER


#define MODULE_CONFIG_DIR "/home/airos/protocol/om"
#define MODULE_NAME "SoundPlayerComponentAdapter"
#define MODULE_CFG_NAME "SoundPlayerComponentAdapter.flag"

//配置参数
struct SoundPlayerConfiger: public afl::base::ConfigerData<SoundPlayerConfiger>
{
    std::string voice = "公交车即将进站 请注意避让";
    std::string peerIp = "192.168.1.100";
    int peerPort = 34508;
    int event_type = 0;
    int play_time = 10;
private:
    virtual void writeToFile(ConfigBlock &j) override
    {
        j = {
            {"voice", voice}
            ,{"peerIp", peerIp}
            ,{"peerPort", peerPort}
            ,{"event_type", event_type}
            ,{"play_time", play_time}

        };
    }

    virtual void readFromFile(const ConfigBlock &j) override
    {

        voice = j.at("voice").get<std::string>();
        peerIp = j.at("peerIp").get<std::string>();
        peerPort = j.at("peerPort").get<int>();
        event_type = j.at("event_type").get<int>();
        play_time = j.at("play_time").get<int>();

    }
};

class SoundPlayerComponentAdapter: public airos::middleware::ComponentAdapter<airos::usecase::EventOutputResult>
                    ,public afl::base::Configurable<SoundPlayerConfiger>
{
public:
    SoundPlayerComponentAdapter();
    virtual ~SoundPlayerComponentAdapter();
    bool Init() override;
    bool Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame) override;

private:
    void initTcpEventLoop();
    bool tcpinit();
    void onTcpConnected(const afl::net::TcpConnectionPtr &conn);
    void onTcpMessage(afl::net::ByteBuffer *buffer);
    void makePlayFrame();
private:
    std::unique_ptr<std::thread> m_TaskTcp;
    std::shared_ptr<afl::net::EventLoop> m_Eventloop;
    std::unique_ptr <afl::net::TcpClient> m_TcpClient;
    afl::net::TcpConnectionPtr m_CurrConn;
    bool m_ConnetedFlag = false;
    std::unique_ptr<afl::net::ByteBuffer> m_tcpSendBuf;
    std::atomic<bool> m_isInPlay{false};
    std::string m_playFrame = "";
};

REGISTER_AIROS_COMPONENT_CLASS(SoundPlayerComponent, airos::usecase::EventOutputResult);
NAMESPACE_ENDED_SOUND_PLAYER_COMPONENT
#endif