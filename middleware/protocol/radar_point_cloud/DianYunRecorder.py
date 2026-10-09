# -*- coding: utf-8 -*-
"""
 @Author: lht
 @Description: 
 @Create: 2024/3/15 11:20
 @Modified By:zhangenwei
 @Project : learn
 @FileName: DianYunRecorder.py
 @Software: PyCharm
 高山仰止,景行行止.虽不能至,然心向往之.
"""
import json
import os
import re
import shutil
import struct
import sys
from logging.handlers import TimedRotatingFileHandler
import socket
import threading
import time
from ftplib import FTP, error_perm
import logging
import numpy as np
from datetime import datetime, timedelta

# ==========================================
# [新增] 全局变量与全局锁
# ==========================================
GLOBAL_FTP_LINK_STATUS = False  # 全局变量：FTP 服务健康状态（注意：不再代表实时 Socket 连接状态）
heartbeat_interval = 5          # 激光雷达心跳发送间隔（秒）
ftp_heartbeat_interval = 5      # 雷达 FTP 链路心跳发送间隔（秒）
ftp_check_interval = 4         # [新增] 单独检测 FTP 连通性的时间间隔（秒）

# 【关键修复】全局锁，用于保护 Cyber RT Writer 的多线程并发写入
CYBER_WRITE_LOCK = threading.Lock()

# ==========================================
# 1. Cyber RT 模块导入与初始化
# ==========================================
MonitorMec = None
MonitorMecTag = None
CYBER_ENABLED = False
cyber = None

try:
    from monitor_mec_pb2 import MonitorMec, MonitorMecTag
except ImportError:
    print("[Warning----->]: MonitorMec Protobuf modules not found. Monitoring disabled.")

try:
    from cyber.python.cyber_py3 import cyber
    CYBER_ENABLED = True
except ImportError:
    print("[Warning=====>]: Cyber RT modules not found. Monitoring disabled.")
    CYBER_ENABLED = False


def cyber_talker_init():
    """
    初始化 Cyber RT 节点和 Writer
    """
    if not CYBER_ENABLED or cyber is None:
        return None, None
    try:
        cyber.init("monitor_mec_radar_point_cloud")
        node = cyber.Node("node_monitor_mec_radar_point_cloud")


        # 创建一个字典来存放所有的 writer
        writers = {}
        # 1. 原有的 Writer (比如用于 RCP)
        writers['data'] = node.create_writer("/v2x/mec/om/check/rcp/data_", MonitorMec, 6)

        # 2. [新增] 新的 Writer (比如用于 Lidar 或其他用途)
        # 请将 "/your/new/channel/name" 替换为你实际需要的 channel 名称
        writers['link'] = node.create_writer("/v2x/mec/om/check/rcp/link_", MonitorMec, 6)
        return node, writers
        # writer = node.create_writer("/v2x/mec/om/check/rcp_", MonitorMec, 6)
        # return node, writer
    except Exception as e:
        print(f"Cyber init failed: {e}")
        return None, None


# ==========================================
# 2. CRC 校验表与辅助函数
# ==========================================

# CRC16 高位表
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

# CRC16 低位表
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
"""
/**********************************************************
* @brieftsCRC16
* @paramptr: 待校验数据
* @paramlen: 数据长度
* @return16bit crc 计算结果
**********************************************************/
u16_t tsCRC16 (u8_t *ptr, u16_t len)
{
	u8_t ucCRCHi = 0xFF;
	u8_t ucCRCLo = 0xFF;
	u32_t iIndex;
	while ( len-- )
	{
	iIndex = ucCRCLo ^ *( ptr++ );
	ucCRCLo = ( u8_t )( ucCRCHi ^ aucCRCHi[iIndex] );
	ucCRCHi = aucCRCLo[iIndex];
	}
	return ( u16_t )( ucCRCHi << 8 | ucCRCLo );
}
"""


