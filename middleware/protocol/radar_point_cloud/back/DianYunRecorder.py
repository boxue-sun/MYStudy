# -*- coding: utf-8 -*-
"""
 @Author: lht
 @Description: DianYunRecorder with Cyber RT Support
 @Modified: Optimized for thread safety and execution flow
"""
import json
import os
import re
import shutil
import struct
import sys
import socket
import threading
import time
import logging
from logging.handlers import TimedRotatingFileHandler
from ftplib import FTP, error_perm
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timedelta
import numpy as np

# 尝试导入 Cyber RT 模块
try:
    import cyber
    # 如果您使用的是自定义的 cyber 封装文件，请确保文件名正确，例如:
    # import my_cyber as cyber
except ImportError:
    print("Warning: 'cyber' module not found. Cyber RT functions will be disabled.")
    cyber = None

# 尝试导入 Protobuf 消息定义
try:
    from monitor_mec_pb2 import MonitorMec, MonitorMecTag
except ImportError:
    print("错误: 找不到 monitor_mec_pb2 模块，请确保已编译 proto 文件。")
    # 如果是必须功能，建议在此退出；如果是可选功能，可设置标志位
    sys.exit(1)

# CRC 表格
aucCRCHi = np.array([
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80,
    0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
], dtype=np.uint8)

aucCRCLo = np.array([
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D,
    0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB,
    0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2,
    0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37,
    0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8,
    0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24,
    0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63,
    0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
    0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F,
    0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C,
    0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46,
    0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
], dtype=np.uint8)


def tsCRC16(data):
    ucCRCHi = 0xFF
    ucCRCLo = 0xFF
    data = np.array(data, dtype=np.uint8)  # 确保data是NumPy数组

    for d in data:
        iIndex = ucCRCLo ^ d
        ucCRCLo = np.uint8(ucCRCHi ^ aucCRCHi[iIndex])
        ucCRCHi = aucCRCLo[iIndex]

    return np.uint16((ucCRCHi << 8) | ucCRCLo)


def generate_filename(upload_period):
    current_time = time.localtime()
    hour_begin = current_time.tm_hour
    minute_begin = current_time.tm_min
    hour_end = hour_begin
    minute_end = minute_begin + int(upload_period / 60)  # 计算5分钟的间隔
    if minute_end >= 60:
        minute_end = minute_end % 60
        hour_end = (hour_begin + 1) % 24
    formatted_time = time.strftime(
        "%Y%m%d_{:02d}{:02d}_{:02d}{:02d}".format(hour_begin, minute_begin, hour_end, minute_end), current_time)
    filename = f"{formatted_time}.tmp"
    return filename


