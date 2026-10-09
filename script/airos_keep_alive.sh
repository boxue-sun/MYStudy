#!/usr/bin/env bash
set -e

# 定义日志路径变量
LOG_PATH="/home/airos/log/airos_keep_alive.log"
LOG_MAX_SIZE_MB=20  # 日志文件最大大小（MB）
LOG_MAX_FILES=2     # 保留的日志文件数量

# 创建日志目录（如果不存在）
mkdir -p "$(dirname "$LOG_PATH")"

# 日志轮转函数
rotate_log() {
  if [[ -f "$LOG_PATH" ]]; then
    local log_size_mb=$(($(stat -c%s "$LOG_PATH") / 1024 / 1024))
    if [[ $log_size_mb -gt $LOG_MAX_SIZE_MB ]]; then
      # 轮转日志文件
      for ((i=$LOG_MAX_FILES; i>1; i--)); do
        if [[ -f "${LOG_PATH}.$((i-1))" ]]; then
          mv "${LOG_PATH}.$((i-1))" "${LOG_PATH}.$i"
        fi
      done
      if [[ -f "$LOG_PATH" ]]; then
        mv "$LOG_PATH" "${LOG_PATH}.1"
      fi
      # 创建新的日志文件
      touch "$LOG_PATH"
      echo "$(date '+%Y-%m-%d %H:%M:%S') - Log rotated due to size limit (${log_size_mb}MB > ${LOG_MAX_SIZE_MB}MB)" > "$LOG_PATH"
    fi
  fi
}

return_info() {
  local DATE_N=$(date "+%Y-%m-%d %H:%M:%S")
  [[ $# -eq 0 ]] && return
  echo -e "\033[1;32m [INFO]--- ${DATE_N} $* \033[0m" >> $LOG_PATH
}

function check_run() {
  local command_line="$1"
  if [[ -n ${command_line} ]]; then
    launch_cmd="${command_line} > /dev/null 2>&1 &"
    eval ${launch_cmd}
    return_info "${launch_cmd} , run successfully! "
  fi
}

function is_running() {
    local process_name="$1"
    if pgrep -f "$process_name" > /dev/null; then
        return 0  # 进程正在运行
    else
        return 1  # 进程未运行
    fi
}

function keep_alive() {
  declare -A processes=(
    ["ccindex.dag"]="mainboard -d dag/ccindex.dag"
    ["mec_service.dag"]="mainboard -d dag/mec_service.dag"
    ["traffic_light_service.dag"]="mainboard -d dag/traffic_light_service.dag"
    ["v2x_codec.dag"]="mainboard -d dag/v2x_codec.dag"
    ["rsu_service.dag"]="mainboard -d dag/rsu_service.dag"
    ["conf/airos_v2x_app.pb"]="./bin/airos_app_framework conf/airos_v2x_app.pb"
    ["conf/airos_v2x_scenario.pb"]="./bin/airos_app_framework conf/airos_v2x_scenario.pb"
    ["rsap.dag"]="mainboard -d dag/rsap.dag"
    ["om.dag"]="mainboard -d dag/om.dag"
    ["om_device_status.dag"]="mainboard -d dag/om_device_status.dag"
    ["om_monitor.dag"]="mainboard -d dag/om_monitor.dag"
#    ["DianYunRecorder"]="python3 /home/airos/protocol/radar_point_cloud/DianYunRecorder.py /home/airos/common_config/work_param_config.flag"
#    ["PTPpartlog"] = "python3 /home/airos/protocol/mec/PTPpartlog.py /home/airos/common_config/work_param_config.flag"
    ["PTPmonitor.py"]="python3 /home/airos/protocol/om/PTPmonitor.py"
    ["spill_reporter.dag"]="mainboard -d dag/spill_reporter.dag"

  )

  cd /home/airos/os

  while true; do
    for process in "${!processes[@]}"; do
      if ! is_running "$process"; then
        return_info "$process is not running. Restarting..."
        check_run "${processes[$process]}"
      fi
    done

    # 定期对日志进行管理
#    python3 /home/airos/script/airos_log_proc.py
    sleep 6  # Check 30s
  done
}

function log_proc() {
  while true; do
    # 检查并轮转日志
    rotate_log
    
    # 定期对日志进行管理
    python3 /home/airos/script/airos_log_proc.py
    sleep 30  # Check 30s
  done
}

# add by lht for dianyun
TARGET="DianYunRecorder"

function dian_yun_keep_alive(){
  while true;do
	is_live=$(ps -ef | grep "$TARGET" | grep -v 'grep' | awk '{print $1}')
	if [ -z "$is_live" ]; then
      	    return_info "start $TARGET..."
	    python3 /home/airos/protocol/radar_point_cloud/DianYunRecorder.py /home/airos/common_config/work_param_config.flag &
	    sleep 5
	else
	  return_info "$TARGET is running..."
	  sleep 30
	fi
  done
}
# end add 
PTP_PROCESS_NAME="PTPpartlog"
function ptp_keep_alive() {
  while true; do
    is_live=$(ps -ef | grep "$PTP_PROCESS_NAME" | grep -v 'grep' | awk '{print $1}')
    if [ -z "$is_live" ]; then
      return_info "start $PTP_PROCESS_NAME..."
      python3 /home/airos/protocol/om/PTPpartlog.py /home/airos/common_config/work_param_config.flag > /dev/null 2>&1 &
      sleep 5
    else
      return_info "$PTP_PROCESS_NAME is running..."
      sleep 30
    fi
  done
}

SSH_SERVER="127.0.0.1"
SSH_PORT=2222
# SSH keep-alive function
function ssh_keep_alive() {
  while true; do
    if ! nc -z "$SSH_SERVER" "$SSH_PORT"; then
      return_info "SSH service has stopped, restarting..."
      # Restart the SSH service
      chmod 600 /etc/ssh/ssh_host_rsa_key
      chmod 600 /etc/ssh/ssh_host_ecdsa_key
      chmod 600 /etc/ssh/ssh_host_ed25519_key
      chmod 700 /var/run/sshd
      chmod 644 /run/sshd.pid
      chown root:root /var/run/sshd
      service ssh start
    else
      return_info "SSH service is running"
    fi
    sleep 30  # 每30秒检查一次
  done
}
MOSQUITTO_PROCESS_NAME="mosquitto"
function mosquitto_keep_alive(){
  while true;do
	is_live=$(ps -ef | grep "${MOSQUITTO_PROCESS_NAME}" | grep -v 'grep' | awk '{print $1}')
	if [ -z "$is_live" ]; then
      	    return_info "start ${MOSQUITTO_PROCESS_NAME}..."
	    mosquitto -d &
	    sleep 2
	else
	  return_info "$TARGET is running..."
	  sleep 10
	fi
  done
}
function main() {
  keep_alive >>  $LOG_PATH 2>&1 &
  dian_yun_keep_alive >>  $LOG_PATH 2>&1 &
  ptp_keep_alive >>  $LOG_PATH 2>&1 &
  ssh_keep_alive >>  $LOG_PATH 2>&1 &
#  mosquitto_keep_alive  >>  $LOG_PATH 2>&1 &
  log_proc >>  $LOG_PATH 2>&1 &
}

main "$@"
