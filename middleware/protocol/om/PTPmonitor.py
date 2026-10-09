#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import struct
import sys
import time
import os
from datetime import datetime
from pathlib import Path

# 读取配置文件
class Config:
    def __init__(self):
        self.UDP_PORT = 10058  # 默认值
        self.DATA_DIR = "/home/airos/log/ptp"  # 默认值
        self.MAX_FILES = 5  # 默认值
        self.TIME_DIFF_THRESHOLD = 10000  # 默认值
        self.load_config()
    
    def load_config(self):
        try:
            config_file = "/home/airos/common_config/PTPmonitor_conf"
            with open(config_file, 'r') as f:
                for line in f:
                    line = line.strip()
                    if line and not line.startswith('#'):
                        key, value = [x.strip() for x in line.split('=', 1)]
                        if hasattr(self, key):
                            if isinstance(getattr(self, key), int):
                                setattr(self, key, int(value))
                            else:
                                setattr(self, key, value)
        except Exception as e:
            print(f"Warning: Failed to load config file, using default values: {e}")

# 创建全局配置实例
config = Config()

class PTPMonitor:
    def __init__(self, port=config.UDP_PORT):
        self.port = port
        self.sock = None
        self.running = True
        # 数据文件配置
        self.data_dir = config.DATA_DIR
        self.max_files = config.MAX_FILES
        self.current_date = None
        self.current_file = None
        
        # 创建数据目录
        Path(self.data_dir).mkdir(exist_ok=True)
        
        # 初始化当前数据文件
        self.init_data_file()
        
    def init_data_file(self):
        current_date = datetime.now().strftime('%Y%m%d')
        
        # 如果日期变化,创建新文件
        if current_date != self.current_date:
            self.current_date = current_date
            self.current_file = f"{self.data_dir}/ptp_time_data_{current_date}.csv"
            
            # 如果文件不存在才写入表头
            if not os.path.exists(self.current_file):
                with open(self.current_file, 'w') as f:
                    f.write("timestamp,error_flag,gnss_local_diff,gnss_ptp_diff,"
                           "gnss_time_sec,gnss_time_nsec,"
                           "local_time_sec,local_time_nsec,"
                           "ptp_time_sec,ptp_time_nsec,raw_data\n")
            
            # 清理旧文件
            self.cleanup_old_files()
    
    def cleanup_old_files(self):
        # 获取所有数据文件并按修改时间排序
        files = sorted(Path(self.data_dir).glob("ptp_time_data_*.csv"),
                      key=lambda x: x.stat().st_mtime,
                      reverse=True)
        
        # 删除超出数量限制的旧文件
        for f in files[self.max_files:]:
            f.unlink()
        
    def init_socket(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.sock.bind(('0.0.0.0', self.port))
            print(f"PTP monitor started on port {self.port}")
        except Exception as e:
            print(f"Failed to initialize socket: {e}")
            sys.exit(1)
            
    def parse_timestamp_msg(self, data):
        try:
            # 每个时间戳占用8字节
            gnss_time = data[0:8]
            local_time = data[8:16]
            ptp_time = data[16:24]
            
            def parse_time(time_bytes):
                # 使用小端序解析时间戳
                tv_sec = int.from_bytes(time_bytes[0:4], byteorder='little')
                tv_nsec = int.from_bytes(time_bytes[4:8], byteorder='little')
                return tv_sec, tv_nsec
            
            gnss_sec, gnss_nsec = parse_time(gnss_time)
            local_sec, local_nsec = parse_time(local_time)
            ptp_sec, ptp_nsec = parse_time(ptp_time)
            
            print(f"gnssTime: tv_sec = {gnss_sec}, tv_nsec = {gnss_nsec}")
            print(f"localTime: tv_sec = {local_sec}, tv_nsec = {local_nsec}")
            print(f"ptpTime: tv_sec = {ptp_sec}, tv_nsec = {ptp_nsec}")
            
            # 计算时间差(纳秒)
            NSEC_PER_SEC = 1000000000  # 每秒的纳秒数
            gnss_total_ns = gnss_sec * NSEC_PER_SEC + gnss_nsec
            local_total_ns = local_sec * NSEC_PER_SEC + local_nsec
            ptp_total_ns = ptp_sec * NSEC_PER_SEC + ptp_nsec
            
            gnss_local_diff = abs(int(gnss_total_ns - local_total_ns))
            gnss_ptp_diff = abs(int(gnss_total_ns - ptp_total_ns))
            
            # 使用配置的阈值判断
            is_error = 1 if (gnss_local_diff > config.TIME_DIFF_THRESHOLD or 
                           gnss_ptp_diff > config.TIME_DIFF_THRESHOLD) else 0
            
            return {
                'gnss_time': {'tv_sec': gnss_sec, 'tv_nsec': gnss_nsec},
                'local_time': {'tv_sec': local_sec, 'tv_nsec': local_nsec},
                'ptp_time': {'tv_sec': ptp_sec, 'tv_nsec': ptp_nsec},
                'gnss_local_diff': gnss_local_diff,
                'gnss_ptp_diff': gnss_ptp_diff,
                'is_error': is_error
            }
        except Exception as e:
            print(f"Failed to parse timestamp message: {e}")
            return None
            
    def process_packet(self, data, addr):
        # 检查是否需要创建新文件
        self.init_data_file()
        
        timestamp_data = self.parse_timestamp_msg(data)
        if timestamp_data:
            current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
            
            # 格式化数据行，分别显示秒和纳秒
            data_line = (
                f"{current_time},"
                f"{timestamp_data['is_error']},"
                f"{timestamp_data['gnss_local_diff']},"
                f"{timestamp_data['gnss_ptp_diff']},"
                f"{timestamp_data['gnss_time']['tv_sec']},{timestamp_data['gnss_time']['tv_nsec']},"
                f"{timestamp_data['local_time']['tv_sec']},{timestamp_data['local_time']['tv_nsec']},"
                f"{timestamp_data['ptp_time']['tv_sec']},{timestamp_data['ptp_time']['tv_nsec']},"
                f"{data.hex()}\n"
            )
            
            print(f"Writing data: {data_line}")  # 添加调试输出
            # 写入数据文件
            try:
                with open(self.current_file, 'a') as f:
                    f.write(data_line)
            except Exception as e:
                print(f"Error writing to file: {e}")
            
    def run(self):
        self.init_socket()
        
        while self.running:
            try:
                data, addr = self.sock.recvfrom(1024)
                self.process_packet(data, addr)
            except Exception as e:
                print(f"Error processing packet: {e}")
                
    def stop(self):
        self.running = False
        if self.sock:
            self.sock.close()
            
def main():
    monitor = PTPMonitor()
    try:
        monitor.run()
    except KeyboardInterrupt:
        print("Shutting down PTP monitor...")
        monitor.stop()
        
if __name__ == "__main__":
    main()