class TCPReceiver:
    def __init__(self, host, port, save_path, sn, type, upload_period, logger):
        self.host = host
        self.port = port
        self.tcp_socket = None
        self.save_path = save_path
        self.sn = sn
        self.type = type
        self.upload_period = upload_period
        self.logger = logger
        self.buffer = bytearray()
        self.handle = None
        self.save_fime = ""
        self.fs_time = int(time.time())
        self.thread = None
        self.running = False
        self.write_buffer = bytearray()
        self.need_crc = False

    def start(self):
        self.running = True
        self.thread = threading.Thread(target=self.run)
        self.thread.daemon = True
        self.thread.start()

    def stop(self):
        self.running = False
        if self.tcp_socket:
            try:
                self.tcp_socket.close()
            except:
                pass
        if self.thread:
            self.thread.join()

    def run(self):
        while self.running:
            try:
                self.connect()
                self.receive_data()
            except Exception as e:
                self.logger.error(f"TCPReceiver 出错: {e}")
                time.sleep(5)

    def connect(self):
        while self.running:
            try:
                self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.tcp_socket.connect((self.host, self.port))
                self.logger.info(f'已连接到 TCP 服务器 {self.host}:{self.port}')
                return
            except Exception as e:
                self.logger.error(f'TCP 连接错误: {e}, {self.host}:{self.port}')
                self.logger.info(f'5秒后尝试重新连接到 TCP 服务器...{self.host}：{self.port}')
                time.sleep(5)

    def receive_data(self):
        while self.running:
            try:
                data = self.tcp_socket.recv(4096)
                if not data:
                    self.logger.info(f'TCP connection closed by remote server {self.host}')
                    self.close_handle()
                    break
                self.buffer += data
                self.process_data()
            except (socket.error, socket.timeout) as e:
                self.logger.error(f'TCP 接收 {self.host} 数据错误: {e}')
                self.close_handle()
                break

    def close_handle(self):
        if self.handle:
            try:
                self.handle.flush()
                self.handle.close()
            except:
                pass
            self.handle = None

    def process_data(self):
        head_index = 0
        valid_headers = {b'TRAB', b'TRAC'}
        buffer_len = len(self.buffer)

        while (buffer_len - head_index) >= 82:
            frame_header = bytes(self.buffer[head_index:head_index + 4])
            if frame_header not in valid_headers:
                head_index += 1
                continue

            try:
                frame_length = struct.unpack('>I', self.buffer[head_index + 4:head_index + 8])[0]
            except Exception:
                head_index += 1
                continue

            total_length = frame_length + 8

            if (buffer_len - head_index) >= total_length:
                if self.need_crc:
                    payload = self.buffer[head_index + 8: head_index + total_length - 2]
                    frame_checksum = struct.unpack('>H', self.buffer[head_index + total_length - 2:head_index + total_length])[0]
                    if self.verify_checksum(payload, frame_checksum):
                        self.process_frame(self.buffer[head_index:head_index + total_length])
                    else:
                        self.logger.warning("Checksum failed, discarding frame.")
                else:
                    self.process_frame(self.buffer[head_index:head_index + total_length])
                head_index += total_length
            else:
                break

        if head_index > 0:
            self.buffer = self.buffer[head_index:]

    def verify_checksum(self, frame_content, frame_checksum):
        desired = tsCRC16(frame_content)
        if desired != frame_checksum:
            self.logger.error(f"checksum failed, need:{desired}, received:{frame_checksum}")
            return False
        return True

    def process_frame(self, frame):
        try:
            self.write_buffer.extend(frame)
            if not self.handle:
                self.save_fime = f'{self.save_path}/{self.type}_{self.sn}_{generate_filename(self.upload_period)}'
                self.handle = open(self.save_fime, 'ab')
                self.fs_time = int(time.time())
                self.logger.debug(f"begin write to {self.save_fime}, now:{self.fs_time}")
            else:
                if len(self.write_buffer) >= 1024 * 1024:
                    self.handle.write(self.write_buffer)
                    self.handle.flush()
                    self.write_buffer.clear()

                now = int(time.time())
                if now - self.fs_time > self.upload_period:
                    self.logger.info("write period reached, rotating file...")
                    self.handle.write(self.write_buffer)
                    self.handle.flush()
                    self.write_buffer.clear()
                    self.handle.close()
                    self.handle = None
                    new_filename = self.save_fime.replace(".tmp", ".data")
                    os.rename(self.save_fime, new_filename)
        except IOError as e:
            self.logger.debug(f"write backup data error：{e}")
            time.sleep(0.1)


class FTPUploader:
    def __init__(self, host, port, username, password, logger):
        self.host = host
        self.port = port
        self.username = username
        self.password = password
        self.ftp = FTP()
        self.logger = logger
        self.connected = False
        self.connect()

    def connect(self):
        try:
            self.ftp.connect(self.host, self.port)
            self.logger.info(f"Connected to FTP server: {self.host}")
            self.connected = True
        except Exception as e:
            self.logger.warning(f'Connection failed: {str(e)}')
            self.connected = False

    def login(self):
        if not self.connected:
            return False
        try:
            self.ftp.login(user=self.username, passwd=self.password)
            self.logger.info('Login in to FTP server')
            return True
        except error_perm as e:
            self.logger.warning(f'Login failed: {str(e)}')
            self.connected = False
            return False

    def reconnect(self):
        self.disconnect()
        self.connect()
        return self.login()

    def upload_file(self, local_path, remote_path):
        if not self.connected:
            if not self.reconnect():
                return False

        try:
            with open(local_path, 'rb') as file:
                self.ftp.storbinary(f'STOR {remote_path}', file)
                self.logger.info(f'Uploaded file to FTP server: {remote_path}')
                return True
        except (ConnectionResetError, error_perm, socket.error) as e:
            self.logger.warning(f'Upload failed: {str(e)}')
            self.connected = False
            return False

    def rename_file(self, old_filename, new_filename):
        try:
            self.ftp.rename(old_filename, new_filename)
        except Exception as e:
            self.logger.warning(f'Rename failed: {e}')

    def disconnect(self):
        try:
            self.ftp.quit()
        except:
            try:
                self.ftp.close()
            except:
                pass
        self.logger.info('Disconnected from FTP server')
        self.connected = False