def tsCRC16(data):
    ucCRCHi = 0xFF
    ucCRCLo = 0xFF
    data = np.array(data, dtype=np.uint8)

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
    minute_end = minute_begin + int(upload_period / 60)
    if minute_end >= 60:
        minute_end = minute_end % 60
        hour_end = (hour_begin + 1) % 24
    formatted_time = time.strftime(
        "%Y%m%d_{:02d}{:02d}_{:02d}{:02d}".format(hour_begin, minute_begin, hour_end, minute_end), current_time)
    filename = f"{formatted_time}.tmp"
    return filename


class TCPReceiver:
    def __init__(self, host, port, save_path, sn, type, upload_period, logger, cyber_writer=None):
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

        self.cyber_writer = cyber_writer
        self.last_cyber_msg_time = 0

    def start(self):
        self.running = True
        self.thread = threading.Thread(target=self.run)
        self.thread.daemon = True
        self.thread.start()

    def stop(self):
        self.running = False
        if self.tcp_socket:
            try:
                self.tcp_socket.shutdown(socket.SHUT_RDWR)
            except Exception:
                pass
            try:
                self.tcp_socket.close()
            except Exception:
                pass
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=2)

    def run(self):
        while self.running:
            try:
                self.connect()
                self.receive_data()
            except Exception as e:
                if self.running:
                    self.logger.error(f"TCPReceiver 出错: {e}")
                    time.sleep(5)

    def connect(self):
        while self.running:
            try:
                self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.tcp_socket.settimeout(15)
                self.tcp_socket.connect((self.host, self.port))
                self.logger.info(f'已连接到 TCP 服务器 {self.host}:{self.port}')
                return
            except Exception as e:
                if self.running:
                    self.logger.error(f'TCP 连接错误: {e}, {self.host}:{self.port}')
                    self.logger.info(f'5秒后尝试重新连接到 TCP 服务器...{self.host}：{self.port}')
                    time.sleep(5)

    def receive_data(self):
        while self.running:
            try:
                data = self.tcp_socket.recv(4096)
                if not data:
                    self.logger.info('TCP connection closed by remote server')
                    if self.handle:
                        self.handle.flush()
                        self.handle.close()
                        self.handle = None
                    break

                if self.cyber_writer and MonitorMec:
                    now = time.time()
                    if now - self.last_cyber_msg_time >= 1.0:
                        try:
                            msg = MonitorMec()
                            msg.md_lidar_cp.tag = MonitorMecTag.MONITOR_TAG_DATA_RADAR_DATA
                            msg.md_lidar_cp.timestamp = int(now * 1000)
                            msg.md_lidar_cp.device_id = self.sn
                            with CYBER_WRITE_LOCK:
                                self.cyber_writer.write(msg)
                            self.last_cyber_msg_time = now
                        except Exception as e:
                            self.logger.error(f"Cyber write failed: {e}")

                self.buffer += data
                self.process_data()
            except socket.timeout:
                # 【修复】只是 15 秒内没收到数据，不需要断开重连，继续等待即可
                continue
            except(socket.error, socket.timeout) as e:
                if self.running:
                    self.logger.error(f'TCP 接收 {self.host} 数据错误/超时: {e}')
                if self.handle:
                    self.handle.flush()
                    self.handle.close()
                    self.handle = None
                break

    def process_data(self):
        head_index = 0
        valid_headers = {b'TRAB', b'TRAC'}
        buffer_len = len(self.buffer)
        while (buffer_len - head_index) >= 82:
            frame_header = bytes(self.buffer[head_index:head_index + 4])
            if frame_header not in valid_headers:
                head_index += 1
                continue
            frame_length = struct.unpack('>I', self.buffer[head_index + 4:head_index + 8])[0]
            total_length = frame_length + 8

            if (buffer_len - head_index) >= total_length:
                if self.need_crc:
                    payload = self.buffer[head_index + 8: head_index + total_length - 2]
                    frame_checksum = \
                        struct.unpack('>H', self.buffer[head_index + total_length - 2:head_index + total_length])[0]

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
        else:
            return True

    def process_frame(self, frame):
        try:
            self.write_buffer.extend(frame)
            if not self.handle:
                self.save_fime = f'{self.save_path}/{self.type}_{self.sn}_{generate_filename(self.upload_period)}'
                self.handle = open(self.save_fime, 'ab')
                self.fs_time = int(time.time())
            else:
                if len(self.write_buffer) >= 1024 * 1024:
                    self.handle.write(self.write_buffer)
                    self.handle.flush()
                    self.write_buffer.clear()

                now = int(time.time())
                if now - self.fs_time > self.upload_period:
                    self.logger.info("write 5 minutes, so close the file...")
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
    """
    FTP 上传工具类
    """
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
        """建立 FTP 连接"""
        while True:
            try:
                self.logger.info(f"Connected to FTP server: {self.host} [host]{self.host} [port]{self.port}")
                self.ftp.connect(self.host, self.port, timeout=10)
                self.connected = True
                break
            except Exception as e:
                self.logger.warning(f'Connection failed: {str(e)}')
                self.logger.warning(f'Retrying connection...{self.host}')
                self.connected = False
                time.sleep(1)

    def login(self):
        """登录 FTP"""
        while True:
            try:
                self.ftp.login(user=self.username, passwd=self.password)
                self.logger.info('Login in to FTP server')
                self.connected = True
                break
            except error_perm as e:
                self.logger.warning(f'Login failed: {str(e)}')
                self.logger.warning('Retrying login...')
                self.connected = False
                time.sleep(1)

    def reconnect(self):
        """重连 FTP"""
        self.disconnect()
        self.connect()
        self.login()

    def upload_file(self, local_path, remote_path):
        """上传文件"""
        while True:
            try:
                with open(local_path, 'rb') as file:
                    self.ftp.storbinary(f'STOR {remote_path}', file)
                    self.logger.info(f'Uploaded file to FTP server: {remote_path}')
                    return True
            except Exception as e:
                self.logger.warning(f'Upload failed due to connection error: {str(e)}')
                self.logger.warning('Reconnecting and retrying...')
                self.reconnect()
                time.sleep(1)
                return False

    def rename_file(self, old_filename, new_filename):
        """重命名远程文件"""
        self.ftp.rename(old_filename, new_filename)

    def disconnect(self):
        """断开 FTP 连接"""
        try:
            self.ftp.quit()
        except Exception:
            pass
        self.logger.info('Disconnected from FTP server')
        self.connected = False


