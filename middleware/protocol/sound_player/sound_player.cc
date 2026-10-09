#include "sound_player.h"
#include "base/common/math_util.h"
NAMESPACE_START_SOUND_PLAYER_COMPONENT
SoundPlayerComponentAdapter::SoundPlayerComponentAdapter()
    : Configurable<SoundPlayerConfiger>(MODULE_NAME, MODULE_CONFIG_DIR, MODULE_CFG_NAME)
{
    m_tcpSendBuf = std::unique_ptr<afl::net::ByteBuffer>(new afl::net::ByteBuffer(128, 2048));
}

SoundPlayerComponentAdapter::~SoundPlayerComponentAdapter()
{

}

bool SoundPlayerComponentAdapter::Init()
{
    m_TaskTcp.reset(new std::thread([&](){ initTcpEventLoop(); }));
    makePlayFrame();
    return true;
}

bool SoundPlayerComponentAdapter::Proc(const std::shared_ptr<const airos::usecase::EventOutputResult> &frame)
{
    if(m_CurrConn == nullptr || m_CurrConn->connected() == false)
    {
        SOUND_PLAYER_ERROR_PRINT << "tcp not connected";
        return false;
    }

    if(m_isInPlay.load() == true)
    {
        SOUND_PLAYER_DEBUG_PRINT << "now is in play";
        return false;
    }

    if(frame == nullptr 
        || frame->events().empty() 
        || !frame->has_header())
    {
        SOUND_PLAYER_DEBUG_PRINT << "not recv events";
        return false;
    }

    for(int i = 0; i < frame->events_size(); i++)
    {
        auto event_info = frame->events(i);
        if(event_info.event_type_mec() != getConfiger().event_type)
        {
            SOUND_PLAYER_DEBUG_PRINT << "current event is " << event_info.event_type_mec() << " not " << getConfiger().event_type;
            continue;
        }
        //播放语音，规定时长，且将播放状态置为“播放中”
        
        m_CurrConn->send(m_playFrame);
        m_isInPlay.store(true);
        //设置定时器，将状态置为“未播放”
        m_Eventloop->addTimer([&](){ m_isInPlay.store(false); }, getConfiger().play_time, false);
        break;
    }
}

void SoundPlayerComponentAdapter::initTcpEventLoop()
{
    m_Eventloop = std::make_shared<afl::net::EventLoop>();
    tcpinit();
    m_Eventloop->loop();
}

bool SoundPlayerComponentAdapter::tcpinit()
{
    afl::net::InetAddress addrUp(getConfiger().peerIp.c_str(), getConfiger().peerPort);

    if(!m_TcpClient)
    {
        m_TcpClient = std::unique_ptr<afl::net::TcpClient>(new afl::net::TcpClient(m_Eventloop.get(), addrUp));
    }
    m_TcpClient->setConnectionCallback(
            std::bind(&SoundPlayerComponentAdapter::onTcpConnected, this, std::placeholders::_1));
    m_TcpClient->setMessageCallback(
            std::bind(&SoundPlayerComponentAdapter::onTcpMessage, this, std::placeholders::_2));
    m_TcpClient->connect();
    m_TcpClient->enableRetry();
    return true;
}

void SoundPlayerComponentAdapter::makePlayFrame()
{
    std::string headPart = "AA 06 01 11 ";
    std::string tailPart = " 02 " + std::to_string(getConfiger().play_time) + " BB EF";
    
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (unsigned char c : getConfiger().voice) 
    {
        ss << std::setw(2) << static_cast<int>(c);
    }
    std::string playPart = ss.str();

    m_playFrame = headPart + playPart + tailPart;
}

void SoundPlayerComponentAdapter::onTcpConnected(const afl::net::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        {
            SOUND_PLAYER_DEBUG_PRINT << "[success]sound player tcp connect success!";
        }
        m_CurrConn = conn;
        m_ConnetedFlag = true;
    }
    else
    {
        SOUND_PLAYER_ERROR_PRINT << "[failed]sound player tcp connect failed!";
        m_ConnetedFlag = false;
        m_CurrConn = conn;
        m_CurrConn->shutdown();
//        m_TcpClient->reconnect();
    }
}

void SoundPlayerComponentAdapter::onTcpMessage(afl::net::ByteBuffer *buffer)
{

    return;
}

NAMESPACE_ENDED_SOUND_PLAYER_COMPONENT