def upload_files_periodically(ftp_server, ftp_port, ftp_user, ftp_passwd, ftp_pasv, data_path, upload_period, logger, writer):
    """
    定期上传文件，并发送雷达原始数据监控心跳
    """
    ftp_uploader = None
    while True:
        # 1. 发送监控心跳 (md_radar_cp)
        if writer is not None:
            try:
                msg = MonitorMec()
                msg.md_radar_cp.tag = MonitorMecTag.MONITOR_TAG_MONITOR_IN_RADAR_DATA
                msg.md_radar_cp.timestamp = int(time.time() * 1000)
                writer.write(msg)
                logger.debug(f"Sent MonitorMec md_radar_cp timestamp: {msg.md_radar_cp.timestamp}")
            except Exception as e:
                logger.error(f"Failed to write cyber message: {e}")

        # 2. FTP 上传逻辑
        try:
            # 懒加载连接
            if ftp_uploader is None:
                ftp_uploader = FTPUploader(ftp_server, ftp_port, ftp_user, ftp_passwd, logger)
                if ftp_uploader.login():
                    ftp_uploader.ftp.set_pasv(ftp_pasv)

            # 遍历目录下的文件
            if os.path.exists(data_path):
                for root, dirs, files in os.walk(data_path):
                    for file in files:
                        if file.endswith(".data"):
                            file_path = os.path.join(root, file)
                            remote_file = os.path.basename(file_path)

                            if ftp_uploader.connected:
                                tmp_file = remote_file + ".tmp"
                                logger.info(f"begin to upload {tmp_file}...")
                                if ftp_uploader.upload_file(file_path, tmp_file):
                                    logger.info(f"upload {tmp_file} success...")
                                    time.sleep(1)
                                    ftp_uploader.rename_file(tmp_file, remote_file)
                                    logger.info(f"rename {tmp_file} to {remote_file} success...")
                                    os.remove(file_path)
                                else:
                                    logger.warning(f"upload {file_path} failed...")
                            else:
                                logger.warning(f"ftp_uploader is not connected, trying reconnect...")
                                ftp_uploader.reconnect()

            # 每次循环后休眠
            time.sleep(upload_period)

        except Exception as e:
            logger.warning(f"ftp upload loop exception: {e}")
            if ftp_uploader:
                ftp_uploader.disconnect()
                ftp_uploader = None
            time.sleep(upload_period)


def remove_file_by_hour(directory, logger, hour=12):
    while True:
        try:
            now = datetime.now()
            twelve_hours_ago = now - timedelta(hours=hour)
            if os.path.exists(directory):
                for filename in os.listdir(directory):
                    file_path = os.path.join(directory, filename)
                    if os.path.isfile(file_path):
                        file_mtime = datetime.fromtimestamp(os.path.getmtime(file_path))
                        if file_mtime < twelve_hours_ago:
                            try:
                                os.remove(file_path)
                                logger.debug(f"Deleted file: {file_path}")
                            except Exception as e:
                                logger.error(f"Failed to delete file {file_path}: {e}")
        except Exception as e:
            logger.error(f"Clean up thread error: {e}")
        time.sleep(3600)


def setup_logger(logpath):
    logger = logging.getLogger()
    logger.setLevel(logging.INFO) # 建议改为 INFO，调试时再改为 DEBUG
    formatter = logging.Formatter('[%(asctime)s|%(levelname)s|%(filename)s(%(lineno)d)]:%(message)s')

    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)

    if not os.path.exists(logpath):
        os.makedirs(logpath)

    file_handler = TimedRotatingFileHandler(os.path.join(logpath, 'ftp_upload.log'), when='midnight', backupCount=7)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)

    return logger


def start_tcp_receiver(device, index, logger, backup_data_path, upload_period):
    device_type = device.get('C_deviceType')
    device_sn = device.get('F_deviceEsn')
    device_ip = device.get('G_deviceIp')
    device_port = device.get('L_radarPort')

    if device_type == 3:  # radar
        logger.info(f"begin to start receive from {device_sn}, ip:{device_ip}, port:{device_port}")
        tcp_receiver = TCPReceiver(device_ip, device_port, backup_data_path, device_sn, 'WAVE', upload_period, logger)
        tcp_receiver.start()


def validate_device_fields(device):
    # 简化校验逻辑，保持原意
    required_fields = ['C_deviceType', 'F_deviceEsn', 'G_deviceIp', 'L_radarPort']
    for field in required_fields:
        if device.get(field) is None:
            return False, f"Missing field: {field}"
    return True, "Valid"


def is_valid_ip(ip):
    ip_pattern = r'^(\d{1,3}\.){3}\d{1,3}$'
    if not isinstance(ip, str) or not re.match(ip_pattern, ip):
        return False
    return all(0 <= int(num) <= 255 for num in ip.split('.'))