def upload_files_periodically(ftp_server, ftp_port, ftp_user, ftp_passwd, ftp_pasv, data_path, upload_period, logger, stop_event):
    """
    定期扫描目录并上传文件到 FTP
    """
    while not stop_event.is_set():
        ftp_initok = False
        ftp_uploader = None
        # 尝试初始化 FTP 连接
        while not ftp_initok and not stop_event.is_set():
            try:
                ftp_uploader = FTPUploader(ftp_server, ftp_port, ftp_user, ftp_passwd, logger)
                ftp_uploader.login()
                ftp_uploader.ftp.set_pasv(ftp_pasv)
                ftp_initok = True

            except Exception as e:
                logger.warning(f"ftp init failed:{e}")
                stop_event.wait(5) # 等待5秒重试

        if stop_event.is_set():
            break

        try:
            # 扫描并上传文件
            for root, dirs, files in os.walk(data_path):
                if stop_event.is_set(): break
                for file in files:
                    if stop_event.is_set(): break
                    if file.endswith(".data"):
                        file_path = os.path.join(root, file)
                        remote_file = os.path.basename(file_path)
                        if ftp_uploader and ftp_uploader.connected:
                            tmp_file = remote_file + ".tmp"
                            logger.info(f"begin to upload {tmp_file}...")
                            if ftp_uploader.upload_file(file_path, tmp_file):
                                logger.info(f"upload {tmp_file} success...")
                                time.sleep(1)
                                ftp_uploader.rename_file(tmp_file, remote_file)
                                logger.info(f"rename {tmp_file} to {remote_file} success...")
                                os.remove(file_path)
                            else:
                                logger.warning(f"upload {file_path} to {remote_file} failed...")

                        else:
                            logger.warning(f"ftp_uploader is not connected...")

            if ftp_uploader:
                # 正常流程结束，主动断开
                ftp_uploader.disconnect()

            stop_event.wait(upload_period) # 等待下一个上传周期
        except Exception as e:
            logger.warning(f"ftp upload exception:{e}")

            if ftp_uploader:
                ftp_uploader.disconnect()
            stop_event.wait(upload_period)


