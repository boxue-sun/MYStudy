#!/bin/bash
# start.sh

# 定义日志路径变量
LOG_PATH="/home/airos/log/airos_auto_start.log"

# 创建日志目录（如果不存在）
mkdir -p "$(dirname "$LOG_PATH")"

# 定义写入带有时间戳的日志函数
write_log() {
    echo "$(date +'%Y-%m-%d %H:%M:%S') - $1" >> $LOG_PATH
}

write_log "Auto start ..."

echo "Script is running" 

# 启动cron服务
write_log "Cron service start"
service cron start >> $LOG_PATH
# 添加定时任务到crontab
crontab /home/airos/common_config/airos_cron_conf

# 启动ssh服务
write_log "ssh service start"
service ssh start >> $LOG_PATH

#启动mosquitto服务
#service mosquitto start >> $LOG_PATH
#mosquitto -d >> $LOG_PATH
# 设置环境变量
write_log "Set environment variables"
source /home/airos/os/setup.bash

# 启动airos程序
write_log "Launch airos"
airos_launch all >> $LOG_PATH

# # 启动保活程序, airos_launch all 已经启动
# write_log "Launch keep alive file"
# bash /home/airos/script/airos_keep_alive.sh

# TODO 添加其他开机启动脚本


# 放在最后，保证容器不会退出
exec /bin/bash