def cyber_talker_init():
    if cyber is None:
        return None, None

    cyber.init("monitor_mec_radar_point_cloud_py")
    node = cyber.Node("node_monitor_mec_radar_point_cloud")
    writer = node.create_writer("/v2x/monitor/mec", MonitorMec, 6)
    return node, writer


def run_lidar_pusher(writer, logger, interval=2.0):
    """
    定时推送激光雷达心跳数据 (线程函数)
    """
    logger.info(f"开始推送激光雷达监控数据 (间隔: {interval}s)...")

    while not cyber.is_shutdown():
        try:
            msg = MonitorMec()
            msg.md_lidar_cp.tag = MonitorMecTag.MONITOR_TAG_MONITOR_IN_LIDAR_DATA
            msg.md_lidar_cp.timestamp = int(time.time() * 1000)

            if writer is not None:
                writer.write(msg)
                logger.debug(f"[Sent] Tag: {msg.md_lidar_cp.tag}, Time: {msg.md_lidar_cp.timestamp}")
        except Exception as e:
            logger.error(f"Lidar pusher error: {e}")

        time.sleep(interval)


def main():
    if len(sys.argv) != 2:
        print("用法: python DianYunRecorder.py configFilePath")
        sys.exit(1)

    config_file = sys.argv[1]
    script_dir = os.path.dirname(os.path.abspath(__file__))
    log_path = os.path.join(script_dir, 'logs')
    backup_data_path = os.path.join(script_dir, 'backupDatas')
    upload_period = 300

    # 1. 初始化 Logger (最先执行，确保后续能打印日志)
    logger = setup_logger(log_path)

    # 2. 检查数据目录
    if not os.path.exists(backup_data_path):
        os.makedirs(backup_data_path)
        logger.info(f"Directory '{backup_data_path}' was created.")

    # 3. 读取配置
    try:
        with open(config_file, 'r') as file:
            config = json.load(file)
    except Exception as e:
        logger.error(f"Failed to load config file: {e}")
        sys.exit(1)

    # 4. 初始化 Cyber RT (获取 writer)
    node, writer = cyber_talker_init()

    # 5. 启动激光雷达心跳推送线程 (非阻塞)
    if node and writer:
        lidar_thread = threading.Thread(target=run_lidar_pusher, args=(writer, logger, 5.0))
        lidar_thread.daemon = True
        lidar_thread.start()
    else:
        logger.warning("Cyber RT init failed, skipping lidar pusher.")

    # 6. 解析 FTP 配置
    ftp_server = None
    ftp_user = None
    ftp_passwd = None
    ftp_pasv = False

    mec_device_config = config.get('B_mecDeviceWorkParam')
    if mec_device_config:
        ftp_server = mec_device_config.get('h_radarPointCloudFtpIp')
        ftp_port = mec_device_config.get('t_radarPointCloudFtpPort')
        ftp_user = mec_device_config.get('i_radarPointCloudFtpUser')
        ftp_passwd = mec_device_config.get('j_radarPointCloudFtpPasswd')

    # 7. 启动业务线程池
    devices = config.get('D_sensorDeviceWorkParamList')
    if not devices:
        logger.error('Config error: D_sensorDeviceWorkParamList not found.')
        return

    with ThreadPoolExecutor(max_workers=10) as executor:
        # 启动 TCP 接收任务
        for i, device in enumerate(devices, start=1):
            is_valid, message = validate_device_fields(device)
            if is_valid:
                executor.submit(start_tcp_receiver, device, i, logger, backup_data_path, upload_period)
            else:
                logger.error(f"Config error: {message}, ignore...")

        # 启动清理任务
        executor.submit(remove_file_by_hour, backup_data_path, logger)

        # 启动 FTP 上传任务 (传入 writer 用于发送雷达心跳)
        if all([ftp_server, ftp_user, ftp_passwd]) and is_valid_ip(ftp_server):
            logger.info(f"begin to start FTP upload thread...")
            executor.submit(upload_files_periodically, ftp_server, ftp_port, ftp_user, ftp_passwd, False, backup_data_path, upload_period, logger, writer)
        else:
            logger.error("FTP config invalid, skipping upload thread.")

        # 8. 主线程保活
        logger.info("Main loop started. Press Ctrl+C to exit.")
        try:
            if node:
                node.spin() # 如果有 Cyber 节点，使用 spin
            else:
                while True: # 否则使用普通死循环
                    time.sleep(1)
        except KeyboardInterrupt:
            logger.info("Shutting down...")
            if cyber:
                cyber.shutdown()

if __name__ == '__main__':
    main()