def check_ftp_connection_periodically(host, port, username, password, interval, logger, stop_event):
    """
    [新增] 独立线程：定期检测 FTP 连接状态
    不进行文件上传，仅测试连接和登录，用于实时更新 GLOBAL_FTP_LINK_STATUS
    """
    global GLOBAL_FTP_LINK_STATUS
    logger.info(f"启动 FTP 连接状态检测线程，检测周期: {interval}秒")
    ftp = None
    while not stop_event.is_set():
        try:
            # 如果没有连接，则建立连接
            if ftp is None:
                ftp = FTP()
                ftp.connect(host, port, timeout=5)
                ftp.login(user=username, passwd=password)

            # 发送 NOOP 命令测试连接是否存活
            ftp.voidcmd("NOOP")
            GLOBAL_FTP_LINK_STATUS = True

        except Exception as e:
            # 连接断开或 NOOP 失败
            GLOBAL_FTP_LINK_STATUS = False
            logger.warning(f"FTP connection check failed: {e}")
            if ftp:
                try:
                    ftp.close()
                except:
                    pass
                ftp = None  # 置空，下次循环将重新连接

        stop_event.wait(interval)

    if ftp:
        try:
            ftp.quit()
        except:
            pass
    logger.info("FTP 连接状态检测线程已停止")


def remove_file_by_hour(directory, logger, stop_event, hour=12):
    while not stop_event.is_set():
        now = datetime.now()
        twelve_hours_ago = now - timedelta(hours=hour)
        logger.debug(f"now:{now}, twelve_hours_ago:{twelve_hours_ago}")
        for filename in os.listdir(directory):
            file_path = os.path.join(directory, filename)
            if os.path.isfile(file_path):
                file_mtime = datetime.fromtimestamp(os.path.getmtime(file_path))
                if file_mtime < twelve_hours_ago:
                    try:
                        os.remove(file_path)
                        logger.debug(f"Deleted file: {file_path}")
                    except Exception as e:
                        logger.error(f"Failed to delete file {file_path}. Reason: {e}")
        stop_event.wait(3600)


def setup_logger(logpath):
    logger = logging.getLogger()
    logger.setLevel(logging.INFO)
    formatter = logging.Formatter('[%(asctime)s|%(levelname)s|%(filename)s(%(lineno)d)]:%(message)s')
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    file_handler = TimedRotatingFileHandler(logpath + '/ftp_upload.log', when='midnight', backupCount=7)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    logger.addHandler(console_handler)
    return logger


def start_tcp_receiver(device, index, logger, backup_data_path, upload_period, cyber_writer=None):
    device_type = device.get('C_deviceType')
    device_sn = device.get('F_deviceEsn')
    device_ip = device.get('G_deviceIp')
    device_port = device.get('L_radarPort')

    if device_type == 3:
        logger.info(f"begin to start receive from {device_sn}, ip:{device_ip}, port:{device_port}")
        tcp_receiver = TCPReceiver(device_ip, device_port, backup_data_path, device_sn, 'WAVE', upload_period, logger,
                                   cyber_writer)
        tcp_receiver.start()
        return tcp_receiver
    return None


def is_valid_device_type(device_type):
    return isinstance(device_type, int) and 0 <= device_type <= 10


def is_valid_device_sn(device_sn):
    return isinstance(device_sn, str) and 8 <= len(device_sn) <= 22 and device_sn.isalnum()


def is_valid_ip(ip):
    ip_pattern = r'^(\d{1,3}\.){3}\d{1,3}$'
    if not isinstance(ip, str) or not re.match(ip_pattern, ip):
        return False
    return all(0 <= int(num) <= 255 for num in ip.split('.'))


def is_valid_port(port):
    return isinstance(port, int) and 0 <= port <= 65535


def validate_device_fields(device):
    fields = {
        'C_deviceType': 'Device Type',
        'F_deviceEsn': 'Device Serial Number',
        'G_deviceIp': 'Device IP',
        'L_radarPort': 'Radar Port'
    }

    missing_fields = []
    for key, name in fields.items():
        if device.get(key) is None:
            missing_fields.append(name)

    if missing_fields:
        return False, f"Missing fields: {', '.join(missing_fields)}"

    if not is_valid_device_type(device['C_deviceType']): return False, f"Invalid device type"
    if not is_valid_device_sn(device['F_deviceEsn']): return False, f"Invalid device sn"
    if not is_valid_ip(device['G_deviceIp']): return False, f"Invalid IP address"
    if not is_valid_port(device['L_radarPort']): return False, f"Invalid port number"

    return True, "All fields are valid"


def send_lidar_heartbeat(cyber_writer, interval, logger, stop_event):
    logger.info(f"启动激光雷达监控心跳线程，发送周期: {interval}秒")
    if not cyber_writer:
        logger.warning("Cyber Writer 未初始化，无法发送心跳")
        return
    if MonitorMec is None:
        logger.warning("MonitorMec 未定义，无法发送心跳")
        return

    while not stop_event.is_set():
        try:
            msg = MonitorMec()
            msg.md_lidar_cp.tag = MonitorMecTag.MONITOR_TAG_LINK_DAR
            msg.md_lidar_cp.timestamp = int(time.time() * 1000)
            with CYBER_WRITE_LOCK:
                cyber_writer.write(msg)
            logger.info(f"[Heartbeat] 成功发送 md_lidar_cp 心跳数据")
        except Exception as e:
            logger.error(f"发送 md_lidar_cp 心跳异常: {e}")

        stop_event.wait(interval)

    logger.info("激光雷达监控心跳线程已停止")


def send_radar_ftp_heartbeat(cyber_writer, interval, logger, stop_event):
    global GLOBAL_FTP_LINK_STATUS
    logger.info(f"启动 Radar FTP 链路监控心跳线程，发送周期: {interval}秒")

    if not cyber_writer:
        logger.warning("Cyber Writer 未初始化，FTP 心跳线程退出")
        return
    if MonitorMec is None:
        logger.warning("MonitorMec 未定义，无法发送心跳")
        return

    while not stop_event.is_set():
        try:
            msg = MonitorMec()
            msg.ml_radar_cp.tag = MonitorMecTag.MONITOR_TAG_LINK_DAR
            msg.ml_radar_cp.timestamp = int(time.time() * 1000)
            msg.ml_radar_cp.con_flag = GLOBAL_FTP_LINK_STATUS
            with CYBER_WRITE_LOCK:
                cyber_writer.write(msg)
            logger.info(f"[Heartbeat] 成功发送 ml_radar_cp 心跳数据 (FTP状态: {GLOBAL_FTP_LINK_STATUS})")
        except Exception as e:
            logger.error(f"发送 ml_radar_cp 心跳异常: {e}")

        stop_event.wait(interval)

    logger.info("Radar FTP 链路监控心跳线程已停止")


def main():
    if len(sys.argv) != 2:
        print("用法: python DianYunRecorder.py configFilePath")
        sys.exit(1)
    print("[success]: argv check sucess!")
    config_file = sys.argv[1]
    script_dir = os.path.dirname(os.path.abspath(__file__))
    log_path = script_dir + '/logs'
    backup_data_path = script_dir + '/backupDatas'
    upload_period = 300
    backup_data_max = 2 * 1024 * 1024 * 1024

    if not os.path.exists(backup_data_path):
        os.makedirs(backup_data_path)
    if not os.path.exists(log_path):
        os.makedirs(log_path)

    logger = setup_logger(log_path)

    with open(config_file, 'r') as file:
        config = json.load(file)

    # cyber_node, cyber_writer = cyber_talker_init()
    # if cyber_writer:
    #     logger.info("Cyber RT init success.")
    # else:
    #     logger.error("Cyber RT init failed or disabled.")
    # 1. 获取 writers 字典
    cyber_node, writers = cyber_talker_init()
    if writers:
        logger.info(f"Cyber RT init success. Writers: {list(writers.keys())}")
    else:
        logger.error("Cyber RT init failed or disabled.")

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
        ftp_pasv = False
        logger.info(f"mec_device_config： {ftp_server} : {ftp_user} , [user]{ftp_user} [passwd] {ftp_passwd}")

    if not all([ftp_server, ftp_user, ftp_passwd]):
        logger.error(f"ftp config is not valid...")
        return

    if not is_valid_ip(ftp_server):
        logger.error(f"ftp server ip:{ftp_server} is not valid...")
        return

    devices = config.get('D_sensorDeviceWorkParamList')
    if not devices:
        logger.error('Config error: D_sensorDeviceWorkParamList not found.')
        return

    logger.info("启动系统服务线程...")

    stop_event = threading.Event()
    tcp_receivers = []

    # if cyber_writer:
    #     logger.info(f"begin to start Radar FTP Heartbeat thread...")
    #     t_radar = threading.Thread(target=send_radar_ftp_heartbeat,
    #                                args=(cyber_writer, ftp_heartbeat_interval, logger, stop_event))
    #     t_radar.daemon = True
    #     t_radar.start()
    # 2. 修改线程启动逻辑，传入指定的 writer
    if writers and 'link' in writers:
        logger.info(f"begin to start Radar FTP Heartbeat thread...")
        # 假设 Radar 心跳还是用原来的 rcp 通道
        t_radar = threading.Thread(target=send_radar_ftp_heartbeat,
                                   args=(writers['link'], ftp_heartbeat_interval, logger, stop_event))
        t_radar.daemon = True
        t_radar.start()

    need_check = False
    for device in devices:
        is_valid, _ = validate_device_fields(device)
        if is_valid: need_check = True

    if need_check:
        logger.info(f"begin to start directory size check thread...")
        t_clean = threading.Thread(target=remove_file_by_hour, args=(backup_data_path, logger, stop_event))
        t_clean.daemon = True
        t_clean.start()

        logger.info(f"begin to start FTP upload thread...")
        t_ftp = threading.Thread(target=upload_files_periodically,
                                 args=(ftp_server, ftp_port, ftp_user, ftp_passwd, False, backup_data_path,
                                       upload_period, logger, stop_event))
        t_ftp.daemon = True
        t_ftp.start()

        # [新增] 启动 FTP 连接状态检测线程
        logger.info(f"begin to start FTP connection check thread...")
        t_ftp_check = threading.Thread(target=check_ftp_connection_periodically,
                                       args=(ftp_server, ftp_port, ftp_user, ftp_passwd, ftp_check_interval, logger, stop_event))
        t_ftp_check.daemon = True
        t_ftp_check.start()

    for i, device in enumerate(devices, start=1):
        is_valid, message = validate_device_fields(device)
        if is_valid:
            logger.info("start_tcp_receiver")
            current_writer = writers.get('data') if writers else None
            receiver = start_tcp_receiver(device, i, logger, backup_data_path, upload_period, current_writer)
            if receiver:
                tcp_receivers.append(receiver)
        else:
            logger.error(f"Config error: {message}, ignore...")

    try:
        if cyber_node:
            logger.info("Cyber RT node initialized. Entering main wait loop...")
            while not stop_event.is_set():
                time.sleep(1)
        else:
            logger.info("No Cyber node, entering wait loop...")
            while not stop_event.is_set():
                time.sleep(1)
    except KeyboardInterrupt:
        logger.info("Program interrupted by user (Ctrl+C).")
    finally:
        logger.info("Shutting down... signaling threads to exit.")
        stop_event.set()
        for receiver in tcp_receivers:
            receiver.stop()
        logger.info('Program terminated successfully.')


if __name__ == '__main__':
    main()